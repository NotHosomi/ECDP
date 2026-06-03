#include "TermGeneric.h"


TermGeneric* TermGeneric::ms_pInstance = nullptr;

TermGeneric* TermGeneric::Get()
{
    return ms_pInstance;
}

void TermGeneric::Set(TermGeneric* pNewInstance)
{
	ms_pInstance = pNewInstance;
}
