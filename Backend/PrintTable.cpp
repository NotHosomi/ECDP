#include "PrintTable.h"
#include <iomanip>
#include <iostream>
#include <assert.h>
#include "Term.h"
#include "StrUtils.h"

PrintTable::PrintTable(const std::vector<std::string>& vHeaders)
{
	for(const auto & str : vHeaders)
	{
		m_tHeaders.vCells.push_back({ str, Term::E_Colour::None });
	}

	m_vColumnWidths.resize(m_tHeaders.vCells.size());
	for (int i = 0; i < m_tHeaders.vCells.size(); ++i)
	{
		m_vColumnWidths[i] = static_cast<int>(m_tHeaders.vCells[i].sText.size()) + 1;
	}
}

void PrintTable::AddRow(const std::vector<std::string>& vRow, Term::E_Colour eRowColour)
{
	std::vector<T_Cell> cells;
	for (const auto& str : vRow)
	{
		cells.emplace_back(str, Term::E_Colour::None);
	}
	m_vRows.emplace_back(cells, eRowColour);
	UpdateColWidths(m_vRows.back());
	for (int i = 0; i < vRow.size(); ++i)
	{
		m_vColumnWidths[i] = std::max(m_vColumnWidths[i], static_cast<int>(vRow[i].size()));
	}
}

void PrintTable::AddRow(const std::vector<std::string>& vRow, const std::vector<Term::E_Colour> eCellColours, Term::E_Colour eRowColour)
{
	assert(vRow.size() == eCellColours.size());

	// todo
}

void PrintTable::Print(Term::E_Colour eColour)
{
	Term::Get()->Colour(eColour);
	PrintRow(m_tHeaders);

	// todo: add fill to Term class
	std::string divider = "  |";
	for (const auto& width : m_vColumnWidths)
	{
		for (int i = 0; i < width; ++i)
		{
			divider += "-";
		}
		divider += "|";
	}
	Term::Get()->Print(divider);

	for (int row = 0; row < m_vRows.size(); ++row)
	{
		PrintRow(m_vRows[row]);
	}
	Term::Get()->Colour(Term::E_Colour::Reset);
}

void PrintTable::PrintRow(const T_Row& tRow)
{
	Term::Get()->Print("  |", tRow.eColour);
	for (int col = 0; col < m_tHeaders.vCells.size(); ++col)
	{
		const auto& cell = tRow.vCells[col];
		Term::Get()->Colour(cell.eColour);
		Term::Get()->Print(cell.sText, cell.eColour, m_vColumnWidths[col]);
		Term::Get()->Print(" |", tRow.eColour);
	}
	Term::Get()->Println("");
}

void PrintTable::UpdateColWidths(const T_Row& tRow)
{
	for (int i = 0; i < m_tHeaders.vCells.size(); ++i)
	{
		m_vColumnWidths[i] = std::max(m_vColumnWidths[i], static_cast<int>(tRow.vCells[i].sText.length()) + 1);
	}
}
