#include "rex_engine/filesystem/vfs.h"

#include "rex_engine/cmdline/cmdline.h"
#include "rex_engine/diagnostics/assert.h"
#include "rex_engine/diagnostics/logging/log_macros.h"
#include "rex_engine/diagnostics/logging/log_verbosity.h"
#include "rex_engine/engine/state_controller.h"
#include "rex_engine/engine/project.h"
#include "rex_engine/engine/casting.h"
#include "rex_engine/filesystem/directory.h"
#include "rex_engine/filesystem/file.h"
#include "rex_engine/filesystem/path.h"
#include "rex_engine/filesystem/internal/queued_request.h"
#include "rex_engine/platform/win/diagnostics/win_call.h"
#include "rex_std/bonus/atomic/atomic.h"
#include "rex_std/bonus/hashtable.h"
#include "rex_std/bonus/memory.h"
#include "rex_std/bonus/platform.h"
#include "rex_std/bonus/string.h"
#include "rex_std/bonus/time/timepoint.h"
#include "rex_std/bonus/utility/enum_reflection.h"
#include "rex_std/ctype.h"
#include "rex_std/mutex.h"
#include "rex_std/thread.h"
#include "rex_std/unordered_map.h"
#include "rex_std/vector.h"

// Rex Engine - Virtual File System
// Users can make simple request to read or write to a file
// that run on the same thread
// eg: vfs::read_file and vfs::write_to_file
//
// the vfs also supports async read requests
// this is done by calling vfs::read_file_async
// This will return a read request immediately
// the user is meant to keep this read request
// alive as it'll be signaled when reading has finished
// After which the user can access its buffer
// to process the data it just read
// It works as follows:
//
// rex::vfs::ReadRequest request = rex::vfs::read_file_async("path/to/file");
//
// Do some other code here
// ..
//
//
// Wait for the file to be processed
// request.wait();
//
// const rsl::byte* buffer = request.buffer();
// rsl::memory_size size = request.size();
//
// Do something with the data..
//
//
//
// ASYNC VFS PIPELINE
//
//
// =====================================+=====================================================================================================================
//                                      |
//           USER CODE                  |                                                 INTERNAL CODE
//                                      |
// =====================================+=====================================================================================================================
//                                      |
// The following happens on the thread  |
// the request comes in from            |
//                                      |
// +-------------------------+          |            +----------------------+      +------------------------+      +-------------------------------+
// | User Requests File Read | ---------+----------> | Read Request Created | ---> | Queued Request Created | ---> | Queued Request Added To Queue |
// +-------------------------+          |            +----------------------+      +------------------------+      +-------------------------------+
//                                      |                                                                                         |
//                                      |                                                                                         |
// +----------------+                   |                                +-----------------------------+                          |
// | User Continues | <-----------------+------------------------------- | Return Read Request To User |   <----------------------+
// +----------------+                   |                                +-----------------------------+
//         |                            |
//         |                            |                                               The following happens on the vfs thread
//         |                            |              +----------------------------------------------------------------------------------------------------+
//         |                            |              |                                                                                                    |
//         |                            |              |       +----------------------------------+      +--------------------------+      +-----------+    |
//         |                            |              |       | Queued Request Popped From Queue | ---> | Queued Request Processed | ---> | File Read |    |
//         v                            |              |       +----------------------------------+      +--------------------------+      +-----------+    |
// +-----------------+                  |              |                                                                                        |           |
// | Other User Code |                  |              |                                                                                        |           |
// +-----------------+                  |              |                                                                                        |           |
//        |                             |              |                                                                                        |           |
//        |                             |              |                                                                                        |           |
//        |                             |              |                                                                                        |           |
//        v                             |              |                                                                                        |           |
// +-----------------------------+      |              |                                                                                        |           |
// | Waits for request to finish |      |              |                                                                                        |           |
// +-----------------------------+      |              |                                                                                        |           |
//        |                             |              |                                                                                        |           |
//        |                             |              |                                                                                        |           |
//        |                             |              |                                                                                        |           |
//        v                             |              |                                                                                        |           |
// +---------------------+              |              |                               +-----------------------+                                |           |
// | User Processes Data | <------------+--------------+-------------------------------| Read Request Signaled | <------------------------------+           |
// +---------------------+              |              |                               +-----------------------+                                            |
//                                      |              |                                                                                                    |
//                                      |              +----------------------------------------------------------------------------------------------------+
//

// #TODO: Remaining cleanup of development/Pokemon -> main merge. ID: MOUNT POINTS

namespace rex
{
  namespace vfs
  {
    DEFINE_LOG_CATEGORY(LogVfs);

    enum class VfsState
    {
      NotInitialized = BIT(0),
      Initializing   = BIT(1),
      Running        = BIT(2),
      ShuttingDown   = BIT(3),
      ShutDown       = BIT(4)
    };

    // NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects)

    // Root paths used by the VFS
    rsl::medium_stack_string g_root; // This the root where all relative paths will start from

    // This controls the state of the vfs
    StateController<VfsState> g_vfs_state_controller(VfsState::NotInitialized);

    // mutices for the asyncronous operation of the vfs
    rsl::mutex g_read_request_mutex;
    rsl::mutex g_closed_request_mutex;

    // queues the vfs uses for its async operations
    rsl::vector<rsl::string_view> g_read_requests_in_order;
    rsl::unordered_map<rsl::string, rsl::unique_ptr<QueuedRequest>> g_queued_requests;
    rsl::vector<rsl::unique_ptr<QueuedRequest>> g_closed_requests;

    // mounted roots
    rsl::unordered_map<MountingPoint, rsl::string> g_mounted_roots;

    // threads used by the vfs to perform the async operations
    rsl::thread g_reading_thread;
    rsl::thread g_closing_thread;

    // NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects)

    // Forward declare internal function
    namespace internal
    {
      rsl::string_view no_mount_path()
      {
        // explicitely stating "no mount found" here
        // as an empty path would indicate it's at the root
        // instead of it not existing

        return rsl::string_view("no mount found");
      }
    }

    rsl::string_view root()
    {
      return g_root;
    }

    void process_read_requests()
    {
      while(g_vfs_state_controller.has_state(VfsState::Running))
      {
        using namespace rsl::chrono_literals; // NOLINT(google-build-using-namespace)
        rsl::this_thread::sleep_for(1ms);

        rsl::unique_lock lock(g_read_request_mutex);
        if(!g_queued_requests.empty())
        {
          // get the queued request and remvoe it from the queue,
          // but remove in the reverse order that they got added
          rsl::string_view filepath = g_read_requests_in_order.front();
          g_read_requests_in_order.erase(g_read_requests_in_order.cbegin());
          rsl::unique_ptr<QueuedRequest> request = rsl::move(g_queued_requests[filepath]);
          g_queued_requests.erase(filepath);

          // we don't need access to the queue anymore, we can unlock its access mutex
          lock.unlock();

          // read the actual file we requested
          memory::Blob buffer = read_file(request->filepath());

          // signal all read requests that this file has now been read
          // it's possible multiple read requests want to access the same file
          // if such requests come in while there's already a request for this file
          // they get added to the original queued request and they now all get notified
          request->signal_requests(buffer.data(), buffer.size());

          // add the closed task to the queue, making sure it stays alive until all requests have finished processing the data
          // This moves ownership of the buffer to the new request in the new queue
          // We do this so that we don't have lock any thread while waiting for the requests to complete
          const rsl::unique_lock closed_req_lock(g_closed_request_mutex);
          g_closed_requests.push_back(rsl::move(request));
        }

      }
    }
    void wait_for_read_requests()
    {
      while(g_vfs_state_controller.has_state(VfsState::Running))
      {
        using namespace rsl::chrono_literals; // NOLINT(google-build-using-namespace)
        rsl::this_thread::sleep_for(20ms);

        const rsl::unique_lock lock(g_closed_request_mutex);
				
        // Remove all requests that have finished
				auto it = rsl::remove_if(g_closed_requests.begin(), g_closed_requests.end(), [](const rsl::unique_ptr<QueuedRequest>& request) { return request->all_requests_finished(); });
				g_closed_requests.erase(it, g_closed_requests.end());

      }
    }
    void start_threads()
    {
      // This thread processes the read requests
      // A queued request is popped from the queue and reads the file specified.
      // after the requested file is read, all read requests for this file receive a signal.
      // and store a pointer to the buffer with its size
      // The request is then passed over to the closing thread queue
      // and stays there until all read requests have finished their processing
      g_reading_thread = rsl::thread(process_read_requests);

      // this thread goes over all the requests that are currently in flight
      // If they've closed, it'll close these requests and remove them from the queue
      g_closing_thread = rsl::thread(wait_for_read_requests);
    }

    void init()
    {
      g_vfs_state_controller.change_state(VfsState::Initializing);

      // Setting the root directory has the effect that all data will be read relative from this directory
      // This has the same effect as if you would put the working directory to this path
      set_root(cmdline::instance()->get_argument("Root").value_or(rex::path::cwd()));

      g_vfs_state_controller.change_state(VfsState::Running);

      // Start the worker threads, listening to file IO requests and processing them
      start_threads();

      REX_INFO(LogVfs, "LogVfs initialized");
    }
    void set_root(rsl::string_view root)
    {
      if(root.empty())
      {
        g_root = rex::path::cwd();
      }
      else if(path::is_absolute(root))
      {
        g_root = root;
      }
      else
      {
        g_root = path::join(path::cwd(), root);
      }

      REX_ASSERT_X(directory::exists(g_root), "root of vfs is not a directory");
      REX_INFO(LogVfs, "LogVfs root changed to: {}", g_root);
    }
    void mount(MountingPoint root, rsl::string_view path)
    {
      rsl::string_view full_path = vfs::abs_path(path);

      REX_ASSERT_X(!g_mounted_roots.contains(root), "root {} is already mapped. currently mapped to '{}'", rsl::enum_refl::enum_name(root), g_mounted_roots.at(root));
      g_mounted_roots[root].assign(full_path);

      // make sure the mount exists
      if (!directory::exists(full_path))
      {
        create_dirs(full_path);
      }
    }
    void shutdown()
    {
      g_vfs_state_controller.change_state(VfsState::ShuttingDown);

      if (g_reading_thread.joinable())
      {
        g_reading_thread.join();
      }
      if (g_closing_thread.joinable())
      {
        g_closing_thread.join();
      }

      for (const auto& [mount, path] : g_mounted_roots)
      {
        if (rex::directory::is_empty(path))
        {
          // As soon as we have a path that can be mounted and is expected to be empty on shutdown
          // we have to extend the mount functionality to give a flag to a mount that it can be empty on shutdown
          REX_WARN(LogVfs, "\"{}\" was mounted to \"{}\" but no files were present on shutdown, is this intended?", path, rsl::enum_refl::enum_name(mount));
        }
      }

      // clean up all data assets
      g_root.clear();
      g_mounted_roots.clear();

      g_vfs_state_controller.change_state(VfsState::ShutDown);
    }

    // --------------------------------
    // CREATING
    // --------------------------------
    Error create_file(MountingPoint root, rsl::string_view path)
    {
      path = path::remove_quotes(path);

      scratch_string fullpath = path::join(g_mounted_roots.at(root), path);
      return file::create_abspath(fullpath);
    }
    Error create_file(rsl::string_view path)
    {
      path = path::remove_quotes(path);
      path = path::unsafe_abs_path(path);

      return file::create_abspath(path);
    }
    Error create_dir(MountingPoint root, rsl::string_view path)
    {
      path = path::remove_quotes(path);

      scratch_string fullpath = path::join(g_mounted_roots.at(root), path);
      return directory::create_abspath(fullpath);
    }
    Error create_dir(rsl::string_view path)
    {
      path = path::remove_quotes(path);
      path = path::unsafe_abs_path(path);

      return directory::create_abspath(path);
    }
    Error create_dirs(rsl::string_view path)
    {
      path = path::remove_quotes(path);
      path = path::unsafe_abs_path(path);

      return directory::create_recursive_abspath(path);
    }

    // --------------------------------
    // DELETING
    // --------------------------------
    Error delete_file(MountingPoint root, rsl::string_view path)
    {
      path = path::remove_quotes(path);

      scratch_string fullpath = path::join(g_mounted_roots.at(root), path);
      return file::del_abspath(fullpath);
    }
    Error delete_file(rsl::string_view path)
    {
      path = path::remove_quotes(path);
      path = path::unsafe_abs_path(path);

      return file::del_abspath(path);
    }
    Error delete_dir(MountingPoint root, rsl::string_view path)
    {
      path = path::remove_quotes(path);

      const scratch_string fullpath = path::join(g_mounted_roots.at(root), path);
      return directory::del_abspath(fullpath);
    }
    Error delete_dir(rsl::string_view path)
    {
      path = path::remove_quotes(path);
      path = path::unsafe_abs_path(path);

      return directory::del_abspath(path);
    }
    Error delete_dir_recursive(MountingPoint root, rsl::string_view path)
    {
      path = path::remove_quotes(path);

      const scratch_string fullpath = path::join(g_mounted_roots.at(root), path);
      return directory::del_recursive_abspath(fullpath);
    }
    Error delete_dir_recursive(rsl::string_view path)
    {
      path = path::remove_quotes(path);
      path = path::unsafe_abs_path(path);

      return directory::del_recursive_abspath(path);
    }

    // --------------------------------
    // READING
    // --------------------------------
    memory::Blob read_file(MountingPoint root, rsl::string_view filepath)
    {
      filepath = path::remove_quotes(filepath);

      const scratch_string fullpath = path::join(g_mounted_roots.at(root), filepath);
      return file::read_file_abspath(fullpath);
    }
    memory::Blob read_file(rsl::string_view path)
    {
      path = path::remove_quotes(path);
      path = path::unsafe_abs_path(path);

      return file::read_file_abspath(path);
    }
    s32 read_file(MountingPoint root, rsl::string_view filepath, rsl::byte* buffer, s32 size)
    {
      filepath = path::remove_quotes(filepath);

      const scratch_string fullpath = path::join(g_mounted_roots.at(root), filepath);
      return file::read_file_abspath(fullpath, buffer, size);
    }
    s32 read_file(rsl::string_view path, rsl::byte* buffer, s32 size)
    {
      path = path::remove_quotes(path);
      path = path::unsafe_abs_path(path);

      return file::read_file_abspath(path, buffer, size);
    }
    ReadRequest read_file_async(rsl::string_view path)
    {
      path = path::remove_quotes(path);
      path = path::unsafe_abs_path(path);

      return read_file_abspath_async(path);
    }
    ReadRequest read_file_async(MountingPoint root, rsl::string_view filepath)
    {
      filepath = path::remove_quotes(filepath);

      const scratch_string fullpath = path::join(g_mounted_roots.at(root), filepath);
      return read_file_abspath_async(fullpath);
    }

    // --------------------------------
    // WRITING
    // --------------------------------
    Error write_to_file(MountingPoint root, rsl::string_view filepath, const void* data, card64 size, AppendToFile shouldAppend)
    {
      filepath = path::remove_quotes(filepath);

      const rsl::string_view path = path::join(g_mounted_roots.at(root), filepath);
      return write_to_file(path, data, size, shouldAppend);
    }
    Error write_to_file(rsl::string_view path, const void* data, card64 size, AppendToFile shouldAppend)
    {
      path = path::unsafe_abs_path(path);

      if (shouldAppend)
      {
        return rex::file::append_text_abspath(path, rsl::string_view((const char8*)data, narrow_cast<s32>(size)));
      }
      else
      {
        return rex::file::write_to_file_abspath(path, data, size);
      }
    }
    Error write_to_file(MountingPoint root, rsl::string_view filepath, rsl::string_view text, AppendToFile shouldAppend)
    {
      return write_to_file(root, filepath, text.data(), text.length(), shouldAppend);
    }
    Error write_to_file(rsl::string_view filepath, rsl::string_view text, AppendToFile shouldAppend)
    {
      return write_to_file(filepath, text.data(), text.length(), shouldAppend);
    }
    Error write_to_file(MountingPoint root, rsl::string_view filepath, const memory::Blob& blob, AppendToFile shouldAppend)
    {
      return write_to_file(root, filepath, blob.data(), blob.size(), shouldAppend);
    }
    Error write_to_file(rsl::string_view filepath, const memory::Blob& blob, AppendToFile shouldAppend)
    {
      return write_to_file(filepath, blob.data(), blob.size(), shouldAppend);
    }

    // --------------------------------
    // CONVERTING
    // --------------------------------
    scratch_string abs_path(MountingPoint root, rsl::string_view path)
    {
      REX_ASSERT_X(g_vfs_state_controller.has_state(VfsState::Running), "Trying to use vfs before it's initialized");
      REX_ASSERT_X(!path::is_absolute(path), "Passed an absolute path into a function that doesn't allow absolute paths");

      path = path::remove_quotes(path);

      return path::join(g_mounted_roots.at(root), path);
    }
    scratch_string abs_path(rsl::string_view path)
    {
      path = path::remove_quotes(path);

      if (path::is_absolute(path))
      {
        return scratch_string(path);
      }

      return path::join(g_root, path);
    }
    rsl::string_view mount_path(MountingPoint mount)
    {
      if (g_mounted_roots.contains(mount))
      {
        return g_mounted_roots.at(mount);
      }

      return internal::no_mount_path();
    }

    // --------------------------------
    // QUERYING
    // --------------------------------
    bool exists(MountingPoint root, rsl::string_view path)
    {
      path = path::remove_quotes(path);

      const rsl::string_view fullpath = path::join(g_mounted_roots.at(root), path);
      return directory::exists_abspath(fullpath) || file::exists_abspath(fullpath);
    }
    bool exists(rsl::string_view path)
    {
      path = path::remove_quotes(path);
      path = path::unsafe_abs_path(path);

      return directory::exists_abspath(path) || file::exists_abspath(path);
    }
    bool is_file(MountingPoint root, rsl::string_view path)
    {
      path = path::remove_quotes(path);

      const rsl::string_view fullpath = path::join(g_mounted_roots.at(root), path);
      return file::exists_abspath(fullpath);
    }
    bool is_file(rsl::string_view path)
    {
      path = path::remove_quotes(path);
      path = path::unsafe_abs_path(path);

      return file::exists_abspath(path);
    }
    bool is_dir(MountingPoint root, rsl::string_view path)
    {
      path = path::remove_quotes(path);

      const rsl::string_view fullpath = path::join(g_mounted_roots.at(root), path);
      return directory::exists_abspath(fullpath);
    }
    bool is_dir(rsl::string_view path)
    {
      path = path::remove_quotes(path);
      path = path::unsafe_abs_path(path);

      return directory::exists_abspath(path);
    }
    bool is_mounted(MountingPoint mount)
    {
      return g_mounted_roots.contains(mount);
    }

    rsl::vector<rsl::string> list_entries(MountingPoint root, rsl::string_view path)
    {
      path = path::remove_quotes(path);

      const rsl::string_view fullpath = path::join(g_mounted_roots.at(root), path);
      return list_entries(fullpath);
    }
    rsl::vector<rsl::string> list_dirs(MountingPoint root, rsl::string_view path)
    {
      path = path::remove_quotes(path);

      const rsl::string_view fullpath = path::join(g_mounted_roots.at(root), path);
      return list_dirs(fullpath);
    }
    rsl::vector<rsl::string> list_files(MountingPoint root, rsl::string_view path)
    {
      path = path::remove_quotes(path);

      const rsl::string_view fullpath = path::join(g_mounted_roots.at(root), path);
      return list_files(fullpath);
    }
    rsl::vector<rsl::string> list_entries(rsl::string_view path)
    {
      path = path::remove_quotes(path);
      path = path::unsafe_abs_path(path);

      return directory::list_entries(path);
    }
    rsl::vector<rsl::string> list_dirs(rsl::string_view path)
    {
      path = path::remove_quotes(path);
      path = path::unsafe_abs_path(path);

      return directory::list_dirs(path);
    }
    rsl::vector<rsl::string> list_files(rsl::string_view path)
    {
      path = path::remove_quotes(path);
      path = path::unsafe_abs_path(path);

      return directory::list_files(path);
    }

    // --------------------------------------------------------------------------------------------------------------------------------
    //                                                  ABSOLUTE PATH IMPLEMENTATIONS
    // --------------------------------------------------------------------------------------------------------------------------------

    // --------------------------------
    // READING
    // --------------------------------
    ReadRequest read_file_abspath_async(rsl::string_view filepath)
    {
      const rsl::unique_lock lock(g_read_request_mutex);

      // If we already have a request for this file, add a new request to signal to the queued request
      auto it = g_queued_requests.find(filepath);
      if (it != g_queued_requests.end())
      {
        ReadRequest request(filepath, it->value.get());
        it->value->add_request_to_signal(&request);
        return request;
      }

      // If we don't have a request for this file yet, create a new one and add it to the hash table
      rsl::unique_ptr<QueuedRequest> queued_request = rsl::make_unique<QueuedRequest>(filepath);
      ReadRequest request(filepath, queued_request.get());

      // create the read request, which holds a link to the queued request
      queued_request->add_request_to_signal(&request);

      auto emplace_res = g_queued_requests.emplace(rsl::string(filepath), rsl::move(queued_request));
      g_read_requests_in_order.push_back(emplace_res.inserted_element->key);

      return request;
    }

  } // namespace vfs
} // namespace rex