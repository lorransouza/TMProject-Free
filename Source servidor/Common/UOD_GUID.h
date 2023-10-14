//---------------------------------------------------------------------------
#ifndef UOD_GuidH
#define UOD_GuidH
//---------------------------------------------------------------------------
#include <guiddef.h>
#include <string>
//---------------------------------------------------------------------------

class UOD_GUID
{
public:

	// Create
	UOD_GUID();
	UOD_GUID(const UOD_GUID&);
	UOD_GUID(UOD_GUID&&);
	UOD_GUID(const GUID& src);

	// Compare
	bool operator==(const UOD_GUID& right) const;
	bool operator!=(const UOD_GUID& right) const;
	bool operator < (const UOD_GUID &) const; // Para pode ser usado como chave de std::map.

	// Assign
	UOD_GUID& operator=(const UOD_GUID& src);
	UOD_GUID& operator=(UOD_GUID&& src);

	// Cast
	operator GUID() const;
	operator std::string() const;

	bool IsEmpty();

	static UOD_GUID Empty;
	static UOD_GUID CreateNew();

private:
	unsigned long  Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char  Data4[8];

};

#endif
