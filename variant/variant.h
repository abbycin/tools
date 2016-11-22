/*********************************************************
          File Name: fakevariant.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Mon 21 Nov 2016 09:32:37 PM CST
**********************************************************/

#ifndef VARIANT_H_
#define VARIANT_H_

#include "util.h"
#include <functional>

namespace nm
{
  template<typename U, typename... Args>
  class FakeVariant
  {
    public:
      template<typename T> void set(T value)
      {
        data_.invisible<T>::value = new T(value);
      }
      template<typename T, typename... Paras> void set(Paras&&... paras)
      {
        data_.invisible<T>::value = new T(std::forward<Paras>(paras)...);
      }
      template<typename T> T& get()
      {
        return *data_.invisible<T>::value;
      }
    private:
      template<typename T>
      struct invisible
      {
        T *value;
        invisible() : value{nullptr}
        {}
        ~invisible()
        {
          if(value)
          {
            delete value;
          }
        }
      };
      meta::GenClasses<typename meta::Unique<typename meta::GenList<U, Args...>::type>::type, invisible> data_;
  };
}

#endif
