/***********************************************
        File Name: trie.cc
        Author: Abby Cin
        Mail: abbytsing@gmail.com
        Created Time: 3/12/20 8:40 PM
***********************************************/

#include <iostream>
#include <list>

class Trie
{
public:
  struct Node
  {
    bool ok;
    Node* children[26];

    Node() : ok{false}, children{} {}
  };

  Trie() = default;

  ~Trie()
  {
    std::list<Node*> tmp;
    for(auto c: root_.children)
    {
      if(c)
      {
        tmp.push_back(c);
      }
    }

    while(!tmp.empty())
    {
      Node* c = tmp.front();
      tmp.pop_front();
      for(auto cc: c->children)
      {
        if(cc)
        {
          tmp.push_back(cc);
        }
      }
      delete c;
    }
  }

  void put(const std::string_view& s)
  {
    Node* p = &root_;
    for(auto c: s)
    {
      if(!p->children[c - 'a'])
      {
        p->children[c - 'a'] = new Node{};
      }
      p = p->children[c - 'a'];
    }
    p->ok = true;
  }

  bool get(const std::string_view& s)
  {
    Node* p = find(s);
    return p != &root_ && p->ok;
  }

  bool has(const std::string_view& s) { return find(s) != &root_; }

private:
  Node root_;

  Node* find(const std::string_view& s)
  {
    Node* p = &root_;
    for(auto c: s)
    {
      if(p->children[c - 'a'])
      {
        p = p->children[c - 'a'];
      }
    }
    return p;
  }
};

int main()
{
  Trie t{};
  t.put("mo");
  t.put("moha");
  std::cerr << t.get("mo") << ", " << t.get("ha") << ", " << t.get("m") << ", " << t.has("ha") << ", " << t.has("moh");
}