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
      template<typename T> bool set(const T& value)
      {
        return data_.invisible<T>::set(value);
      }
      template<typename T, typename... Paras>
      bool set(Paras&&... paras)
      {
        return data_.invisible<T>::set(std::forward<Paras>(paras)...);
      }
      template<typename T> const T& get() const
      {
        return data_.invisible<T>::get();
      }
      template<typename T> bool valid() const
      {
        return data_.invisible<T>::valid();
      }
    private:
      template<typename T>
      class invisible
      {
        public:
          invisible()
            : value_{nullptr}
          {}
          ~invisible()
          {
            clear();
          }
          template<typename... Paras>
          bool set(Paras&&... paras)
          {
            bool res = valid();
            clear();
            value_ = new T(std::forward<Paras>(paras)...);
            return !res;
          }
          const T& get() const
          {
            return *value_;
          }
          bool valid() const
          {
            return value_ != nullptr;
          }
        private:
          T* value_;
          void clear()
          {
            if(valid())
            {
              delete value_;
              value_ = nullptr;
            }
          }
      };
      meta::GenClasses<typename meta::Unique<typename meta::GenList<U, Args...>
        ::type>::type, invisible> data_;
  };
}

#endif
