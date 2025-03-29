#pragma once

#include "rex_std/bonus/string.h"
#include "rex_std/bonus/types.h"
#include "rex_std/vector.h"
#include "rex_std/type_traits.h"
#include "rex_std/bonus/utility.h"
#include "rex_std/bonus/string.h"
#include "rex_engine/engine/types.h"

#include "rex_engine/text_processing/text_iterator.h"
#include "rex_engine/engine/globals.h"

namespace rex
{
  namespace path
  {
    struct SplitResult
    {
      rsl::string_view head;
      rsl::string_view tail;
    };
    struct SplitRootResult
    {
      rsl::string_view drive;
      rsl::string_view root;
      rsl::string_view tail;
    };

    namespace internal
    {
      // concat the arg to the string in filepath format
      template <typename StringLike>
      void join_string_view(StringLike& str, rsl::string_view arg)
      {
        if (arg.empty())
        {
          return;
        }

        str += arg;

        if (!str.ends_with(seperation_char()))
        {
          str += seperation_char();
        }
      }
      template <typename StringLike, typename Enum, rsl::enable_if_t<rsl::is_enum_v<Enum>, bool> = true>
      void join_enum(StringLike& str, Enum e)
      {
        rsl::string_view enum_name = rsl::enum_refl::enum_name(e);
        join_impl(str, enum_name);
      }
      template <typename StringLike, typename PathLikeType>
      void join_path_like(StringLike& str, PathLikeType&& arg)
      {
        join_string_view(str, rsl::string_view(rsl::forward<PathLikeType>(arg))); // NOLINT(google-readability-casting)
      }

      template <typename StringLike, typename PathLikeType>
      void join_impl(StringLike& str, PathLikeType&& firstArg)
      {
        if constexpr (rsl::is_same_v<rsl::string_view, PathLikeType>)
        {
          join_string_view(str, firstArg);
        }
        else if constexpr (rsl::is_enum_v<PathLikeType>)
        {
          join_enum(str, firstArg);
        }
        else
        {
          join_path_like(str, firstArg);
        }
      }

      template <typename StringLike, typename PathLikeType, typename... Args>
      void join_impl(StringLike& str, PathLikeType&& firstArg, Args&&... args)
      {
        join_impl(str, rsl::forward<PathLikeType>(firstArg));
        join_impl(str, rsl::forward<Args>(args)...);          // append the rest
      }
    }                                                         // namespace internal

    class PathIterator : public TextIterator
    {
		public:
      PathIterator();
      PathIterator(rsl::string_view path);
    };

    // -------------------------------------------------------------------------
    // THESE FUNCTIONS ARE REQUIRED TO BE IMPLEMENTED BY PLATFORM SPECIFIC CODE
    // -------------------------------------------------------------------------
    
    // --------------------------------
    // QUERYING
    // --------------------------------

    // Returns the current working directory
    rsl::string_view cwd();

    // Sets a new working directory and returns the old one
    // A valid and existing path is expected or an assert is raised
    scratch_string set_cwd(rsl::string_view dir);

    // Returns if the given path is an absolute path
    bool is_absolute(rsl::string_view path);

    // Returns if a file is under a certain directory
    bool is_under_dir(rsl::string_view path, rsl::string_view dir);

    // Returns true if the given path points to a junction
    bool is_junction(rsl::string_view path);

    // Returns true if the given path points to a symlink
    bool is_link(rsl::string_view path);

    // Returns true if absolute paths on this platform have a drive letter
    bool abs_needs_drive();

    // --------------------------------
    // UTILITY
    // --------------------------------

    // Returns the path of the current user's temp folder
    scratch_string temp_path();

    // For symlinks, returns the path the link points to
    // Otherwise returns the input
    scratch_string real_path(rsl::string_view path);

    // Splits the path into a head and a tail
    // the head is either the mount point or an empty string
    // the tail is everything else
    SplitResult split_origin(rsl::string_view path);

    // Split the path into 3 components
    // drive - root - tail
    // drive: mounting point
    // root: string of separators after the drive
    // tail: everything after the root
    // eg: c:/Users/Sam (Windows)
    // drive: C:
    // root: /
    // tail: Users/Sam

    SplitRootResult split_root(rsl::string_view path);

    // ------------------------------------------------------------------------------
    //                          ABSOLUTE PATH IMPLEMENTATIONS
    // ------------------------------------------------------------------------------

    // --------------------------------
    // QUERYING
    // --------------------------------
    // Sets a new working directory and returns the old one
    // A valid and existing path is expected or an assert is raised
    scratch_string set_cwd_abspath(rsl::string_view dir);

    // --------------------------------
    // UTILITY
    // --------------------------------
    // For symlinks, returns the path the link points to
    // Otherwise returns the input
    scratch_string real_path_abspath(rsl::string_view path);

    // -------------------------------------------------------------------------
    // END OF PLATFORM SPECIFIC FUNCTIONS
    // -------------------------------------------------------------------------

    // --------------------------------
    // CONVERTING
    // --------------------------------
    // Returns the absolute path for the given path
    scratch_string abs_path(rsl::string_view path);

    // Tries to convert the path to an abs path
    // and returns a string_view to this
    // This method works as it assumes using a scratch string
    // who's underlying memory is not deallocated after it goes out of scope
    rsl::string_view unsafe_abs_path(rsl::string_view path);

    // Tries to convert the path to an norm path
    // and returns a string_view to this
    // This method works as it assumes using a scratch string
    // who's underlying memory is not deallocated after it goes out of scope
    rsl::string_view unsafe_norm_path(rsl::string_view path);

    // Changes the extension of a path string_view
    // If extension argument is empty, the extension is removed
    // if the path doesn't have an extension, the extension specified gets appended
    scratch_string change_extension(rsl::string_view path, rsl::string_view extension);

    // Returns the fullpath without the drive, if it's present
    rsl::string_view remove_drive(rsl::string_view path);

    // --------------------------------
    // QUERYING
    // --------------------------------
    // returns the seperation char for paths
    char8 seperation_char();

    // Return the max length a file can have
    constexpr s32 max_path_length()
    {
      // MAX_PATH on Windows is 260 by default
      // You can switch a flag to support longer paths (bugged in Windows 11 though)
      // https://answers.microsoft.com/en-us/windows/forum/all/long-file-path-issues-in-windows-11-pro-with/0c440721-e35a-4b58-9f5a-645656768f9d
      // but by default this is disabled
      return 256;
    }

    // returns an array of path names that aren't allowed to be used
    const rsl::vector<rsl::string_view>& invalid_path_names();
    // returns an array of invalid characters for directories
    const rsl::vector<char8>& invalid_path_chars();

    // returns true if it's a valid path, returns false otherwise
    bool is_valid_path(rsl::string_view path);
    // returns true if it's a valid filename, returns false otherwise
    bool is_valid_filename(rsl::string_view filename);

    // Returns the directory path of the given path
    rsl::string_view dir_name(rsl::string_view path);
    // Returns the extension of the given path
    rsl::string_view extension(rsl::string_view path);
    // Returns the filename of the given path
    rsl::string_view filename(rsl::string_view path);
    // Returns the filename of the given path without its extension
    rsl::string_view stem(rsl::string_view path);

    // returns how deep a path is
    // basically counts the number of slashes
    // it does this without converting it to an absolute path
		// a file directly under the root has a depth of 1, any subdirectory increments depth by 1
    // this makes it so that the root always has a depth of 0
    s32 depth(rsl::string_view path, rsl::string_view root = cwd());

    // Retruns the root directory path of the given path
    rsl::string_view path_root(rsl::string_view path);
    // Returns a random directory, but doesn't create it
    scratch_string random_dir();
    // Returns a random filename, but doesn't create it
    scratch_string random_filename();
    // Returns the longest common sub-path of each pathname in the sequence
    rsl::string_view common_path(rsl::range<rsl::string_view> paths);
    // Normalizes the path, removing redundant dots for current and parent directories
    // Converts forward slashes to backward slashes
    scratch_string norm_path(rsl::string_view path);
    // Returns a relative path to path, starting from the current working directory
    scratch_string rel_path(rsl::string_view path);
    // Returns a relative path to path, starting from the start directory
    scratch_string rel_path(rsl::string_view path, rsl::string_view start);
    // Returns if the given path has an extension
    bool has_extension(rsl::string_view path);
    // Returns if the given path is a relative path
    bool is_relative(rsl::string_view path);
    // Returns true if 2 paths point to the same file or directory
    bool is_same(rsl::string_view path1, rsl::string_view path2);

    // returns how deep from the root path is
    // basically counts the number of slashes
    // a file directly under the root has a depth of 1, any subdirectory increments depth by 1
    // this makes it so that the root always has a depth of 0
    s32 abs_depth(rsl::string_view path);
    // Returns true if the path starts with a drive letter, false otherwise
    bool has_drive(rsl::string_view path);
    // Returns true if the path given is the drive
    bool is_drive(rsl::string_view path);
    // Returns true if the path is pointing to the root
    bool is_root(rsl::string_view path);
    // Returns true if both paths are on the same mount
    bool has_same_root(rsl::string_view lhs, rsl::string_view rhs);
    // Returns true if the path is normalized
    // A path is normalized if it doesn't have any extra slashes
    // and it doesn't have any "current_dir" and "parent_dir" tokens
    bool is_normalized(rsl::string_view path);

    // --------------------------------
    // UTILITY
    // --------------------------------

    // Splits the path into a head and a tail
    // the tail is the last pathname component
    // the head is everything leading up to that
    SplitResult split(rsl::string_view path);
    // Splits the path into a head and a tail
    // the head is the directory and the stem
    // the tail is the extension
    SplitResult split_ext(rsl::string_view path);

    template <typename ResultType = scratch_string, typename... PathLikeTypes>
    void join_to(ResultType& outResult, PathLikeTypes&&... paths)
    {
      if (!outResult.empty() && !outResult.ends_with(seperation_char()))
      {
        outResult += seperation_char();
      }

      internal::join_impl(outResult, rsl::forward<PathLikeTypes>(paths)...);
      
      if (!outResult.empty())
      {
        outResult.pop_back();
      }
    }

    // Join multiple paths together
    template <typename ResultType = scratch_string, typename... PathLikeTypes>
    ResultType join(PathLikeTypes&&... paths)
    {
      // To avoid over allocation we generate the full path on the stack
      // after which we copy this memory into a heap based allocation
      // has a little impact on performance, but reduces memory usage
      path_stack_string stack_res;
      internal::join_impl(stack_res, rsl::forward<PathLikeTypes>(paths)...);

      if (!stack_res.empty())
      {
        stack_res.pop_back(); // remove the last seperation char
      }

      // If the user is expected a stack string of the same size
      // There's no point in copying it over to another one, just return this one
      if constexpr (rsl::is_same_v<decltype(stack_res), ResultType>)
      {
        return stack_res;
      }

      // Now that we have the final result, copy it over into the destination type
      ResultType res;
      res.assign(stack_res.to_view());

      return res;
    }

    // removes leading and trailing quotes from a path
    rsl::string_view remove_quotes(rsl::string_view path);

    // ------------------------------------------------------------------------------
    //                          ABSOLUTE PATH IMPLEMENTATIONS
    // ------------------------------------------------------------------------------

    // --------------------------------
    // QUERYING
    // --------------------------------

    // Returns a relative path to path, starting from the start directory
    scratch_string rel_path_abspath(rsl::string_view path, rsl::string_view start);
    // Returns true if 2 paths point to the same file or directory
    bool is_same_abspath(rsl::string_view path1, rsl::string_view path2);

    // returns how deep from the root path is
    // basically counts the number of slashes
    // a file directly under the root has a depth of 1, any subdirectory increments depth by 1
    // this makes it so that the root always has a depth of 0
    s32 abs_depth_abspath(rsl::string_view path);
    // Returns true if both paths are on the same mount
    bool has_same_root_abspath(rsl::string_view lhs, rsl::string_view rhs);

  } // namespace path

  using path_stack_string = rsl::stack_string<char8, path::max_path_length()>;
} // namespace rex

#ifdef REX_PLATFORM_WINDOWS
  #include "rex_engine/platform/win/filesystem/win_path.h"
#endif