#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>

template <class T>
std::ostream& operator << (std::ostream& oStream, const std::vector<T>& container) {
    oStream << "{";
    bool first = true;
    for (const auto& value : container) {
        if (!first) {
            oStream << ", ";
        }
        first = false;
        oStream << value;
    }
    return oStream << "}";
}

template <class T>
std::ostream& operator << (std::ostream& oStream, const std::set<T>& container) {
    oStream << "{";
    bool first = true;
    for (const auto& value : container) {
        if (!first) {
            oStream << ", ";
        }
        first = false;
        oStream << value;
    }
    return oStream << "}";
}

template <class K, class V>
std::ostream& operator << (std::ostream& oStream, const std::map<K, V>& container) {
    oStream << "{";
    bool first = true;
    for (const auto& keyValue : container) {
        if (!first) {
            oStream << ", ";
        }
        first = false;
        oStream << keyValue.first << ": " << keyValue.second;
    }
    return oStream << "}";
}

template<class T, class U>
void AssertEqual(const T& t, const U& u, const std::string& hint = {}) {
    if (t != u) {
        std::ostringstream oStream;
        oStream << "Assertion failed: " << t << " != " << u;
        if (!hint.empty()) {
            oStream << " hint: " << hint;
        }
        throw std::runtime_error(oStream.str());
    }
}

void Assert(bool b, const std::string& hint);

class TestRunner {
public:
    template <class TestFunc>
    void RunTest(TestFunc func, const std::string& test_name) {
        try {
            func();
            std::cerr << test_name << " OK" << std::endl;
        }
        catch (std::exception& e) {
            ++fail_count;
            std::cerr << test_name << " fail: " << e.what() << std::endl;
        }
        catch (...) {
            ++fail_count;
            std::cerr << "Unknown exception caught" << std::endl;
        }
    }

    ~TestRunner();

private:
    int fail_count = 0;
};