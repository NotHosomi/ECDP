#include "CsvOut.h"
#include <fstream>

CsvOut::CsvOut(std::vector<std::string> vHeaders, char cDelimiter)
{
    m_cDelimiter = cDelimiter;
    m_nColCount = static_cast<int>(vHeaders.size());
    AddRow(vHeaders);
}

bool CsvOut::SaveTo(std::string path)
{
    std::ofstream out(path);
    if (!out.is_open())
    {
        return false;
    }
    try
    {
        out.write(m_sData.data(), m_sData.size());
        out.close();
        return true;
    }
    catch (...)
    {
        out.close();
        return false;
    }
}
