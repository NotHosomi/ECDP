#pragma once
#include <string>
#include <string_view>
#include <iostream>
#include <fstream>
#include <vector>

namespace CSV
{
	std::ifstream openFileR(std::string sName);
	std::ofstream openFileW(std::string sName);

	// switch to string_view if performance is poor

	std::vector<std::string> delimit(std::string s, std::string delimiter);
	std::pair<std::string, std::string> delimitOnce(std::string s, std::string delimiter);
};

