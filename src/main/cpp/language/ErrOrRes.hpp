#pragma once

#include "strings/StringLiteral.hpp" // string_view
#include <optional> // std::in_place_t, std::in_place
#include <string>

/**
 * Contains a result or an error message.
 */
template <typename RESULT>
class [[nodiscard]] ErrOrRes final {
private:
  std::optional<std::string> m_err;
  std::optional<RESULT> m_res;
  /**
   * builds an result.
   */
  explicit constexpr ErrOrRes(RESULT r) : m_err {}, m_res { r } {}
  /**
   * builds an error message. we use std::in_place_t to disambiguate in case
   * RESULT is constructible from std::string_view
   */
  constexpr ErrOrRes(std::in_place_t, std::string_view s) : m_err { s }, m_res {} {}

public:
  template <StringLiteral ERR_MSG>
  [[nodiscard]] static constexpr ErrOrRes err() { return ErrOrRes(std::in_place, ERR_MSG.value); }

  [[nodiscard]] static constexpr ErrOrRes err(std::string_view s) { return ErrOrRes(std::in_place, s); }
  [[nodiscard]] static constexpr ErrOrRes res(RESULT arg) { return ErrOrRes(arg); }
  [[nodiscard]] constexpr bool isErr() const noexcept { return m_err.has_value(); }
  [[nodiscard]] constexpr bool isRes() const noexcept { return m_res.has_value(); }
  [[nodiscard]] constexpr std::string getErr() const { return m_err.value(); }
  [[nodiscard]] constexpr const RESULT& getRes() const { return m_res.value(); }
  [[nodiscard]] constexpr RESULT& getRes() { return m_res.value(); }
}; // class ErrOrRes
