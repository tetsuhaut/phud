#pragma once

// Got from https://github.com/SuperV1234/vittorioromeo.info/blob/master/extra/passing_functions_to_functions/FunctionView.hpp


#include <type_traits>
#include <functional>

template <typename...>
using void_t = void;

template <class T, class R = void, class = void>
struct is_callable : std::false_type {};

template <class T>
struct is_callable<T, void, void_t<std::result_of_t<T>>> : std::true_type {};

template <class T, class R>
struct is_callable<T, R, void_t<std::result_of_t<T>>>
  : std::is_convertible<std::result_of_t<T>, R> {};

template <typename TSignature>
struct signature_helper;

template <typename TReturn, typename... TArgs>
struct signature_helper<TReturn(TArgs...)> {
  using fn_ptr_type = TReturn(*)(TArgs...);
};

template <typename TSignature>
using fn_ptr = typename signature_helper<TSignature>::fn_ptr_type;

template <typename T>
struct dependent_false : std::false_type {};

template <typename TSignature>
class FunctionView;

template <typename TReturn, typename... TArgs>
class [[nodiscard]] FunctionView<TReturn(TArgs...)> final {
private:
  using signature_type = TReturn(void*, TArgs...);

  void* m_ptr;
  TReturn(*m_erasedFn)(void*, TArgs...);

public:
  template < typename T, typename = std::enable_if_t < std::is_callable<T&(TArgs...)> {}
  && !std::is_same<std::decay_t<T>, FunctionView> {} >>
      FunctionView(T&& x) noexcept : m_ptr {(void*)std::addressof(x)} {
    m_erasedFn = [](void* ptr, TArgs... xs) -> TReturn {
      return (*reinterpret_cast<std::add_pointer_t<T>>(ptr))(
        std::forward<TArgs>(xs)...);
    };
  }

  decltype(auto) operator()(TArgs... xs) const
  noexcept(noexcept(m_erasedFn(m_ptr, std::forward<TArgs>(xs)...))) {
    return m_erasedFn(m_ptr, std::forward<TArgs>(xs)...);
  }
};