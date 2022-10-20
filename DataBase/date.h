#pragma once

#include <sstream>
#include <iostream>

class Date {
public:
    Date();
    Date(const int& year, const int& month, const int& day);

    int GetYear() const;
    int GetMonth() const;
    int GetDay() const;

private:
    int year_;
    int month_;
    int day_;
};

bool operator < (const Date& lhs, const Date& rhs);
bool operator <= (const Date& lhs, const Date& rhs);
bool operator == (const Date& lhs, const Date& rhs);
bool operator > (const Date& lhs, const Date& rhs);
bool operator >= (const Date& lhs, const Date& rhs);
bool operator !=(const Date& lhs, const Date& rhs);

std::ostream& operator << (std::ostream& os, const Date& date);

Date ParseDate(std::istream& stream);