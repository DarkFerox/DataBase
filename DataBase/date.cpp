#include <iostream>
#include <sstream>
#include <string>
#include <exception>
#include <iomanip>
#include <vector>
#include <memory>

#include "date.h"

using namespace std;

Date::Date() : year_(0), month_(0), day_(0) {}

Date::Date(const int& year, const int& month, const int& day)
{    
    year_ = year;
    month_ = month;
    day_ = day;
}

int Date::GetYear() const { return year_; }
int Date::GetMonth() const { return month_; }
int Date::GetDay() const { return day_; }


bool operator < (const Date& lhs, const Date& rhs)
{
    return vector<int>{lhs.GetYear(), lhs.GetMonth(), lhs.GetDay()} <
        vector<int>{rhs.GetYear(), rhs.GetMonth(), rhs.GetDay()};
}

bool operator > (const Date& lhs, const Date& rhs)
{
    return vector<int>{lhs.GetYear(), lhs.GetMonth(), lhs.GetDay()} >
        vector<int>{rhs.GetYear(), rhs.GetMonth(), rhs.GetDay()};
}

bool operator >= (const Date& lhs, const Date& rhs)
{
    return !(vector<int>{lhs.GetYear(), lhs.GetMonth(), lhs.GetDay()} <
        vector<int>{rhs.GetYear(), rhs.GetMonth(), rhs.GetDay()});
}

bool operator <= (const Date& lhs, const Date& rhs)
{
    return !(vector<int>{lhs.GetYear(), lhs.GetMonth(), lhs.GetDay()} >
        vector<int>{rhs.GetYear(), rhs.GetMonth(), rhs.GetDay()});
}

bool operator == (const Date& lhs, const Date& rhs)
{
    return vector<int>{lhs.GetYear(), lhs.GetMonth(), lhs.GetDay()} ==
        vector<int>{rhs.GetYear(), rhs.GetMonth(), rhs.GetDay()};
}

bool operator != (const Date& lhs, const Date& rhs)
{
    return !(vector<int>{lhs.GetYear(), lhs.GetMonth(), lhs.GetDay()} ==
        vector<int>{rhs.GetYear(), rhs.GetMonth(), rhs.GetDay()});
}

ostream& operator << (ostream& oStream, const Date& date)
{
    oStream << setfill('0') << setw(4) << date.GetYear() << "-"
        << setfill('0') << setw(2) << date.GetMonth() << "-"
        << setfill('0') << setw(2) << date.GetDay();
    return oStream;
}

Date ParseDate(istream& iStream)
{    
    int year;
    iStream >> year;    
    char c;
    iStream >> c;
    int month;
    iStream >> month;
    iStream >> c;
    int day;
    iStream >> day;    
    return Date(year, month, day);
}