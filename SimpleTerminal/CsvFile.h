#pragma once
#include <string>
#include <vector>
#include "Symbols.h"

class CsvFile
{
public:
	CsvFile(const std::string& sFilePath, char cDelimiter = ',', char sRowDelimiter = '\n');

	std::vector<std::string> GetHeadings() const;
	std::vector<std::string> GetRow(unsigned int index) const;
	const std::vector<std::string>& GetCol(unsigned int index) const;
	const std::vector<std::string>& GetCol(std::string header) const;
	int FindHeader(const std::string& header) const;

	std::vector<double> GetColAsDouble(unsigned int index) const;
	std::vector<double> GetColAsDouble(std::string header) const;
	std::vector<int> GetColAsInt(unsigned int index) const;
	std::vector<int> GetColAsInt(std::string header) const;


	const std::string& GetFilename() const;
private:
	std::string m_sFilename = "None";
	std::vector<std::string> m_vHeadings;

	std::vector<std::vector<std::string>> m_vColumns;
};

