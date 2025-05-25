#pragma once

#include "rex_engine/engine/defines.h"
#include "rex_engine/engine/state_controller.h"
#include "rex_engine/filesystem/mounting_point.h"
#include "rex_engine/filesystem/directory.h"
#include "rex_engine/filesystem/read_request.h"
#include "rex_engine/filesystem/go_recursive_enum.h"
#include "rex_engine/diagnostics/error.h"
#include "rex_engine/memory/blob.h"
#include "rex_engine/memory/memory_types.h"
#include "rex_std/bonus/attributes.h"
#include "rex_std/bonus/memory/memory_size.h"
#include "rex_std/bonus/string.h"
#include "rex_std/bonus/types.h"
#include "rex_std/bonus/utility/yes_no.h"
#include "rex_std/string_view.h"
#include "rex_std/thread.h"

#include "rex_engine/engine/globals.h"

/// Rex VFS system supports syncronous and async file IO
/// We want to go for very simplistic design API at the moment
/// we may want to change this in the future if it's needed
///
/// SYNCRONOUS FILE IO
///
/// File Reading:
// rex::memory::Blob content = rex::vfs::instance()->read_file("path/to/file");
///
/// File Writing:
// int x = 0;
// rex::vfs::instance()->write_to_file("path/to/file.txt", &x, sizeof(x);
///
/// ASYNC FILE IO
///
/// We only support async file reading, as async file writing
/// Doesn't make much sense for the moment
///
// rex::vfs::instance()->ReadRequest read_request = rex::vfs::instance()->read_file_async("path/to/file");
///
/// Do some code ..
///
/// Wait for the request to finish reading its data
// read_request.wait();
///
// const rsl::byte* content = read_request.buffer();
// rsl::memory_size size = read_request.size();
///
/// Do something with the content
/// ..

namespace rex
{
  DEFINE_LOG_CATEGORY(LogVfs);

  enum class VfsState
  {
    NotInitialized = BIT(0),
    Initializing = BIT(1),
    Running = BIT(2),
    ShuttingDown = BIT(3),
    ShutDown = BIT(4)
  };

  DEFINE_YES_NO_ENUM(AppendToFile);

  class VfsBase
  {
  public:
    VfsBase(rsl::string_view root);
    virtual ~VfsBase();

    // Returns the root of all the files
    rsl::string_view root() const;

    // Mounts a new point to a path
    void mount(MountingPoint root, rsl::string_view path);

    // --------------------------------
    // CREATING
    // --------------------------------
    Error create_file(MountingPoint root, rsl::string_view filepath)                  ;
    Error create_dir(MountingPoint root, rsl::string_view path)                       ;
    
    virtual Error create_file(rsl::string_view filepath)                           = 0;
    virtual Error create_dir(rsl::string_view path)                                = 0;
    virtual Error create_dirs(rsl::string_view path)                               = 0;

    // --------------------------------
    // DELETING
    // --------------------------------
    Error delete_file(MountingPoint root, rsl::string_view path)           ;
    Error delete_dir(MountingPoint root, rsl::string_view path)            ;
    Error delete_dir_recursive(MountingPoint root, rsl::string_view path)  ;
    
    virtual Error delete_file(rsl::string_view path)                               = 0;
    virtual Error delete_dir(rsl::string_view path)                                = 0;
    virtual Error delete_dir_recursive(rsl::string_view path)                      = 0;

    // --------------------------------
    // READING
    // --------------------------------
    REX_NO_DISCARD memory::Blob read_file(MountingPoint root, rsl::string_view filepath)           ;
    s32 read_file(MountingPoint root, rsl::string_view filepath, rsl::byte* buffer, s32 size)      ;
    REX_NO_DISCARD ReadRequest read_file_async(MountingPoint root, rsl::string_view filepath)      ;
    REX_NO_DISCARD ReadRequest read_file_async(rsl::string_view filepath)                          ;

    REX_NO_DISCARD virtual memory::Blob read_file(rsl::string_view filepath)                               = 0;
    virtual s32 read_file(rsl::string_view filepath, rsl::byte* buffer, s32 size)                          = 0;

    // --------------------------------
    // WRITING
    // --------------------------------
    Error write_to_file(MountingPoint root, rsl::string_view filepath, const void* data, card64 size, AppendToFile shouldAppend)   ;
    Error write_to_file(MountingPoint root, rsl::string_view filepath, rsl::string_view text, AppendToFile shouldAppend)           ;
    Error write_to_file(MountingPoint root, rsl::string_view filepath, const memory::Blob& blob, AppendToFile shouldAppend)        ;
    
    virtual Error write_to_file(rsl::string_view filepath, const void* data, card64 size, AppendToFile shouldAppend)                       = 0;
    virtual Error write_to_file(rsl::string_view filepath, rsl::string_view text, AppendToFile shouldAppend)                               = 0;
    virtual Error write_to_file(rsl::string_view filepath, const memory::Blob& blob, AppendToFile shouldAppend)                            = 0;

    // --------------------------------
    // CONVERTING
    // --------------------------------
    scratch_string abs_path(MountingPoint root, rsl::string_view path) const          ;
    rsl::string_view mount_path(MountingPoint mount) const                            ;
    
    virtual scratch_string abs_path(rsl::string_view path) const                              = 0;

    // --------------------------------
    // QUERYING
    // --------------------------------
    bool is_directory(MountingPoint root, rsl::string_view path) const       ;
    bool is_file(MountingPoint root, rsl::string_view path) const       ;
    bool exists(MountingPoint root, rsl::string_view path) const      ;
    bool is_mounted(MountingPoint mount) const                         ;
    
    virtual bool is_directory(rsl::string_view path) const                     = 0;
    virtual bool is_file(rsl::string_view path) const                          = 0;
    virtual bool exists(rsl::string_view path) const                           = 0;

    REX_NO_DISCARD rsl::vector<rsl::string> list_entries(MountingPoint root, rsl::string_view path, Recursive recursive)      ;
    REX_NO_DISCARD rsl::vector<rsl::string> list_dirs(MountingPoint root, rsl::string_view path)         ;
    REX_NO_DISCARD rsl::vector<rsl::string> list_files(MountingPoint root, rsl::string_view path)        ;
    
    REX_NO_DISCARD virtual rsl::vector<rsl::string> list_entries(rsl::string_view path, Recursive recursive)    = 0;
    REX_NO_DISCARD virtual rsl::vector<rsl::string> list_dirs(rsl::string_view path)                             = 0;
    REX_NO_DISCARD virtual rsl::vector<rsl::string> list_files(rsl::string_view path)                            = 0;

  protected:
    rsl::string_view no_mount_path() const;

  private:
    // Root paths used by the VFS
    rsl::medium_stack_string m_root; // This the root where all relative paths will start from

    // This controls the state of the vfs
    StateController<VfsState> m_vfs_state_controller;

    // mutices for the asyncronous operation of the vfs
    rsl::mutex m_read_request_mutex;
    rsl::mutex m_closed_request_mutex;

    // queues the vfs uses for its async operations
    rsl::vector<rsl::string_view> m_read_requests_in_order;
    rsl::unordered_map<rsl::string, rsl::unique_ptr<QueuedRequest>> m_queued_requests;
    rsl::vector<rsl::unique_ptr<QueuedRequest>> m_closed_requests;

    // mounted roots
    rsl::unordered_map<MountingPoint, rsl::string> m_mounted_roots;

    // threads used by the vfs to perform the async operations
    rsl::thread m_reading_thread;
    rsl::thread m_closing_thread;
  };

  namespace vfs
  {
    void init(globals::GlobalUniquePtr<VfsBase> vfs);
    VfsBase* instance();
    void shutdown();

  } // namespace vfs
} // namespace rex
