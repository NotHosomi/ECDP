#include "DataEIS.h"
#include <filesystem>
#include <Stats.h>
#include <string>
#include "DataManager.h"
#include "Config.h"
#include "CsvParsing.h"
#include <cassert>

#define fs std::filesystem

T_EisElectrode ReadFile(std::filesystem::path filepath)
{
    // get electrode Id
    std::string sName = filepath.string();
    std::vector<std::string> vpath = CSV::delimit(sName, "/");
    assert(vpath.size() > 1 && "Use \\ instead of /");
    sName = vpath[vpath.size() - 1];
    sName = CSV::delimitOnce(sName, "_").second;
    sName = CSV::delimitOnce(sName, ".").first;
    T_EisElectrode electrode;
    electrode.uElectrode = std::atoi(sName.c_str());

    // open the file
    std::ifstream infile = CSV::openFileR(filepath.string());
    std::string line;
    std::getline(infile, line);

    // read headers
    std::vector<std::string> fields = CSV::delimit(line, ",");
    int freqCol = -1;
    int impCol = -1;
    int phaseCol = -1;
    for (int i = 0; i < fields.size(); ++i)
    {
        if (fields[i] == "Frequency(Hz)")
        {
            freqCol = i;
            continue;
        }
        if (fields[i][0] == 'Z' && fields[i][1] == ' ')
        {
            int impCol = i;
            continue;
        }
        if (fields[i][0] == '-' && fields[i][1] == 'P' && fields[i][2] == 'h')
        {
            int phaseCol = i;
            continue;
        }
    }
    
    // read body
    while (std::getline(infile, line))
    {
        fields = CSV::delimit(line, ",");
        T_EisPoint point;
        point.fHz = static_cast<float>(std::atof(fields[freqCol].c_str()));
        point.fZ = static_cast<float>(std::atof(fields[impCol].c_str()));
        point.fPhase = static_cast<float>(std::atof(fields[phaseCol].c_str()));
        electrode.vSweep.push_back(point);
    }
    infile.close();
    return electrode;
}

T_EisGross::T_EisGross(DeviceId device, const std::string& sDirectory)
{
    for (auto& entry : fs::directory_iterator{ sDirectory })
    {
        if (!fs::is_regular_file(entry.path())) { continue; }
        T_EisElectrode electrode = ReadFile(entry.path());
        vElectrodeData.push_back(electrode);
    }
    std::vector<float> impedancesAt1kHz = DataManager::Get().GetDevice(device).GetImpedences();
    for (int i = 0; i < vElectrodeData[0].vSweep.size(); ++i)
    {
        std::vector<float> impedance;
        std::vector<float> phase;
        for (int electrode = 0; electrode < vElectrodeData.size(); ++i)
        {
            if (impedancesAt1kHz[electrode] > Config::Get().ImpedenceLimits().fImpedanceAliveMax) { continue; }
            impedance.push_back(vElectrodeData[electrode].vSweep[i].fZ);
            phase.push_back(vElectrodeData[electrode].vSweep[i].fPhase);
        }

        T_EisPoint mean;
        mean.fHz = vElectrodeData[0].vSweep[i].fHz;
        mean.fZ = Stats::Mean(impedance);
        mean.fPhase = Stats::Mean(phase);
        vMean.push_back(mean);

        T_EisPoint stddev;
        stddev.fHz = vElectrodeData[0].vSweep[i].fHz;
        stddev.fZ = Stats::Stddev(impedance);
        stddev.fPhase = Stats::Stddev(phase);
        vStddev.push_back(stddev);
    }
}
