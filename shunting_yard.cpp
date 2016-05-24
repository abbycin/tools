/*********************************************************
          File Name:shunting_yard.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Thu 04 Feb 2016 10:15:59 AM CST
**********************************************************/

#include <iostream>
#include <vector>
#include <stack>
#include <string>
#include <sstream>

bool is_digit(std::string &s)
{
    int count = 0;
    int minus_count = 0;
    if(s.size() == 1 && (s[0] == '.' || s[0] == '-'))
      return false;
    for(auto &x: s)
    {
        if('0' > x || x > '9')
        {
            if(x != '.' && x != '-')
                return false;
            if(x == '.')
                count += 1;
            if(x == '-')
                minus_count += 1;
            if(count > 1 || minus_count > 1)
            {
                std::cerr << "bad expression!\n";
                exit(1);
            }
        }
    }

    return true;
}

int is_parentheses(std::string &s)
{
    if(s.size() > 1)
        return 0;
    if(s[0] == ')')
        return 2;
    if(s[0] == '(')
        return 1;
    return 0;
}

bool is_operator(std::string &s)
{
    if(s.size() > 1)
        return false;
    if(s[0] == '+' || s[0] == '-' || s[0] == '*' || s[0] == '/')
        return true;
    return false;
}

// 0 < 1 < 2 : '(', ')' < '*', '/' < '+', '-'
int get_priority(std::string &s)
{
    char ch = s[0];
    switch(ch)
    {
        case '*': case '/':
            return 2;
        case '+': case '-':
            return 1;
    }

    return 0;
}

std::vector<std::string> get_res(std::vector<std::string> &input)
{
    std::vector<std::string> res;
    std::stack<std::string> stk;
    int tmp = 0;
    bool ok = false;

    for(auto x: input)
    {
        if(is_digit(x))
        {
            res.push_back(x);
        }
        else if(is_operator(x))
        {
            if(stk.size() == 0)
            {
                stk.push(x);
            }
            else
            {
                // pop heigh priority operator
                while(!stk.empty() && (get_priority(stk.top()) >= get_priority(x)))
                {
                    res.push_back(stk.top());
                    stk.pop();
                }
                stk.push(x);
            }
        }
        else if((tmp = is_parentheses(x)) != 0)
        {
            if(tmp == 1) // left paretheses
            {
                stk.push(x);
            }
            else if(tmp == 2)
            {
                ok = false;
                while(!stk.empty())
                {
                    if(stk.top() == "(")
                    {
                        stk.pop();
                        ok = true;
                        break;
                    }
                    res.push_back(stk.top());
                    stk.pop();
                }

                if(stk.empty() && !ok)
                {
                    std::cerr << "parenthese not match\n";
                    exit(1);
                }
            }
        }
    }

    while(!stk.empty())
    {
        if(is_parentheses(stk.top()))
        {
            std::cerr << "parenthese not match\n";
            exit(1);
        }
        res.push_back(stk.top());
        stk.pop();
    }

    return res;
}

std::vector<std::string> split(const std::string &s, char delim)
{
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> res;
    while(std::getline(ss, item, delim))
        res.push_back(item);

    return res;
}

static std::string token{"+-*/"};

double eval(std::stack<std::string> &stk)
{
    if(stk.empty())
    {
        std::cerr << "stack is empty!\n";
        exit(1);
    }
    double x, y;
    std::string value = stk.top();
    stk.pop();
    size_t n = stk.size();

    if(n >= 1 && token.find(value) != token.npos)
    {
        y = eval(stk);
        x = eval(stk);

        if(value[0] == '+')
            x += y;
        else if(value[0] == '-')
            x -= y;
        else if(value[0] == '*')
            x *= y;
        else 
        {
            if(y == 0)
            {
                std::cerr << "divide by zero!\n";
                exit(1);
            }
            x /= y;
        }
    }
    else
    {
        size_t i = 0;
        x = std::stod(value, &i);
    }
    return x;
}

int main(int argc, char *argv[])
{
    if(argc < 2)
        return -1;
    auto input = split(argv[1], ' ');
    /*
    std::vector<std::string> input;
    for(int i = 0; i < argc; ++i)
        input.push_back(argv[i]);
    */

    auto tmp = get_res(input);
    std::stack<std::string> rpn;
    for(auto x: tmp)
    {
        rpn.push(x);
        std::cout << x << " ";
    }

    std::cout << std::endl;
    double outcome = eval(rpn);
    std::cout << outcome << std::endl;

    return 0;
}
