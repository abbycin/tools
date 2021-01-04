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
  auto r2 = nm::json::parse2(s);
  if(!r2)
  {
    std::cout << r2.trace() << '\n';
    return 1;
  }
  auto r3 = r.clone();
  auto r4 = nm::json::parse2(nm::json::parse2(r2.to_string2()).to_string2());
  std::cout << std::boolalpha << (r == r2) << ' ' << (r3 == r4) << '\n';
  std::cout << "is object: " << std::boolalpha << r.is_object() << '\n';
  auto o = r.as<nm::json::object_t>();
  std::cout << "object size: " << o.size() << '\n';
  auto& elder = r.as<nm::json::object_t>()["name"];
  std::cout << "name: " << elder.as<nm::json::string_t>() << '\n';
  std::cout << "age: " << r.as<nm::json::object_t>()["age"].as<nm::json::number_t>() << '\n';
  r["male"] = nm::json::array_t{"he is male?", true};

  std::cout << "stringify: \n";
  std::cout << r.to_string2(1) << '\n';

  std::cout << "-------------------------------\n";
  nm::json::JsonValue j{{{"name", "elder"},
                         {"age", 1926.800090},
                         {"\tmotto\t", nm::json::array_t{"too young too simple\t", "sometimes naive!"}}}};

  std::cout << j.to_string() << '\n';
  std::cout << std::setprecision(9) << j["age"] << '\n';
  std::cout << j["\tmotto\t"][0] << j["\tmotto\t"][1] << '\n';

  {
    using namespace nm;
    json::JsonValue j{{
        {"heterogeneous", json::array_t{1,
                                        2,
                                        3,
                                        true,
                                        false,
                                        "+1s",
                                        json::array_t{2, 3, 3},
                                        {{"name", "elder"}, {"age", json::array_t{1926, 8}}}}},
    }};
    j["elder"] = 1;
    auto j2 = j.clone();
    j["elder"] = 2;
    j["heterogeneous"] = json::null_t{};
    j2["heterogeneous"][0] = 1926;
    std::cout << j << '\n';
    std::cout << j2.to_string() << '\n';
  }
  return 0;
}
