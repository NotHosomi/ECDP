// SimpleTerminal.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "TerminalColours.h"
#include "StrUtils.h"
#include "Core.h"
#include "Commands.h"
#include "Options.h"

int main(int argc, char* argv[])
{
	//SetConsoleOutputCP(CP_UTF8);
	std::cout << TERM_RESET;

	Options::Get();
	Core core;
	Commands cmd(&core);

	std::string rawInput;
	for (;;)
	{
		std::cout << TERM_GREEN << "\n>> " << TERM_RESET << std::flush;
		std::getline(std::cin, rawInput);
		std::pair<std::string, std::string> input = SU::DelimitOnce(rawInput, " ");
		SU::ToLower(input.first);
		if (input.first == "quit") { break; }
		cmd.TryCommand(input.first, input.second);
	}
	Options::Get().SaveOpts();
	std::cout << "Terminating..." << std::endl;
}