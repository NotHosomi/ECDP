#pragma once
#include <functional>
#include <map>
#include <string>
#include <Backend/Core.h>

enum E_Command
{
	Invalid,
	SingleDevice, // "Dev"
	CompareDevices,
	AverageDevices,
	SetDataDirectory,
	SetMode,
	Settings,
	Help,
	Quit
};

class Commands
{
public:
	Commands(Core* pCore);
	E_CmdErr TryCommand(std::string cmd, std::string args);

private:
	E_CmdErr SingleDevice(const std::string& sArgs);
	E_CmdErr MultiDevice(const std::string& sArgs);
	E_CmdErr Plot(const std::string& sArgs);
	E_CmdErr CompareDevices(const std::string& sArgs);
	E_CmdErr AverageDevices(const std::string& sArgs);
	E_CmdErr GetOpt(const std::string& sArgs);
	E_CmdErr SetOpt(const std::string& sArgs);
	E_CmdErr ListOpts(const std::string& sArgs);
	E_CmdErr SaveOpts(const std::string& sArgs);
	E_CmdErr LoadOpts(const std::string& sArgs);
	E_CmdErr Exec(const std::string& sArgs);
	E_CmdErr Help(const std::string& sArgs);
	E_CmdErr Quit(const std::string& sArgs);

	Core::E_DataTypes ParseMode(const std::string& sMode, bool required=false);

	std::map<std::string, std::function<E_CmdErr(const std::string&)>> m_mCommands;

	Core* m_pCore;
};

