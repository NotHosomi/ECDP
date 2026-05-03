#pragma once
#include <string>
#include <vector>
#include <utility>

namespace SU
{
	void ToLower(std::string& sStr);
	void ToUpper(std::string& sStr);
	void RemoveLeadingChar(std::string& sStr, char cChar);
	void RemoveTrailingChar(std::string& sStr, char cChar);

	std::string RoundToStr(double num);

	std::vector<std::string> Delimit(const std::string& s, const std::string& delimiter = "\r\n");
	std::pair<std::string, std::string> DelimitOnce(const std::string& s, const std::string& delimiter);

};

