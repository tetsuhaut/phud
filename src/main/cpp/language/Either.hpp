// #pragma once
//
// #include <optional> // std::in_place_t, std::in_place
//
// template <typename LEFT, typename RIGHT>
// class [[nodiscard]] Either final {
// private:
//   template <typename T>
//   struct [[nodiscard]] Left final {
//     constexpr explicit Left(T v) : m_value(std::move(v)) {}
//     T m_value;
//   };
//
//   template <typename T>
//   struct [[nodiscard]] Right final {
//     constexpr explicit Right(T v) : m_value(std::move(v)) {}
//     T m_value;
//   };
//
//   std::optional<LEFT> m_left;
//   std::optional<RIGHT> m_right;
//   explicit constexpr Either(Left<LEFT> l) : m_left { l.m_value }, m_right {} {}
//   explicit constexpr Either(Right<RIGHT> r) : m_left {}, m_right { r.m_value } {}
//
// public:
//   [[nodiscard]] static constexpr Either left(LEFT arg) { return Either(Left(std::move(arg))); }
//   [[nodiscard]] static constexpr Either right(RIGHT arg) { return Either(Right(std::move(arg)));
//   }
//   [[nodiscard]] constexpr bool isLeft() const noexcept { return m_left.has_value(); }
//   [[nodiscard]] constexpr bool isRight() const noexcept { return m_right.has_value(); }
//   [[nodiscard]] constexpr const LEFT& getLeft() const { return m_left.value(); }
//   [[nodiscard]] constexpr LEFT& getLeft() { return m_left.value(); }
//   [[nodiscard]] constexpr const RIGHT& getRight() const { return m_right.value(); }
//   [[nodiscard]] constexpr RIGHT& getRight() { return m_right.value(); }
// }; // class Either
