#include "TermCout.h"
#include <iostream>
#include "TerminalColours.h"

void TermCout::Print(const std::string& sText, E_Colour sColour)
{
	std::cout << GetTermCol(sColour) << sText << std::flush;
}

void TermCout::Println(const std::string& sText, E_Colour sColour)
{
	std::cout << GetTermCol(sColour) << sText << std::endl;
}

void TermCout::Colour(E_Colour sColour)
{
	m_eCol = sColour;
	std::cout << GetTermCol(sColour);
}

void TermCout::AddToBuffer(const std::string& sText, E_Colour sColour)
{
	std::cout << GetTermCol(sColour) << sText;
}

void TermCout::Flush()
{
	std::cout << std::flush;
}

void TermCout::Endline()
{
	std::cout << std::endl;
}

const std::string& TermCout::GetTermCol(E_Colour eCol)
{
	switch (eCol)
	{
	case E_Colour::None:
		return "";
	case E_Colour::Reset:
		return TERM_RESET;
	case E_Colour::Black:
		return TERM_BLACK;
	case E_Colour::Red:
		return TERM_RED;
	case E_Colour::Green:
		return TERM_GREEN;
	case E_Colour::Yellow:
		return TERM_YELLOW;
	case E_Colour::Blue:
		return TERM_BLUE;
	case E_Colour::Magenta:
		return TERM_MAGENTA;
	case E_Colour::Cyan:
		return TERM_CYAN;
	case E_Colour::White:
		return TERM_WHITE;
	case E_Colour::BlackBold:
		return TERM_BOLDBLACK;
	case E_Colour::RedBold:
		return TERM_BOLDRED;
	case E_Colour::GreenBold:
		return TERM_BOLDGREEN;
	case E_Colour::YellowBold:
		return TERM_BOLDYELLOW;
	case E_Colour::BlueBold:
		return TERM_BOLDBLUE;
	case E_Colour::MagentaBold:
		return TERM_BOLDMAGENTA;
	case E_Colour::CyanBold:
		return TERM_BOLDCYAN;
	case E_Colour::WhiteBold:
		return TERM_BOLDWHITE;
	default:
		return "";
	};
}
