#include "token.h"

#include <stdexcept>

using namespace std;

vector<Token> Tokenize(istream& iStream)
{
    vector<Token> tokens;

    char c;
    while (iStream >> c)
    {
        if (isdigit(c))
        {
            string date(1, c);
            for (int i = 0; i < 3; ++i)
            {
                while (isdigit(iStream.peek()))
                {
                    date += iStream.get();
                }
                if (i < 2)
                {
                    date += iStream.get(); // Consume '-'
                }
            }
            tokens.push_back({ date, TokenType::DATE });
        }
        else if (c == '"')
        {
            string event;
            getline(iStream, event, '"');
            tokens.push_back({ event, TokenType::EVENT });
        }
        else if (c == 'd')
        {
            if (iStream.get() == 'a' && iStream.get() == 't' && iStream.get() == 'e')
            {
                tokens.push_back({ "date", TokenType::COLUMN });
            }
            else 
            {
                throw logic_error("Unknown token");
            }
        }
        else if (c == 'e')
        {
            if (iStream.get() == 'v' && iStream.get() == 'e' && iStream.get() == 'n' && iStream.get() == 't')
            {
                tokens.push_back({ "event", TokenType::COLUMN });
            }
            else 
            {
                throw logic_error("Unknown token");
            }
        }
        else if (c == 'A')
        {
            if (iStream.get() == 'N' && iStream.get() == 'D')
            {
                tokens.push_back({ "AND", TokenType::LOGICAL_OP });
            }
            else
            {
                throw logic_error("Unknown token");
            }
        }
        else if (c == 'O')
        {
            if (iStream.get() == 'R')
            {
                tokens.push_back({ "OR", TokenType::LOGICAL_OP });
            }
            else
            {
                throw logic_error("Unknown token");
            }
        }
        else if (c == '(')
        {
            tokens.push_back({ "(", TokenType::PAREN_LEFT });
        }
        else if (c == ')')
        {
            tokens.push_back({ ")", TokenType::PAREN_RIGHT });
        }
        else if (c == '<')
        {
            if (iStream.peek() == '=')
            {
                iStream.get();
                tokens.push_back({ "<=", TokenType::COMPARE_OP });
            }
            else
            {
                tokens.push_back({ "<", TokenType::COMPARE_OP });
            }
        }
        else if (c == '>')
        {
            if (iStream.peek() == '=')
            {
                iStream.get();
                tokens.push_back({ ">=", TokenType::COMPARE_OP });
            }
            else
            {
                tokens.push_back({ ">", TokenType::COMPARE_OP });
            }
        }
        else if (c == '=')
        {
            if (iStream.get() == '=')
            {
                tokens.push_back({ "==", TokenType::COMPARE_OP });
            }
            else
            {
                throw logic_error("Unknown token");
            }
        }
        else if (c == '!')
 {
            if (iStream.get() == '=')
            {
                tokens.push_back({ "!=", TokenType::COMPARE_OP });
            }
            else
            {
                throw logic_error("Unknown token");
            }
        }
    }

    return tokens;
}