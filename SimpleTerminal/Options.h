#pragma once
#include <string>
#include <variant>
#include <any>
#include <map>
#include <type_traits>
#include <cassert>
#include <nlohmann\json.hpp>

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
	static Options& Get();

	const T_Opt& GetOpt(const std::string& sName);

	template<typename T>
	T GetOpt(const std::string& sName);

	bool SetOpt(const std::string& sOptName, int val);
	bool SetOpt(const std::string& sOptName, double val);
	bool SetOpt(const std::string& sOptName, const std::string& val);

	bool SaveOpts();

	const std::map<std::string, T_Opt>& Data();

private:
	void AddOpt(const T_Opt& tOpt);
	template <typename T>
	bool SetOpt(const std::string& sOptName, T val, E_OptType eType);

	bool Serialise();
	bool Deserialise();

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
T Options::GetOpt(const std::string& sName)
{
	const T_Opt opt = m_mOptions.at(opt);
	if constexpr (std::is_same<T, int>::value)
	{
		//return std::any_cast<int>(opt.val);
		return std::get<int>(opt.val);
	}
	else if constexpr (std::is_floating_point_v<T>)
	{
		//return static_cast<T>(std::any_cast<double>(opt.val));
		return std::get<double>(opt.val);
	}
	else if constexpr (std::is_same<T, std::string>::value)
	{
		//return std::any_cast<std::string>(opt.val);
		return std::get<std::string>(opt.val);
	}
	else
	{
		static_assert(false, "Invalid type used with GetOpt");
	}
}