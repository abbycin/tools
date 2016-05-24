/*********************************************************
          File Name:rpn.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Thu 04 Feb 2016 01:48:06 PM CST
**********************************************************/

#ifndef rpn_h_
#define rpn_h_

#include <string>
#include <vector>
#include <stack>
#include <stdexcept>
#include <sstream>
#include <iostream>

namespace rpn
{
    static std::string token{"+-*/"};

    inline static bool is_digit(const std::string &s)
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
                    throw std::runtime_error("bad expression!");
                }
            }
        }
        return true;
    }

    inline static bool is_operator(const std::string &s)
    {
        if(s.size() > 1)
            return false;
        if(token.find(s[0]) == token.npos)
            return false;
        return true;
    }

    inline static int is_parentheses(const std::string &s)
    {
        if(s.size() > 1)
            return 0;
        if(s[0] == '(')
            return 1;
        if(s[0] == ')')
            return 2;
        return 0;
    }

    inline static int get_priority(const std::string &s)
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
    
    static std::vector<std::string> shunting_yard(std::vector<std::string> &input)
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
                        throw std::runtime_error("parenthese not match!");
                    }
                }
            }
        }

        while(!stk.empty())
        {
            if(is_parentheses(stk.top()))
            {
                throw std::runtime_error("parenthese not match!");
            }
            res.push_back(stk.top());
            stk.pop();
        }

        return res;
    }

    static double eval_rpn(std::stack<std::string> &stk)
    {
        if(stk.empty())
        {
            throw std::runtime_error("error!");
        }
        double x, y;
        std::string value = stk.top();
        stk.pop();
        size_t n = stk.size();

        if(n >= 1 && token.find(value) != token.npos)
        {
            y = eval_rpn(stk);
            x = eval_rpn(stk);

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
                    throw std::runtime_error("divide by zero!");
                }
                x /= y;
            }
        }
        else
        {
            size_t i = 0;
            try
            {
                x = std::stod(value, &i);
            }
            catch(std::exception &e)
            {
                std::string err(e.what());
                err += value + " bad expression!";
                throw std::runtime_error(err);
            }
        }
        return x;
    }

    double get_answer(std::vector<std::string> &vs)
    {
        auto shunt = shunting_yard(vs);
        std::stack<std::string> rpn_;
        for(const auto &x: shunt)
            rpn_.push(x);

        double res = eval_rpn(rpn_);

        return res;
    }
}

#endif
