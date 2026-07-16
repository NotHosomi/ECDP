#pragma once
#include <string>
#include <vector>
#include <type_traits>
#include "StrUtils.h"
#include "Term.h"

class PrintTable
{
public:
	PrintTable(const std::vector<std::string>& vHeaders);

	void AddRow(const std::vector<std::string>& vRow, std::string sColourCode = "");
	template <typename T>
	void AddRow(const std::string& key, const std::vector<T>& vRow, std::string sColourCode = "", bool bRound = false);

	void Print(Term::E_Colour eColour = Term::E_Colour::Reset);
private:
	struct T_Cell
	{
		std::string sText;
		Term::E_Colour eColour;
	};
	struct T_Row
	{
		std::vector<T_Cell> vCells;
		Term::E_Colour eColour;
	};

	void PrintRow(const T_Row& vRow);
	void UpdateColWidths(const T_Row& vRow);
	std::vector<T_Row> m_vRows;
	T_Row m_tHeaders;
	std::vector<int> m_vColumnWidths;
};

template<typename T>
inline void PrintTable::AddRow(const std::string& key, const std::vector<T>& vRow, std::string sColourCode, bool bRound)
{
	std::vector<std::string> row;
	row.push_back(key);
	for (auto val : vRow)
	{
		if constexpr (std::is_floating_point_v<T>)
		{
			if (bRound)
			{
				row.push_back(SU::RoundToStr(val));
			}
			else
			{
				row.push_back(std::to_string(val));
			}
		}
		else
		{
			if constexpr (std::is_arithmetic_v<T>)
			{
				row.push_back(std::to_string(val));
			}
			else
			{
				row.push_back(val);
			}
		}
	}
	AddRow(row, sColourCode);
}
