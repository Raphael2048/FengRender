#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "util/defines.hpp"
#include "util/math.hpp"
#pragma warning(push, 0)

#define D3DX12_INC d3dx12_rt.h

using Microsoft::WRL::ComPtr;
namespace feng {


/*Helper function to get readable error messages from HResults
code originated from https://docs.microsoft.com/en-us/windows/desktop/cossdk/interpreting-error-codes
*/
inline std::string HResultToString(HRESULT hr)
{
	if (FACILITY_WINDOWS == HRESULT_FACILITY(hr))
	{
		hr = HRESULT_CODE(hr);
	}
	TCHAR* sz_err_msg;

	if (FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&sz_err_msg, 0, NULL) != 0)
	{
		std::string retval = sz_err_msg;
		LocalFree(sz_err_msg);
		return(retval);
	}
	else
	{
		return(std::string("[Could not find a description for error # %#x.", hr));
	}
}

//! Checks whether the d3d12 object exists before releasing it.
#define SAFE_RELEASE(obj) { if ( obj ) { obj->Release(); obj = NULL; } }

//! Handles a hresult.
#define TRY(result) if (FAILED(result)) { FMSG("An hresult returned a error!. File: " + std::string(__FILE__) + " Line: " + std::to_string(__LINE__) + " HRResult: " +  HResultToString(result)); }

//! Handles a hresult and outputs a specific message.
#define TRY_M(result, msg) if (FAILED(result)) { FMSG(static_cast<std::string>(msg) + " HRResult: " + HResultToString(result)); }

//! This macro is used to name d3d12 resources.
#define NAME_D3D12RESOURCE(r, n) { auto temp = std::string(__FILE__); \
r->SetName(std::wstring(std::wstring(n) + L" (line: " + std::to_wstring(__LINE__) + L" file: " + std::wstring(temp.begin(), temp.end())).c_str()); }

//! This macro is used to name d3d12 resources with a placeholder name. TODO: "Unamed Resource" should be "Unamed [typename]"
#define NAME_D3D12RESOURCE(r) { auto temp = std::string(__FILE__); \
r->SetName(std::wstring(L"Unnamed Resource (line: " + std::to_wstring(__LINE__) + L" file: " + std::wstring(temp.begin(), temp.end())).c_str()); }

// Particular version automatically rounds the alignment to a two power.
template<typename T, typename A>
constexpr inline T SizeAlignTwoPower(T size, A alignment)
{
	return (size + (alignment - 1U)) & ~(alignment - 1U);
}

// Particular version always aligns to the provided alignment
template<typename T, typename A>
constexpr inline T SizeAlignAnyAlignment(T size, A alignment)
{
	return (size / alignment + (size%alignment > 0))*alignment;
}

#pragma warning(pop)

#define BACK_BUFFER_SIZE 3


static size_t ConstantBufferSize(size_t size)
{
	return (size + 255) & ~255;
}
}