/*********************************************************
          File Name: main.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sun 03 Jun 2018 10:24:50 AM CST
**********************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <type_traits>
#include <tuple>
#include <functional>

namespace meta
{
  template<typename> struct Inspector;
  template<typename R, typename... Args>
  struct Inspector<R(Args...)>
  {
    using arg_t = std::tuple<std::remove_reference_t<Args>...>;
    using res_t = R;
  };
  template<typename R, typename... Args>
  struct Inspector<R(*)(Args...)> : Inspector<R(Args...)> {};
  template<typename R, typename... Args>
  struct Inspector<R(&)(Args...)> : Inspector<R(Args...)> {};
  template<typename R, typename Object, typename... Args>
  struct Inspector<R(Object::*)(Args...)> : Inspector<R(Args...)> {};
  template<typename R, typename Object, typename... Args>
  struct Inspector<R(Object::*)(Args...) const> : Inspector<R(Args...)> {};
  template<typename R, typename Object, typename... Args>
  struct Inspector<R(Object::*)(Args...) volatile> : Inspector<R(Args...)> {};
  template<typename R, typename Object, typename... Args>
  struct Inspector<R(Object::*)(Args...) const volatile> : Inspector<R(Args...)> {};

  // functor like
  template<typename Lambda>
  struct Inspector : Inspector<decltype(&Lambda::operator())> {};
  template<typename Lambda>
  struct Inspector<Lambda&> : Inspector<decltype(&Lambda::operator())> {};
  template<typename Lambda>
  struct Inspector<Lambda&&> : Inspector<Lambda&> {};
}

extern "C" {
  void* switch_stack(void* obj, void* context, void* arg);
}

class Coroutine;

void call_member_function(Coroutine* obj, void* ctx);

class Coroutine
{
  public:
    friend void call_member_function(Coroutine*, void*);

    enum { Fixed_Stack_Size = 64 * 1024};

    class Iterator
    {
      public:
        Iterator()
          : co_{nullptr}
        {}

        Iterator(Coroutine* co)
          : co_{co}
        {}

        ~Iterator() {}

        bool operator== (const Iterator& rhs)
        {
          return this->co_ == rhs.co_;
        }

        bool operator!= (const Iterator& rhs)
        {
          return this->co_ != rhs.co_;
        }

        Iterator& operator++ ()
        {
          if(co_ == nullptr)
          {
            return *this;
          }
          if(co_->done_)
          {
            co_ = nullptr;
          }
          return *this;
        }

        Coroutine& operator* ()
        {
          return *co_;
        }

      private:
        Coroutine* co_;
    };

    Coroutine() = default;

    ~Coroutine()
    {
      free(stack_);
      stack_ = nullptr;
    }

    Coroutine(const Coroutine&) = delete;
    Coroutine(Coroutine&&) = delete;
    Coroutine& operator= (const Coroutine&) = delete;
    Coroutine& operator= (Coroutine&&) = delete;

    template<typename F, typename... Args>
    void start(F&& f, Args&&... args)
    {
      this->~Coroutine();
      stack_ = malloc(Fixed_Stack_Size);
      memset(stack_, 0, Fixed_Stack_Size);
      // call pop in `switch_stack` will grow rsp, here I reserve 8k space for push
      // and I don't check alignment, you can adjust by using std::align since C++11
      new_sp_ = (char*)stack_ + 8192;

      this->done_ = false;
      using arg_t = typename meta::Inspector<F>::arg_t;
      arg_ = new arg_t{std::forward<Args>(args)...};
      fp_ = [f = std::forward<F>(f)](void* arg) {
        arg_t* param = static_cast<arg_t*>(arg);
        std::apply(f, *param);
        delete param; // free `arg_`
        param = nullptr;
      };
      // skip 6 register r12~r15 and rbx rbp, assign next instruction to rip
      *((void**)((char*)new_sp_ + 6 * 8)) = (void*)call_member_function;
      new_sp_ = switch_stack(this, new_sp_, arg_);
    }

    template<typename T>
    void yield(T* t = nullptr)
    {
      res_ = t;
      new_sp_ = switch_stack(nullptr, new_sp_, nullptr);
    }

    void yield()
    {
      new_sp_ = switch_stack(nullptr, new_sp_, nullptr);
    }

    int resume()
    {
      if(done_)
      {
        return -1;
      }
      new_sp_ = switch_stack(nullptr, new_sp_, nullptr);
      return 0;
    }

    bool is_finished() { return done_; }

    template<typename T> const T& as() const
    {
      if(res_ == nullptr)
      {
        throw std::runtime_error("yield value is null");
      }
      return *static_cast<const T*>(res_);
    }

    Iterator begin()
    {
      return {this};
    }

    Iterator end()
    {
      return {};
    }

  private:
    void* new_sp_{nullptr};
    void* stack_{nullptr};
    bool done_{false};
    std::function<void(void*)> fp_;
    void* arg_{nullptr};
    const void* res_{nullptr};

    void done()
    {
      done_ = true;
      this->yield();
    }

    void agent(void* ctx)
    {
      new_sp_ = switch_stack(nullptr, ctx, nullptr);
      fp_(arg_);
      this->done();
    }
};

void call_member_function(Coroutine* obj, void* ctx)
{
  obj->agent(ctx);
}

void foo(Coroutine* co, const std::string& str)
{
  for(auto& c: str)
  {
    printf("%c", c);
    co->yield();
  }
  printf("\n");
}

int main()
{
  Coroutine co;
  co.start(foo, &co, "hello world");

  for(auto iter = co.begin(); iter != co.end(); ++iter)
  {
    co.resume();
  }

  co.start([&co](int x) {
    int first = 1;
    co.yield(&first);

    int second = 1;
    co.yield(&second);

    for(int i = 0; i < x; ++i)
    {
      int third = first + second;
      first = second;
      second = third;
      co.yield(&third);
    }
  }, 10);

  for(auto& _: co)
  {
    co.resume();
    if(!_.is_finished())
    {
      printf("%d\t", _.as<int>());
    }
  }
  printf("\n");
}
