#pragma once
#include <string>
#include "dllspec.h"

class Term
{
private:
	static Term* ms_pInstance;
public:
	static DLL Term* Get();
	static DLL void Set(Term* pNewInstance);
	static DLL void Delete();

	enum class E_Colour
	{
		None,
		Reset,
		Black,
		Red,
		Green,
		Yellow,
		Blue,
		Magenta,
		Cyan,
		White,
		BlackBold,
		RedBold,
		GreenBold,
		YellowBold,
		BlueBold,
		MagentaBold,
		CyanBold,
		WhiteBold
	};

	DLL virtual void Print(const std::string& sText, E_Colour sColour = E_Colour::None, size_t nWidth = 0) = 0;
	DLL virtual void Println(const std::string& sText, E_Colour sColour = E_Colour::None) = 0;
	DLL virtual void Colour(E_Colour sColour) = 0;
	DLL virtual void AddToBuffer(const std::string& sText, E_Colour sColour = E_Colour::None) = 0;
	DLL virtual void Flush() = 0;
	DLL virtual void Endline() = 0;
	DLL virtual void Read(std::string& rInput) = 0;
	DLL virtual void Read(int& rInput) = 0;
	DLL virtual void Read(double& rInput) = 0;
	DLL virtual void Read(float& rInput) = 0;
protected:
	E_Colour m_eCol = E_Colour::None;
	std::string m_sBuffer;


};

