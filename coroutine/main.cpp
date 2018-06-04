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

template<typename> class Coroutine;

template<typename F>
void call_member_function(Coroutine<F>* obj, void* ctx, void* arg);

template<typename F>
class Coroutine
{
  public:
    template<typename Fp>
    friend void call_member_function(Coroutine<Fp>*, void*, void*);
    using arg_t = typename meta::Inspector<F>::arg_t;

    enum { Fixed_Stack_Size = 64 * 1024};
    Coroutine()
    {
      stack_ = malloc(Fixed_Stack_Size);
      // call pop in `switch_stack` will grow rsp, here I reserve 8k space for push
      // and I don't check alignment, you can adjust by using std::align since C++11
      new_sp_ = (char*)stack_ + 8192;
    }

    ~Coroutine()
    {
      free(stack_);
    }

    template<typename Fp, typename... Args>
    void start(Fp fp, Args... args)
    {
      static_assert(std::is_same_v<Fp, F>, "function type not match");
      using type = std::tuple<std::remove_reference_t<Args>...>;
      static_assert(std::is_same_v<type, arg_t>, "argument type not match");
      arg_ = type{args...};
      fp_ = fp;
      // skip 6 register r12~r15 and rbx rbp, assign next instruction to rip
      *((void**)((char*)new_sp_ + 6 * 8)) = (void*)call_member_function<F>;
      old_sp_ = switch_stack(this, new_sp_, &arg_);
    }

    void yield()
    {
      new_sp_ = switch_stack(nullptr, new_sp_, &arg_);
    }

    void resume()
    {
      old_sp_ = switch_stack(nullptr, old_sp_, &arg_);
    }

  private:
    void* old_sp_{nullptr};
    void* new_sp_{nullptr};
    void* stack_{nullptr};
    F fp_;
    typename meta::Inspector<F>::arg_t arg_;

    void agent(void* ctx, void* arg)
    {
      new_sp_ = switch_stack(this, ctx, arg);
      std::apply(fp_, arg_);
    }
};

template<typename Fp>
void call_member_function(Coroutine<Fp>* obj, void* ctx, void* arg)
{
  obj->agent(ctx, arg);
}

Coroutine<void(*)(std::string)> co;

void foo(std::string str)
{
  for(int i = 0; i < str.size(); ++i)
  {
    printf("%c", str.at(i));
    co.yield();
  }
  printf("\n");
}

int main()
{
  std::string str{"hello world"};
  co.start(foo, str);

  for(int i = 0; i < 5; ++i)
  {
    co.resume();
  }
}
