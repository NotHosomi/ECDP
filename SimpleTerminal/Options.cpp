#include "Options.h"
#include <iostream>
#include "JsonLoader.h"

Options::Options()
{
	std::cout << "Loading options..." << std::flush;
	if (Deserialise())
	{
		std::cout << "Done" << std::endl;
		return;
	}
	std::cout << "Failed. Using defaults" << std::endl;

	AddOpt({ "demoOpt", "This is an example option", E_OptType::Int, 1 });
	AddOpt({ "demoOpt2", "This is an example option", E_OptType::Float, 0.5 });
	AddOpt({ "demoOpt3", "This is an example option", E_OptType::String, "Demo" });

	AddOpt({ "eis-plot-avrg", "Plot per-device EIS graph", E_OptType::Int, 1 });
	AddOpt({ "eis-plot-each", "Plot per-electrode EIS graph", E_OptType::Int, 0 });
	AddOpt({ "cv-plot-avrg", "Plot per-device CV loop", E_OptType::Int, 1 });
	AddOpt({ "cv-plot-each", "Plot per-electrode CV loops", E_OptType::Int, 0 });
	AddOpt({ "cil-plot-avrg", "Plot per-device CILs", E_OptType::Int, 1 });
	AddOpt({ "cil-plot-each", "Plot per-electrode CILs", E_OptType::Int, 0 });
}

Options& Options::Get()
{
	static Options instance;
	return instance;
}

const T_Opt& Options::GetOpt(const std::string& sName)
{
	static T_Opt nullopt;
	if (!m_mOptions.contains(sName))
	{
		return nullopt;
	}
	return m_mOptions.at(sName);
}

bool Options::SetOpt(const std::string& sOptName, int val)
{
	return SetOpt(sOptName, val, E_OptType::Int);
}

bool Options::SetOpt(const std::string& sOptName, double val)
{
	return SetOpt(sOptName, val, E_OptType::Float);
}

bool Options::SetOpt(const std::string& sOptName, const std::string& val)
{
	return SetOpt(sOptName, val, E_OptType::String);
}

bool Options::SaveOpts()
{
	if (!Serialise())
	{
		std::cout << "Failed to save options" << std::endl;
		return false;
	}
	std::cout << "Saved" << std::endl;
	return true;
}

const std::map<std::string, T_Opt>& Options::Data()
{
	return m_mOptions;
}

void Options::AddOpt(const T_Opt& tOpt)
{
	m_mOptions.insert({ tOpt.sName, tOpt });
}

struct T_JsonOpt
{
	std::string name = "";
	std::string desc = "";
	E_OptType type = E_OptType::ErrType;
	std::string value = "";
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(T_JsonOpt, name, desc, type, value);
};

bool Options::Serialise()
{
	std::vector<T_JsonOpt> vec;
	for (const auto& [key, opt] : m_mOptions)
	{
		T_JsonOpt jopt;
		jopt.name = opt.sName;
		jopt.desc = opt.sDesc;
		jopt.type = opt.eType;
		switch (opt.eType)
		{
		case E_OptType::ErrType: jopt.value = "";
			break;
		case E_OptType::Int: jopt.value = std::to_string(std::get<int>(opt.val));
			break;
		case E_OptType::Float: jopt.value = std::to_string(std::get<double>(opt.val));
			break;
		case E_OptType::String: jopt.value = std::get<std::string>(opt.val);
			break;
		}
		vec.emplace_back(jopt);
	}
	return SaveJson("./options.json", vec);
}

bool Options::Deserialise()
{
	std::map<std::string, T_Opt> temp;
	std::vector<T_JsonOpt> vec;
	if (!LoadJson("./options.json", vec)) { return false; }
	for (const auto& jopt : vec)
	{
		T_Opt opt;
		opt.sName = jopt.name;
		opt.sDesc = jopt.desc;
		opt.eType = jopt.type;
		try
		{
			switch (opt.eType)
			{
			case E_OptType::ErrType: opt.val = 0;
				break;
			case E_OptType::Int: opt.val = std::stoi(jopt.value);
				break;
			case E_OptType::Float: opt.val = std::stof(jopt.value);
				break;
			case E_OptType::String: opt.val = jopt.value;
				break;
			}
		}
		catch(std::invalid_argument e)
		{
			std::cout << "options.json corrupted value at \"name\": \"" << jopt.name << "\"..." << std::flush;
			return false;
		}
		temp.insert({ opt.sName, opt });
	}
	m_mOptions = temp;
	return true;
}
