#pragma once
#include <Backend/Term.h>
#include <string>

class TermCout : public Term
{
public:
	void Print(const std::string& sText, E_Colour sColour = E_Colour::None, size_t nWidth = 0) override;
	void Println(const std::string& sText, E_Colour sColour = E_Colour::None) override;
	void Colour(E_Colour sColour) override;
	void AddToBuffer(const std::string& sText, E_Colour sColour = E_Colour::None) override;
	void Flush() override;
	void Endline() override;
private:
	const std::string& GetTermCol(E_Colour eCol);
};

