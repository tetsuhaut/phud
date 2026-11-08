#include "gui/HudElement.hpp"
#include "gui/HudLayout.hpp" // std::optional, std::span, std::string, std::vector
#include "gui/PlayerIndicatorWindowAppearance.hpp" // PlayerIndicatorWindow::surface
#include <algorithm>                               // std::ranges::*
#include <charconv>                                // std::from_chars
#include <format>
#include <ranges>

namespace piws = PlayerIndicatorWindow::surface;

namespace {
  constexpr char ITEM_SEPARATOR {';'};
  constexpr char FIELD_SEPARATOR {':'};
  constexpr std::string_view DEFAULT_LAYOUT {
      "nbHands:0:0;vpip:2:0;pfr:3:0;af:4:0;playerName:0:1"};

  [[nodiscard]] bool covers(const HudItem& item, int column, int row) noexcept {
    return row == item.row and item.column <= column and
           column < item.column + item.pElement->widthInCells;
  }

  [[nodiscard]] bool overlaps(const HudItem& item, const HudElement& element, int column,
                              int row) noexcept {
    return row == item.row and column < item.column + item.pElement->widthInCells and
           item.column < column + element.widthInCells;
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

    return HudItem {.pElement = pElement, .column = *column, .row = *row};
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
        ret.canPlace(*oItem->pElement, oItem->column, oItem->row)) {
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

    ret += std::format("{}{}{}{}{}", item.pElement->id, FIELD_SEPARATOR, item.column,
                       FIELD_SEPARATOR, item.row);
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

const HudElement* HudLayout::elementAt(int column, int row) const noexcept {
  const auto it = std::ranges::find_if(
      m_items, [column, row](const auto& item) { return covers(item, column, row); });
  return (m_items.end() == it) ? nullptr : it->pElement;
}

bool HudLayout::canPlace(const HudElement& element, int column, int row) const noexcept {
  if (0 > column or 0 > row or piws::nbRows <= row or
      piws::nbColumns < column + element.widthInCells) {
    return false;
  }

  return std::ranges::none_of(m_items, [&element, column, row](const auto& item) {
    return &element != item.pElement and overlaps(item, element, column, row);
  });
}

std::optional<std::pair<int, int>>
HudLayout::findFreePlace(const HudElement& element) const noexcept {
  for (auto row = 0; piws::nbRows > row; ++row) {
    for (auto column = 0; piws::nbColumns > column; ++column) {
      if (canPlace(element, column, row)) {
        return std::pair {column, row};
      }
    }
  }

  return std::nullopt;
}

bool HudLayout::addElement(const HudElement& element) {
  const auto oPlace = findFreePlace(element);

  if (contains(element) or !oPlace.has_value()) {
    return false;
  }

  m_items.push_back({.pElement = &element, .column = oPlace->first, .row = oPlace->second});
  return true;
}

bool HudLayout::moveElement(const HudElement& element, int column, int row) noexcept {
  const auto it = std::ranges::find(m_items, &element, &HudItem::pElement);

  if (m_items.end() == it or !canPlace(element, column, row)) {
    return false;
  }

  it->column = column;
  it->row = row;
  return true;
}

void HudLayout::removeElement(const HudElement& element) {
  std::erase_if(m_items, [&element](const auto& item) { return &element == item.pElement; });
}

int HudLayout::getWidth() const noexcept {
  auto nbColumns {1};

  for (const auto& item : m_items) {
    nbColumns = std::max(nbColumns, item.column + item.pElement->widthInCells);
  }

  return nbColumns * piws::cellWidth;
}

int HudLayout::getHeight() const noexcept {
  auto nbRows {1};

  for (const auto& item : m_items) {
    nbRows = std::max(nbRows, item.row + 1);
  }

  return nbRows * piws::cellHeight;
}
