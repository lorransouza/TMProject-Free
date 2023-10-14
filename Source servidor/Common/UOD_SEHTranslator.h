#ifndef UOD_SEHTranslatorH
#define UOD_SEHTranslatorH

#include <windows.h>

typedef void(__cdecl *_se_translator_function)(unsigned int, struct _EXCEPTION_POINTERS*);

// --------------------------------------------------------------------------
class TOD_SEHTranslator
{
public:
	TOD_SEHTranslator();
	~TOD_SEHTranslator();

private:
	_se_translator_function oldFunction;
};

#endif