#include <vector>
#include <map>

struct T_CilData
{
	std::vector<int> vPulseWidths;
	std::map<int, std::vector<float>> mCilVals;
	std::vector<float> vAvrgCils;
};