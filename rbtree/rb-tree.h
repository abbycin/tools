/***********************************************
        File Name: rb-tree.h
        Author: Abby Cin
        Mail: abbytsing@gmail.com
        Created Time: 12/24/16 2:17 PM
***********************************************/

#ifndef RED_BLACK_TREE_H_
#define RED_BLACK_TREE_H_

#ifdef DEBUG
#include <iostream>
#endif

namespace nm
{
  template<typename Key, typename Val, typename Cmp = std::less<Key>>
  class RBtree
  {
    private:
      enum Color
      {
        RED,
        BLACK
      };
      struct node
      {
        node* parent;
        node* lhs;
        node* rhs;
        Color color;
        Key key;
        Val val;
        node()
          : parent(nullptr), lhs(nullptr),
            rhs(nullptr), color(RED),
            key(), val()
        {}
        node(const Key& k, const Val& v)
          : parent(nullptr), lhs(nullptr),
            rhs(nullptr), color(RED), key(k), val(v)
        {}
      };
      struct rbnode
      {
        node* nil;
        node* root;
        rbnode()
          : nil(new node), root(nil)
        {
          nil->color = BLACK;
        }
        ~rbnode()
        {
          delete nil;
          nil = nullptr;
        }
      };

      rbnode* root_;
      size_t size_;
      Cmp cmp_;

    public:
      class iterator
      {
        public:
          iterator()
            : data_(nullptr)
          {}
          ~iterator() = default;

          bool operator== (const iterator& rhs)
          {
            return data_ == rhs.data_;
          }

          explicit operator bool () const
          {
            return data_ != nullptr;
          }

          bool operator! () const
          {
            return data_ == nullptr;
          }

          const Val& operator* () const
          {
            return data_->val;
          }

        protected:
          friend class RBtree;

          node* data() const
          {
            return data_;
          }
          iterator(node* x)
            : data_(x)
          {}

        private:
          node* data_;
      };

      RBtree()
        : root_(new rbnode), size_(0), cmp_()
      {
      }
      RBtree(const std::initializer_list<std::pair<Key, Val>>& rhs)
        : RBtree()
      {
        for(const auto& x: rhs)
          insert(x);
      }
      ~RBtree()
      {
        this->clear(root_, root_->root);
        delete root_;
        root_ = nullptr;
#ifdef DEBUG
        std::cout << "---------------------------------------------\n";
#endif
      }

      bool insert(const std::pair<Key, Val>& rhs)
      {
        return insert(rhs.first, rhs.second);
      }

      bool insert(const Key& key, const Val& val)
      {
        bool res = this->insert(root_, new node(key, val));
        if(res)
          ++size_;
        return res;
      }

      bool remove(const Key& key)
      {
        node* res = this->find(root_, key);
        if(res == nullptr)
          return false;
        this->remove(root_, res);
        --size_;
        return true;
      }

      bool remove(const iterator& iter)
      {
        if(!iter)
          return false;
        this->remove(root_, iter.data());
        return true;
      }

      iterator get(const Key& key)
      {
        return iterator{this->find(root_, key)};
      }

      size_t size() const
      {
        return size_;
      }

      void clear()
      {
        this->clear(root_);
      }

    private:

      /*
       *      x                     y
       *     /  \                  /  \
       *    a    y      =>        x    c
       *        / \              / \
       *       b    c           a   b
       *
       */

      void left_rotate(rbnode* t, node* x)
      {
        node* y = x->rhs;
        x->rhs = y->lhs;

        if(y->lhs != t->nil)
        {
          y->lhs->parent = x;
        }
        y->parent = x->parent;
        if(x->parent == t->nil)
          t->root = y;
        else
        {
          if(x == x->parent->lhs)
            x->parent->lhs = y;
          else
            x->parent->rhs = y;
        }
        y->lhs = x;
        x->parent = y;
      }

      /*
       *
       *      y               x
       *     / \             / \
       *    x   c     =>    a   y
       *   / \                 / \
       *  a   b               b   c
       *
       */

      void right_rotate(rbnode* t, node* y)
      {
        node* x = y->lhs;
        y->lhs = x->rhs;

        if(x->rhs != t->nil)
        {
          x->rhs->parent = y;
        }
        x->parent = y->parent;
        if(y->parent == t->nil)
          t->root = x;
        else
        {
          if(y == y->parent->lhs)
            y->parent->lhs = x;
          else
            y->parent->rhs = x;
        }
        x->rhs = y;
        y->parent = x;
      }

      node* find_min(rbnode* t, node* x)
      {
        while(x->lhs != t->nil)
        {
          x = x->lhs;
        }
        return x;
      }

      node* successor(rbnode* t, node* x)
      {
        if(x->rhs != t->nil)
        {
          x = find_min(t, x->rhs);
        }
        else
        {
          node* tmp = x->parent;
          while(tmp != t->nil && x == tmp->rhs)
          {
            x = tmp;
            tmp = x->parent;
          }
        }
        return x;
      }

      void transplant(rbnode* t, node* u, node* v)
      {
        if(u->parent == t->nil)
        {
          t->root = v;
        }
        else if(u == u->parent->lhs)
        {
          u->parent->lhs = v;
        }
        else
        {
          u->parent->rhs = v;
        }
        v->parent = u->parent;
      }

      void insert_fixup(rbnode* t, node* x)
      {
        node* y = nullptr;
        while(x->parent->color == RED)
        {
          if(x->parent == x->parent->parent->lhs)
          {
            y = x->parent->parent->rhs;
            if(y->color == RED) // a red node can't have red child
            {
              /*
               *  BEFORE INSERT, THE TREE IS SATISFY RED-BLACK TREE'S PROPERTIES
               *  THUS, ppa IS BLACK, AND WE DON'T CARE x IS LEFT CHILD OR RIGHT
               *  CHILD.
               *        ppa(black)                ppa(red)
               *        /         \               /       \
               *      pa(red)     y(red)  =>    pa(black)   y(black)
               *        |                         |
               *       x(red)                     x(red)
               *
               *
               *
               *            x(ppa: red)               black heights of x(ppa) are
               *            /         \               the same(left: 2, right: 2)
               *   =>      pa(black)   y(black)   =>
               *            |
               *           old x (red)
               */
              x->parent->color = BLACK;
              y->color = BLACK;
              x->parent->parent->color = RED;
              x = x->parent->parent; // fix-up till root, go next loop
            }
            else
            {
              if(x == x->parent->rhs)
              {
                x = x->parent;
                left_rotate(t, x);
              }
              x->parent->color = BLACK;
              x->parent->parent->color = RED;
              right_rotate(t, x->parent->parent);
            }
          }
          else // mirror operation
          {
            y = x->parent->parent->lhs;
            if(y->color == RED)
            {
              x->parent->color = BLACK;
              y->color = BLACK;
              x->parent->parent->color = RED;
              x = x->parent->parent;
            }
            else
            {
              if(x == x->parent->lhs)
              {
                x = x->parent;
                right_rotate(t, x);
              }
              x->parent->color = BLACK;
              x->parent->parent->color = RED;
              left_rotate(t, x->parent->parent);
            }
          }
        }
        t->root->color = BLACK;
      }

      void delete_fixup(rbnode* t, node* x)
      {
        node* y = nullptr;
        while(x != t->root && x->color == BLACK)
        {
          if(x == x->parent->lhs)
          {
            y = x->parent->rhs;
            if(y->color == RED)
            {
              y->color = BLACK;
              x->parent->color = RED;
              left_rotate(t, x->parent);
              y = x->parent->rhs;
            }
            if(y->lhs->color == BLACK && y->rhs->color == BLACK)
            {
              y->color = RED;
              x = x->parent;
            }
            else
            {
              if(y->rhs->color == BLACK)
              {
                y->lhs->color = BLACK;
                y->color = RED;
                right_rotate(t, y);
                y = x->parent->rhs;
              }
              y->color = x->parent->color;
              x->parent->color = BLACK;
              y->rhs->color = BLACK;
              left_rotate(t, x->parent);
              x = t->root;
            }
          }
          else // mirror operation
          {
            y = x->parent->lhs;
            if(y->color == RED)
            {
              y->color = BLACK;
              x->parent->color = RED;
              right_rotate(t, x->parent);
              y = x->parent->lhs;
            }
            if(y->lhs->color == BLACK && y->rhs->color == BLACK)
            {
              y->color = RED;
              x = x->parent;
            }
            else
            {
              if(y->lhs->color == BLACK)
              {
                y->rhs->color = BLACK;
                y->color = RED;
                left_rotate(t, y);
                y = x->parent->lhs;
              }
              y->color = x->parent->color;
              x->parent->color = BLACK;
              y->rhs->color = BLACK;
              right_rotate(t, x->parent);
              x = t->root;
            }
          }
        }
        x->color = BLACK;
      }

      bool insert(rbnode* t, node* z)
      {
        node* x = t->root;
        node* y = t->nil;
        while(x != t->nil)
        {
          y = x;
          if(cmp_(z->key, x->key))
          {
            x = x->lhs;
          }
          else if(cmp_(x->key, z->key))
          {
            x = x->rhs;
          }
          else
          {
            x->val = z->val; // replace old value
            delete z;
            z = nullptr;
            return false;
          }
        }
        z->parent = y;
        if(y == t->nil)
        {
          t->root = z;
        }
        else if(cmp_(z->key, y->key))
        {
          y->lhs = z;
        }
        else
        {
          y->rhs = z;
        }
        z->lhs = t->nil;
        z->rhs = t->nil;
        insert_fixup(t, z);
        return true;
      }

      void remove(rbnode* t, node* z)
      {
        node* y = z;
        node* x = nullptr;
        Color color = y->color;
        if(z->lhs == t->nil)
        {
          x = z->rhs;
          transplant(t, z, z->rhs);
        }
        else if(z->lhs == t->nil)
        {
          x = z->lhs;
          transplant(t, z, z->lhs);
        }
        else
        {
          y = successor(t, z);
          color = y->color;
          x = y->rhs;
          if(y->parent == z)
          {
            x->parent = y;
          }
          else
          {
            transplant(t, y, y->rhs);
            y->rhs = z->rhs;
            y->lhs->parent = y;
          }
          transplant(t, z, y);
          y->lhs = z->lhs;
          y->lhs->parent = y;
          y->color = z->color;
        }
        delete z;
        z = nullptr;
        if(color == BLACK)
          delete_fixup(t, x);
      }

      node* find(rbnode* t, const Key& key)
      {
        node* tmp = t->root;
        while(tmp != t->nil)
        {
          if(cmp_(tmp->key, key))
            tmp = tmp->rhs;
          else if(cmp_(key, tmp->key))
            tmp = tmp->lhs;
          else
            return tmp;
        }
        return nullptr;
      }

      void clear(rbnode* t, node* root)
      {
        node* tmp = root;
        if(tmp != t->nil)
        {
          clear(t, tmp->lhs);
          clear(t, tmp->rhs);
#ifdef DEBUG
          std::cout << tmp->key << " => "
            << (tmp->color == RED ? "\e[31mred\e[0m" : "\e[7mblack\e[0m")
            << (tmp->parent == t->nil ? " (\e[32mroot\e[0m)" : "")
            << std::endl;
#endif
          delete tmp;
          tmp = nullptr;
        }
      }
  };
}

#endif //RED_BLACK_TREE_H_
