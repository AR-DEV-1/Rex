#include "rex_engine/filesystem/vfs.h"

#include "rex_engine/cmdline/cmdline.h"
#include "rex_engine/diagnostics/assert.h"
#include "rex_engine/diagnostics/logging/log_macros.h"
#include "rex_engine/diagnostics/logging/log_verbosity.h"
#include "rex_engine/engine/state_controller.h"

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
// eg: vfs::instance()->read_file and vfs::instance()->write_to_file
//
// the vfs also supports async read requests
// this is done by calling vfs::instance()->read_file_async
// This will return a read request immediately
// the user is meant to keep this read request
// alive as it'll be signaled when reading has finished
// After which the user can access its buffer
// to process the data it just read
// It works as follows:
//
// rex::vfs::instance()->ReadRequest request = rex::vfs::instance()->read_file_async("path/to/file");
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
  VfsBase::VfsBase(rsl::string_view root)
    : m_vfs_state_controller(VfsState::NotInitialized)
    , m_root(root)
  {}

  VfsBase::~VfsBase() = default;

  rsl::string_view VfsBase::root() const
  {
    return m_root;
  }
  void VfsBase::mount(MountingPoint root, rsl::string_view path)
  {
    rsl::string_view full_path = vfs::instance()->abs_path(path);

    REX_ASSERT_X(!m_mounted_roots.contains(root), "root {} is already mapped. currently mapped to '{}'", rsl::enum_refl::enum_name(root), m_mounted_roots.at(root));
    m_mounted_roots[root].assign(full_path);

    // make sure the mount exists
    if (!directory::exists(full_path))
    {
      create_dirs(full_path);
    }
  }

  // --------------------------------
  // CREATING
  // --------------------------------
  Error VfsBase::create_file(MountingPoint root, rsl::string_view filepath)
  {
    filepath = path::remove_quotes(filepath);

    scratch_string fullpath = path::join(m_mounted_roots.at(root), filepath);
    return create_file(fullpath);
  }
  Error VfsBase::create_dir(MountingPoint root, rsl::string_view path)
  {
    path = path::remove_quotes(path);

    scratch_string fullpath = path::join(m_mounted_roots.at(root), path);
    return create_dir(fullpath);
  }

  // --------------------------------
  // DELETING
  // --------------------------------
  Error VfsBase::delete_file(MountingPoint root, rsl::string_view path)
  {
    path = path::remove_quotes(path);

    scratch_string fullpath = path::join(m_mounted_roots.at(root), path);
    return delete_file(fullpath);
  }
  Error VfsBase::delete_dir(MountingPoint root, rsl::string_view path)
  {
    path = path::remove_quotes(path);

    const scratch_string fullpath = path::join(m_mounted_roots.at(root), path);
    return delete_dir(fullpath);
  }
  Error VfsBase::delete_dir_recursive(MountingPoint root, rsl::string_view path)
  {
    path = path::remove_quotes(path);

    const scratch_string fullpath = path::join(m_mounted_roots.at(root), path);
    return delete_dir_recursive(fullpath);
  }

  // --------------------------------
  // READING
  // --------------------------------
  REX_NO_DISCARD memory::Blob VfsBase::read_file(MountingPoint root, rsl::string_view filepath)
  {
    filepath = path::remove_quotes(filepath);

    const scratch_string fullpath = path::join(m_mounted_roots.at(root), filepath);
    return read_file(fullpath);
  }
  s32 VfsBase::read_file(MountingPoint root, rsl::string_view filepath, rsl::byte* buffer, s32 size)
  {
    filepath = path::remove_quotes(filepath);

    const scratch_string fullpath = path::join(m_mounted_roots.at(root), filepath);
    return read_file(fullpath, buffer, size);
  }
  REX_NO_DISCARD ReadRequest VfsBase::read_file_async(MountingPoint root, rsl::string_view filepath)
  {
    filepath = path::remove_quotes(filepath);

    const scratch_string fullpath = path::join(m_mounted_roots.at(root), filepath);
    return read_file_async(fullpath);
  }
  ReadRequest VfsBase::read_file_async(rsl::string_view path)
  {
    path = path::remove_quotes(path);
    path = path::unsafe_abs_path(path);

    // If we already have a request for this file, add a new request to signal to the queued request
    auto it = m_queued_requests.find(path);
    if (it != m_queued_requests.end())
    {
      ReadRequest request(path, it->value.get());
      it->value->add_request_to_signal(&request);
      return request;
    }

    // If we don't have a request for this file yet, create a new one and add it to the hash table
    rsl::unique_ptr<QueuedRequest> queued_request = rsl::make_unique<QueuedRequest>(path);
    ReadRequest request(path, queued_request.get());

    // create the read request, which holds a link to the queued request
    queued_request->add_request_to_signal(&request);

    auto emplace_res = m_queued_requests.emplace(rsl::string(path), rsl::move(queued_request));
    m_read_requests_in_order.push_back(emplace_res.inserted_element->key);

    return request;
  }

  // --------------------------------
  // WRITING
  // --------------------------------
  Error VfsBase::write_to_file(MountingPoint root, rsl::string_view filepath, const void* data, card64 size, AppendToFile shouldAppend)
  {
    filepath = path::remove_quotes(filepath);

    const rsl::string_view path = path::join(m_mounted_roots.at(root), filepath);
    return write_to_file(path, data, size, shouldAppend);
  }
  Error VfsBase::write_to_file(MountingPoint root, rsl::string_view filepath, rsl::string_view text, AppendToFile shouldAppend)
  {
    return write_to_file(root, filepath, text.data(), text.length(), shouldAppend);
  }
  Error VfsBase::write_to_file(MountingPoint root, rsl::string_view filepath, const memory::Blob& blob, AppendToFile shouldAppend)
  {
    return write_to_file(root, filepath, blob.data(), blob.size(), shouldAppend);
  }

  // --------------------------------
  // CONVERTING
  // --------------------------------
  scratch_string VfsBase::abs_path(MountingPoint root, rsl::string_view path)
  {
    REX_ASSERT_X(m_vfs_state_controller.has_state(VfsState::Running), "Trying to use vfs before it's initialized");
    REX_ASSERT_X(!path::is_absolute(path), "Passed an absolute path into a function that doesn't allow absolute paths");

    scratch_string mount_root = path::join(m_mounted_roots.at(root), path);

    return abs_path(mount_root);
  }
  rsl::string_view VfsBase::mount_path(MountingPoint mount)
  {
    if (m_mounted_roots.contains(mount))
    {
      return m_mounted_roots.at(mount);
    }

    return no_mount_path();
  }

  // --------------------------------
  // QUERYING
  // --------------------------------
  bool VfsBase::exists(MountingPoint root, rsl::string_view path)
  {
    path = path::remove_quotes(path);

    scratch_string fullpath = path::join(m_mounted_roots.at(root), path);
    return exists(fullpath);
  }
  bool VfsBase::is_mounted(MountingPoint mount)
  {
    return m_mounted_roots.contains(mount);
  }

  REX_NO_DISCARD rsl::vector<rsl::string> VfsBase::list_entries(MountingPoint root, rsl::string_view path)
  {
    path = path::remove_quotes(path);

    const rsl::string_view fullpath = path::join(m_mounted_roots.at(root), path);
    return list_entries(fullpath);
  }
	REX_NO_DISCARD rsl::vector<rsl::string> VfsBase::list_dirs(MountingPoint root, rsl::string_view path)
	{
    path = path::remove_quotes(path);

    const rsl::string_view fullpath = path::join(m_mounted_roots.at(root), path);
    return list_dirs(fullpath);
  }
	REX_NO_DISCARD rsl::vector<rsl::string> VfsBase::list_files(MountingPoint root, rsl::string_view path)
	{
    path = path::remove_quotes(path);

    const rsl::string_view fullpath = path::join(m_mounted_roots.at(root), path);
    return list_files(fullpath);
  }

  rsl::string_view VfsBase::no_mount_path() const
  {
    // explicitely stating "no mount found" here
    // as an empty path would indicate it's at the root
    // instead of it not existing

    return rsl::string_view("no mount found");
  }

  namespace vfs
  {
    globals::GlobalUniquePtr<VfsBase> g_vfs;
    void init(globals::GlobalUniquePtr<VfsBase> vfs)
    {
      g_vfs = rsl::move(vfs);
    }
    VfsBase* instance()
    {
      return g_vfs.get();
    }
    void shutdown()
    {
      g_vfs.reset();
    }
  } // namespace vfs
} // namespace rex