#include <string>
#include <memory>

#include "node.h"
#include "date.h"

using namespace std;

Node::Node() {}
Node::Node(const string& type) : type_(type) {}
string Node::GetType() { return type_; }

EmptyNode::EmptyNode() : Node("Empty") {}
bool EmptyNode::Evaluate(const Date& date = {}, const string& event = "") const { return true; }

DateComparisonNode::DateComparisonNode(const Comparison& comp, const Date& date) : 
	Node("Date"), comp_(comp), date_(date) {}
bool DateComparisonNode::Evaluate(const Date& date = {}, const string& event = "") const {
	if (date.GetDay() == 0) return false;
	if (comp_ == Comparison::Less) return date < date_;
	if (comp_ == Comparison::LessOrEqual) return date <= date_;
	if (comp_ == Comparison::Greater) return date > date_;
	if (comp_ == Comparison::GreaterOrEqual) return date >= date_;
	if (comp_ == Comparison::Equal) return date == date_;
	if (comp_ == Comparison::NotEqual) return date != date_;
}

EventComparisonNode::EventComparisonNode(const Comparison& comp, const string& event) : 
	Node("Event"),
	comp_(comp),
	event_(event) {}

bool EventComparisonNode::Evaluate(const Date& date = {}, const string& e = "") const
{
	if (e == "") return false;
	if (comp_ == Comparison::Less) return e < event_;
	if (comp_ == Comparison::LessOrEqual) return e <= event_;
	if (comp_ == Comparison::Greater) return e > event_;
	if (comp_ == Comparison::GreaterOrEqual) return e >= event_;
	if (comp_ == Comparison::Equal) return e == event_;
	if (comp_ == Comparison::NotEqual) return e != event_;
}

LogicalOperationNode::LogicalOperationNode(const LogicalOperation& log_op, shared_ptr<Node> left, shared_ptr<Node> right) : 
	Node("Operation"),
	log_op_(log_op),
	left_(left),
	right_(right)
{}

bool LogicalOperationNode::Evaluate(const Date& date = {}, const string& event = "") const
{
	if (log_op_ == LogicalOperation::And) return left_->Evaluate(date, event) && right_->Evaluate(date, event);
	else return left_->Evaluate(date, event) || right_->Evaluate(date, event);
}