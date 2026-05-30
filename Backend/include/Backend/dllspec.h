#pragma once

#ifdef ECDP_BACKEND
	#define DLL __declspec(dllexport)
#else
	#define DLL __declspec(dllimport)
#endif