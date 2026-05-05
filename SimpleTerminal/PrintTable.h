#pragma once
#include <string>
#include <vector>
#include <type_traits>
#include "StrUtils.h"

class PrintTable
{
public:
	PrintTable(const std::vector<std::string>& vHeaders);

	void AddRow(const std::vector<std::string>& vRow, std::string sColourCode = "");
	template <typename T>
	void AddRow(const std::string& key, const std::vector<T>& vRow, std::string sColourCode = "", bool bRound = false);

	void Print(std::string sColour = "");
private:
	void PrintRow(const std::vector<std::string>& vRow);
	void UpdateColWidths(const std::vector<std::string>& vRow);
	std::vector<std::vector<std::string>> m_vRows;
	std::vector<std::string> m_vRowColours;
	std::vector<std::string> m_vHeaders;
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
