#include "CsvFile.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

CsvFile::CsvFile(std::string sFilePath, char cDelimiter, char sRowDelimiter)
{
    std::ifstream file(sFilePath);
    if (!file.is_open())
    {
        return;
    }
    std::string line;
    std::string cell;

    // Get headers
    std::getline(file, line, sRowDelimiter);
    std::istringstream linestream(line);
    while(std::getline(linestream, cell, cDelimiter))
    {
        m_vHeadings.push_back(cell);
        m_vColumns.emplace_back();
    }

    while (std::getline(file, line, sRowDelimiter))
    {
        std::istringstream linestream(line);
        for (int i = 0; i < m_vColumns.size(); ++i)
        {
            std::getline(linestream, cell, cDelimiter);
            m_vColumns[i].push_back(cell);
        }
    }
    file.close();
}

std::vector<std::string> CsvFile::GetHeadings()
{
    return m_vHeadings;
}

std::vector<std::string> CsvFile::GetRow(unsigned int index)
{
    std::vector<std::string> row;
    if (index > m_vColumns[0].size())
    {
        return row;
    }
    for (int i = 0; i < m_vColumns.size(); ++i)
    {
        row.push_back(m_vColumns[i][index]);
    }
    return row;
}

const std::vector<std::string>& CsvFile::GetCol(unsigned int index)
{
    return m_vColumns[index];
}

const std::vector<std::string>& CsvFile::GetCol(std::string header)
{
    for (int i = 0; i < m_vHeadings.size(); ++i)
    {
        if (header == m_vHeadings[i])
        {
            return GetCol(i);
        }
    }
    throw std::invalid_argument("Header \"" + header + "\" does not exist in CSV");
    
}
