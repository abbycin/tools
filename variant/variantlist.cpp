/*********************************************************
          File Name: variantlist.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 17 Jun 2017 09:18:19 AM CST
**********************************************************/

#include "variant.h"
#include <iostream>

namespace nm
{
  template<typename T, typename... Rest>
  class variant_list
  {
    private:
      struct Node
      {
        variant<T, Rest...> data;
        Node* next;
        template<typename U>
        Node(U&& u, Node* nxt)
          : data(std::forward<U>(u)), next(nxt)
        {}
      };
    public:
      class iterator
      {
        public:
          iterator(Node* n)
            : data_(n)
          {}

          iterator(const iterator& rhs)
          {
            data_ = rhs.data_;
          }

          iterator& operator= (const iterator& rhs)
          {
            if(this != &rhs)
            {
              data_ = rhs.data_;
            }
            return *this;
          }

          iterator& operator++ ()
          {
            data_ = data_->next;
            return *this;
          }

          bool operator== (const iterator& rhs)
          {
            return data_ == rhs.data_;
          }

          bool operator!= (const iterator& rhs)
          {
            return !(*this == rhs);
          }

          variant<T, Rest...>* operator-> ()
          {
            return &data_->data;
          }

          variant<T, Rest...>& operator* ()
          {
            return data_->data;
          }
        private:
          Node* data_;
      };
    public:
      variant_list() : root_(nullptr) {}
      ~variant_list() { this->clear(); }

      template<typename U> void push(U&& u)
      {
        if(root_ == nullptr)
        {
          root_ = new Node(std::forward<U>(u), nullptr);
          tail_ = root_;
        }
        else
        {
          auto tmp = new Node(std::forward<U>(u), nullptr);
          tail_->next = tmp;
          tail_ = tmp;
        }
      }

      iterator begin()
      {
        return iterator(root_);
      }

      iterator end()
      {
        return iterator(nullptr);
      }

      void clear()
      {
        Node* tmp = nullptr;
        while(root_)
        {
          tmp = root_->next;
          delete root_;
          root_ = tmp;
        }
        root_ = tail_ = nullptr;
      }

    private:
      Node* root_;
      Node* tail_;
  };
}

int main()
{
  nm::variant_list<int, double, std::string> l;
  l.push(233);
  l.push(3809.929);
  l.push("ha??");
  for(auto& x: l)
  {
    std::cout << x << '\n';
  }
}
