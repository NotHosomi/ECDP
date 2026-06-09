#include "Term.h"
#include <cassert>

Term* Term::ms_pInstance = nullptr;

Term* Term::Get()
{
	assert(ms_pInstance != nullptr && "Terminal instance not set");
    return ms_pInstance;
}

void Term::Set(Term* pNewInstance)
{
	ms_pInstance = pNewInstance;
}
