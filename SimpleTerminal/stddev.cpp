#include "stddev.h"

T_Stats combine_stats(const std::vector<T_Stats>& stats)
{
    if (stats.empty()) return T_Stats();

    T_Stats combined{};

    // Accumulate total samples and sum
    for (const auto& s : stats) {
        combined.samples += s.samples;
        combined.sum += s.sum;
    }

    if (combined.samples == 0) return { 0, 0.0, 0.0, 0.0 };

    // Combined mean
    combined.mean = combined.sum / combined.samples;

    // Combined variance using the parallel/pooled formula
    double combined_variance = 0.0;
    for (const auto& s : stats) {
        if (s.samples == 0) continue;
        double variance_i = s.stddev * s.stddev;
        double mean_delta_i = s.mean - combined.mean;
        combined_variance += s.samples * (variance_i + mean_delta_i * mean_delta_i);
    }
    combined_variance /= combined.samples;

    combined.stddev = sqrt(combined_variance);

    return combined;
}

T_Stats PooledStddev(const std::vector<T_StatGroup> vGroups)
{
    T_Stats out;
    out.samples = 0;
    double weightedMeanSum = 0.0;
    for (const auto& group : vGroups)
    {
        out.samples += group.n;
        weightedMeanSum += group.n * group.mean;
    }
    out.mean = weightedMeanSum / out.samples;

    double numerator = 0.0;
    for (const auto& g : vGroups)
    {
        double withinGroup = (g.n - 1) * g.sd * g.sd;
        double betweenGroup = g.n * std::pow(g.mean - out.mean, 2);
        numerator += withinGroup + betweenGroup;
    }
    out.stddev = std::sqrt(numerator / out.samples - vGroups.size());
    return out;
}
