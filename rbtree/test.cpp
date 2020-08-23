/*********************************************************
          File Name: test.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 24 Dec 2016 07:02:40 PM CST
**********************************************************/

#include <iostream>
#include "rb-tree.h"

int main()
{
  nm::RBtree<int, int> tree;
  // insert
  for(int i = 0; i < 10; ++i)
  {
    tree.insert(i, i * 2);
  }
  // update
  if(!tree.insert(3, 9))
    std::cout << "update value of key " << 3 << " to " << 9 << "\n";
  // find
  auto iter = tree.find(3);
  // delete
  if(iter != tree.end())
  {
    std::cout << "remove value: " << iter->first << ", " << iter->second;
    tree.remove(iter);
    if(!tree.remove(3))
      std::cout << " Ok.\n";
    else
      std::cout << " Failed.\n";
  }
  else
    std::cout << "not found\n";
  nm::RBtree<std::string, std::string> tree1{{"id", "abby"}, {"site", "isliberty.me"}};
  auto iter1 = tree1.find("site");
  if(iter1 == tree1.end())
    std::cout << "not found\n";
  else
    std::cout << iter1->second << std::endl;
  iter1 = tree1.find("+1s");
  if(iter1 == tree1.end())
    std::cout << "-1s\n";

  nm::RBtree<int, int> lt{{1, 0}, {2, 0}, {5, 0}, {7, 0}, {8, 0}, {11, 0}, {14, 0}, {15, 0}, {4, 0}};

  for(auto& i: lt)
  {
    std::cerr << i.first << ":" << i.second << '\n';
  }
  return 0;
}
