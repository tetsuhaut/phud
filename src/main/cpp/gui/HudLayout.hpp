#pragma once

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility> // std::pair
#include <vector>

// forward declaration
struct HudElement;

/**
 * A HudElement at a given place of the grid of cells of a PlayerIndicator. It is one row high, and
 * starts at the given column.
 */
struct [[nodiscard]] HudItem final {
  const HudElement* pElement;
  int column;
  int row;

  [[nodiscard]] bool operator==(const HudItem&) const noexcept = default;
};

/**
 * Which elements are displayed on the PlayerIndicators, and where. An element is displayed at most
 * once, and elements never overlap.
 */
class [[nodiscard]] HudLayout final {
private:
  std::vector<HudItem> m_items {};

public:
  /**
   * @returns the layout of a user who has not customized anything.
   */
  [[nodiscard]] static HudLayout defaultLayout();

  /**
   * Ignores what can't be read, so that the preferences saved by another version of the program
   * can't prevent it from working.
   * @param text the result of serialize()
   */
  [[nodiscard]] static HudLayout parse(std::string_view text);

  [[nodiscard]] std::string serialize() const;

  [[nodiscard]] std::span<const HudItem> getItems() const noexcept { return m_items; }

  [[nodiscard]] bool contains(const HudElement& element) const noexcept;

  /**
   * @returns the element covering the given cell, or nullptr if the cell is free.
   */
  [[nodiscard]] const HudElement* elementAt(int column, int row) const noexcept;

  /**
   * @returns the item of the given element, or nullptr if it is not in the layout.
   */
  [[nodiscard]] const HudItem* find(const HudElement& element) const noexcept;

  /**
   * @returns true if the element fits in the grid at the given place, without overlapping another
   * element. The cells it already covers count as free, so it can be moved a little.
   */
  [[nodiscard]] bool canPlace(const HudElement& element, int column, int row) const noexcept;

  /**
   * @returns the first {column, row} where the element fits, if any.
   */
  [[nodiscard]] std::optional<std::pair<int, int>>
  findFreePlace(const HudElement& element) const noexcept;

  /**
   * Adds the element at the first free place.
   * @returns false if the element is already there, or if it does not fit.
   */
  bool addElement(const HudElement& element);

  /**
   * @returns false if the element is not there, or if canPlace() is false. Nothing changes then.
   */
  bool moveElement(const HudElement& element, int column, int row) noexcept;

  void removeElement(const HudElement& element);

  /**
   * @returns the size in pixels of a PlayerIndicator: at least one cell, so that it can still be
   * seen and moved when nothing is displayed.
   */
  [[nodiscard]] int getWidth() const noexcept;
  [[nodiscard]] int getHeight() const noexcept;

  [[nodiscard]] bool operator==(const HudLayout&) const = default;
}; // class HudLayout
