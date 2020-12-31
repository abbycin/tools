/***********************************************
        File Name: main.cpp
        Author: Abby Cin
        Mail: abbytsing@gmail.com
        Created Time: 12/29/20 2:22 PM
***********************************************/

#include "json.h"
#include <iostream>

int main()
{
  std::string s{R"~(
{
  "name": "elder",
  "age": 1926.8,
  "hobby": {
    "mo": ["too young", "too simple"],
    "ha": "+1s",
    "profile": [1926, 8, "naive!", true]
  },
  "male": true
}
                )~"};
  auto r = nm::json::parse(s);
  if(!r)
  {
    std::cout << r.trace() << '\n';
    return 1;
  }
  std::cout << "is object: " << std::boolalpha << r.is_object() << '\n';
  auto o = r.as<nm::json::object_t>();
  std::cout << "object size: " << o->size() << '\n';
  auto& elder = r.get<nm::json::object_t>()["name"];
  std::cout << "name: " << elder.get<nm::json::string_t>() << '\n';
  std::cout << "age: " << r.get<nm::json::object_t>()["age"].get<nm::json::number_t>() << '\n';
  r["male"] = nm::json::array_t{"he is male?", true};

  std::cout << "stringify: \n";
  std::cout << r.to_string(2) << '\n';

  std::cout << "-------------------------------\n";
  nm::json::JsonValue j{{{"name", "elder"},
                         {"age", 1926.800090},
                         {"\tmotto\t", nm::json::array_t{"too young too simple\t", "sometimes naive!"}}}};

  std::cout << j.to_string() << '\n';
  std::cout << std::setprecision(9) << j["age"] << '\n';
  std::cout << j["\tmotto\t"][0] << j["\tmotto\t"][1] << '\n';
  return 0;
}
