#pragma once
#include <utility>
namespace
{
	constexpr int const_atoi(char c)
	{
		return c - '0';
	}
}

#ifdef _MSC_VER
#define ALWAYS_INLINE __forceinline
#else
#define ALWAYS_INLINE __attribute__((always_inline))
#endif

template<typename _string_type, size_t _length>
class _Basic_XorStr
{
	using value_type = typename _string_type::value_type;
	static constexpr auto _length_minus_one = _length - 1;

public:
	constexpr ALWAYS_INLINE _Basic_XorStr(value_type const (&str)[_length])
		: _Basic_XorStr(str, std::make_index_sequence<_length_minus_one>())
	{

	}

	inline auto c_str() const
	{
		decrypt();

		return data;
	}

	inline auto str() const
	{
		decrypt();

		return _string_type(data, data + _length_minus_one);
	}

	inline operator _string_type() const
	{
		return str();
	}

private:
	template<size_t... indices>
	constexpr ALWAYS_INLINE _Basic_XorStr(value_type const (&str)[_length], std::index_sequence<indices...>)
		: data{ crypt(str[indices], indices)..., '\0' },
		encrypted(true)
	{

	}

	static constexpr auto XOR_KEY = static_cast<value_type>(
		const_atoi(__TIME__[7]) +
		const_atoi(__TIME__[6]) * 10 +
		const_atoi(__TIME__[4]) * 60 +
		const_atoi(__TIME__[3]) * 600 +
		const_atoi(__TIME__[1]) * 3600 +
		const_atoi(__TIME__[0]) * 36000
		);

	static ALWAYS_INLINE constexpr auto crypt(value_type c, int i)
	{
		return static_cast<value_type>(c ^ (XOR_KEY + i));
	}

	inline void decrypt() const
	{
		if (encrypted)
		{
			for (size_t t = 0; t < _length_minus_one; t++)
			{
				data[t] = crypt(data[t], t);
			}
			encrypted = false;
		}
	}

	mutable value_type data[_length];
	mutable bool encrypted;
};
//---------------------------------------------------------------------------
template<size_t _length>
using XorStrA = _Basic_XorStr<std::string, _length>;
template<size_t _length>
using XorStrW = _Basic_XorStr<std::wstring, _length>;
template<size_t _length>
using XorStrU16 = _Basic_XorStr<std::u16string, _length>;
template<size_t _length>
using XorStrU32 = _Basic_XorStr<std::u32string, _length>;
//---------------------------------------------------------------------------
template<typename _string_type, size_t _length, size_t _length2>
inline auto operator==(const _Basic_XorStr<_string_type, _length>& lhs, const _Basic_XorStr<_string_type, _length2>& rhs)
{
	static_assert(_length == _length2, "XorStr== different length");

	return _length == _length2 && lhs.str() == rhs.str();
}
//---------------------------------------------------------------------------
template<typename _string_type, size_t _length>
inline auto operator==(const _string_type& lhs, const _Basic_XorStr<_string_type, _length>& rhs)
{
	return lhs.size() == _length && lhs == rhs.str();
}
//---------------------------------------------------------------------------
template<typename _stream_type, typename _string_type, size_t _length>
inline auto& operator<<(_stream_type& lhs, const _Basic_XorStr<_string_type, _length>& rhs)
{
	lhs << rhs.c_str();

	return lhs;
}
//---------------------------------------------------------------------------
template<typename _string_type, size_t _length, size_t _length2>
inline auto operator+(const _Basic_XorStr<_string_type, _length>& lhs, const _Basic_XorStr<_string_type, _length2>& rhs)
{
	return lhs.str() + rhs.str();
}
//---------------------------------------------------------------------------
template<typename _string_type, size_t _length>
inline auto operator+(const _string_type& lhs, const _Basic_XorStr<_string_type, _length>& rhs)
{
	return lhs + rhs.str();
}
//---------------------------------------------------------------------------
template<size_t _length>
constexpr ALWAYS_INLINE auto _xor(char const (&str)[_length])
{
	return XorStrA<_length>(str);
}
//---------------------------------------------------------------------------
template<size_t _length>
constexpr ALWAYS_INLINE auto _xor(wchar_t const (&str)[_length])
{
	return XorStrW<_length>(str);
}
//---------------------------------------------------------------------------
template<size_t _length>
constexpr ALWAYS_INLINE auto _xor(char16_t const (&str)[_length])
{
	return XorStrU16<_length>(str);
}
//---------------------------------------------------------------------------
template<size_t _length>
constexpr ALWAYS_INLINE auto _xor(char32_t const (&str)[_length])
{
	return XorStrU32<_length>(str);
}
//---------------------------------------------------------------------------

#define kernel32 (_xor("kernel32.dll").c_str())//_xor("kernel32.dll").c_str();
#define CloseHandle_func (_xor("CloseHandle").c_str()) //_xor("CloseHandle").c_str();
#define d3d9 (_xor("d3d9.dll").c_str())/*d3d9.dll*/
#define ntdll (_xor("ntdll.dll").c_str())/*ntdll.dll*/
#define CShellDll (_xor("CShell.dll").c_str())/*CShell.dll*/
//#define ClientFxFxd (_xor("ClientFx.fxd").c_str())/*ClientFx.fxd*/
#define LdrGetDllHandle_func (_xor("LdrGetDllHandle").c_str())/*LdrGetDllHandle*/
#define LdrGetDllHandleEx_func (_xor("LdrGetDllHandleEx").c_str())/*LdrGetDllHandleEx*/
#define LdrLoadDll_func (_xor("LdrLoadDll").c_str())/*LdrLoadDll*/
#define NtQueryInformationProcess_func (_xor("NtQueryInformationProcess").c_str())/*NtQueryInformationProcess*/
#define NtQuerySystemInformation_func (_xor("NtQuerySystemInformation").c_str())/*NtQuerySystemInformation*/
#define CreateDirectoryA_func (_xor("CreateDirectoryA").c_str())/*CreateDirectoryA*/
#define CreateRemoteThread_func (_xor("CreateRemoteThread").c_str())/*CreateRemoteThread*/
#define ExitProcess_func (_xor("ExitProcess").c_str())/*ExitProcess*/
#define GetCurrentProcess_func (_xor("GetCurrentProcess").c_str())/*GetCurrentProcess*/
#define GetModuleFileNameA_func (_xor("GetModuleFileNameA").c_str())/*GetModuleFileNameA*/
#define GetModuleHandleA_func (_xor("GetModuleHandleA").c_str())/*GetModuleHandleA*/
#define OpenProcess_func (_xor("OpenProcess").c_str())/*OpenProcess*/
#define VirtualProtect_func (_xor("VirtualProtect").c_str())/*VirtualProtect*/
#define dbghelp (_xor("Dbghelp.dll").c_str())/*Dbghelp.dll*/
#define SymFromAddr_func (_xor("SymFromAddr").c_str())/*SymFromAddr*/
#define user32 (_xor("user32.dll").c_str())/*user32.dll*/
#define MessageBox_func (_xor("MessageBoxA").c_str())/*MessageBoxA*/
#define TestSigningErrorMsg (_xor("Test Signing is enabled! Disable it if you want to play!").c_str())/*Test Signing is enabled! Disable it if you want to play!*/
#define unknownFunc (_xor("???").c_str())/*???*/
#define AC_STRING_NOSPECULAR (_xor("nospecular.txt").c_str())/*nospecular.txt*/