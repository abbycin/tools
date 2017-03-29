/*********************************************************
          File Name:test.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Wed 29 Mar 2017 08:34:26 PM CST
**********************************************************/

#include <iostream>
#include "clp.h"

using namespace nm;
using namespace std;

int main(int argc, char* argv[])
{
  Clp clpl{argc, argv};
  clpl.parse().parse(Clp::Option::LONG);
  auto clp = std::move(clpl);
  if(!clp.ok())
  {
    cout << clp.msg() << endl;
    return 1;
  }
  if(auto help = clp.contain("help"); help)
  {
    cout << argv[0] << " -key value ...\n";
    cout << "\t-name name [--name]\n";
    cout << "\t-age age [--age]\n";
    cout << "\t-height height [--height]\n";
    return 1;
  }
  if(auto n = clp.get<std::string>("name"); n)
    cout << "name: " << n.value() << endl;
  else
    cout << clp.msg() << endl;
  if(auto a = clp.get<size_t>("age"); a)
    cout << "age: " << a.value() << endl;
  else
    cout << clp.msg() << endl;
  if(auto h = clp.get<double>("height"); h)
    cout << "height: " << h.value() << endl;
  else
    cout << clp.msg() << endl;
  if(auto es = clp.get<std::string>("ex"); es)
    cout << "escape: " << es.value() << endl;
  else
    cout << clp.msg() << endl;
  if(clp.contain("ex"))
    cout << "single option `ex`\n";
  cout << "none:\n";
  for(auto&x: clp.none())
    cout << x << endl;
}
