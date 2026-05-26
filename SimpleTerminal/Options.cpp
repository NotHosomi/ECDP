#include "Options.h"
#include <iostream>
#include "JsonLoader.h"

Options::Options()
{
	std::cout << "Loading options..." << std::flush;
	if (LoadOpts())
	{
		std::cout << "Done" << std::endl;
	}
	else
	{
		std::cout << "Failed. Using defaults" << std::endl;
		m_mOptions.clear();
	}

	AddOpt({ "eis-plot-avrg", "Plot per-device EIS graph", E_OptType::Int, 1 });
	AddOpt({ "eis-plot-each", "Plot per-electrode EIS graph", E_OptType::Int, 0 });
	AddOpt({ "cv-plot-avrg", "Plot per-device CV loop", E_OptType::Int, 1 });
	AddOpt({ "cv-plot-each", "Plot per-electrode CV loops", E_OptType::Int, 0 });
	AddOpt({ "cil-plot-avrg", "Plot per-device CILs", E_OptType::Int, 1 });
	AddOpt({ "cil-plot-each", "Plot per-electrode CILs", E_OptType::Int, 0 });

	AddOpt({ "arch-recalc", "Ignore archive entries; recalculate & tabulate values from raw data files", E_OptType::Int, 0 });
	AddOpt({ "arch-overwrite", "Replace existing archive entries", E_OptType::Int, 0 });

	AddOpt({ "plotter-force-replot", "Always generate and render a plot, even if the output file already exists", E_OptType::Int, 0 });
	AddOpt({ "plotter-backend", "Which backend to use for plotting <internal/python/qt>", E_OptType::String, "python" });
	AddOpt({ "plotter-pyscript-eis", "Filename of the EIS plotter script in /scripts/. Does nothing if backend is not \"python\"", E_OptType::String, "plotter.py" });
	AddOpt({ "plotter-pyscript-cv", "Filename of the CV plotter script in /scripts/. Does nothing if backend is not \"python\"", E_OptType::String, "plotter.py" });
	AddOpt({ "plotter-pyscript-cil", "Filename of the CIL plotter script in /scripts/. Does nothing if backend is not \"python\"", E_OptType::String, "plotter.py" });
	AddOpt({ "plotter-hold-temp", "Retain unique temporary plotting files", E_OptType::Int, 1 });

	AddOpt({ "plotter-dpi", "Resolution of the plotter output in dot per inch", E_OptType::Int, 100 });
	AddOpt({ "plotter-fontsize-title", "Resolution of the plotter output in dot per inch", E_OptType::Int, 14 });
	AddOpt({ "plotter-fontsize-labels", "Resolution of the plotter output in dot per inch", E_OptType::Int, 12 });
	AddOpt({ "plotter-fontsize-ticks", "Resolution of the plotter output in dot per inch", E_OptType::Int, 10 });

	AddOpt({ "plotter-eis-ymax", "in ohms", E_OptType::Float, 1000000.0 });
	AddOpt({ "plotter-eis-ymin", "in ohms", E_OptType::Float, 1000.0 });
	AddOpt({ "plotter-cv-ymax", "in microamps", E_OptType::Float, 2.0 });
	AddOpt({ "plotter-cv-ymin", "in microamps", E_OptType::Float, -2.0 });

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

bool Options::SaveOpts(const std::string& sFilename)
{
	std::filesystem::create_directory("options");
	if (!Serialise("./options/" + sFilename + ".json"))
	{
		std::cout << "Failed to save options" << std::endl;
		return false;
	}
	std::cout << "Saved" << std::endl;
	return true;
}

bool Options::LoadOpts(const std::string& sFilename)
{
	if (!Deserialise("./options/" + sFilename + ".json"))
	{
		std::cout << "Failed to load options" << std::endl;
		return false;
	}
	std::cout << "Loaded options" << std::endl;
	return true;
}

const std::map<std::string, T_Opt>& Options::Data()
{
	return m_mOptions;
}

void Options::AddOpt(const T_Opt& tOpt)
{
	if (m_mOptions.contains(tOpt.sName))
	{
		return;
	}
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

bool Options::Serialise(const std::string& sFilename)
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
	return SaveJson(sFilename, vec);
}

bool Options::Deserialise(const std::string& sFilename)
{
	std::map<std::string, T_Opt> temp;
	std::vector<T_JsonOpt> vec;
	if (!LoadJson(sFilename, vec)) { return false; }
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
			std::cout << sFilename << " has corrupted value at \"name\": \"" << jopt.name << "\"..." << std::flush;
			return false;
		}
		temp.insert({ opt.sName, opt });
	}
	m_mOptions = temp;
	return true;
}
