#pragma once

#include "date.h"

#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <string>

class Database {
public:
    void Add(const Date& date, const std::string& event);

    template <typename T>
    int RemoveIf(T predicate)
    {
        int eventCount = 0;
        if (predicate({}, ""))
        {
            for (auto dateEvents : data_) eventCount += dateEvents.second.size();
            data_.clear();
            sorted_.clear();
            return eventCount;
        }
        for (auto dateEvents = data_.begin(); dateEvents != data_.end();)
        {
            if (dateEvents->second.empty())
            {
                if (sorted_.count(dateEvents->first) > 0) sorted_.erase(dateEvents->first);
                dateEvents = data_.erase(dateEvents);
                continue;
            }
            bool flag = false;
            if (predicate(dateEvents->first, ""))
            {
                eventCount += data_[dateEvents->first].size();
                if (sorted_.count(dateEvents->first) > 0) sorted_.erase(dateEvents->first);
                dateEvents = data_.erase(dateEvents);
                flag = true;
            }
            else
            {
                auto it = std::stable_partition(dateEvents->second.begin(), dateEvents->second.end(),
                    [dateEvents, predicate](std::string s) { return !predicate(dateEvents->first, s); });
                if (it != dateEvents->second.end())
                {
                    if (it == dateEvents->second.begin())
                    {
                        eventCount += dateEvents->second.size();
                        if (sorted_.count(dateEvents->first) > 0) sorted_.erase(dateEvents->first);
                        dateEvents = data_.erase(dateEvents);
                        flag = true;
                    }
                    else
                    {
                        auto it2 = it;
                        while (it2 != dateEvents->second.end())
                        {
                            if (sorted_.count(dateEvents->first) > 0) sorted_[dateEvents->first].erase(*it2);
                            it2++;
                            eventCount++;
                        }
                        dateEvents->second.erase(it, dateEvents->second.end());
                    }
                }
            }
            if (!flag) dateEvents++;
        }
        return eventCount;
    }
        
    template <typename T>
    std::vector<std::pair<Date, std::string>> FindIf(T predicate) const 
    {
        std::vector<std::pair<Date, std::string>> result;
        if (data_.empty()) return result;
        for (auto dateEvents : data_) {
            auto it = dateEvents.second.begin();
            while (it != dateEvents.second.end()) {
                it = std::find_if(it, dateEvents.second.end(), 
                    [dateEvents, predicate](const std::string& s) { return predicate(dateEvents.first, s); });
                if (it != dateEvents.second.end()) 
                {
                    result.push_back(std::make_pair(dateEvents.first, *it));
                    it++;
                }                
            }
        }        
        return result;
    }

    void Print(std::ostream& oStream) const;
    std::string Last(const Date& date) const;

private:
    std::map<Date, std::vector<std::string>> data_;
    std::map<Date, std::set<std::string>> sorted_;
};

std::ostream& operator << (std::ostream& oStream, const std::pair<const Date, std::vector<std::string>>& dateEvents);

std::ostream& operator << (std::ostream& oStream, const std::pair<const Date, const std::string>& dateEvent);