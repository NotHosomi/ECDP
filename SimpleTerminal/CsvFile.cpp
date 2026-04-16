#include "CsvFile.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <algorithm>

CsvFile::CsvFile(const std::string& sFilePath, char cDelimiter, char sRowDelimiter)
{
    std::ifstream file(sFilePath);
    if (!file.is_open())
    {
        return;
    }
    m_sFilename = std::filesystem::path(sFilePath).filename().string();
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

std::vector<std::string> CsvFile::GetHeadings() const
{
    return m_vHeadings;
}

std::vector<std::string> CsvFile::GetRow(unsigned int index) const
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

const std::vector<std::string>& CsvFile::GetCol(unsigned int index) const
{
    return m_vColumns[index];
}

const std::vector<std::string>& CsvFile::GetCol(std::string header) const
{
    int i = FindHeader(header);
    if (i >= 0)
    {
        return GetCol(i);
    }
    throw std::invalid_argument("Header \"" + header + "\" does not exist in CSV");
}

int CsvFile::FindHeader(const std::string& header) const
{
    for (int i = 0; i < m_vHeadings.size(); ++i)
    {
        if (header == m_vHeadings[i])
        {
            return i;
        }
    }
    return -1;
}

std::vector<double> CsvFile::GetColAsDouble(unsigned int index) const
{
    auto col = GetCol(index);
    std::vector<double> out(col.size());
    std::transform(col.begin(), col.end(), out.begin(),
        [](const std::string& cell) {
            return atof(cell.c_str());
        });
    return out;
}

std::vector<double> CsvFile::GetColAsDouble(std::string header) const
{
    int i = FindHeader(header);
    if (i >= 0)
    {
        return GetColAsDouble(i);
    }
    throw std::invalid_argument("Header \"" + header + "\" does not exist in CSV");
}

std::vector<int> CsvFile::GetColAsInt(unsigned int index) const
{
    auto col = GetCol(index);
    std::vector<int> out(col.size());
    std::transform(col.begin(), col.end(), out.begin(),
        [](const std::string& cell) {
            return atoi(cell.c_str());
        });
    return out;
}

std::vector<int> CsvFile::GetColAsInt(std::string header) const
{
    int i = FindHeader(header);
    if (i >= 0)
    {
        return GetColAsInt(i);
    }
    throw std::invalid_argument("Header \"" + header + "\" does not exist in CSV");
}

const std::string& CsvFile::GetFilename() const
{
    return m_sFilename;
}
