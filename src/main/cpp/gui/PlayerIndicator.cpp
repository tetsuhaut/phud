#include "gui/HudElement.hpp" // formatHudElement
#include "gui/PlayerIndicatorWindowAppearance.hpp" // PlayerIndicatorWindow::surface
#include "gui/PlayerIndicator.hpp" // Fl_Double_Window, HudLayout, std::string, std::string_view
#include "log/Logger.hpp"          // fmt::*, Logger, CURRENT_FILE_NAME
#include "statistics/PlayerStatistics.hpp"
#include <ranges> // std::views::zip


#if defined(_MSC_VER) // removal of specific msvc warnings due to FLTK
#  pragma warning(push)
#  pragma warning(disable : 4191 4244 4365 4514 4625 4626 4820 5026 5027)
#endif // _MSC_VER

#include <FL/Fl_Box.H>

#if defined(_MSC_VER) // end of specific msvc warnings removal
#  pragma warning(pop)
#endif // _MSC_VER

namespace piws = PlayerIndicatorWindow::surface;

static Logger& LOG() {
  static auto logger = Logger(CURRENT_FILE_NAME);
  return logger;
}

PlayerIndicator::PlayerIndicator(const std::pair<int, int>& position, std::string_view playerName,
                                 const HudLayout& layout)
  : DragAndDropWindow(
        {.x = position.first,
         .y = position.second,
         .w = layout.getWidth(),
         .h = layout.getHeight()},
        "",
        // Callback called when the user moves manually the PlayerIndicator
        [this](int newX, int newY) {
          m_userOffsetX = newX - m_basePosition.first;
          m_userOffsetY = newY - m_basePosition.second;
          LOG().debug<"User moved PlayerIndicator, offset: ({}, {})">(m_userOffsetX, m_userOffsetY);
        }),
    m_layout {layout},
    m_playerName {playerName},
    m_basePosition {position} {
  LOG().debug<"creation du PlayerIndicator {}">(playerName);
  copy_label(m_playerName.c_str());
  box(FL_FLAT_BOX);
  color(FL_WHITE);
  clear_border();
  end();
  buildBoxes();
  Fl_Double_Window::show();
}

void PlayerIndicator::buildBoxes() {
  clear(); // deletes the boxes
  m_boxes.clear();
  size(m_layout.getWidth(), m_layout.getHeight());
  begin();

  for (const auto& item : m_layout.getItems()) {
    auto* const pBox = new Fl_Box(item.column * piws::cellWidth, item.row * piws::cellHeight,
                                  item.pElement->widthInCells * piws::cellWidth, piws::cellHeight);
    pBox->labelsize(piws::fontSize);
    pBox->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
    m_boxes.push_back(pBox);
  }

  end();
}

void PlayerIndicator::displayStats() const {
  if (nullptr == m_pStats) {
    return;
  }

  for (const auto& [item, pBox] : std::views::zip(m_layout.getItems(), m_boxes)) {
    pBox->copy_label(formatHudElement(*item.pElement, *m_pStats).c_str());
  }
}

void PlayerIndicator::setStats(std::unique_ptr<PlayerStatistics> pStats) {
  m_pStats = std::move(pStats);
  m_playerName = m_pStats->getPlayerName();
  copy_label(m_playerName.c_str());
  displayStats();
  redraw();
}

void PlayerIndicator::setLayout(const HudLayout& layout) {
  if (layout == m_layout) {
    return;
  }

  m_layout = layout;
  buildBoxes();
  displayStats();
  redraw();
}

std::string PlayerIndicator::getPlayerName() const {
  return m_playerName;
}

void PlayerIndicator::updateBasePosition(const std::pair<int, int>& basePosition) {
  m_basePosition = basePosition;
  // Appliquer la nouvelle position de base + l'offset utilisateur
  const auto finalX = basePosition.first + m_userOffsetX;
  const auto finalY = basePosition.second + m_userOffsetY;
  position(finalX, finalY);
}

PlayerIndicator::~PlayerIndicator() = default;
