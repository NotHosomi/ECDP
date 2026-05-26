#include "Commands.h"
#include <iostream>
#include <fstream>
#include "Ingester.h"
#include "StrUtils.h"
#include "Options.h"


Commands::Commands(Core* pCore) :
	m_pCore(pCore)
{
	m_mCommands.emplace("dev", std::bind_front(&Commands::SingleDevice, this));
	m_mCommands.emplace("multi", std::bind_front(&Commands::MultiDevice, this));
	m_mCommands.emplace("getopt", std::bind_front(&Commands::GetOpt, this));
	m_mCommands.emplace("setopt", std::bind_front(&Commands::SetOpt, this));
	m_mCommands.emplace("listopts", std::bind_front(&Commands::ListOpts, this));
	m_mCommands.emplace("saveopts", std::bind_front(&Commands::SaveOpts, this));
	m_mCommands.emplace("loadopts", std::bind_front(&Commands::LoadOpts, this));
	m_mCommands.emplace("exec", std::bind_front(&Commands::Exec, this));
	m_mCommands.emplace("compare", std::bind_front(&Commands::CompareDevices, this));
	m_mCommands.emplace("average", std::bind_front(&Commands::AverageDevices, this));
	m_mCommands.emplace("set", std::bind_front(&Commands::SetDataDirectory, this));
	m_mCommands.emplace("help", std::bind_front(&Commands::Help, this));

}

E_CmdErr Commands::TryCommand(std::string cmd, std::string args)
{
	if (cmd == "") { return E_CmdErr::BadCmd; }

	if (!m_mCommands.contains(cmd))
	{
		if (args == "")
		{
			args = cmd;
			cmd = "dev";
		}
		else
		{
			std::cout << "Unknown command. Try \"Help\"" << std::endl;
			return E_CmdErr::BadCmd;
		}

	}
	E_CmdErr ec = m_mCommands.at(cmd)(args);
	if (ec != E_CmdErr::None)
	{
		std::cout << "Command failed" << std::endl;
	}
	return ec;
}

E_CmdErr Commands::SingleDevice(const std::string& args)
{
	std::pair<std::string, std::string> split = SU::DelimitOnce(args, " ");
	std::string mode;
	std::string deviceId;
	if (split.second == "")
	{
		deviceId = split.first;
		mode = "all";
	}
	else
	{
		deviceId = split.second;
		mode = split.first;
	}
	while (deviceId == "")
	{
		std::cout << "Input device ID: ";
		std::cin >> deviceId;
		std::cin.clear();
	}

	return m_pCore->Run(deviceId, ParseMode(mode));
}

E_CmdErr Commands::MultiDevice(const std::string& vArgs)
{
	std::vector<std::string> deviceIds = SU::Delimit(vArgs, " ");
	Core::E_DataTypes mode = ParseMode(deviceIds[0]);
	if (mode != Core::E_DataTypes::kNone)
	{
		deviceIds.erase(deviceIds.begin());
	}
	else
	{
		mode = Core::E_DataTypes::kAll;
	}
	E_CmdErr sumEc = E_CmdErr::None;
	for (const auto& str : deviceIds)
	{
		E_CmdErr ec = SingleDevice({ str });
		if (ec != E_CmdErr::None)
		{
			sumEc = ec;
		}
	}
	return sumEc;
}

E_CmdErr Commands::Plot(const std::string& sArgs)
{
	return E_CmdErr();
}

E_CmdErr Commands::CompareDevices(const std::string& vArgs)
{
	return E_CmdErr();
}

E_CmdErr Commands::AverageDevices(const std::string& vArgs)
{
	m_pCore->BatchAverages(SU::Delimit(vArgs, " "));
	return E_CmdErr::None;
}

E_CmdErr Commands::SetDataDirectory(const std::string& vArgs)
{
	if (!std::filesystem::exists(vArgs))
	{
		std::cout << "Could not find the specified directory. Aborting" << std::endl;
		return E_CmdErr::BadArgs;
	}

	m_pCore->UserConfig().dataDirectory = vArgs;
	std::cout << "data directory set to \"" << vArgs << "\"" << std::endl;
	return E_CmdErr::None;
}

E_CmdErr Commands::GetOpt(const std::string& vArgs)
{
	const T_Opt& opt = Options::Get().GetOpt(vArgs);
	switch (opt.eType)
	{
	case E_OptType::ErrType:
		std::cout << "Unrecognised option" << std::endl;
		return E_CmdErr::BadArgs;
	case E_OptType::Int:
		std::cout << "  " << std::get<int>(opt.val) << std::endl;
		break;
	case E_OptType::Float:
		std::cout << "  " << std::get<double>(opt.val) << std::endl;
		break;
	case E_OptType::String:
		std::cout << "  " << std::get<std::string>(opt.val) << std::endl;
		break;
	}
	return E_CmdErr::None;
}

E_CmdErr Commands::SetOpt(const std::string& vArgs)
{
	auto pair = SU::DelimitOnce(vArgs, " ");
	if (pair.first == "" || pair.second == "")
	{
		std::cout << "Invalid parameters" << std::endl;
		return E_CmdErr::BadArgs;
	}
	E_OptType eType = Options::Get().GetOpt(pair.first).eType;
	try
	{
		switch (eType)
		{
		case E_OptType::ErrType:
			std::cout << "Unrecognised option" << std::endl;
			return E_CmdErr::BadArgs;
		case E_OptType::Int:
			Options::Get().SetOpt(pair.first, std::stoi(pair.second));
			break;
		case E_OptType::Float:
			Options::Get().SetOpt(pair.first, std::stod(pair.second));
			break;
		case E_OptType::String:
			Options::Get().SetOpt(pair.first, pair.second);
			break;
		}
	}
	catch (std::invalid_argument e)
	{
		std::cout << "Invalid option value" << std::endl;
	}
	return E_CmdErr::None;
}

E_CmdErr Commands::ListOpts(const std::string& vArgs)
{
	for (const auto& [sName, opt] : Options::Get().Data())
	{
		std::cout << " - " << sName << "\t\t";
		switch (opt.eType)
		{
		case E_OptType::Int: std::cout << std::get<int>(opt.val); break;
		case E_OptType::Float: std::cout << std::get<double>(opt.val); break;
		case E_OptType::String: std::cout << std::get<std::string>(opt.val); break;
		case E_OptType::ErrType: std::cout << "<ERR>"; break;
		}
		std::cout << "\t\t" << opt.sDesc << std::endl;
	}
	return E_CmdErr::None;
}

E_CmdErr Commands::SaveOpts(const std::string& vArgs)
{
	bool succcess;
	if (vArgs == "")
	{
		succcess = Options::Get().SaveOpts();
	}
	else
	{
		succcess = Options::Get().SaveOpts(vArgs);
	}
	return succcess ? E_CmdErr::None : E_CmdErr::Other;
}

E_CmdErr Commands::LoadOpts(const std::string& vArgs)
{
	bool succcess;
	if (vArgs == "")
	{
		std::cout << "File not specified, loading default" << std::endl;
		succcess = Options::Get().LoadOpts();
	}
	else
	{
		succcess = Options::Get().LoadOpts(vArgs);
	}
	return succcess ? E_CmdErr::None : E_CmdErr::Other;
}

E_CmdErr Commands::Exec(const std::string& vArgs)
{
	std::string path = "./scripts/" + vArgs + ".txt";
	std::ifstream file(path);
	if (!file.is_open())
	{
		std::cout << "Could not open " + path << std::endl;
		return E_CmdErr::BadArgs;
	}
	for (std::string line; std::getline(file, line); )
	{
		SU::ToLower(line);
		std::pair<std::string, std::string> input = SU::DelimitOnce(line, " ");
		TryCommand(input.first, input.second);
	}
	file.close();
	return E_CmdErr();
}

E_CmdErr Commands::Help(const std::string& vArgs)
{
	std::cout << "Commands:" << std::endl;
	std::cout << " - <deviceId>\t\t\t\t\t\tParses EIS, CV, and CIL for the specified device" << std::endl;
	std::cout << " - Dev <Eis/Cv/Cil/All> <deviceId>\t\t\tParses data for the specified device" << std::endl;
	std::cout << " - Multi <Eis/Cv/Cil/All> <deviceId> <deviceId> ...\tParses data for each of the devices specified" << std::endl;
	std::cout << " - Compare <Eis/Cv/Cil/All> <deviceId> <deviceId> ...\tPlots multiple devices onto shared graph" << std::endl;
	std::cout << " - Average <Eis/Cv/Cil/All> <deviceId> <deviceId> ...\tParses EIS, CV, and CIL for each of the devices specified" << std::endl;
	std::cout << " - GetOpt <optionName>\t\tPrints the value of the specified option" << std::endl;
	std::cout << " - SetOpt <optionName> <value>\t\tSets the value of the specified option" << std::endl;
	std::cout << " - ListOpts\t\t\t\t\t\tLists all settings and their values" << std::endl;
	std::cout << " - SaveOpts <filename>\t\t\t\t\t\tSaves the current options. Filename" << std::endl;
	std::cout << " - LoadOpts\t\t\t\t\t\tSaves your current options for next time" << std::endl;
	std::cout << " - Exec <filename>\t\t\t\t\t\t\tRuns the commands contained in the specified file" << std::endl;
	std::cout << " - Help\t\t\t\t\t\t\tLists available commands" << std::endl;
	std::cout << " - Quit\t\t\t\t\t\t\tterminates the program" << std::endl;

	return E_CmdErr::None;
}

E_CmdErr Commands::Quit(const std::string& vArgs)
{
	std::cout << "Cannot quit during scripted execution" << std::endl;
	return E_CmdErr::BadArgs;
}

Core::E_DataTypes Commands::ParseMode(const std::string& sMode)
{
	// todo: split sMode and check the result for each type
	int eModes = Core::kNone;
	if (sMode == "all")
	{
		eModes |= Core::E_DataTypes(Core::kEis | Core::kCv | Core::kCil);
	}
	else if (sMode == "eis")
	{
		eModes |= Core::kEis;
	}
	else if (sMode == "cv")
	{
		eModes |= Core::kCv;
	}
	else if (sMode == "cil")
	{
		eModes |= Core::kCil;
	}
	return Core::E_DataTypes(eModes);
}
