#pragma once
#include <string>
#include <vector>
#include <type_traits>
#include <assert.h>
#include <functional>

class CsvOut
{
public:
	CsvOut(std::vector<std::string> vHeaders, char cDelimiter=',');
	bool SaveTo(std::string path);

	template <typename T>
	void AddRow(std::vector<T> vRow);


private:
	template <typename T>
	std::string Convert(T val);

	char m_cDelimiter;
	int m_nColCount;
	std::string m_sData;
};

template<typename T>
inline void CsvOut::AddRow(std::vector<T> vRow)
{
	assert(vRow.size() == m_nColCount);
	m_sData += Convert(vRow[0]);
	for (int i = 1; i < vRow.size(); ++i)
	{
		m_sData += m_cDelimiter;
		m_sData += Convert(vRow[i]);
	}
	m_sData += "\n";
}

template<typename T>
inline std::string CsvOut::Convert(T val)
{
	if constexpr (std::is_same<T, std::string>::value)
	{
		return val;
	}
	else if constexpr (std::is_arithmetic<T>::value)
	{
		return std::to_string(val);
	}
	else
	{
		static_assert(false && "Invalid parameter type to CsvOut::Convert()");
	}
	return "";
}
