#include "Options.h"
#include <iostream>
#include "JsonLoader.h"

Options::Options()
{
	std::cout << "Loading options..." << std::flush;
	if (LoadJson("./opts.json", m_mOptions))
	{
		std::cout << "Done" << std::endl;
		return;
	}
	std::cout << "Failed. Using defaults" << std::endl;

	AddOpt({ "demoOpt", "This is an example option", E_OptType::Int, 1 });
	AddOpt({ "demoOpt2", "This is an example option", E_OptType::Float, 0.5 });
	AddOpt({ "demoOpt3", "This is an example option", E_OptType::Float, "Demo" });
}

Options& Options::Get()
{
	static Options instance;
	return instance;
}

const T_Opt& Options::GetOpt(const std::string& sName)
{
	static T_Opt nullopt = {};
	if (!m_mOptions.contains(sName))
	{
		return nullopt;
	}
	return m_mOptions.at(sName);
}

bool Options::SetOpt(const std::string& sOptName, int val)
{
	SetOpt(sOptName, val, E_OptType::Int);
}

bool Options::SetOpt(const std::string& sOptName, double val)
{
	SetOpt(sOptName, val, E_OptType::Float);
}

bool Options::SetOpt(const std::string& sOptName, const std::string& val)
{
	SetOpt(sOptName, val, E_OptType::String);
}

void Options::SaveOpts()
{
	if (SaveJson("./opts.json", m_mOptions))
	{
		std::cout << "Failed to save options" << std::endl;
		return;
	}
	std::cout << "Saved" << std::endl;
}

const std::map<std::string, T_Opt>& Options::Data()
{
	return m_mOptions;
}

void Options::AddOpt(const T_Opt& tOpt)
{
	m_mOptions.insert({ tOpt.sName, tOpt });
}
