#include "Commands.h"
#include <iostream>
#include "Ingester.h"
#include "StrUtils.h"


Commands::Commands(Core* pCore) :
	m_pCore(pCore)
{
	m_mCommands.emplace("dev", std::bind_front(&Commands::SingleDevice, this));
	m_mCommands.emplace("multi", std::bind_front(&Commands::MultiDevice, this));
	m_mCommands.emplace("GetOpt", std::bind_front(&Commands::GetOpt, this));
	m_mCommands.emplace("SetOpt", std::bind_front(&Commands::SetOpt, this));
	m_mCommands.emplace("List", std::bind_front(&Commands::ListOpt, this));
	m_mCommands.emplace("Compare", std::bind_front(&Commands::CompareDevices, this));
	m_mCommands.emplace("Average", std::bind_front(&Commands::AverageDevices, this));
	m_mCommands.emplace("Set", std::bind_front(&Commands::SetDataDirectory, this));
	m_mCommands.emplace("multi", std::bind_front(&Commands::MultiDevice, this));
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

E_CmdErr Commands::AverageDevices(const std::string& vArgs)
{

	return E_CmdErr();
}
