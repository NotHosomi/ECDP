#pragma once
#include <Backend/Term.h>
#include <string>
#include <iostream>

class TermCout : public Term
{
public:
	void Print(const std::string& sText, E_Colour sColour = E_Colour::None, size_t nWidth = 0) override;
	void Println(const std::string& sText, E_Colour sColour = E_Colour::None) override;
	void Colour(E_Colour sColour) override;
	void AddToBuffer(const std::string& sText, E_Colour sColour = E_Colour::None) override;
	void Flush() override;
	void Endline() override;

	void Read(std::string& rInput) override;
	void Read(int& rInput) override;
	void Read(double& rInput) override;
	void Read(float& rInput) override;
private:
	const std::string GetTermCol(E_Colour eCol);

	template<typename T>
	void ReadImpl(T& rInput);
};

template<typename T>
inline void TermCout::ReadImpl(T& rInput)
{
	while (!(std::cin >> rInput))
	{
		std::cout << "Bad type, try again: " << std::flush;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
}
