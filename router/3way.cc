/***********************************************
        File Name: 3way.cc
        Author: Abby Cin
        Mail: abbytsing@gmail.com
        Created Time: 3/14/20 16:50 PM
***********************************************/

#include <iostream>
#include <string_view>
#include <list>

class TWay
{
public:
  struct Node
  {
    bool word = false;
    Node* left;
    Node* mid;
    Node* right;
    char c;
  };

  TWay() = default;

  ~TWay()
  {
    if(root_)
    {
      std::list<Node*> tmp;
      drop_in(tmp, root_);
      while(!tmp.empty())
      {
        Node* n = tmp.front();
        tmp.pop_front();
        drop_in(tmp, n);
        delete n;
      }
      delete root_;
    }
  }

  void put(std::string_view s) { root_ = put_impl(root_, s, 0); }

  bool has_prefix(std::string_view s)
  {
    Node* n = get_impl(root_, s, 0);
    return n != nullptr;
  }

  bool has_word(std::string_view s)
  {
    Node* n = get_impl(root_, s, 0);
    return n != nullptr && n->word;
  }

private:
  Node* root_;

  Node* put_impl(Node* n, std::string_view s, size_t d)
  {
    char c = s[d];
    if(n == nullptr)
    {
      n = new Node{};
      n->c = c;
    }
    if(c < n->c)
    {
      n->left = put_impl(n->left, s, d);
    }
    else if(c > n->c)
    {
      n->right = put_impl(n->right, s, d);
    }
    else if(d < s.size() - 1)
    {
      n->mid = put_impl(n->mid, s, d + 1);
    }
    else
    {
      n->word = true;
    }
    return n;
  }

  Node* get_impl(Node* n, std::string_view s, size_t d)
  {
    if(n == nullptr)
    {
      return nullptr;
    }
    char c = s[d];
    if(c < n->c)
    {
      return get_impl(n->left, s, d);
    }
    else if(c > n->c)
    {
      return get_impl(n->right, s, d);
    }
    else if(d < s.size() - 1)
    {
      return get_impl(n->mid, s, d + 1);
    }
    else
    {
      return n;
    }
  }

  static void drop_in(std::list<Node*>& l, Node* n)
  {
    if(n->left)
    {
      l.push_back(n->left);
    }
    if(n->mid)
    {
      l.push_back(n->mid);
    }
    if(n->right)
    {
      l.push_back(n->right);
    }
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
  TWay r{};
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