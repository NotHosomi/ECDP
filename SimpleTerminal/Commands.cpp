#include "Commands.h"
#include <iostream>
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
	std::string deviceId = args;
	while (deviceId == "")
	{
		std::cout << "Input device ID: ";
		std::cin >> deviceId;
		std::cin.clear();
	}

	std::filesystem::path devicePath = m_pCore->UserConfig().dataDirectory + "/" + deviceId;
	if (!std::filesystem::exists(devicePath))
	{
		std::cout << "Could not find " << deviceId << std::endl;
		deviceId = "";
		return E_CmdErr::BadArgs;
	}

	std::cout << "\nReading device " << deviceId << "\n-------------------" << std::endl;
	Ingester ingest(devicePath);

	m_pCore->Eis(deviceId, ingest, m_pCore->UserConfig().eis);
	m_pCore->Cv(deviceId, ingest, m_pCore->UserConfig().cv);
	m_pCore->Cil(deviceId, ingest, m_pCore->UserConfig().cil);

	std::cout << "\nFinished " << deviceId << "\n" << std::endl;
	deviceId = "";
	return E_CmdErr::None;
}

E_CmdErr Commands::MultiDevice(const std::string& vArgs)
{
	std::vector<std::string> deviceIds = SU::Delimit(vArgs, ", ");
	if (deviceIds.size() <= 1)
	{
		deviceIds = SU::Delimit(vArgs, ",");
		if (deviceIds.size() <= 1)
		{
			deviceIds = SU::Delimit(vArgs, " ");
		}
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

E_CmdErr Commands::CompareDevices(const std::string& vArgs)
{
	return E_CmdErr();
}

E_CmdErr Commands::AverageDevices(const std::string& vArgs)
{

	return E_CmdErr();
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
	return Options::Get().SaveOpts() ? E_CmdErr::None : E_CmdErr::Other;
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
	std::cout << " - SaveOpts\t\t\t\t\t\tSaves your current options for next time" << std::endl;
	std::cout << " - Help\t\t\t\t\t\t\tLists available commands" << std::endl;
	std::cout << " - Quit\t\t\t\t\t\t\tterminates the program" << std::endl;

	return E_CmdErr::None;
}
