#pragma once
#include <functional>
#include <map>
#include <string>
#include "Core.h"

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
	E_CmdErr SingleDevice(const std::string& vArgs);
	E_CmdErr MultiDevice(const std::string& vArgs);
	E_CmdErr CompareDevices(const std::string& vArgs);
	E_CmdErr AverageDevices(const std::string& vArgs);
	E_CmdErr SetDataDirectory(const std::string& vArgs);
	E_CmdErr GetOpt(const std::string& vArgs);
	E_CmdErr SetOpt(const std::string& vArgs);
	E_CmdErr ListOpt(const std::string& vArgs);
	E_CmdErr Help(const std::string& vArgs);

	std::map<std::string, std::function<E_CmdErr(const std::string&)>> m_mCommands;

	Core* m_pCore;
};

