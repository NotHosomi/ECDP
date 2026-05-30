#pragma once

struct T_BatchData
{
	std::map<std::string, std::vector<double>> mEisAverages;
	std::map<std::string, std::vector<double>> mEisStdDev;

	double dCvAverage = -1.0;
	double dCvStdDev = -1.0;

	std::map<int, double> mCilAverages;
	std::map<int, double> mCilStdDev;

	std::map<int, double> mCilNormAverages;
	std::map<int, double> mCilNormStdDev;

};