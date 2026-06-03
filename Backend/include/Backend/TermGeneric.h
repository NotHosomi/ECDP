#pragma once
#include <string>

class TermGeneric
{
private:
	static TermGeneric* ms_pInstance;
public:
	static TermGeneric* Get();
	static void Set(TermGeneric* pNewInstance);

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

	virtual void Print(const std::string& sText, E_Colour sColour = E_Colour::None) = 0;
	virtual void Println(const std::string& sText, E_Colour sColour = E_Colour::None) = 0;
	virtual void Colour(E_Colour sColour) = 0;
	virtual void AddToBuffer(const std::string& sText, E_Colour sColour = E_Colour::None) = 0;
	virtual void Flush() = 0;
	virtual void Endline() = 0;
protected:
	E_Colour m_eCol = E_Colour::None;
	std::string m_sBuffer;


};

