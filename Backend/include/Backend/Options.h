#pragma once
#include <string>
#include <variant>
#include <any>
#include <map>
#include <type_traits>
#include <cassert>
#include <nlohmann\json.hpp>
#include "dllspec.h"

enum class E_OptType
{
	Int,
	Float,
	String,
	ErrType
};

struct T_Opt
{
	std::string sName = "";
	std::string sDesc = "";
	E_OptType eType = E_OptType::ErrType;
	std::variant<int, double, std::string> val = 0;
};

class Options
{
private:
	Options();
public:
	DLL static Options& Get();

	DLL const T_Opt& GetOpt(const std::string& sName);

	template<typename T>
	DLL const T& GetOpt(const std::string& sName);

	DLL bool SetOpt(const std::string& sOptName, int val);
	DLL bool SetOpt(const std::string& sOptName, double val);
	DLL bool SetOpt(const std::string& sOptName, const std::string& val);

	DLL bool SaveOpts(const std::string& filename = "default");
	DLL bool LoadOpts(const std::string& filename = "default");

	DLL const std::map<std::string, T_Opt>& Data();

private:
	void AddOpt(const T_Opt& tOpt);
	template <typename T>
	bool SetOpt(const std::string& sOptName, T val, E_OptType eType);

	bool Serialise(const std::string& filename);
	bool Deserialise(const std::string& filename);

	std::map<std::string, T_Opt> m_mOptions;
};

template<typename T>
bool Options::SetOpt(const std::string& sOptName, T val, E_OptType eType)
{
	if (m_mOptions.contains(sOptName) && m_mOptions.at(sOptName).eType == eType)
	{
		m_mOptions.at(sOptName).val = val;
		return true;
	}
	return false;
}

template<typename T>
const T& Options::GetOpt(const std::string& sName)
{
	const T_Opt opt = m_mOptions.at(sName);
	if constexpr (std::is_same<T, int>::value)
	{
		return std::get<int>(opt.val);
	}
	else if constexpr (std::is_same<T, double>::value)
	{
		return std::get<double>(opt.val);
	}
	else if constexpr (std::is_same<T, float>::value)
	{
		return static_cast<float>(std::get<double>(opt.val));
	}
	else if constexpr (std::is_same<T, std::string>::value)
	{
		return std::get<std::string>(opt.val);
	}
	else if constexpr (std::is_same<T, bool>::value)
	{
		return static_cast<bool>(std::get<int>(opt.val));
	}
	else
	{
		static_assert(false, "Invalid type used with GetOpt");
	}
}