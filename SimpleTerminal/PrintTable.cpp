#include "PrintTable.h"
#include <iomanip>
#include <iostream>
#include "TerminalColours.h"
#include "StrUtils.h"

PrintTable::PrintTable(const std::vector<std::string>& vHeaders) :
	m_vHeaders(vHeaders)
{
	m_vColumnWidths.resize(m_vHeaders.size());
	for (int i = 0; i < m_vHeaders.size(); ++i)
	{
		m_vColumnWidths[i] = static_cast<int>(m_vHeaders[i].size());
	}
}

void PrintTable::AddRow(const std::vector<std::string>& vRow, std::string sColourCode)
{
	m_vRows.push_back(vRow);
	m_vRowColours.push_back(sColourCode);
	UpdateColWidths(vRow);
	for (int i = 0; i < vRow.size(); ++i)
	{
		m_vColumnWidths[i] = std::max(m_vColumnWidths[i], static_cast<int>(vRow[i].size()));
	}
}

void PrintTable::Print(std::string sColour)
{
	std::cout << sColour;
	PrintRow(m_vHeaders);

	std::cout << "  |" << std::setfill('-');
	for (auto colWidth : m_vColumnWidths)
	{
		std::cout << std::setw(colWidth + 3) << "|";
	}
	std::cout << std::setfill(' ') << std::endl;

	for (int row = 0; row < m_vRows.size(); ++row)
	{
		std::cout << m_vRowColours[row];
		PrintRow(m_vRows[row]);
		std::cout << sColour;
	}
	std::cout << TERM_RESET;
}

void PrintTable::PrintRow(const std::vector<std::string>& vRow)
{
	std::cout << "  |";
	for (int col = 0; col < m_vHeaders.size(); ++col)
	{
		std::string cellprint = vRow[col] + " |";
		std::cout << std::setw(m_vColumnWidths[col]+3) << cellprint;
	}
	std::cout << std::endl;
}

void PrintTable::UpdateColWidths(const std::vector<std::string>& vRow)
{
	for (int i = 0; i < m_vHeaders.size(); ++i)
	{
		m_vColumnWidths[i] = std::max(m_vColumnWidths[i], static_cast<int>(vRow[i].size()));
	}
}
