#include "database.h"
#include "date.h"

#include <iostream>
#include <map>
#include <set>
#include <algorithm>
#include <sstream>
#include <memory>

using namespace std;

void Database::Add(const Date& date, const string& event)
{
    if (sorted_.find(date) == sorted_.end())
    {
        data_[date].push_back(event);
    }
    else
    {
        if (sorted_.at(date).find(event) == sorted_.at(date).end())
        {
            data_[date].push_back(event);
        }
    }
    sorted_[date].insert(event);
}

void Database::Print(ostream& oStream) const 
{
    for (const auto& dateEvent : data_) oStream << dateEvent;
}

string Database::Last(const Date& date) const 
{
    if (data_.empty() || date < data_.begin()->first) throw invalid_argument("");
    auto it = data_.upper_bound(date);
    it--;
    ostringstream oStream;
    for (int i = it->second.size() - 1; i >= 0; i--)
    {            
        if (!it->second[i].empty())
        {
            oStream << make_pair(it->first, it->second[i]);
            break;
        }
    }
    return oStream.str();
}

ostream& operator << (ostream& oStream, const pair<const Date, vector<string>>& p)
{
    for (const auto& value : p.second) oStream << p.first << " " << value << endl;
    return oStream;
}

ostream& operator << (ostream& oStream, const pair<const Date, const string>& p)
{
    oStream << p.first << " " << p.second;
    return oStream;
}