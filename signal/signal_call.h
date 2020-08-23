/*********************************************************
          File Name:signal_call.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Wed 11 May 2016 09:56:38 PM CST
**********************************************************/

#ifndef SIGNAL_CLASS_H_
#define SIGNAL_CLASS_H_

#include <list>
#include <utility>
#include <mutex>

namespace nm
{
  namespace
  {
    struct thread_safe
    {
    };
    struct non_thread_safe
    {
    };
    struct member_function
    {
    };
    struct non_member_function
    {
    };

    template<typename T>
    struct Trait;

    // specialization
    template<typename Obj, typename Res, typename... Args>
    struct Trait<Res (Obj::*)(Args...)>
    {
      typedef thread_safe safe_type;
      typedef member_function func_type;
      typedef Obj Object;
      typedef Res (Obj::*Func)(Args...);
      static void call(Func f, Obj* o, Args&&... args) { (o->*f)(std::forward<Args>(args)...); }
    };

    // specialization
    template<typename Res, typename... Args>
    struct Trait<Res (*)(Args...)>
    {
      typedef thread_safe safe_type;
      typedef non_member_function func_type;
      typedef Res (*Func)(Args...);
      typedef Res Object;
      static void call(Func f, Object*, Args&&... args) { (*f)(std::forward<Args>(args)...); }
    };
  }

  template<typename T>
  using SafeTrait = Trait<T>;
  // template<typename T> using UnSafeTrait = SomeTrait<T>;

  template<typename T, template<typename> class Default = SafeTrait>
  class Signal
  {
  public:
    using Handle = typename std::list<T>::iterator;
    Signal() { obj = nullptr; }

    Signal(const Signal&) = delete;

    Signal& operator=(const Signal&) = delete;

    ~Signal()
    {
      callable.clear();
      obj = nullptr;
    }

    template<typename Func>
    Handle connect(typename Trait<Func>::Object* recv, Func slot)
    {
      obj = recv;
      callable.push_back(slot);
      return --callable.end();
    }

    template<typename Func>
    Handle connect(Func&& f)
    {
      callable.push_back(f);
      return --callable.end();
    }

    void disconnect(Handle iter) { callable.erase(iter); }

    void disconnect(T fp)
    {
      for(auto iter = callable.begin(); iter != callable.end();)
      {
        if(*iter == fp)
          iter = callable.erase(iter);
        else
          ++iter;
      }
    }

    template<typename... Args>
    void emit(Args&&... args)
    {
      this->exec(typename Default<T>::safe_type(), std::forward<Args>(args)...);
    }

  private:
    typename Default<T>::Object* obj;
    std::list<T> callable;
    std::mutex l;

    template<typename... Args>
    void do_exec(member_function, Args&&... args)
    {
      for(auto& x: callable)
        Default<T>::call(x, obj, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void do_exec(non_member_function, Args&&... args)
    {
      for(auto& x: callable)
        Default<T>::call(x, nullptr, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void exec(thread_safe, Args&&... args)
    {
      std::lock_guard<std::mutex> lock(l);
      do_exec(typename Default<T>::func_type(), std::forward<Args>(args)...);
    }

    template<typename... Args>
    void exec(non_thread_safe, Args&&... args)
    {
      do_exec(typename Default<T>::func_type(), std::forward<Args>(args)...);
    }
  };
}

#endif
