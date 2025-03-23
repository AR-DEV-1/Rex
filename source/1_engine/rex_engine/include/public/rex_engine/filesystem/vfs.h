#pragma once

#include "rex_engine/engine/defines.h"
#include "rex_engine/string/tmp_string.h"
#include "rex_engine/filesystem/mounting_point.h"
#include "rex_engine/filesystem/directory.h"
#include "rex_engine/filesystem/read_request.h"
#include "rex_engine/diagnostics/error.h"
#include "rex_engine/memory/blob.h"
#include "rex_engine/memory/memory_types.h"
#include "rex_std/bonus/attributes.h"
#include "rex_std/bonus/memory/memory_size.h"
#include "rex_std/bonus/string.h"
#include "rex_std/bonus/types.h"
#include "rex_std/bonus/utility/yes_no.h"
#include "rex_std/string_view.h"

/// Rex VFS system supports syncronous and async file IO
/// We want to go for very simplistic design API at the moment
/// we may want to change this in the future if it's needed
///
/// SYNCRONOUS FILE IO
///
/// File Reading:
// rex::memory::Blob content = rex::vfs::read_file("path/to/file");
///
/// File Writing:
// int x = 0;
// rex::vfs::write_to_file("path/to/file.txt", &x, sizeof(x);
///
/// ASYNC FILE IO
///
/// We only support async file reading, as async file writing
/// Doesn't make much sense for the moment
///
// rex::vfs::ReadRequest read_request = rex::vfs::read_file_async("path/to/file");
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
  namespace vfs
  {
    // Returns the root of all files
    rsl::string_view root();

    // Returns the root directory of the engine files
    rsl::string_view engine_root();

    // Returns the root directory of the current project
    rsl::string_view project_root();

    // Returns the root for all sessions data
    rsl::string_view sessions_root();

    // Returns the root for all sessions data of this project
    rsl::string_view project_sessions_root();

    // Returns the root for all files outputed during this session run (eg. logs)
    rsl::string_view current_session_root();

    DEFINE_YES_NO_ENUM(AppendToFile);

    void init();
    void set_root(rsl::string_view root);
    void mount(MountingPoint root, rsl::string_view path);
    void shutdown();

    // All functions have an equivalent "_abspath" function
    // The "_abspath" functions can be used for performance/memory optimisations
    // They take the same arguments as input, but the arguments are expected to be an absolute path already
    // Avoiding need to convert it to an abs path in later stacks

    // --------------------------------
    // CREATING
    // --------------------------------
    Error create_file(MountingPoint root, rsl::string_view filepath);
    Error create_file(rsl::string_view filepath);
    Error create_dir(MountingPoint root, rsl::string_view path);
    Error create_dir(rsl::string_view path);
    Error create_dirs(rsl::string_view path);

    // --------------------------------
    // DELETING
    // --------------------------------
    Error delete_file(MountingPoint root, rsl::string_view path);
    Error delete_file(rsl::string_view path);
    Error delete_dir(MountingPoint root, rsl::string_view path);
    Error delete_dir(rsl::string_view path);
    Error delete_dir_recursive(MountingPoint root, rsl::string_view path);
    Error delete_dir_recursive(rsl::string_view path);

    // --------------------------------
    // READING
    // --------------------------------
    REX_NO_DISCARD memory::Blob read_file(MountingPoint root, rsl::string_view filepath);
    REX_NO_DISCARD memory::Blob read_file(rsl::string_view filepath);
    s32 read_file(MountingPoint root, rsl::string_view filepath, rsl::byte* buffer, s32 size);
    s32 read_file(rsl::string_view filepath, rsl::byte* buffer, s32 size);
    REX_NO_DISCARD ReadRequest read_file_async(MountingPoint root, rsl::string_view filepath);
    REX_NO_DISCARD ReadRequest read_file_async(rsl::string_view filepath);

    // --------------------------------
    // WRITING
    // --------------------------------
    Error write_to_file(MountingPoint root, rsl::string_view filepath, const void* data, card64 size, AppendToFile shouldAppend);
    Error write_to_file(rsl::string_view filepath, const void* data, card64 size, AppendToFile shouldAppend);
    Error write_to_file(MountingPoint root, rsl::string_view filepath, rsl::string_view text, AppendToFile shouldAppend);
    Error write_to_file(rsl::string_view filepath, rsl::string_view text, AppendToFile shouldAppend);
    Error write_to_file(rsl::string_view filepath, const memory::Blob& blob, AppendToFile shouldAppend);
    Error write_to_file(MountingPoint root, rsl::string_view filepath, const memory::Blob& blob, AppendToFile shouldAppend);

    // --------------------------------
    // CONVERTING
    // --------------------------------
    scratch_string abs_path(rsl::string_view path);
    scratch_string abs_path(MountingPoint root, rsl::string_view path);
    rsl::string_view mount_path(MountingPoint mount);

    // --------------------------------
    // QUERYING
    // --------------------------------
    bool exists(rsl::string_view path);
    bool exists(MountingPoint root, rsl::string_view path);
    bool is_file(MountingPoint root, rsl::string_view path);
    bool is_file(rsl::string_view path);
    bool is_dir(MountingPoint root, rsl::string_view path);
    bool is_dir(rsl::string_view path);
    bool is_mounted(MountingPoint mount);

    REX_NO_DISCARD rsl::vector<rsl::string> list_entries(MountingPoint root, rsl::string_view path);
    REX_NO_DISCARD rsl::vector<rsl::string> list_dirs(MountingPoint root, rsl::string_view path);
    REX_NO_DISCARD rsl::vector<rsl::string> list_files(MountingPoint root, rsl::string_view path);
    REX_NO_DISCARD rsl::vector<rsl::string> list_entries(rsl::string_view path);
    REX_NO_DISCARD rsl::vector<rsl::string> list_dirs(rsl::string_view path);
    REX_NO_DISCARD rsl::vector<rsl::string> list_files(rsl::string_view path);

    // --------------------------
    // ABSOLUTE PATH IMPLEMENTATIONS
    // --------------------------

    // --------------------------------
    // READING
    // --------------------------------
    REX_NO_DISCARD ReadRequest read_file_abspath_async(rsl::string_view filepath);

  } // namespace vfs
} // namespace rex

#ifdef REX_PLATFORM_WINDOWS
  #include "rex_engine/platform/win/filesystem/win_vfs.h"
#endif