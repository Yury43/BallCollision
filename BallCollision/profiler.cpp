#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <numeric>
#include <ostream>
#include <string>
#include <unordered_map>
#include <iomanip>
#include "profiler.h"
#include <map>


// std::map<std::string, std::vector<double>> durations_mcs_by_name = {};
std::unordered_map<std::string, std::vector<double>> durations_mcs_by_name = {};

ProfilingTimer::~ProfilingTimer()
{
    durations_mcs_by_name[name].push_back(
        static_cast<double>(
            std::chrono::duration_cast<
                // std::chrono::milliseconds>
                std::chrono::microseconds>
                    (clock::now() - start_tp).count()));
}

template<typename T>
static T calc_median(std::vector<T> values)
{
    std::sort(values.begin(), values.end());
    size_t size = values.size();
    return size % 2 == 0 ? (values[size / 2 - 1] + values[size / 2]) / 2:  values[size / 2];
}

void ProfilingTimer::print_stats()
{
    typedef std::pair<std::string, std::vector<double>> Record;
    
    std::vector<Record> sorted_stats;
    std::unordered_map<std::string, double> sums;
    int max_name_len = 0;
    
    for (const auto & item : durations_mcs_by_name)
    {
        sorted_stats.push_back(item);
        sums[item.first] = std::accumulate(item.second.begin(), item.second.end(), 0.);
        max_name_len = std::max(max_name_len, static_cast<int>(item.first.size()));
    }
    
            
    std::sort(sorted_stats.begin(), sorted_stats.end(), [&sums](const Record & a, const Record & b)
    {
        return sums[a.first] > sums[b.first];
    });
    
    for (const auto & item : sorted_stats)
    {
        const std::string & name = item.first;
        const std::vector<double>  & durations = item.second;
        assert(!durations.empty());

        // milliseconds
        size_t calls = durations.size();
        double tot = sums[name] / 1000;
        double avg = tot / static_cast<double>(durations.size());
        double max = *std::max_element(durations.begin(), durations.end()) / 1000;
        double min = *std::min_element(durations.begin(), durations.end()) / 1000;
        double med = calc_median(durations) / 1000;

        int w = 7;
        int precision = 0;
        
        std::cout
        << std::setw(max_name_len) << name << " "
        << std::setw(w) << std::fixed << std::setprecision(precision) << tot << " tot "
        << std::setw(w) << std::fixed << std::setprecision(precision) << calls << " calls "
        << std::setw(w) << std::fixed << std::setprecision(precision) << avg << " avg "
        << std::setw(w) << std::fixed << std::setprecision(precision) << max << " max "
        << std::setw(w) << std::fixed << std::setprecision(precision) << min << " min "
        << std::setw(w) << std::fixed << std::setprecision(precision) << med << " med "
        << std::endl;
    }
}


