/*********************************************************
          File Name: optional.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Thu 04 Apr 2019 01:42:41 PM DST
**********************************************************/

#ifndef OPTIONAL_H_
#define OPTIONAL_H_

#include <type_traits>
#include <stdexcept>

template<typename T>
class Optional
{
public:
  Optional() : data_{}, valid_{false}
  {

  }

  ~Optional()
  {
    this->clear();
  }

  explicit Optional(const T& t) : Optional{}
  {
    this->assign(t);
  }

  explicit Optional(T&& t) noexcept : Optional{}
  {
 
    this->move(std::move(t));
  }

  template<typename... Args>
  explicit Optional(Args&&... args) : Optional{}
  {
    this->emplace(std::forward<Args>(args)...);
  }

  Optional(const Optional& rhs) : Optional{}
  {
    *this = rhs;
  }

  Optional(Optional&& rhs) noexcept : Optional{}
  {
    *this = std::move(rhs);
  }

  Optional& operator= (const T& t)
  {
    this->assign(t);
    return *this;
  }

  Optional& operator= (T&& t)
  {
    this->move(std::move(t));
    return *this;
  }

  Optional& operator= (const Optional& rhs)
  {
    if(this != &rhs && rhs.valid_)
    {
      this->assign(*rhs);
    }
    return *this;
  }

  Optional& operator= (Optional&& rhs) noexcept
  {
    if(this != &rhs && rhs.valid_)
    {
      this->move(std::move(*rhs));
      rhs.clear();
      rhs.valid_ = false;
    }
    return *this;
  }

  template<typename... Args, typename R = typename std::enable_if<std::is_constructible<T, Args...>::value>::type>
  R emplace(Args&& ... args)
  {
    if(!valid_)
    {
      new(this->raw()) T{std::forward<Args>(args)...};
      valid_ = true;
    }
  }

  explicit operator bool()
  {
    return valid_;
  }

  operator T() = delete;

  T& operator* ()
  {
    return *operator->();
  }

  const T& operator* () const
  {
    return *operator->();
  }

  T* operator-> ()
  {
    if(!valid_)
    {
      throw std::logic_error("invalid optional");
    }
    return static_cast<T*>(this->raw());
  }

  const T* operator->() const
  {
    if (!valid_)
    {
      throw std::logic_error("invalid optional");
    }
    return static_cast<const T*>(this->raw());
  }

private:
  using data_t = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
  data_t data_;
  bool valid_;

  void assign(const T& t)
  {
    this->clear();
    valid_ = true;
    new(this->raw()) T{t};
  }

  void move(T&& t)
  {
    this->clear();
    valid_ = true;
    new(this->raw()) T{std::move(t)};
  }

  void* raw()
  {
    return reinterpret_cast<void*>(&data_);
  }

  const void* raw() const
  {
    return reinterpret_cast<const void*>(&data_);
  }

  void clear()
  {
    if(valid_)
    {
      static_cast<T*>(this->raw())->~T();
    }
  }
};

#endif // OPTIONAL_H_
