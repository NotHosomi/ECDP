#pragma once
#include <string>
#include <vector>
#include "Symbols.h"

class CsvFile
{
public:
	CsvFile(std::string sFilePath, char cDelimiter = ',', char sRowDelimiter = '\n');

	std::vector<std::string> GetHeadings();
	std::vector<std::string> GetRow(unsigned int index);
	const std::vector<std::string>& GetCol(unsigned int index);
	const std::vector<std::string>& GetCol(std::string header);

private:
	std::vector<std::string> m_vHeadings;

	std::vector<std::vector<std::string>> m_vColumns;
};

