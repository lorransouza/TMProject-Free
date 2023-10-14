// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
#include <Windows.h>
#include "UOD_Guid.h"
#include <exception>
#include <string>
#include <sstream>
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
// Auxiliar
////////////////////////////////////////////////////////////////////////////////
inline void SetData4(unsigned char* dst, const unsigned char* src)
{
	*(reinterpret_cast<uint64_t*>(dst)) = *(reinterpret_cast<const uint64_t*>(src));
}

inline bool CompareData4(const unsigned char* l, const unsigned char* r)
{
	return *(reinterpret_cast<const uint64_t*>(l)) == *(reinterpret_cast<const uint64_t*>(r));
}

////////////////////////////////////////////////////////////////////////////////
// Constructors
////////////////////////////////////////////////////////////////////////////////

UOD_GUID UOD_GUID::Empty = UOD_GUID();

// ---------------------------------------------------------------------------
UOD_GUID UOD_GUID::CreateNew()
{
	GUID guidReference;
	HRESULT hr = CoCreateGuid(&guidReference);

	if (hr != 0)
		throw std::exception(std::to_string(hr).c_str());

	UOD_GUID res = guidReference;
	return res;
}

// ---------------------------------------------------------------------------
UOD_GUID::UOD_GUID()
{
	Data1 = 0;
	Data2 = 0;
	Data3 = 0;
	*reinterpret_cast<uint64_t*>(&Data4[0]) = 0;
}

// ---------------------------------------------------------------------------
UOD_GUID::UOD_GUID(const UOD_GUID& src)
{
	Data1 = src.Data1;
	Data2 = src.Data2;
	Data3 = src.Data3;
	SetData4(Data4, src.Data4);
}

// ---------------------------------------------------------------------------
UOD_GUID::UOD_GUID(UOD_GUID&& src)
{
	Data1 = src.Data1;
	Data2 = src.Data2;
	Data3 = src.Data3;
	SetData4(Data4, src.Data4);

	// Limpa a origem.
	src.Data1 = 0;
	src.Data2 = 0;
	src.Data3 = 0;
	*(reinterpret_cast<uint64_t*>(src.Data4)) = 0;
}

// ---------------------------------------------------------------------------
UOD_GUID::UOD_GUID(const GUID& src)
{
	Data1 = src.Data1;
	Data2 = src.Data2;
	Data3 = src.Data3;
	SetData4(Data4, src.Data4);
}

////////////////////////////////////////////////////////////////////////////////
// Compares
////////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
bool UOD_GUID:: operator == (const UOD_GUID& right) const
{
	return (
		(Data1 == right.Data1) &&
		(Data2 == right.Data2) &&
		(Data3 == right.Data3) &&
		CompareData4(Data4, right.Data4)
		);
}

// ---------------------------------------------------------------------------
bool UOD_GUID:: operator != (const UOD_GUID& right) const
{
	return !(*this == right);
}

// ---------------------------------------------------------------------------
// Para pode ser usado como chave de std::map.
bool UOD_GUID:: operator < (const UOD_GUID& right) const
{
	if (Data1 != right.Data1)
		return (Data1 < right.Data1);

	if (Data2 != right.Data2)
		return (Data2 < right.Data2);

	if (Data3 != right.Data3)
		return (Data3 < right.Data3);

	const uint64_t d4 = *(reinterpret_cast<const uint64_t*>(Data4));
	const uint64_t rd4 = *(reinterpret_cast<const uint64_t*>(right.Data4));
	return (d4 < rd4);
}

////////////////////////////////////////////////////////////////////////////////
// Assigns
////////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Copy-assign
UOD_GUID& UOD_GUID:: operator = (const UOD_GUID& src)
{
	Data1 = src.Data1;
	Data2 = src.Data2;
	Data3 = src.Data3;
	SetData4(Data4, src.Data4);
	return *this;
}

// ---------------------------------------------------------------------------
// Move-assign
UOD_GUID& UOD_GUID:: operator = (UOD_GUID&& src)
{
	Data1 = src.Data1;
	Data2 = src.Data2;
	Data3 = src.Data3;
	SetData4(Data4, src.Data4);

	// Limpa a origem.
	src.Data1 = 0;
	src.Data2 = 0;
	src.Data3 = 0;
	*(reinterpret_cast<uint64_t*>(src.Data4)) = 0;
	return *this;
}

// ---------------------------------------------------------------------------
UOD_GUID:: operator GUID() const
{
	GUID ret;
	ret.Data1 = Data1;
	ret.Data2 = Data2;
	ret.Data3 = Data3;
	SetData4(ret.Data4, Data4);
	return ret;
}

// ---------------------------------------------------------------------------
UOD_GUID:: operator std::string() const
{
	char guid_cstr[39];
	snprintf(guid_cstr, sizeof(guid_cstr),
		"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
		Data1, Data2, Data3,
		Data4[0], Data4[1], Data4[2], Data4[3],
		Data4[4], Data4[5], Data4[6], Data4[7]);

	return std::string(guid_cstr);
}

// ---------------------------------------------------------------------------
bool UOD_GUID::IsEmpty()
{
	return
		(Data1 == 0) &&
		(Data2 == 0) &&
		(Data3 == 0) &&
		(*(reinterpret_cast<uint64_t*>(Data4)) == 0);
}
