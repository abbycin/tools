/*********************************************************
          File Name: functor_map.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 07 Apr 2018 03:43:56 PM CST
**********************************************************/

#include <iostream>
#include <map>
#include "../meta/function_traits.h"

namespace nm
{
  template<typename R> struct call_impl
  {
    template<typename F, typename Arg>
    static R call(F&& f, Arg& args)
    {
      R res;
      std::forward<F>(f)(&args, &res);
      return res;
    }

    template<typename F, typename TupleType, size_t... Is>
    static void invoke(R* res, F&& f, TupleType& args, std::index_sequence<Is...>)
    {
      *res = std::forward<F>(f)(std::forward<std::tuple_element_t<Is, std::remove_reference_t<TupleType>>>(std::get<Is>(args))...);
    }
  };
  template<> struct call_impl<void>
  {
    template<typename F, typename Arg>
    static void call(F&& f, Arg& args)
    {
      std::forward<F>(f)(&args, nullptr);
    }

    template<typename F, typename TupleType, size_t... Is>
    static void invoke(void*, F&& f, TupleType& args, std::index_sequence<Is...>)
    {
      std::forward<F>(f)(std::forward<std::tuple_element_t<Is, std::remove_reference_t<TupleType>>>(std::get<Is>(args))...);
    }
  };

  class FunctorMap
  {
    public:
      using Fp = std::function<void(void*, void*)>;

      FunctorMap() = default;

      template<typename F>
      void bind(const std::string& sig, F&& f)
      {
        auto fp = [f = std::forward<F>(f)](void* args, void* res) {
          using arg_t = typename meta::Inspector<F>::arg_t;
          using res_t = typename meta::Inspector<F>::res_t;
          using indices = std::make_index_sequence<std::tuple_size<arg_t>::value>;
          arg_t& arg = *static_cast<arg_t*>(args);
          res_t* tmp = static_cast<res_t*>(res);
          call_impl<res_t>::invoke(tmp, f, arg, indices{});
        };
        functors_[sig] = std::move(fp);
      }

      template<typename R, typename... Args>
      R call(const std::string& sig, Args&&... args)
      {
        auto params = std::make_tuple(std::forward<Args>(args)...);
        if(functors_.find(sig) == functors_.end())
        {
          return R();
        }
        auto& fp = functors_[sig];
        return call_impl<R>::call(fp, params);
      }

    private:
      std::map<std::string, Fp> functors_;
  };
}

std::string to_str(int x)
{
  return std::to_string(x);
}

struct Foo
{
  static std::string foo() { return __func__; }
  int operator() (int x) { return x * x; }
};

int main()
{
  using std::cout;
  using std::string;
  nm::FunctorMap fm;

  // bind
  fm.bind("lambda", [](int lhs, int rhs) { return lhs + rhs; });
  fm.bind("lambda2", [](const string& lhs, const string& rhs) { return lhs + rhs; });
  fm.bind("void_arg_res", [] { std::cout << "void\n"; });
  fm.bind("void_arg", [] { return "666"; });
  fm.bind("free_function", to_str);
  fm.bind("static_member_function", &Foo::foo);
  Foo foo;
  fm.bind("capture", [&foo](int x) { return foo(x); });

  fm.call<string>("none exist");
  cout << fm.call<int>("lambda", 1, 2) << '\n';
  cout << fm.call<string>("lambda2", string{"hello "}, string{"world"}) << '\n';
  fm.call<void>("void_arg_res");
  cout << fm.call<const char*>("void_arg") << '\n';
  cout << fm.call<string>("free_function", 233) << '\n';
  cout << fm.call<string>("static_member_function") << '\n';
  cout << fm.call<int>("capture", 16) << '\n';
}
