#include "StrUtils.h"
#include <algorithm>
#include <stdexcept>

void SU::ToLower(std::string& sStr)
{
	std::transform(sStr.begin(), sStr.end(), sStr.begin(),
		[](unsigned char c) { return std::tolower(c); });
}

void SU::ToUpper(std::string& sStr)
{
	std::transform(sStr.begin(), sStr.end(), sStr.begin(),
		[](unsigned char c) { return std::toupper(c); });
}

void SU::RemoveLeadingChar(std::string& sStr, char cChar)
{
    for (int i = static_cast<int>(sStr.size() - 1); i >= 0; --i)
    {
        if (sStr[i] != cChar)
        {
            sStr = sStr.substr(i, sStr.size() - i);
            return;
        }
    }
}

void SU::RemoveTrailingChar(std::string& sStr, char cChar)
{
    for (int i = static_cast<int>(sStr.size() - 1); i >= 0; --i)
    {
        if (sStr[i] != cChar)
        {
            sStr.resize(i);
            return;
        }
    }
}

std::string SU::RoundToStr(double num)
{
	return std::to_string(static_cast<int>(num < 0 ? num - 0.5 : num + 0.5));
}

std::vector<std::string> SU::Delimit(const std::string& s, const std::string& delimiter)
{
    std::vector<std::string> tokens;
    size_t pos = s.find(delimiter);
    size_t prev = 0;
    while (pos != std::string::npos)
    {
        tokens.emplace_back(s.substr(prev, pos - prev));
        prev = pos;
        pos = s.find(delimiter, pos);
    }
    tokens.emplace_back(s.substr(prev, s.size() - prev));
    return tokens;
}

std::pair<std::string, std::string> SU::DelimitOnce(const std::string& s, const std::string& delimiter)
{
    std::pair<std::string, std::string> tokens;
    size_t pos = s.find(delimiter);
    if (pos == std::string::npos)
    {
        return { s, "" };
    }
    tokens.first = s.substr(0, pos);
    tokens.second = s.substr(pos + delimiter.length(), s.length());
    return tokens;
}
