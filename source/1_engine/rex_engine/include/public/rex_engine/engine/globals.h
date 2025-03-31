#pragma once

#include "rex_std/type_traits.h"
#include "rex_std/bonus/utility.h"
#include "rex_engine/diagnostics/assert.h"

namespace rex
{
	namespace globals
	{
		void enable_global_destruction();
		void disable_global_destruction();
		bool is_global_destruction_enabled();

		template <typename T>
		struct GlobalObjectDeleter
		{
		public:
			constexpr GlobalObjectDeleter() = default;

			template <typename T2, rsl::enable_if_t<rsl::is_convertible_v<T2*, T*>, bool> = true>
			constexpr GlobalObjectDeleter(const GlobalObjectDeleter<T2>& /*unused*/) // NOLINT(google-explicit-constructor)
			{
			}

			constexpr void operator()(T* ptr) const
			{
				static_assert(sizeof(T) > 0, "can't delete an incomplete type"); // NOLINT(bugprone-sizeof-expression)
				REX_ASSERT_X(globals::is_global_destruction_enabled() == true || ptr == nullptr, "Deleting global object when not allowed to do so. {}", rsl::type_id<T>().name());

				delete ptr;
			}

		private:
		};
		template <typename T>
		using GlobalUniquePtr = rsl::unique_ptr<T, GlobalObjectDeleter<T>>;

		template <typename T, typename ... Args>
		GlobalUniquePtr<T> make_unique(Args&& ... args)
		{
			return GlobalUniquePtr<T>(new T(rsl::forward<Args>(args)...));
		}
	}
}