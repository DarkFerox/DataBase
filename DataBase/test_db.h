#pragma once

#include "test_runner.h"
#include "database.h"
#include "condition_parser.h"
#include <sstream>
#include "node.h"
#include "test_runner.h"

#include <sstream>

std::string Entry(const Date date, const std::string& s) 
{
    std::ostringstream stream;
    stream << std::make_pair(date, s);
    return stream.str();
}

class AlwaysFalseNode : public Node
{
    bool Evaluate(const Date&, const std::string& event) const override {
        return false;
    }
};

std::string ParseEvent(std::istream& iStream);

int DoRemove(Database& db, const std::string& str)
{
    std::istringstream stream(str);
    auto condition = ParseCondition(stream);
    auto predicate = [condition](const Date& date, const std::string& event) {
        return condition->Evaluate(date, event);
    };
    return db.RemoveIf(predicate);
}

std::string DoFind(Database& db, const std::string& str)
{
    std::istringstream iStream(str);
    auto condition = ParseCondition(iStream);
    auto predicate = [condition](const Date& date, const std::string& event) {
        return condition->Evaluate(date, event);
    };
    const auto entries = db.FindIf(predicate);
    std::ostringstream oStream;
    for (const auto& entry : entries) {
        oStream << entry << std::endl;
    }
    oStream << entries.size();
    return oStream.str();
}

void TestDbAdd() {
    {
        Database db;
        db.Add({ 2017, 1, 1 }, "new year");
        db.Add({ 2017, 1, 7 }, "xmas");
        std::ostringstream out;
        db.Print(out);
        AssertEqual("2017-01-01 new year\n2017-01-07 xmas\n", out.str(), "straight ordering");
    }
    {
        Database db;
        db.Add({ 2017, 1, 1 }, "new year");
        db.Add({ 2017, 1, 1 }, "holiday");
        std::ostringstream out;
        db.Print(out);
        AssertEqual("2017-01-01 new year\n2017-01-01 holiday\n", out.str(), "several in one day");
    }
    {
        Database db;
        db.Add({ 2017, 1, 7 }, "xmas");
        db.Add({ 2017, 1, 1 }, "new year");
        std::ostringstream out;
        db.Print(out);
        AssertEqual("2017-01-01 new year\n2017-01-07 xmas\n", out.str(), "reverse ordering");
    }
    {
        Database db;
        db.Add({ 2017, 1, 1 }, "new year");
        db.Add({ 2017, 1, 1 }, "new year");
        db.Add({ 2017, 1, 1 }, "xmas");
        db.Add({ 2017, 1, 1 }, "new year");
        std::ostringstream out;
        db.Print(out);
        AssertEqual("2017-01-01 new year\n2017-01-01 xmas\n", out.str(), "uniq adding");
    }
}

void TestDbFind() {
    {
        Database db;
        db.Add({ 2017, 1, 1 }, "new year");
        db.Add({ 2017, 1, 7 }, "xmas");
        AssertEqual("2017-01-01 new year\n1", DoFind(db, "date == 2017-01-01"), "simple find by date");
        AssertEqual("2017-01-01 new year\n2017-01-07 xmas\n2", DoFind(db, "date < 2017-01-31"), "multiple find by date");
        AssertEqual("2017-01-01 new year\n1", DoFind(db, R"(event != "xmas")"), "multiple find by holiday");
    }
    {
        Database db;
        db.Add({ 2017, 1, 1 }, "new year");
        db.Add({ 2017, 1, 1 }, "new year2");
        db.Add({ 2017, 1, 7 }, "xmas");
        AssertEqual("2017-01-01 new year\n2017-01-07 xmas\n2", DoFind(db, R"(date == 2017-01-07 OR event == "new year")"),
            "complex find or");
        AssertEqual("2017-01-01 new year\n1", DoFind(db, R"(date == 2017-01-01 AND event == "new year")"),
            "complex find and");
        AssertEqual("0", DoFind(db, R"(date == 2017-01-09 AND event == "new year")"),
            "complex find and, nothing");
    }
}

void TestDbLast() {
    Database db;
    db.Add({ 2017, 1, 1 }, "new year");
    db.Add({ 2017, 1, 7 }, "xmas");
    {
        try {
            db.Last({ 2016, 12, 31 });
            Assert(false, "last, found no entries");
        }
        catch (...) {
            Assert(true, "last, found no entries");

        }
    }
    {
        std::ostringstream oStream;
        oStream << db.Last({ 2017, 1, 2 });
        AssertEqual("2017-01-01 new year", oStream.str(), "greater than date");
    }
    {
        std::ostringstream oStream;
        oStream << db.Last({ 2017, 1, 1 });
        AssertEqual("2017-01-01 new year", oStream.str(), "eq to date");
    }
    {
        std::ostringstream oStream;
        oStream << db.Last({ 2017, 1, 10 });
        AssertEqual("2017-01-07 xmas", oStream.str(), "greater than max date");
    }
}

void TestDbRemoveIf() {
    {
        Database db;
        db.Add({ 2017, 1, 1 }, "new year");
        db.Add({ 2017, 1, 7 }, "xmas");
        AssertEqual(0, DoRemove(db, R"(event == "something")"), "Remove nothing");
        AssertEqual(1, DoRemove(db, R"(date == "2017-01-01")"), "Remove by date");
        std::ostringstream out;
        db.Print(out);
        AssertEqual("2017-01-07 xmas\n", out.str(), "Remove by date, left");
    }
    {
        Database db;
        db.Add({ 2017, 1, 1 }, "new year");
        db.Add({ 2017, 1, 7 }, "xmas");
        AssertEqual(1, DoRemove(db, R"(event == "xmas")"), "Remove by event");
        std::ostringstream out;
        db.Print(out);
        AssertEqual("2017-01-01 new year\n", out.str(), "Remove by event, left");
    }
    {
        Database db;
        db.Add({ 2017, 1, 1 }, "new year");
        db.Add({ 2017, 1, 7 }, "xmas");
        db.Add({ 2017, 1, 7 }, "new year");
        AssertEqual(2, DoRemove(db, R"(event == "new year")"), "Multiple remove by event");
        std::ostringstream out;
        db.Print(out);
        AssertEqual("2017-01-07 xmas\n", out.str(), "Multiple remove by event, left");
    }
}

void TestInsertionOrder()
{
    {
        Database db;
        db.Add({ 2017, 1, 1 }, "new year");
        db.Add({ 2017, 1, 7 }, "xmas");
        db.Add({ 2017, 1, 7 }, "party");
        db.Add({ 2017, 1, 7 }, "pie");
        std::ostringstream out;
        db.Print(out);
        AssertEqual("2017-01-01 new year\n2017-01-07 xmas\n2017-01-07 party\n2017-01-07 pie\n", out.str(), "Remove by date, left");
    }

}

void TestsMyCustom()
{
    {
        Database db;
        db.Add({ 2019, 12, 18 }, "Kolya");
        db.Add({ 2020, 1, 15 }, "Katya");
        db.Add({ 2020, 2, 22 }, "Riding");
        db.Add({ 2019, 12, 9 }, "Go home");
        db.Add({ 2019, 12, 9 }, "Read");
        std::ostringstream out;
        db.Print(out);
        AssertEqual("2019-12-09 Go home\n2019-12-09 Read\n2019-12-18 Kolya\n2020-01-15 Katya\n2020-02-22 Riding\n", out.str(), "Order of insertion");
    }

    {
        Database db;
        db.Add({ 2019, 12, 1 }, "a");
        db.Add({ 2019, 12, 1 }, "b");
        db.Add({ 2019, 12, 2 }, "c");
        db.Add({ 2019, 12, 2 }, "d");
        db.Add({ 2019, 12, 3 }, "e");
        std::ostringstream out;
        db.Print(out);
        AssertEqual("2019-12-01 a\n2019-12-01 b\n2019-12-02 c\n2019-12-02 d\n2019-12-03 e\n", out.str(), "Order of insertion");
    }

    {
        Database db;
        std::istringstream stream("    a");
        const auto event = ParseEvent(stream);
        db.Add({ 2019, 12, 1 }, event);
        std::ostringstream out;
        db.Print(out);
        AssertEqual("2019-12-01 a\n", out.str(), "My test 20");
    }

    {
        Database db;
        db.Add({ 2019, 12, 1 }, "a");
        db.Add({ 2019, 12, 1 }, "b");
        db.Add({ 2019, 12, 2 }, "c c");
        db.Add({ 2019, 12, 2 }, "d");
        db.Add({ 2019, 12, 3 }, "e");
        db.Add({ 2019, 12, 3 }, "f");
        std::ostringstream out;
        db.Print(out);
        AssertEqual("2019-12-01 a\n2019-12-01 b\n2019-12-02 c c\n2019-12-02 d\n2019-12-03 e\n2019-12-03 f\n", out.str(), "My test 2");
        AssertEqual(1, DoRemove(db, R"(event == "c" OR event == "d")"), "My test 3");
        AssertEqual(1, DoRemove(db, R"(event == "e" AND event != "a")"), "My test 4");
        db.Add({ 2019, 11, 30 }, "a");
        AssertEqual("2019-12-03 f\n1", DoFind(db, R"(date >= 2019-12-3)"), "My test 5");
        AssertEqual(Entry({ 2019, 12, 3 }, "f"), db.Last({ 2019, 12, 4 }), " My test 6");

        try
        {
            db.Last({ 2019, 11, 3 });
        }
        catch (std::invalid_argument&)
        {
            std::cerr << "Test no entries OK" << std::endl;
        }

        AssertEqual(Entry({ 2019, 12, 2 }, "c c"), db.Last({ 2019, 12, 2 }), " My test 7");

        AssertEqual(Entry({ 2019, 12, 3 }, "f"), db.Last({ 2019, 12, 4 }), " My test 8");

        db.Add({ 2019, 12, 3 }, "m");
        AssertEqual(Entry({ 2019, 12, 3 }, "m"), db.Last({ 2019, 12, 3 }), " My test 9");

        AssertEqual(1, DoRemove(db, R"(event == "e" AND event != "a" OR event == "m" AND date == 2019-12-3)"), "My test 10");

        std::ostringstream out2;
        db.Print(out2);
        AssertEqual("2019-11-30 a\n2019-12-01 a\n2019-12-01 b\n2019-12-02 c c\n2019-12-03 f\n", out2.str(), "My test 11");
    }

    {
        Database db;
        db.Add({ 2019, 12, 1 }, "a");
        db.Add({ 2019, 12, 1 }, "b");
        db.Add({ 2019, 12, 1 }, "a");
        db.Add({ 2019, 12, 2 }, "c");
        db.Add({ 2019, 12, 2 }, "a");
        db.Add({ 2019, 12, 2 }, "a");

        AssertEqual(2, DoRemove(db, R"(event == "a")"), "My test 12");
    }

    {
        Database db;
        db.Add({ 2019, 12, 1 }, "a");
        db.Add({ 2019, 12, 1 }, "aa");
        db.Add({ 2019, 12, 1 }, "aaa");

        AssertEqual(2, DoRemove(db, R"(event >= "aa")"), "My test 13");

        std::ostringstream out;
        db.Print(out);
        AssertEqual("2019-12-01 a\n", out.str(), "My test 14");
    }

    {
        Database db;
        db.Add({ 2019, 12, 1 }, "a");
        db.Add({ 2019, 12, 1 }, "aa");
        db.Add({ 2019, 12, 1 }, "aaa");
        db.Add({ 2019, 12, 2 }, "b");
        db.Add({ 2019, 12, 2 }, "a");

        AssertEqual(2, DoRemove(db, R"(event > "aa")"), "My test 15");

        std::ostringstream out;
        db.Print(out);
        AssertEqual("2019-12-01 a\n2019-12-01 aa\n2019-12-02 a\n", out.str(), "My test 16");
    }

    {
        Database db;
        db.Add({ 2019, 12, 1 }, "a");
        db.Add({ 2019, 12, 1 }, "aa");
        db.Add({ 2019, 12, 1 }, "aaa");
        db.Add({ 2019, 12, 2 }, "b");
        db.Add({ 2019, 12, 2 }, "a");

        AssertEqual(2, DoRemove(db, R"(event < "aa")"), "My test 17");

        std::ostringstream out;
        db.Print(out);
        AssertEqual("2019-12-01 aa\n2019-12-01 aaa\n2019-12-02 b\n", out.str(), "My test 18");
    }

    {
        Database db;
        db.Add({ 2019, 12, 1 }, "a");
        db.Add({ 2019, 12, 1 }, "b");

        AssertEqual(1, DoRemove(db, R"(event != "b")"), "My test 19");

        db.Add({ 2019, 12, 1 }, "c");

        AssertEqual(Entry({ 2019, 12, 1 }, "c"), db.Last({ 2019, 12, 1 }), " My test 20");

        db.Add({ 2019, 12, 1 }, "b");
        AssertEqual(Entry({ 2019, 12, 1 }, "c"), db.Last({ 2019, 12, 1 }), " My test 21");

        std::ostringstream out;
        db.Print(out);
        AssertEqual("2019-12-01 b\n2019-12-01 c\n", out.str(), "My test 22");
    }


}

void TestDatabase() {
    std::istringstream empty_is("");
    auto empty_condition = ParseCondition(empty_is);
    auto empty_predicate = [empty_condition](const Date& date, const std::string& event)
    {
        return empty_condition->Evaluate(date, event);
    };

    // Add 2 - Del 1 - Add deleted again
    {
        Database db;
        Date d(2019, 1, 1);
        db.Add(d, "e1");
        db.Add(d, "e2");
        std::istringstream iStream(R"(event == "e1")");
        auto condition = ParseCondition(iStream);
        auto predicate = [condition](const Date& date, const std::string& event)
        {
            return condition->Evaluate(date, event);
        };
        AssertEqual(db.RemoveIf(predicate), 1, "Db Add2-Del-Add 1");
        db.Add(d, "e1");
        AssertEqual(db.FindIf(empty_predicate).size(), 2, "Db Add2-Del-Add 2");
    }

    // Add
    {
        Database db;
        Date d(2019, 1, 1);
        db.Add(d, "e1");
        db.Add(d, "e1");
        std::istringstream iStream("date == 2019-01-01");
        auto condition = ParseCondition(iStream);
        auto predicate = [condition](const Date& date, const std::string& event) {
            return condition->Evaluate(date, event);
        };
        AssertEqual(db.FindIf(predicate).size(), 1, "Db Add Duplicates 1");
    }

    // Last
    {
        Database db;
        Date d(2019, 1, 1);
        Date d1(2019, 1, 2);
        Date d2(2018, 12, 22);
        db.Add(d1, "e1");
        db.Add(d2, "e2");
        AssertEqual(db.Last(d), Entry({ 2018, 12, 22 }, "e2"), "Db Last 1");
        Date d3(2018, 12, 24);
        db.Add(d3, "e3");
        AssertEqual(db.Last(d), Entry({ 2018, 12, 24 }, "e3"), "Db Last 2");

        // Get last event for date before first event 
        try {
            Date d4(2017, 2, 2);
            db.Last(d4);
            Assert(false, "Db Last 3");
        }
        catch (std::invalid_argument e) {
            // Pass
        }

        // Delete event and get last
        std::istringstream iStream("date == 2018-12-24");
        auto condition = ParseCondition(iStream);
        auto predicate = [condition](const Date& date, const std::string& event) {
            return condition->Evaluate(date, event);
        };
        db.RemoveIf(predicate);
        AssertEqual(db.Last(d), Entry({ 2018, 12, 22 }, "e2"), "Db Last 4");

        AssertEqual(db.Last(d1), Entry({ 2019, 1, 2 }, "e1"), "Db Last 5");
        db.Add(d2, "e4");
        AssertEqual(db.Last(d2), Entry({ 2018, 12, 22 }, "e4"), "Db Last 6");
    }

    // Del
    {
        Database db;
        db.Add({ 2019, 1, 1 }, "e1");
        db.Add({ 2019, 1, 1 }, "e2");
        db.Add({ 2018, 1, 7 }, "e3");
        db.Add({ 2018, 1, 7 }, "e4");
        std::istringstream iStream("date == 2018-01-07");
        auto condition = ParseCondition(iStream);
        auto predicate = [condition](const Date& date, const std::string& event) {
            return condition->Evaluate(date, event);
        };
        AssertEqual(db.RemoveIf(predicate), 2, "Db Del 1");
    }
    {
        Database db;
        db.Add({ 2019, 1, 1 }, "e1");
        db.Add({ 2019, 1, 1 }, "e2");
        db.Add({ 2018, 1, 7 }, "e3");
        db.Add({ 2018, 1, 7 }, "e4");
        std::istringstream iStream("date >= 2018-01-07 AND date <= 2020-01-01");
        auto condition = ParseCondition(iStream);
        auto predicate = [condition](const Date& date, const std::string& event) {
            return condition->Evaluate(date, event);
        };
        AssertEqual(db.RemoveIf(predicate), 4, "Db Del 2");
    }
    {
        Database db;
        db.Add({ 2019, 1, 1 }, "e1");
        db.Add({ 2019, 1, 1 }, "e2");
        db.Add({ 2018, 1, 7 }, "e3");
        db.Add({ 2018, 1, 7 }, "e4");
        AssertEqual(db.RemoveIf(empty_predicate), 4, "Db Del 3");
    }
    {
        Database db;
        db.Add({ 2019, 1, 1 }, "e1");
        db.Add({ 2019, 1, 1 }, "e2");
        db.Add({ 2018, 1, 7 }, "e3");
        db.Add({ 2018, 1, 7 }, "e4");
        std::istringstream iStream(R"(event == "e1")");
        auto condition = ParseCondition(iStream);
        auto predicate = [condition](const Date& date, const std::string& event) {
            return condition->Evaluate(date, event);
        };
        AssertEqual(db.RemoveIf(predicate), 1, "Db Del 4");
    }

    {
        Database db;
        db.Add({ 2019, 1, 1 }, "e1");
        db.Add({ 2019, 1, 1 }, "e2");
        db.Add({ 2018, 1, 7 }, "e3");
        db.Add({ 2018, 1, 7 }, "e4");
        std::istringstream iStream(R"(event == "e1" OR date == 2019-01-01)");
        auto condition = ParseCondition(iStream);
        auto predicate = [condition](const Date& date, const std::string& event) {
            return condition->Evaluate(date, event);
        };
        AssertEqual(db.RemoveIf(predicate), 2, "Db Del 5");
    }

    // Find
    {
        Database db;
        db.Add({ 2019, 1, 1 }, "e1");
        db.Add({ 2019, 1, 1 }, "e2");
        db.Add({ 2018, 1, 7 }, "e3");
        db.Add({ 2018, 1, 7 }, "e4");
        std::istringstream iStream("date == 2018-01-07");
        auto condition = ParseCondition(iStream);
        auto predicate = [condition](const Date& date, const std::string& event) {
            return condition->Evaluate(date, event);
        };
        AssertEqual(db.FindIf(predicate).size(), 2, "Db Find 1");
    }
    {
        Database db;
        db.Add({ 2019, 1, 1 }, "e1");
        db.Add({ 2019, 1, 1 }, "e2");
        db.Add({ 2018, 1, 7 }, "e3");
        db.Add({ 2018, 1, 7 }, "e4");
        std::istringstream iStream("date >= 2018-01-07 AND date <= 2020-01-01");
        auto condition = ParseCondition(iStream);
        auto predicate = [condition](const Date& date, const std::string& event) {
            return condition->Evaluate(date, event);
        };
        AssertEqual(db.FindIf(predicate).size(), 4, "Db Find 2");
    }
    {
        Database db;
        db.Add({ 2019, 1, 1 }, "e1");
        db.Add({ 2019, 1, 1 }, "e2");
        db.Add({ 2018, 1, 7 }, "e3");
        db.Add({ 2018, 1, 7 }, "e4");
        AssertEqual(db.FindIf(empty_predicate).size(), 4, "Db Find 3");
    }
    {
        Database db;
        db.Add({ 2019, 1, 1 }, "e1");
        db.Add({ 2019, 1, 1 }, "e2");
        db.Add({ 2018, 1, 7 }, "e3");
        db.Add({ 2018, 1, 7 }, "e4");
        std::istringstream iStream(R"(event == "e1")");
        auto condition = ParseCondition(iStream);
        auto predicate = [condition](const Date& date, const std::string& event) {
            return condition->Evaluate(date, event);
        };
        AssertEqual(db.FindIf(predicate).size(), 1, "Db Find 4");
    }

    {
        Database db;
        db.Add({ 2019, 1, 1 }, "e1");
        db.Add({ 2019, 1, 1 }, "e2");
        db.Add({ 2018, 1, 7 }, "e3");
        db.Add({ 2018, 1, 7 }, "e4");
        std::istringstream iStream(R"(event == "e1" OR date == 2019-01-01)");
        auto condition = ParseCondition(iStream);
        auto predicate = [condition](const Date& date, const std::string& event) {
            return condition->Evaluate(date, event);
        };
        AssertEqual(db.FindIf(predicate).size(), 2, "Db Find 5");
    }

    // Add - Del - Add - Del
    {
        Database db;
        db.Add({ 2019, 1, 1 }, "e1");
        db.Add({ 2019, 1, 1 }, "e2");
        AssertEqual(db.RemoveIf(empty_predicate), 2, "Db Add-Del-Add-Del 1");

        db.Add({ 2019, 1, 1 }, "e1");
        db.Add({ 2019, 1, 1 }, "e2");
        AssertEqual(db.RemoveIf(empty_predicate), 2, "Db Add-Del-Add-Del 1");
    }

    {
        Database db;
        db.Add({ 2017, 1, 1 }, "first");
        db.Add({ 2017, 1, 1 }, "second");
        db.Add({ 2017, 1, 1 }, "third");
        db.Add({ 2017, 1, 1 }, "fourth");
        db.Add({ 2017, 1, 1 }, "five");
        AssertEqual(2, DoRemove(db, R"(event == "second" OR event == "fourth")"), "Remove several");
        std::ostringstream oStream;
        db.Print(oStream);
        AssertEqual("2017-01-01 first\n2017-01-01 third\n2017-01-01 five\n", oStream.str(),
            "Check print after remove several- 3");
    }
}

void TestDateComparisonNode() {
    {
        DateComparisonNode dateComparisonNode(Comparison::Equal, { 2017, 11, 18 });
        Assert(dateComparisonNode.Evaluate(Date{ 2017, 11, 18 }, ""), "DateComparison 1");
        Assert(!dateComparisonNode.Evaluate(Date{ 2017, 11, 19 }, ""), "DateComparison 2");
    }
    {
        DateComparisonNode dateComparisonNode(Comparison::NotEqual, { 2017, 11, 18 });
        Assert(dateComparisonNode.Evaluate(Date{ 2017, 11, 19 }, ""), "DateComparison 3");
        Assert(!dateComparisonNode.Evaluate(Date{ 2017, 11, 18 }, ""), "DateComparison 4");
    }
    {
        DateComparisonNode dateComparisonNode(Comparison::Less, { 2017, 11, 18 });
        Assert(dateComparisonNode.Evaluate(Date{ 2017, 11, 17 }, ""), "DateComparison 5");
        Assert(!dateComparisonNode.Evaluate(Date{ 2017, 11, 18 }, ""), "DateComparison 6");
        Assert(!dateComparisonNode.Evaluate(Date{ 2017, 11, 19 }, ""), "DateComparison 7");
    }
    {
        DateComparisonNode dateComparisonNode(Comparison::Greater, { 2017, 11, 18 });
        Assert(dateComparisonNode.Evaluate(Date{ 2017, 11, 19 }, ""), "DateComparison 8");
        Assert(!dateComparisonNode.Evaluate(Date{ 2017, 11, 18 }, ""), "DateComparison 9");
        Assert(!dateComparisonNode.Evaluate(Date{ 2017, 11, 17 }, ""), "DateComparison 10");
    }
    {
        DateComparisonNode dateComparisonNode(Comparison::LessOrEqual, { 2017, 11, 18 });
        Assert(dateComparisonNode.Evaluate(Date{ 2017, 11, 17 }, ""), "DateComparison 11");
        Assert(dateComparisonNode.Evaluate(Date{ 2017, 11, 18 }, ""), "DateComparison 12");
        Assert(!dateComparisonNode.Evaluate(Date{ 2017, 11, 19 }, ""), "DateComparison 13");
    }
    {
        DateComparisonNode dateComparisonNode(Comparison::GreaterOrEqual, { 2017, 11, 18 });
        Assert(dateComparisonNode.Evaluate(Date{ 2017, 11, 19 }, ""), "DateComparison 14");
        Assert(dateComparisonNode.Evaluate(Date{ 2017, 11, 18 }, ""), "DateComparison 15");
        Assert(!dateComparisonNode.Evaluate(Date{ 2017, 11, 17 }, ""), "DateComparison 16");
    }
}

void TestEventComparisonNode() {
    {
        EventComparisonNode eventComparisonNode(Comparison::Equal, "abc");
        Assert(eventComparisonNode.Evaluate(Date{ 0, 1, 1 }, "abc"), "EventComparison 1");
        Assert(!eventComparisonNode.Evaluate(Date{ 0, 1, 1 }, "cbe"), "EventComparison 2");
    }
    {
        EventComparisonNode eventComparisonNode(Comparison::NotEqual, "abc");
        Assert(eventComparisonNode.Evaluate(Date{ 0, 1, 1 }, "cde"), "EventComparison 3");
        Assert(!eventComparisonNode.Evaluate(Date{ 0, 1, 1 }, "abc"), "EventComparison 4");
    }
    {
        EventComparisonNode eventComparisonNode(Comparison::Less, "abc");
        Assert(eventComparisonNode.Evaluate(Date{ 0, 1, 1 }, "abb"), "EventComparison 5");
        Assert(!eventComparisonNode.Evaluate(Date{ 0, 1, 1 }, "abc"), "EventComparison 6");
        Assert(!eventComparisonNode.Evaluate(Date{ 0, 1, 1 }, "abd"), "EventComparison 7");
    }
    {
        EventComparisonNode eventComparisonNode(Comparison::Greater, "abc");
        Assert(eventComparisonNode.Evaluate(Date{ 0, 1, 1 }, "abd"), "EventComparison 8");
        Assert(!eventComparisonNode.Evaluate(Date{ 0, 1, 1 }, "abc"), "EventComparison 9");
        Assert(!eventComparisonNode.Evaluate(Date{ 0, 1, 1 }, "abb"), "EventComparison 10");
    }
    {
        EventComparisonNode eventComparisonNode(Comparison::LessOrEqual, "abc");
        Assert(eventComparisonNode.Evaluate(Date{ 0, 1, 1 }, "abb"), "EventComparison 11");
        Assert(eventComparisonNode.Evaluate(Date{ 0, 1, 1 }, "abc"), "EventComparison 12");
        Assert(!eventComparisonNode.Evaluate(Date{ 0, 1, 1 }, "abd"), "EventComparison 13");
    }
    {
        EventComparisonNode eventComparisonNode(Comparison::GreaterOrEqual, "abc");
        Assert(eventComparisonNode.Evaluate(Date{ 0, 1, 1 }, "abd"), "EventComparison 14");
        Assert(eventComparisonNode.Evaluate(Date{ 0, 1, 1 }, "abc"), "EventComparison 15");
        Assert(!eventComparisonNode.Evaluate(Date{ 0, 1, 1 }, "abb"), "EventComparison 16");
    }
}

void TestLogicalOperationNode() {
    {
        LogicalOperationNode logicalOperationNode(LogicalOperation::And, std::make_shared<EmptyNode>(),
            std::make_shared<EmptyNode>());
        Assert(logicalOperationNode.Evaluate(Date{ 0, 1, 1 }, "abc"), "LogicalOperationNode 1");
    }
    {
        LogicalOperationNode logicalOperationNode(LogicalOperation::And, std::make_shared<AlwaysFalseNode>(),
            std::make_shared<EmptyNode>());
        Assert(!logicalOperationNode.Evaluate(Date{ 0, 1, 1 }, "abc"), "LogicalOperationNode 2");
    }
    {
        LogicalOperationNode logicalOperationNode(LogicalOperation::And, std::make_shared<EmptyNode>(),
            std::make_shared<AlwaysFalseNode>());
        Assert(!logicalOperationNode.Evaluate(Date{ 0, 1, 1 }, "abc"), "LogicalOperationNode 3");
    }
    {
        LogicalOperationNode logicalOperationNode(LogicalOperation::And, std::make_shared<AlwaysFalseNode>(),
            std::make_shared<AlwaysFalseNode>());
        Assert(!logicalOperationNode.Evaluate(Date{ 0, 1, 1 }, "abc"), "LogicalOperationNode 4");
    }
    {
        LogicalOperationNode logicalOperationNode(LogicalOperation::Or, std::make_shared<EmptyNode>(),
            std::make_shared<EmptyNode>());
        Assert(logicalOperationNode.Evaluate(Date{ 0, 1, 1 }, "abc"), "LogicalOperationNode 5");
    }
    {
        LogicalOperationNode logicalOperationNode(LogicalOperation::Or, std::make_shared<AlwaysFalseNode>(),
            std::make_shared<EmptyNode>());
        Assert(logicalOperationNode.Evaluate(Date{ 0, 1, 1 }, "abc"), "LogicalOperationNode 6");
    }
    {
        LogicalOperationNode logicalOperationNode(LogicalOperation::Or, std::make_shared<EmptyNode>(),
            std::make_shared<AlwaysFalseNode>());
        Assert(logicalOperationNode.Evaluate(Date{ 0, 1, 1 }, "abc"), "LogicalOperationNode 7");
    }
    {
        LogicalOperationNode logicalOperationNode(LogicalOperation::Or, std::make_shared<AlwaysFalseNode>(),
            std::make_shared<AlwaysFalseNode>());
        Assert(!logicalOperationNode.Evaluate(Date{ 0, 1, 1 }, "abc"), "LogicalOperationNode 8");
    }
}

void TestEmptyNode() {
    {
        EmptyNode emptyNode;
        Assert(emptyNode.Evaluate(Date{ 0, 1, 1 }, "abc"), "EmptyNode 1");
        Assert(emptyNode.Evaluate(Date{ 2017, 11, 18 }, "def"), "EmptyNode 2");
        Assert(emptyNode.Evaluate(Date{ 9999, 12, 31 }, "ghi"), "EmptyNode 3");
    }
}