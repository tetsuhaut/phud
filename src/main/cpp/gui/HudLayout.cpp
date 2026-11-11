#include "gui/HudElement.hpp"
#include "gui/HudLayout.hpp" // CellPosition, std::optional, std::span, std::string, std::vector
#include "gui/PlayerIndicatorWindowAppearance.hpp" // PlayerIndicatorWindow::surface
#include <algorithm>                               // std::ranges::*
#include <charconv>                                // std::from_chars
#include <format>
#include <ranges>

namespace piws = PlayerIndicatorWindow::surface;

namespace {
  constexpr char ITEM_SEPARATOR {';'};
  constexpr char FIELD_SEPARATOR {':'};
  // the saved format is 'id:column:row', kept as is to read the preferences already saved
  constexpr std::string_view DEFAULT_LAYOUT {
      "nbHands:0:0;vpip:2:0;pfr:3:0;af:4:0;playerName:0:1"};

  /**
   * @returns the 1st column after the item
   */
  [[nodiscard]] int endColumn(const HudItem& item) noexcept {
    return item.position.column + item.pElement->widthInCells;
  }
  /**
   * @returns the 1st line after the item. as an item is always 1 line high, it is always
   * current item row + 1.
   */
  [[nodiscard]] int endRow(const HudItem& item) noexcept {
    return item.position.row + 1;
  }

  /**
   * @returns the number of cells needed along one axis (width or height), at least 1.
   * @param end where an item ends along this axis: endColumn or endRow
   */
  [[nodiscard]] int countCells(std::span<const HudItem> items, auto end) {
    // max(max(max(1, end(item0)), end(item1)), end(item2))... returns 1 if there is no item.
    return std::ranges::fold_left(items | std::views::transform(end), 1, std::ranges::max);
  }

  [[nodiscard]] bool covers(const HudItem& item, CellPosition cell) noexcept {
    return cell.row == item.position.row and item.position.column <= cell.column and
           cell.column < ::endColumn(item);
  }

  [[nodiscard]] bool overlaps(const HudItem& item, const HudElement& element,
                              CellPosition position) noexcept {
    return position.row == item.position.row and position.column < ::endColumn(item) and
           item.position.column < position.column + element.widthInCells;
  }

  [[nodiscard]] std::optional<int> toInt(std::string_view text) noexcept {
    int ret {0};
    const auto end = text.data() + text.size();
    const auto [ptr, ec] = std::from_chars(text.data(), end, ret);
    return (std::errc {} == ec and end == ptr) ? std::optional(ret) : std::nullopt;
  }

  /**
   * @returns the item described by 'id:column:row', with no guarantee that it can be placed.
   */
  [[nodiscard]] std::optional<HudItem> parseItem(std::string_view text) {
    const auto fields = text | std::views::split(FIELD_SEPARATOR) |
                        std::views::transform([](auto&& s) { return std::string_view(s); }) |
                        std::ranges::to<std::vector>();

    if (3 != fields.size()) {
      return std::nullopt;
    }

    const auto* pElement = findHudElement(fields[0]);
    const auto column = toInt(fields[1]);
    const auto row = toInt(fields[2]);

    if (nullptr == pElement or !column.has_value() or !row.has_value()) {
      return std::nullopt;
    }

    return HudItem {.pElement = pElement, .position = {.row = *row, .column = *column}};
  }
} // anonymous namespace

HudLayout HudLayout::defaultLayout() {
  return parse(DEFAULT_LAYOUT);
}

HudLayout HudLayout::parse(std::string_view text) {
  HudLayout ret;

  for (const auto chunk : text | std::views::split(ITEM_SEPARATOR)) {
    const auto oItem = parseItem(std::string_view(chunk));

    if (oItem.has_value() and !ret.contains(*oItem->pElement) and
        ret.canPlace(*oItem->pElement, oItem->position)) {
      ret.m_items.push_back(*oItem);
    }
  }

  return ret;
}

std::string HudLayout::serialize() const {
  std::string ret;

  for (const auto& item : m_items) {
    if (!ret.empty()) {
      ret += ITEM_SEPARATOR;
    }

    ret += std::format("{}{}{}{}{}", item.pElement->id, FIELD_SEPARATOR, item.position.column,
                       FIELD_SEPARATOR, item.position.row);
  }

  return ret;
}

const HudItem* HudLayout::find(const HudElement& element) const noexcept {
  const auto it = std::ranges::find(m_items, &element, &HudItem::pElement);
  return (m_items.end() == it) ? nullptr : &*it;
}

bool HudLayout::contains(const HudElement& element) const noexcept {
  return nullptr != find(element);
}

const HudElement* HudLayout::elementAt(CellPosition cell) const noexcept {
  const auto it =
      std::ranges::find_if(m_items, [cell](const auto& item) { return covers(item, cell); });
  return (m_items.end() == it) ? nullptr : it->pElement;
}

bool HudLayout::canPlace(const HudElement& element, CellPosition position) const noexcept {
  if (0 > position.row or piws::nbRows <= position.row or 0 > position.column or
      piws::nbColumns < position.column + element.widthInCells) {
    return false;
  }

  return std::ranges::none_of(m_items, [&element, position](const auto& item) {
    return &element != item.pElement and overlaps(item, element, position);
  });
}

std::optional<CellPosition> HudLayout::findFreePlace(const HudElement& element) const noexcept {
  for (auto row = 0; piws::nbRows > row; ++row) {
    for (auto column = 0; piws::nbColumns > column; ++column) {
      if (const CellPosition position {.row = row, .column = column};
          canPlace(element, position)) {
        return position;
      }
    }
  }

  return std::nullopt;
}

bool HudLayout::addElement(const HudElement& element) {
  const auto oPosition = findFreePlace(element);

  if (contains(element) or !oPosition.has_value()) {
    return false;
  }

  m_items.push_back({.pElement = &element, .position = *oPosition});
  return true;
}

bool HudLayout::moveElement(const HudElement& element, CellPosition position) noexcept {
  const auto it = std::ranges::find(m_items, &element, &HudItem::pElement);

  if (m_items.end() == it or !canPlace(element, position)) {
    return false;
  }

  it->position = position;
  return true;
}

void HudLayout::removeElement(const HudElement& element) {
  std::erase_if(m_items, [&element](const auto& item) { return &element == item.pElement; });
}

int HudLayout::getWidth() const noexcept {
  return countCells(m_items, ::endColumn) * piws::cellWidth;
}

int HudLayout::getHeight() const noexcept {
  return countCells(m_items, ::endRow) * piws::cellHeight;
}
