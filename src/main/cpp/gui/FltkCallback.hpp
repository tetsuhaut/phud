#pragma once

#include <functional> // std::invoke

// forward declaration
class Fl_Widget;

namespace fltkCallback::detail {
  /**
   * The type of the object the function is called on: Target of 'void Target::f()' or of
   * 'void f(Target&)'.
   */
  template <typename FUNCTION>
  struct TargetOf;

  template <typename R, typename T>
  struct TargetOf<R (T::*)()> {
    using type = T;
  };

  template <typename R, typename T>
  struct TargetOf<R (T::*)() noexcept> {
    using type = T;
  };

  template <typename R, typename T>
  struct TargetOf<R (T::*)() const> {
    using type = const T;
  };

  template <typename R, typename T>
  struct TargetOf<R (T::*)() const noexcept> {
    using type = const T;
  };

  template <typename R, typename T>
  struct TargetOf<R (*)(T&)> {
    using type = T;
  };

  template <typename R, typename T>
  struct TargetOf<R (*)(T&) noexcept> {
    using type = T;
  };
} // namespace fltkCallback::detail

/**
 * An Fl_Callback calling FUNCTION on the object given as the callback user data, so that the
 * callbacks don't have to cast the user data themselves:
 *   button->callback(callbackOn<&Editor::onOk>, pEditor);       // calls pEditor->onOk()
 *   button->callback(callbackOn<&startHud>, pImplementation);   // calls startHud(*pImplementation)
 * @tparam FUNCTION a member function without parameter, or a function taking a reference
 */
template <auto FUNCTION>
void callbackOn(Fl_Widget* /*widget*/, void* hiddenTarget) {
  using Target = typename fltkCallback::detail::TargetOf<decltype(FUNCTION)>::type;
  std::invoke(FUNCTION, *static_cast<Target*>(hiddenTarget));
}
