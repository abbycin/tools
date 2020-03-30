/***********************************************
        File Name: router.h
        Author: Abby Cin
        Mail: abbytsing@gmail.com
        Created Time: 3/12/20 8:58 PM
***********************************************/

#ifndef ROUTER_H_
#define ROUTER_H_

#include <algorithm>
#include <iostream>
#include <optional>
#include <functional>
#include <list>
#include <string>
#include <vector>
#include <map>
#include <string>

using namespace std::string_literals;

using Pairs = std::map<std::string, std::string>;
using Handler = std::function<void(Pairs)>;
using Error = std::optional<std::string>;

class Router
{
  enum NodeType
  {
    STATIC,
    ROOT,
    WILD,
    PARAM,
  };

  static std::string to_string(NodeType t)
  {
    switch(t)
    {
    case STATIC:
      return "static";
    case ROOT:
      return "root";
    case WILD:
      return "wild";
    case PARAM:
      return "param";
    }
    return "";
  }

  struct Node
  {
    std::string path;
    std::string seqs;
    NodeType type;
    bool wildcard;
    Handler handler;
    std::vector<Node*> children;

    Node() : path{}, type{STATIC}, wildcard{false}, seqs{}, handler{}, children{} {}

    Node(const std::string& v) : path{v}, seqs{}, type{STATIC}, wildcard{false}, handler{}, children{} {}
  };

public:
  Router() : root_{} {}

  ~Router()
  {
    std::list<Node*> tmp;
    for(auto c: root_.children)
    {
      tmp.push_back(c);
    }
    while(!tmp.empty())
    {
      auto n = tmp.front();
      tmp.pop_front();
      for(auto c: n->children)
      {
        tmp.push_back(c);
      }
      delete n;
    }
    std::cout << "===========================\n";
  }

  Error add(std::string path, Handler handler)
  {
    Node* n = &root_;
    Error res;
    add_impl(n, path, &handler, res);
    return res;
  }

  auto get(std::string path) -> std::tuple<Handler, Pairs, bool>
  {
    Node* n = &root_;
    return get_impl(n, path);
  }

private:
  Node root_;

  static void add_impl(Node* n, std::string path, Handler* handler, Error& res)
  {
    if(n->path.empty() && n->seqs.empty())
    {
      insert(n, path, handler, res);
      n->type = ROOT;
      return;
    }

  next:
    auto l = common_prefix(n->path, path);

    // split parent
    if(l < n->path.size())
    {
      auto child = new Node{n->path.substr(l)};
      child->children = n->children;
      child->seqs = n->seqs;
      child->type = STATIC;
      child->wildcard = n->wildcard;
      child->handler = n->handler;

      n->children.clear();
      n->seqs.clear();
      n->children.push_back(child);
      n->seqs.push_back(n->path[l]);
      n->path = n->path.substr(0, l);
      n->handler = Handler{};
      n->wildcard = false;
    }

    if(l < path.size())
    {
      path = path.substr(l);

      /**
       * case1
       * n->type is wildcard, then it should NOT have any child, since '*' will match all the rest path
       *
       * case2
       * ```
       * path = "/:mo"
       * n->path = "/:moha"
       * ```
       * => conflict
       *
       * case3
       * ```
       * path = "/:moha"
       * n->path = "/:mo"
       * ```
       * => conflict
       *
       * case4
       * ```
       * path = "/:mo/ha"
       * n->path = "/:mo"
       * ```
       * => valid
       *
       * case5
       * ```
       * path = "/:mo/:ha"
       * n->path = "/:mo"
       * ```
       * => valid
       */

      if(n->wildcard)
      {
        n = n->children[0];

        if(n->type == WILD && n->handler)
        {
          res = "path is already registered as wild";
          return;
        }

        if(path.size() >= n->path.size() && n->path == path.substr(0, n->path.size()) &&
           (n->path.size() >= path.size() || path[n->path.size()] == '/'))
        {
          goto next;
        }
        else
        {
          res = "conflict with registered param path: " + n->path;
          return;
        }
      }

      char c = path[0];
      if(n->type == PARAM && c == '/' && n->children.size() == 1)
      {
        n = n->children[0];
        goto next;
      }

      for(size_t i = 0; i < n->seqs.size(); ++i)
      {
        if(c == n->seqs[i])
        {
          n = n->children[i];
          goto next;
        }
      }

      if(c != ':' && c != '*')
      {
        n->seqs.push_back(c);
        auto child = new Node{};
        n->children.push_back(child);
        n = child;
      }
      insert(n, path, handler, res);
      return;
    }

    if(n->handler)
    {
      res = "a handler already registered to same path";
    }
    else
    {
      n->handler = *handler;
    }
  }

  static void insert(Node* n, std::string& path, Handler* handler, Error& res)
  {
    for(;;)
    {
      auto [wildcard, start, valid] = find_wildcard(path);
      // no wildcard, simply add to child
      if(start < 0)
      {
        break;
      }

      if(!valid)
      {
        res = "invalid wildcard";
        return;
      }

      if(wildcard.size() < 2)
      {
        res = "wildcard must have a name";
        return;
      }

      if(n->children.size() > 0)
      {
        res = "already registered as " + to_string(n->type);
        return;
      }

      if(wildcard[0] == ':')
      {
        // path maybe mo/ha/:elder, then n->path is mo/ha/
        if(start > 0)
        {
          n->path = path.substr(0, start);
          path = path.substr(start);
        }

        n->wildcard = true;
        auto child = new Node{};
        child->type = PARAM;
        child->path = wildcard; // :elder

        n->children.clear();
        n->children.push_back(child);
        n = child;

        // maybe mo/ha/:elder/naive/:+1s
        if(wildcard.size() < path.size())
        {
          path = path.substr(wildcard.size());
          child = new Node{};
          n->children.push_back(child);
          n = child;
          continue;
        }

        n->handler = *handler;
        return;
      }
      else if(wildcard[0] == '*')
      {
        // for wildcard is *elder, path is *elder/naive
        if((start + wildcard.size() != path.size()) || (wildcard.find('/') != wildcard.npos))
        {
          res = "wild must at the end of path";
          return;
        }

        // for wildcard is /naive/*elder, n->path is /naive/, which means conflict with /naive/:whatever
        if(n->path.size() > 0 && n->path.back() == '/')
        {
          res = "wild conflict with root";
          return;
        }

        start -= 1;
        // wild must be /*mo not /elder*mo
        if(path[start] != '/')
        {
          res = "wild must be /*xx not /xx*xx";
          return;
        }

        // not including '/'
        n->path = path.substr(0, start);
        auto child = new Node{};
        child->wildcard = true;
        child->type = WILD;
        n->children.push_back(child);
        n->seqs.push_back('/');
        n = child;

        // add name (wildcard)
        child = new Node{};
        child->path = path.substr(start); // maybe /*mo
        child->type = WILD;
        child->handler = *handler;
        n->children.push_back(child);

        return;
      }
      else
      {
        res = "something went wrong, that's impossible";
        return;
      }
    }
    n->path = path;
    n->handler = *handler;
  }

  static auto get_impl(Node* n, std::string path) -> std::tuple<Handler, Pairs, bool>
  {
    Pairs args{};
  next:
    for(;;)
    {
      auto prefix = n->path;
      if(path.size() > prefix.size())
      {
        size_t i = common_prefix(prefix, path);
        if(i == prefix.size())
        {
          path = path.substr(i);

          if(!n->wildcard)
          {
            auto c = path[0];
            for(size_t j = 0; j < n->seqs.size(); ++j)
            {
              if(c == n->seqs[j])
              {
                n = n->children[j];
                goto next;
              }
            }

            return {Handler{}, Pairs{}, false};
          }

          // wildcard
          n = n->children[0];

          if(n->type == PARAM)
          {
            // extract params
            size_t j = 0;
            while(j < path.size() && path[j] != '/')
            {
              ++j;
            }
            // excluding ':'
            args.try_emplace(n->path.substr(1), path.substr(0, j));

            // find param till the end of path
            if(j < path.size())
            {
              if(!n->children.empty())
              {
                path = path.substr(j);
                n = n->children[0];
                goto next;
              }

              return {Handler{}, Pairs{}, false};
            }

            if(n->handler)
            {
              return {n->handler, args, true};
            }

            // don't allow empty wildcard, if route is /foo/:bar, path is /foo or /foo/
            // it will not match
            return {Handler{}, Pairs{}, false};
          }
          else if(n->type == WILD)
          {
            // n->path maybe /*mo
            args.try_emplace(n->path.substr(2), path.substr(1));
            return {n->handler, args, true};
          }
          else
          {
            // TODO: logging
            //  "invalid route"
            break;
          }
        }
      }
      else if(path == prefix)
      {
        if(n->handler)
        {
          return {n->handler, args, true};
        }
      }

      break;
    }
    return {Handler{}, Pairs{}, false};
  }

  static size_t common_prefix(const std::string& l, const std::string& r)
  {
    size_t n = std::min(l.size(), r.size());
    size_t i = 0;
    while(i < n && l[i] == r[i])
    {
      ++i;
    }
    return i;
  }

  static auto find_wildcard(std::string& path) -> std::tuple<std::string, int, bool>
  {
    auto valid = false;
    for(size_t start = 0; start < path.size(); ++start)
    {
      auto c = path[start];
      if(c != ':' && c != '*')
      {
        continue;
      }

      valid = true;
      // name after ':' or '*' till '/'
      auto name = path.substr(start + 1);
      for(size_t i = 0; i < name.size(); ++i)
      {
        auto nc = name[i];
        // name is end(i.e., *name/ or :name/)
        if(nc == '/')
        {
          return {path.substr(start, start + i), start, valid};
        }
        // validcard can contain only one of : or *
        if(nc == ':' || nc == '*')
        {
          valid = false;
        }
      }
      // invalid wildcard
      return {path.substr(start), start, valid};
    }
    // no wildcard found
    return {""s, -1, false};
  }
};

#endif // ROUTER_H_
