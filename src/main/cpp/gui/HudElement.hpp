#pragma once

#include <span>
#include <string>
#include <string_view>

// forward declaration
class PlayerStatistics;

/**
 * Something that can be displayed on a PlayerIndicator: the player name, the number of known hands,
 * or a statistic.
 */
struct [[nodiscard]] HudElement final {
  using Formatter = std::string (*)(const PlayerStatistics&);

  std::string_view id;        // saved in the user preferences: must never change
  std::string_view shortName; // displayed in the HUD editor grid
  std::string_view name;
  std::string_view description;
  Formatter format {nullptr}; // nullptr: the statistic is not computed yet
  int widthInCells {1};
};

/**
 * @returns every element which can be displayed, including the statistics not computed yet.
 */
[[nodiscard]] std::span<const HudElement> getHudElements() noexcept;

/**
 * @returns the element with the given id, or nullptr if there is none.
 */
[[nodiscard]] const HudElement* findHudElement(std::string_view id) noexcept;

[[nodiscard]] constexpr bool hasData(const HudElement& element) noexcept {
  return nullptr != element.format;
}

/**
 * @returns the text to display for the given player, which is a dash if there is no data: either
 * the statistic is not computed yet, or the player never had the opportunity.
 */
[[nodiscard]] std::string formatHudElement(const HudElement& element, const PlayerStatistics& ps);
