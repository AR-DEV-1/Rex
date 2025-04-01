#pragma once

#include "rex_engine/engine/defines.h"
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

#include "rex_engine/filesystem/vfs.h"

namespace rex
{
	class NativeFileSystem : public VfsBase
	{
  public:
    NativeFileSystem(rsl::string_view root);

    // --------------------------------
    // CREATING
    // --------------------------------
    Error create_file(rsl::string_view filepath) override;
    Error create_dir(rsl::string_view path) override;
    Error create_dirs(rsl::string_view path) override;

    // --------------------------------
    // DELETING
    // --------------------------------
    Error delete_file(rsl::string_view path) override;
    Error delete_dir(rsl::string_view path) override;
    Error delete_dir_recursive(rsl::string_view path) override;

    // --------------------------------
    // READING
    // --------------------------------
    REX_NO_DISCARD memory::Blob read_file(rsl::string_view filepath) override;
    s32 read_file(rsl::string_view filepath, rsl::byte* buffer, s32 size) override;

    // --------------------------------
    // WRITING
    // --------------------------------
    Error write_to_file(rsl::string_view filepath, const void* data, card64 size, AppendToFile shouldAppend) override;
    Error write_to_file(rsl::string_view filepath, rsl::string_view text, AppendToFile shouldAppend) override;
    Error write_to_file(rsl::string_view filepath, const memory::Blob& blob, AppendToFile shouldAppend) override;

    // --------------------------------
    // CONVERTING
    // --------------------------------
    scratch_string abs_path(rsl::string_view path) override;

    // --------------------------------
    // QUERYING
    // --------------------------------
    bool exists(rsl::string_view path) override;

    REX_NO_DISCARD rsl::vector<rsl::string> list_entries(rsl::string_view path) override;
    REX_NO_DISCARD rsl::vector<rsl::string> list_dirs(rsl::string_view path) override;
    REX_NO_DISCARD rsl::vector<rsl::string> list_files(rsl::string_view path) override;
	};
}