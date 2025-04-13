#include "rex_engine/platform/win/win_com_library.h"

#include "rex_engine/diagnostics/log.h"

#include "rex_engine/platform/win/diagnostics/win_call.h"

#include <coml2api.h>
#include <objbase.h>
#include <objidl.h>
#include <shlguid.h>
#include <shobjidl_core.h>
#include <wtypes.h>

namespace rex
{
	namespace win
	{
		DEFINE_LOG_CATEGORY(WinComLibrary);

		ComLibrary::ComLibrary()
		{
			const bool success = HR_SUCCESS(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)); // NOLINT(hicpp-signed-bitwise)

			REX_ERROR_X(WinComLibrary, success, "Failed to initialized Windows Com Library");

			// It's possible, even on successful init, that SetLastError was called with an error code
			rex::win::clear_win_errors();
		}
		ComLibrary::~ComLibrary()
		{
			CoUninitialize();
		}

		// Read a symbolic link's path and return the path it actually points to
		scratch_string ComLibrary::read_link(rsl::string_view filepath) // NOLINT(readability-convert-member-functions-to-static)
		{
			IShellLinkW* psl = nullptr;
			HRESULT hres = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, reinterpret_cast<LPVOID*>(&psl)); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
			rsl::big_stack_string res;

			if (SUCCEEDED(hres))
			{
				IPersistFile* ppf = nullptr;
				hres = psl->QueryInterface(IID_IPersistFile, reinterpret_cast<LPVOID*>(&ppf)); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)

				if (SUCCEEDED(hres))
				{
					const rsl::wstring wide_filepath(filepath.cbegin(), filepath.cend());
					hres = ppf->Load(wide_filepath.c_str(), STGM_READ);

					if (SUCCEEDED(hres))
					{
						hres = psl->GetPath((LPWSTR)res.data(), res.max_size(), nullptr, SLGP_UNCPRIORITY);
						res.reset_null_termination_offset();
					}

					ppf->Release();
				}

				psl->Release();
			}

			return scratch_string(res);
		}

		namespace com_lib
		{
			globals::GlobalUniquePtr<ComLibrary> g_win_com_lib;
			void init(globals::GlobalUniquePtr<ComLibrary> comLib)
			{
				g_win_com_lib = rsl::move(comLib);
			}
			ComLibrary* instance()
			{
				return g_win_com_lib.get();
			}
			void shutdown()
			{
				g_win_com_lib.reset();
			}
		} // namespace com_lib
	}   // namespace win
} // namespace rex