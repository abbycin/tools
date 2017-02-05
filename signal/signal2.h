/*********************************************************
          File Name:signal.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sun 17 Jul 2016 07:34:00 PM CST
**********************************************************/

#ifndef SIGNAL_H_
#define SIGNAL_H_

#include <utility>
#include <list>
#include <functional>

namespace nm
{
  namespace
  {
    template<typename> struct sig_trait;
    template<typename Res, typename... Args>
    struct sig_trait<Res(Args...)>
    {
      using Func =  std::function<Res(void*, Args...)>;
      sig_trait(const sig_trait& ) = delete;
      sig_trait& operator= (const sig_trait&) = delete;
      sig_trait(sig_trait&& rhs)
      {
        obj = rhs.obj;
        callable = rhs.callable;
      }
      sig_trait(void* o, Func fp)
      {
        obj = o;
        callable = fp;
      }
      template<typename Obj, typename F>
      static sig_trait connect(Obj* o, F&& fp)
      {
        return {o, fp};
      }
      template<typename F>
      static sig_trait connect(F&& fp)
      {
        return {nullptr, fp};
      }
      void* obj;
      Func callable;
      template<typename... Paras>
      Res call(Paras&&... paras) const
      {
        return callable(obj, std::forward<Paras>(paras)...);
      }
    };
  }
  namespace signal
  {
    namespace
    {
      using std::list;
    }
    template<typename> class Signal;
    template<typename Res, typename... Paras>
    class Signal<Res(Paras...)>
    {
      public:
        using data_type = Res(Paras...);
        using handle_type = typename list<sig_trait<data_type>>::iterator;
        Signal(){}
        Signal(const Signal&) = delete;
        Signal& operator= (const Signal&) = delete;
        ~Signal(){}
        template<typename Obj, typename F>
        handle_type& connect(Obj* o, F&& f)
        {
          auto fp = [f](void* obj, Paras... paras)
          {
            return (static_cast<Obj*>(obj)->*f)(std::forward<Paras>(paras)...);
          };
          objs.push_back(sig_trait<data_type>::template connect(o, fp));
          return --objs.end();
        }
        template<typename F>
        handle_type& connect(F&& f)
        {
          auto fp = [f](void*, Paras... paras)
          {
            return f(std::forward<Paras>(paras)...);
          };
          objs.push_back(sig_trait<data_type>::connect(fp));
          return --objs.end();
        }
        void disconnect(handle_type& handle)
        {
          objs.erase(handle);
        }
        template<typename... Args>
        void emit(Args&&... args) const
        {
          for(auto& x: objs)
            x.call(std::forward<Args>(args)...);
        }
        std::size_t size() const
        {
          return objs.size();
        }
        bool empty() const
        {
          return objs.empty();
        }
      private:
        list<sig_trait<data_type>> objs;
    };
  }
}

#endif
