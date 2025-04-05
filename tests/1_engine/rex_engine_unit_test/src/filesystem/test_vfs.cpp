#include "rex_unit_test/rex_catch2.h"

#include "rex_engine/filesystem/vfs.h"
#include "rex_engine/filesystem/native_filesystem.h"
#include "rex_engine/filesystem/path.h"
#include "rex_engine/filesystem/file.h"
#include "rex_engine/filesystem/directory.h"
#include "rex_engine/filesystem/tmp_dir.h"


#include "rex_engine/engine/casting.h"

namespace rex::test
{
	class ScopedVfsInitialization
	{
	public:
		ScopedVfsInitialization()
		{
			vfs::init(globals::make_unique<NativeFileSystem>(path::cwd()));
		}
		~ScopedVfsInitialization()
		{
			vfs::shutdown();
		}

	private:

	};
}

TEST_CASE("TEST - VFS - init & shutdown")
{
	REX_CHECK(rex::path::is_same(rex::vfs::instance()->root(), ""));
	REX_CHECK(rex::vfs::instance()->is_mounted(rex::MountingPoint::TestPath1) == false);

	rex::vfs::init(rex::globals::make_unique<rex::NativeFileSystem>(rex::path::cwd()));

	REX_CHECK(rex::path::is_same(rex::vfs::instance()->root(), rex::path::cwd()));

	rex::vfs::shutdown();

	REX_CHECK(rex::path::is_same(rex::vfs::instance()->root(), ""));
	REX_CHECK(rex::vfs::instance()->is_mounted(rex::MountingPoint::TestPath1) == false);
}
TEST_CASE("TEST - VFS - mount")
{
	rex::test::ScopedVfsInitialization vfs_init;

	// A mounting point should be unmounted by default
	// The user should always mount these manually
	REX_CHECK(rex::vfs::instance()->is_mounted(rex::MountingPoint::TestPath1) == false);
	REX_CHECK(rex::vfs::instance()->is_mounted(rex::MountingPoint::TestPath2) == false);
	REX_CHECK(rex::vfs::instance()->is_mounted(rex::MountingPoint::TestPath3) == false);

	rex::scratch_string test_path1 = rex::path::join(rex::engine::instance()->current_session_root(), "test_path1");
	rex::scratch_string test_path2 = rex::path::join(rex::engine::instance()->current_session_root(), "test_path2");

	REX_CHECK(!rex::directory::exists(test_path1));
	REX_CHECK(!rex::directory::exists(test_path2));

	// Test if we can mount a path that already exists
	rex::directory::create(test_path1);
	rex::vfs::instance()->mount(rex::MountingPoint::TestPath1, test_path1);
	REX_CHECK(rex::directory::exists(test_path1));	// A mounted directory should always exist after its mounted
	REX_CHECK(rex::vfs::instance()->is_mounted(rex::MountingPoint::TestPath1));
	REX_CHECK(rex::path::is_same(rex::vfs::instance()->mount_path(rex::MountingPoint::TestPath1), test_path1));

	// Test if we can mount a path that doesn't already exist
	rex::vfs::instance()->mount(rex::MountingPoint::TestPath2, test_path2);
	REX_CHECK(rex::directory::exists(test_path2));	// A mounted directory should always exist after its mounted
	REX_CHECK(rex::vfs::instance()->is_mounted(rex::MountingPoint::TestPath2));
	REX_CHECK(rex::path::is_same(rex::vfs::instance()->mount_path(rex::MountingPoint::TestPath2), test_path2));
}
TEST_CASE("TEST - VFS - read file")
{
	rex::test::ScopedVfsInitialization vfs_init;

	rsl::string_view test_path1 = "vfs_tests";
	rex::vfs::instance()->mount(rex::MountingPoint::TestPath1, test_path1);

	rsl::string_view filename = "vfs_test_file.txt";
	rex::scratch_string filepath = rex::path::join(test_path1, filename);
	
	rsl::string_view expected_content = "this is a test file";

	// Read a file using just the filepath
	rex::memory::Blob file_blob = rex::vfs::instance()->read_file(filepath);
	REX_CHECK(rex::memory::blob_to_string_view(file_blob) == expected_content);

	// Read a file using a mounting point
	file_blob = rex::vfs::instance()->read_file(rex::MountingPoint::TestPath1, filename);
	REX_CHECK(rex::memory::blob_to_string_view(file_blob) == expected_content);

	filename = "vfs_file_that_doesnt_exist.txt";
	filepath = rex::path::join(test_path1, filename);
	expected_content = "";

	// Try reading a file that doesn't exist using just the filepath
	file_blob = rex::vfs::instance()->read_file(filepath);
	REX_CHECK(rex::memory::blob_to_string_view(file_blob) == expected_content);

	// Try reading a file that doesn't exist using a mounting point
	file_blob = rex::vfs::instance()->read_file(rex::MountingPoint::TestPath1, filename);
	REX_CHECK(rex::memory::blob_to_string_view(file_blob) == expected_content);
}
TEST_CASE("TEST - VFS - read file async")
{
	rex::test::ScopedVfsInitialization vfs_init;

	rsl::string_view test_path1 = "vfs_tests";
	rex::vfs::instance()->mount(rex::MountingPoint::TestPath1, test_path1);

	rsl::string_view filename = "vfs_test_file.txt";
	rex::scratch_string filepath = rex::path::join(test_path1, filename);

	rsl::string_view expected_content = "this is a test file";

	// Read a file using just the filepath
	auto read_request = rex::vfs::instance()->read_file_async(filepath);
	read_request.wait();
	rsl::string_view file_blob(rex::char_cast(read_request.data()) , rex::narrow_cast<s32>(read_request.count().size_in_bytes()));
	REX_CHECK(file_blob == expected_content);

	// Read a file using a mounting point
	read_request = rex::vfs::instance()->read_file_async(rex::MountingPoint::TestPath1, filename);
	read_request.wait();
	file_blob = rsl::string_view(rex::char_cast(read_request.data()), rex::narrow_cast<s32>(read_request.count().size_in_bytes()));
	REX_CHECK(file_blob == expected_content);

	filename = "vfs_file_that_doesnt_exist.txt";
	filepath = rex::path::join(test_path1, filename);
	expected_content = "";

	// Try reading a file that doesn't exist using just the filepath
	read_request = rex::vfs::instance()->read_file_async(filepath);
	read_request.wait();
	file_blob = rsl::string_view(rex::char_cast(read_request.data()), rex::narrow_cast<s32>(read_request.count().size_in_bytes()));
	REX_CHECK(file_blob == expected_content);

	// Try reading a file that doesn't exist using a mounting point
	read_request.wait();
	read_request = rex::vfs::instance()->read_file_async(rex::MountingPoint::TestPath1, filename);
	file_blob = rsl::string_view(rex::char_cast(read_request.data()), rex::narrow_cast<s32>(read_request.count().size_in_bytes()));
	REX_CHECK(file_blob == expected_content);
}
TEST_CASE("TEST - VFS - save to file")
{
	rex::test::ScopedVfsInitialization vfs_init;

	rsl::string_view test_path1 = "vfs_tests";
	rex::vfs::instance()->mount(rex::MountingPoint::TestPath1, test_path1);

	rex::scratch_string filename = rex::path::random_filename();
	rex::scratch_string filepath = rex::path::join(test_path1, filename);

	rsl::string_view dummy_content = "this is some dummy content";

	// Save a file using just the filepath, no appending
	rex::vfs::instance()->write_to_file(filepath, dummy_content, rex::AppendToFile::no);

	REX_CHECK(rex::vfs::instance()->exists(filepath));
	rex::memory::Blob file_blob = rex::vfs::instance()->read_file(filepath);
	REX_CHECK(rex::memory::blob_to_string_view(file_blob) == dummy_content);

	// Save a file using just the filepath, with appending
	rex::scratch_string dummy_content_appended(dummy_content);
	dummy_content_appended += dummy_content;
	rex::vfs::instance()->write_to_file(filepath, dummy_content, rex::AppendToFile::yes);
	file_blob = rex::vfs::instance()->read_file(filepath);
	REX_CHECK(rex::memory::blob_to_string_view(file_blob) == dummy_content_appended);

	rex::Error error = rex::Error::no_error();
	rex::vfs::instance()->delete_file(filepath);
	filename = rex::path::random_filename();
	filepath = rex::path::join(test_path1, filename);

	// Save a file using a mounting point, no appending
	rex::vfs::instance()->write_to_file(rex::MountingPoint::TestPath1, filename, dummy_content, rex::AppendToFile::no);
	rex::vfs::instance()->exists(rex::MountingPoint::TestPath1, filename);
	file_blob = rex::vfs::instance()->read_file(rex::MountingPoint::TestPath1, filename);
	REX_CHECK(rex::memory::blob_to_string_view(file_blob) == dummy_content);

	// Save a file using a mounting point, with appending
	rex::vfs::instance()->write_to_file(rex::MountingPoint::TestPath1, filename, dummy_content, rex::AppendToFile::yes);
	file_blob = rex::vfs::instance()->read_file(rex::MountingPoint::TestPath1, filename);
	REX_CHECK(rex::memory::blob_to_string_view(file_blob) == dummy_content_appended);

	rex::vfs::instance()->delete_file(filepath);
}
TEST_CASE("TEST - VFS - create dir")
{
	rex::test::ScopedVfsInitialization vfs_init;

	rsl::string_view test_path1 = "vfs_tests";
	rex::vfs::instance()->mount(rex::MountingPoint::TestPath1, test_path1);
	rsl::string dirname;
	rsl::string subdirname;

	// Create a single directory and delete it
	dirname.assign(rex::path::random_dir());
	rex::vfs::instance()->create_dir(dirname);
	REX_CHECK(rex::vfs::instance()->exists(dirname));
	REX_CHECK(rex::vfs::instance()->exists(dirname));
	rex::vfs::instance()->delete_dir(dirname);
	REX_CHECK(rex::vfs::instance()->exists(dirname) == false);
	REX_CHECK(rex::vfs::instance()->exists(dirname) == false);

	// Create a single directory and delete it
	dirname.assign(rex::path::random_dir());
	rex::vfs::instance()->create_dir(dirname);
	REX_CHECK(rex::vfs::instance()->exists(dirname));
	REX_CHECK(rex::vfs::instance()->exists(dirname));
	rex::vfs::instance()->delete_dir_recursive(dirname);
	REX_CHECK(rex::vfs::instance()->exists(dirname) == false);
	REX_CHECK(rex::vfs::instance()->exists(dirname) == false);

	// Create a single direcotry, then sub directory, then delete the root
	dirname.assign(rex::path::random_dir());
	subdirname.assign(rex::path::join(dirname, rex::path::random_dir()));
	rex::vfs::instance()->create_dir(dirname);
	rex::vfs::instance()->create_dir(subdirname);
	REX_CHECK(rex::vfs::instance()->exists(dirname));
	REX_CHECK(rex::vfs::instance()->exists(dirname));
	REX_CHECK(rex::vfs::instance()->exists(subdirname));
	REX_CHECK(rex::vfs::instance()->exists(subdirname));
	rex::vfs::instance()->delete_dir(dirname);
	REX_CHECK(rex::vfs::instance()->exists(dirname));
	REX_CHECK(rex::vfs::instance()->exists(dirname));
	REX_CHECK(rex::vfs::instance()->exists(subdirname));
	REX_CHECK(rex::vfs::instance()->exists(subdirname));
	rex::vfs::instance()->delete_dir_recursive(dirname);
	REX_CHECK(rex::vfs::instance()->exists(dirname) == false);
	REX_CHECK(rex::vfs::instance()->exists(dirname) == false);
	REX_CHECK(rex::vfs::instance()->exists(subdirname) == false);
	REX_CHECK(rex::vfs::instance()->exists(subdirname) == false);

	// Create multiple directories in 1 go, then delete the root
	dirname.assign(rex::path::random_dir());
	subdirname.assign(rex::path::join(dirname, rex::path::random_dir()));
	rex::vfs::instance()->create_dirs(subdirname);
	REX_CHECK(rex::vfs::instance()->exists(dirname));
	REX_CHECK(rex::vfs::instance()->exists(dirname));
	REX_CHECK(rex::vfs::instance()->exists(subdirname));
	REX_CHECK(rex::vfs::instance()->exists(subdirname));
	rex::vfs::instance()->delete_dir(dirname);
	REX_CHECK(rex::vfs::instance()->exists(dirname));
	REX_CHECK(rex::vfs::instance()->exists(dirname));
	REX_CHECK(rex::vfs::instance()->exists(subdirname));
	REX_CHECK(rex::vfs::instance()->exists(subdirname));
	rex::vfs::instance()->delete_dir_recursive(dirname);
	REX_CHECK(rex::vfs::instance()->exists(dirname) == false);
	REX_CHECK(rex::vfs::instance()->exists(dirname) == false);
	REX_CHECK(rex::vfs::instance()->exists(subdirname) == false);
	REX_CHECK(rex::vfs::instance()->exists(subdirname) == false);
}
TEST_CASE("TEST - VFS - create file")
{
	rex::test::ScopedVfsInitialization vfs_init;

	rsl::string_view test_path1 = "vfs_tests";
	rex::vfs::instance()->mount(rex::MountingPoint::TestPath1, test_path1);
	rex::scratch_string dirname;
	rex::scratch_string filename;
	rex::Error error = rex::Error::no_error();

	// Create file under root
	filename = rex::path::random_filename();
	error = rex::vfs::instance()->create_file(filename);
	
	REX_CHECK(error.has_error() == false);
	REX_CHECK(rex::vfs::instance()->exists(filename));
	REX_CHECK(rex::vfs::instance()->exists(filename));
	rex::vfs::instance()->delete_file(filename);

	// Create file in sub dir
	// this should fail as the directory isn't created yet
	dirname = rex::path::random_dir();
	rex::scratch_string filepath = rex::path::join(dirname, filename);
	error = rex::vfs::instance()->create_file(filepath);

	REX_CHECK(error.has_error());
	REX_CHECK(rex::vfs::instance()->exists(filepath) == false);
	REX_CHECK(rex::vfs::instance()->exists(filepath) == false);

	// Create the dir, then the file
	rex::vfs::instance()->create_dir(dirname);
	error = rex::vfs::instance()->create_file(filepath);
	REX_CHECK(error.has_error() == false);
	REX_CHECK(rex::vfs::instance()->exists(filepath));
	REX_CHECK(rex::vfs::instance()->exists(filepath));

	rex::vfs::instance()->delete_dir_recursive(dirname);
}
TEST_CASE("TEST - VFS - exists")
{
	rex::test::ScopedVfsInitialization vfs_init;

	rsl::string_view test_path1 = "vfs_tests";
	rex::vfs::instance()->mount(rex::MountingPoint::TestPath1, test_path1);

	REX_CHECK(rex::vfs::instance()->exists(rex::path::join(test_path1, "dummy_dir")));
	REX_CHECK(rex::vfs::instance()->exists(rex::path::join(test_path1, "dummy_dir", "dummy_file.txt")));

	REX_CHECK(rex::vfs::instance()->exists(rex::MountingPoint::TestPath1, "dummy_dir"));
	REX_CHECK(rex::vfs::instance()->exists(rex::MountingPoint::TestPath1, rex::path::join("dummy_dir", "dummy_file.txt")));

}
TEST_CASE("TEST - VFS - is dir")
{
	rex::test::ScopedVfsInitialization vfs_init;

	rsl::string_view test_path1 = "vfs_tests";
	rex::vfs::instance()->mount(rex::MountingPoint::TestPath1, test_path1);

	REX_CHECK(rex::vfs::instance()->exists(rex::path::join(test_path1, "dummy_dir")));
	REX_CHECK(rex::vfs::instance()->exists(rex::MountingPoint::TestPath1, "dummy_dir"));
}
TEST_CASE("TEST - VFS - is file")
{
	rex::test::ScopedVfsInitialization vfs_init;

	rsl::string_view test_path1 = "vfs_tests";
	rex::vfs::instance()->mount(rex::MountingPoint::TestPath1, test_path1);

	REX_CHECK(rex::vfs::instance()->exists(rex::path::join(test_path1, "dummy_dir", "dummy_file.txt")));
	REX_CHECK(rex::vfs::instance()->exists(rex::MountingPoint::TestPath1, rex::path::join("dummy_dir", "dummy_file.txt")));
}
TEST_CASE("TEST - VFS - abs path")
{
	rex::test::ScopedVfsInitialization vfs_init;

	rex::scratch_string cwd(rex::path::cwd());

	REX_CHECK(rex::path::abs_path("foo.txt") == rex::path::join(cwd, "foo.txt"));
	REX_CHECK(rex::path::abs_path("foo.txt.bar") == rex::path::join(cwd, "foo.txt.bar"));
	REX_CHECK(rex::path::abs_path("foo") == rex::path::join(cwd, "foo"));

	REX_CHECK(rex::path::abs_path("bar/foo.txt") == rex::path::join(cwd, "bar", "foo.txt"));
	REX_CHECK(rex::path::abs_path("bar/foo.txt.bar") == rex::path::join(cwd, "bar", "foo.txt.bar"));
	REX_CHECK(rex::path::abs_path("bar/foo") == rex::path::join(cwd, "bar", "foo"));

	REX_CHECK(rex::path::abs_path("c:/bar/foo.txt") == "c:/bar/foo.txt");
	REX_CHECK(rex::path::abs_path("c:/bar/foo.txt.bar") == "c:/bar/foo.txt.bar");
	REX_CHECK(rex::path::abs_path("c:/bar/foo") == "c:/bar/foo");
}
TEST_CASE("TEST - VFS - list files")
{
	rex::test::ScopedVfsInitialization vfs_init;
	rsl::string_view test_path1 = "vfs_tests";
	rex::vfs::instance()->mount(rex::MountingPoint::TestPath1, test_path1);
	rsl::vector<rsl::string> files;

	files = rex::vfs::instance()->list_files(rex::path::join(test_path1, "list_dir"));

	REX_CHECK(rsl::find_if(files.cbegin(), files.cend(), [&test_path1](rsl::string_view file) {return rex::path::is_same(file, "file1.txt"); }) != files.cend());
	REX_CHECK(rsl::find_if(files.cbegin(), files.cend(), [&test_path1](rsl::string_view file) {return rex::path::is_same(file, "file2.txt"); }) != files.cend());
	REX_CHECK(rsl::find_if(files.cbegin(), files.cend(), [&test_path1](rsl::string_view file) {return rex::path::is_same(file, "file3.txt"); }) != files.cend());

	files = rex::vfs::instance()->list_files(rex::MountingPoint::TestPath1 , "list_dir");

	REX_CHECK(rsl::find_if(files.cbegin(), files.cend(), [&test_path1](rsl::string_view file) {return rex::path::is_same(file, "file1.txt"); }) != files.cend());
	REX_CHECK(rsl::find_if(files.cbegin(), files.cend(), [&test_path1](rsl::string_view file) {return rex::path::is_same(file, "file2.txt"); }) != files.cend());
	REX_CHECK(rsl::find_if(files.cbegin(), files.cend(), [&test_path1](rsl::string_view file) {return rex::path::is_same(file, "file3.txt"); }) != files.cend());
}
TEST_CASE("TEST - VFS - list entries")
{
	rex::test::ScopedVfsInitialization vfs_init;
	rsl::string_view test_path1 = "vfs_tests";
	rex::vfs::instance()->mount(rex::MountingPoint::TestPath1, test_path1);
	rsl::vector<rsl::string> entries;

	entries = rex::vfs::instance()->list_entries(rex::path::join(test_path1, "list_dir"));

	REX_CHECK(rsl::find_if(entries.cbegin(), entries.cend(), [&test_path1](rsl::string_view entry) {return rex::path::is_same(entry, "folder1"); }) != entries.cend());
	REX_CHECK(rsl::find_if(entries.cbegin(), entries.cend(), [&test_path1](rsl::string_view entry) {return rex::path::is_same(entry, "folder2"); }) != entries.cend());
	REX_CHECK(rsl::find_if(entries.cbegin(), entries.cend(), [&test_path1](rsl::string_view entry) {return rex::path::is_same(entry, "folder3"); }) != entries.cend());
	REX_CHECK(rsl::find_if(entries.cbegin(), entries.cend(), [&test_path1](rsl::string_view entry) {return rex::path::is_same(entry, "file1.txt"); }) != entries.cend());
	REX_CHECK(rsl::find_if(entries.cbegin(), entries.cend(), [&test_path1](rsl::string_view entry) {return rex::path::is_same(entry, "file2.txt"); }) != entries.cend());
	REX_CHECK(rsl::find_if(entries.cbegin(), entries.cend(), [&test_path1](rsl::string_view entry) {return rex::path::is_same(entry, "file3.txt"); }) != entries.cend());

	entries = rex::vfs::instance()->list_entries(rex::MountingPoint::TestPath1, "list_dir");

	REX_CHECK(rsl::find_if(entries.cbegin(), entries.cend(), [&test_path1](rsl::string_view entry) {return rex::path::is_same(entry, "folder1"); }) != entries.cend());
	REX_CHECK(rsl::find_if(entries.cbegin(), entries.cend(), [&test_path1](rsl::string_view entry) {return rex::path::is_same(entry, "folder2"); }) != entries.cend());
	REX_CHECK(rsl::find_if(entries.cbegin(), entries.cend(), [&test_path1](rsl::string_view entry) {return rex::path::is_same(entry, "folder3"); }) != entries.cend());
	REX_CHECK(rsl::find_if(entries.cbegin(), entries.cend(), [&test_path1](rsl::string_view entry) {return rex::path::is_same(entry, "file1.txt"); }) != entries.cend());
	REX_CHECK(rsl::find_if(entries.cbegin(), entries.cend(), [&test_path1](rsl::string_view entry) {return rex::path::is_same(entry, "file2.txt"); }) != entries.cend());
	REX_CHECK(rsl::find_if(entries.cbegin(), entries.cend(), [&test_path1](rsl::string_view entry) {return rex::path::is_same(entry, "file3.txt"); }) != entries.cend());
}
TEST_CASE("TEST - VFS - list dirs")
{
	rex::test::ScopedVfsInitialization vfs_init;
	rsl::string_view test_path1 = "vfs_tests";
	rex::vfs::instance()->mount(rex::MountingPoint::TestPath1, test_path1);
	rsl::vector<rsl::string> dirs;

	dirs = rex::vfs::instance()->list_dirs(rex::path::join(test_path1, "list_dir"));

	REX_CHECK(rsl::find_if(dirs.cbegin(), dirs.cend(), [&test_path1](rsl::string_view dir) {return rex::path::is_same(dir, "folder1"); }) != dirs.cend());
	REX_CHECK(rsl::find_if(dirs.cbegin(), dirs.cend(), [&test_path1](rsl::string_view dir) {return rex::path::is_same(dir, "folder2"); }) != dirs.cend());
	REX_CHECK(rsl::find_if(dirs.cbegin(), dirs.cend(), [&test_path1](rsl::string_view dir) {return rex::path::is_same(dir, "folder3"); }) != dirs.cend());

	dirs = rex::vfs::instance()->list_dirs(rex::MountingPoint::TestPath1, "list_dir");

	REX_CHECK(rsl::find_if(dirs.cbegin(), dirs.cend(), [&test_path1](rsl::string_view dir) {return rex::path::is_same(dir, "folder1"); }) != dirs.cend());
	REX_CHECK(rsl::find_if(dirs.cbegin(), dirs.cend(), [&test_path1](rsl::string_view dir) {return rex::path::is_same(dir, "folder2"); }) != dirs.cend());
	REX_CHECK(rsl::find_if(dirs.cbegin(), dirs.cend(), [&test_path1](rsl::string_view dir) {return rex::path::is_same(dir, "folder3"); }) != dirs.cend());
}
