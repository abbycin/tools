/*********************************************************
          File Name: crypto.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Fri 16 Dec 2016 08:49:11 PM CST
**********************************************************/

#ifndef CRYPTO_H_
#define CRYPTO_H_

#include <algorithm>
#include <vector>
#include <string>
#include <cstring>

static const char hex_table[] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

using Callback = void(*)(int, std::string&);

class Crypto
{
  public:
    Crypto() = default;
    ~Crypto() = default;

    std::string encode(const std::string& s, int magic,  Callback cb)
    {
      std::string res = u2h(s);
      res = h2b(res);
      cb(magic, res);
      return res;
    }

    std::string decode(const std::string& s, int magic, Callback cb)
    {
      std::string res = s;
      cb(magic, res);
      res = b2h(res);
      res = h2u(res);
      return res;
    }

  private:

    std::string d2h(char n, bool& neg)
    {
      std::vector<char> v;
      int num = n;
      neg = false;
      if(num < 0)
      {
        neg = true;
        num = 256 + num;
      }
      while(num > 0)
      {
        v.push_back(hex_table[num % 16]);
        num /= 16;
      }
      std::reverse(v.begin(), v.end());
      return std::string(v.begin(), v.end());
    }

    std::string h2b(const std::string& hex)
    {
      std::vector<int> tmp;
      int b = 0;
      bool high = false;
      for(size_t i = 0; i < hex.size(); ++i)
      {
        if(hex[i] >= '0' && hex[i] <= '9')
        {
          b = hex[i] - '0';
        }
        else if(hex[i] >= 'a' && hex[i] <= 'f')
        {
          b = hex[i] - 'a';
          high = true;
        }
        else
          b = hex[i];
        tmp.push_back(b);
        if(high)
        {
          high = false;
          tmp.push_back('.');
        }
      }
      return std::string(tmp.begin(), tmp.end());
    }

    std::string b2h(const std::string& bin)
    {
      std::vector<char> tmp;
      char hex = 0;
      for(size_t i = 0; i < bin.size(); ++i)
      {
        if(bin[i] != '.')
        {
          if(bin[i+1] == '.')
          {
            hex = bin[i] + 'a';
            ++i;
          }
          else
            hex = bin[i] + '0';
        }
        if(bin[i] == '`' || bin[i] == '^')
          hex = bin[i];
        tmp.push_back(hex);
      }
      return std::string(tmp.begin(), tmp.end());
    }

    std::string u2h(const std::string& utf)
    {
      std::string res;
      bool neg = false;
      for(auto x: utf)
      {
        res.append(d2h(x, neg));
        if(neg)
          res.append("`");
        else
          res.append("^");
      }
      return res;
    }

    std::string h2u(const std::string& hex)
    {
      char* tmp = new char[hex.size()];
      memset(tmp, 0, hex.size());
      char* p = nullptr;
      int n = 0;
      const char* start = hex.data();
      const char* end = start + hex.size();
      for(p = tmp; start <= end; ++p)
      {
        if(*(start + 2) == '`')
        {
          // FIXME: UTF-8 is variable length
          if(sscanf(start, "%2x", &n))
            *p = n;
          start += 2;
        }
        else if(*(start + 2) == '^')
        {
          n = *start++ - '0';
          n <<= 4;
          if(*start >= 'a' && *start <= 'f')
            n += (*start - 'a') + 10;
          else if(*start >= '0' && *start <= '9')
            n += (*start - '0');
          ++start;
          *p = n;
        }
        ++start;
      }
      size_t len = p - tmp;
      std::string res{tmp, len};
      delete [] tmp;
      return res;
    }
};

#endif
