/***********************************************
        File Name: radix.cc
        Author: Abby Cin
        Mail: abbytsing@gmail.com
        Created Time: 3/12/20 8:44 PM
***********************************************/

#include <string>
#include <list>
#include <vector>
#include <iostream>
#include <algorithm>

struct Node
{
  bool word;
  std::string path;
  std::vector<Node*> children;
  std::vector<char> indices;

  Node(const std::string_view& s) : word{false}, path{s.data(), s.size()}, children{}, indices{} {}
};

class Router
{
public:
  Router() : root_{{}} {}

  ~Router()
  {
    std::cerr << std::boolalpha;
    std::cerr << __LINE__ << "-->> " << root_.path << ", " << root_.word << '\n';
    std::list<Node*> tmp;
    for(auto n: root_.children)
    {
      std::cerr << __LINE__ << "-->> " << n->path << ", " << n->word << '\n';
      tmp.push_back(n);
    }

    while(!tmp.empty())
    {
      Node* n = tmp.front();
      tmp.pop_front();

      for(auto c: n->children)
      {
        std::cerr << __LINE__ << "-->> " << c->path << ", " << c->word << '\n';
        tmp.push_back(c);
      }

      delete n;
    }
  }

  void put(const std::string& s)
  {
    Node* n = &root_;
    auto path = s;
    put_impl(n, path);
  }

  bool has_word(const std::string& s)
  {
    bool res = false;
    Node* n = &root_;
    get_impl(n, s, res, true);
    return res;
  }

  bool has_prefix(const std::string& s)
  {
    bool res = false;
    Node* n = &root_;
    get_impl(n, s, res);
    return res;
  }

private:
  Node root_;

  void put_impl(Node* n, std::string_view path)
  {
    if(n->path.empty() && n->indices.empty())
    {
      n->word = true;
      n->path = path;
      return;
    }
    size_t i = common_prefix(n->path, path);
    /*
    // put dog, doges
            [dog]
            /
         [es]

    // put doll
            [do]
            /  \
          [g]   [ll]
          /
        [es]
    */
    if(i < n->path.size())
    {
      // split edge
      Node* c = new Node{n->path.substr(i)};
      c->children = n->children;
      c->indices = n->indices;
      c->word = true;

      n->children.clear();
      n->children.push_back(c);
      n->indices.clear();
      n->indices.push_back(n->path[i]);
      n->path = n->path.substr(0, i);
      n->word = false;
    }

    if(i < path.size())
    {
      path = path.substr(i);
      for(size_t idx = 0; idx < n->indices.size(); ++idx)
      {
        if(n->indices[idx] == path[0])
        {
          n = n->children[idx];
          put_impl(n, path);
          return;
        }
      }
      Node* c = new Node{path};
      c->word = true;
      n->children.push_back(c);
      n->indices.push_back(path[0]);
    }
  }

  void get_impl(Node* n, std::string path, bool& res, bool full = false)
  {
    if(path.size() > n->path.size())
    {
      if(path.substr(0, n->path.size()) == n->path)
      {
        path = path.substr(n->path.size());
        for(size_t idx = 0; idx < n->indices.size(); ++idx)
        {
          if(n->indices[idx] == path[0])
          {
            n = n->children[idx];
            get_impl(n, path, res, full);
          }
        }
      }
    }
    else if(path.size() < n->path.size())
    {
      // doge, did
      int i = common_prefix(n->path, path);
      if(i < path.size())
      {
        path = path.substr(i);
        for(size_t idx = 0; idx < n->indices.size(); ++idx)
        {
          if(n->indices[idx] == path[0])
          {
            n = n->children[idx];
            get_impl(n, path, res, full);
          }
        }
      }
      else
      {
        if(!full)
        {
          res = true;
        }
      }
    }
    else if(path == n->path)
    {
      if(full)
      {
        res = n->word;
      }
      else
      {
        res = true;
      }
    }
  }

  static size_t common_prefix(const std::string_view& l, const std::string_view& r)
  {
    int m = std::min(l.size(), r.size());
    size_t i = 0;
    for(; i < m && l[i] == r[i];)
    {
      ++i;
    }
    return i;
  }
};

#define put(x) \
  do \
  { \
    std::cerr << "put: " << x << '\n'; \
    r.put(x); \
  } while(0)
#define has_prefix(x) \
  do \
  { \
    std::cerr << "has_prefix: " << x << "? " << (r.has_prefix(x) ? "true" : "false") << '\n'; \
  } while(0)
#define has_word(x) \
  do \
  { \
    std::cerr << "has_word: " << x << "? " << (r.has_word(x) ? "true" : "false") << '\n'; \
  } while(0)

int main()
{
  Router r{};
  put("dog");
  put("doges");
  put("did");
  put("doll");

  has_prefix("d");
  has_prefix("do");
  has_prefix("dog");
  has_prefix("doge");
  has_prefix("doges");
  has_prefix("di");
  has_prefix("did");
  has_prefix("dol");
  has_prefix("doll");

  has_word("d");
  has_word("do");
  has_word("dog");
  has_word("doge");
  has_word("doges");
  has_word("di");
  has_word("did");
  has_word("dol");
  has_word("doll");
}