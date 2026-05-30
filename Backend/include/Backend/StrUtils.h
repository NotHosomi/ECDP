#pragma once
#include <string>
#include <vector>
#include <utility>
#include "dllspec.h"

namespace SU
{
	DLL void ToLower(std::string& sStr);
	DLL void ToUpper(std::string& sStr);
	DLL void RemoveLeadingChar(std::string& sStr, char cChar);
	DLL void RemoveTrailingChar(std::string& sStr, char cChar);

	DLL std::string RoundToStr(double num);

	DLL std::vector<std::string> Delimit(const std::string& s, const std::string& delimiter = "\r\n");
	DLL std::pair<std::string, std::string> DelimitOnce(const std::string& s, const std::string& delimiter);
	DLL std::vector<std::string> DelimitWithQuotes(const std::string& s, const std::string& delimiter);

};

