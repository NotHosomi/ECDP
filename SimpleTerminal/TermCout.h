#pragma once
#include <Backend/TermGeneric.h>
#include <string>

class TermCout : public TermGeneric
{
public:
	void Print(const std::string& sText, E_Colour sColour = E_Colour::None) override;
	void Println(const std::string& sText, E_Colour sColour = E_Colour::None) override;
	void Colour(E_Colour sColour) override;
	void AddToBuffer(const std::string& sText, E_Colour sColour = E_Colour::None) override;
	void Flush() override;
	void Endline() override;
private:
	const std::string& GetTermCol(E_Colour eCol);
};

