#pragma once

#include <string>
#include <memory>
#include "date.h"

enum class LogicalOperation {
	Or,
	And
};

enum class Comparison {
	Less,
	LessOrEqual,
	Greater,
	GreaterOrEqual,
	Equal,
	NotEqual
};

class Node {
public:
	Node();
	Node(const std::string& type);
	virtual bool Evaluate(const Date& date, const std::string& event) const = 0;
	std::string GetType();

private:
	const std::string type_;
};

class EmptyNode : public Node {
public:
	EmptyNode();
	bool Evaluate(const Date& date, const std::string& event) const;
};

class DateComparisonNode : public Node {
public:
	DateComparisonNode(const Comparison& comp, const Date& date);
	bool Evaluate(const Date& date, const std::string& event) const;

private:
	const Comparison comp_;
	const Date date_;
};

class EventComparisonNode : public Node {
public:
	EventComparisonNode(const Comparison& comp, const std::string& event);
	bool Evaluate(const Date& date, const std::string& event) const;

private:
	const Comparison comp_;
	const std::string event_;
};

class LogicalOperationNode : public Node {
public:
	LogicalOperationNode(const LogicalOperation& log_op, std::shared_ptr<Node> left, std::shared_ptr<Node> right);
	bool Evaluate(const Date& date, const std::string& event) const;

private:
	const LogicalOperation log_op_;
	std::shared_ptr<Node> left_;
	std::shared_ptr<Node> right_;
};