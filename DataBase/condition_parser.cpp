#include "condition_parser.h"
#include "token.h"
#include "node.h"

#include <map>

using namespace std;

template <class It> shared_ptr<Node> ParseComparison(It& current, It end)
{
    if (current == end)
    {
        throw logic_error("Expected column name: date or event");
    }

    Token& column = *current;
    if (column.type != TokenType::COLUMN)
    {
        throw logic_error("Expected column name: date or event");
    }
    ++current;

    if (current == end)
    {
        throw logic_error("Expected comparison operation");
    }

    Token& operation = *current;
    if (operation.type != TokenType::COMPARE_OP)
    {
        throw logic_error("Expected comparison operation");
    }
    ++current;

    if (current == end)
    {
        throw logic_error("Expected right value of comparison");
    }

    Comparison comparison;
    if (operation.value == "<") 
    {
        comparison = Comparison::Less;
    }
    else if (operation.value == "<=")
    {
        comparison = Comparison::LessOrEqual;
    }
    else if (operation.value == ">")
    {
        comparison = Comparison::Greater;
    }
    else if (operation.value == ">=")
    {
        comparison = Comparison::GreaterOrEqual;
    }
    else if (operation.value == "==")
    {
        comparison = Comparison::Equal;
    }
    else if (operation.value == "!=")
    {
        comparison = Comparison::NotEqual;
    }
    else
    {
        throw logic_error("Unknown comparison token: " + operation.value);
    }

    const string& value = current->value;
    ++current;

    if (column.value == "date")
    {
        istringstream iStream(value);
        return make_shared<DateComparisonNode>(comparison, ParseDate(iStream));
    }
    else
    {
        return make_shared<EventComparisonNode>(comparison, value);
    }
}

template <class It>
shared_ptr<Node> ParseExpression(It& current, It end, unsigned precedence)
{
    if (current == end)
    {
        return shared_ptr<Node>();
    }

    shared_ptr<Node> left;

    if (current->type == TokenType::PAREN_LEFT)
    {
        ++current; // consume '('
        left = ParseExpression(current, end, 0u);
        if (current == end || current->type != TokenType::PAREN_RIGHT)
        {
            throw logic_error("Missing right paren");
        }
        ++current; // consume ')'
    }
    else
    {
        left = ParseComparison(current, end);
    }

    const map<LogicalOperation, unsigned> precedences = {
        {LogicalOperation::Or, 1},
        {LogicalOperation::And, 2}
    };

    while (current != end && current->type != TokenType::PAREN_RIGHT)
    {
        if (current->type != TokenType::LOGICAL_OP)
        {
            throw logic_error("Expected logic operation");
        }

        const auto logical_operation = current->value == "AND" ? LogicalOperation::And
            : LogicalOperation::Or;
        const auto current_precedence = precedences.at(logical_operation);
        if (current_precedence <= precedence) 
        {
            break;
        }

        ++current; // consume op

        left = make_shared<LogicalOperationNode>(
            logical_operation, left,
            ParseExpression(current, end, current_precedence)
            );
    }
    return left;
}

shared_ptr<Node> ParseCondition(istream& is)
{
    auto tokens = Tokenize(is);
    auto current = tokens.begin();
    auto top_node = ParseExpression(current, tokens.end(), 0u);

    if (!top_node) 
    {
        top_node = make_shared<EmptyNode>();
    }

    if (current != tokens.end())
    {
        throw logic_error("Unexpected tokens after condition");
    }
    return top_node;
}