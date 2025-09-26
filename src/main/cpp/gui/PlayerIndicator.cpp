#include "gui/PlayerIndicatorWindowAppearance.hpp" // PlayerIndicatorWindow::surface, phud::Rectangle
#include "gui/PlayerIndicator.hpp" // Fl_Double_Window, std::string, std::string_view
#include "log/Logger.hpp" // fmt::*, Logger, CURRENT_FILE_NAME
#include "statistics/PlayerStatistics.hpp"


#if defined(_MSC_VER) // removal of specific msvc warnings due to FLTK
#  pragma warning(push)
#  pragma warning(disable : 4191 4244 4365 4514 4625 4626 4820 5026 5027 )
#endif  // _MSC_VER

#include <FL/Fl_Box.H>

#if defined(_MSC_VER)  // end of specific msvc warnings removal
#  pragma warning(pop)
#endif  // _MSC_VER

namespace piws = PlayerIndicatorWindow::surface;

static Logger LOG { CURRENT_FILE_NAME };

PlayerIndicator::PlayerIndicator(const std::pair<int, int>& position, std::string_view playerName)
  : DragAndDropWindow({ .x = position.first, .y = position.second, .w = piws::width, .h = piws::height },
playerName),
m_textStats { std::make_unique<Fl_Box>(0, 0, w(), piws::statsHeight) },
m_textPlayerName { std::make_unique<Fl_Box>(0, piws::statsHeight + 2, w(), piws::playerNameHeight) } {
  LOG.debug<"creation du PlayerIndicator {}">(playerName);
  box(FL_FLAT_BOX);
  color(FL_WHITE);
  m_textStats->labelsize(piws::statsHeight);
  m_textPlayerName->labelsize(piws::playerNameHeight);
  m_textPlayerName->copy_label(playerName.data());
  clear_border();
  end();
  Fl_Double_Window::show();
  LOG.debug<"m_textPlayerName->label()={}">(m_textPlayerName->label());
}

// TODO unused
void PlayerIndicator::setStats(const PlayerStatistics& ps) const {
  m_textStats->copy_label(
    fmt::format("{}|{}/{}/{}", ps.getNbHands(),
                ps.getVoluntaryPutMoneyInPot(),
                ps.getPreFlopRaise(), ps.getAggressionFactor()).c_str());
}

std::string PlayerIndicator::getPlayerName() const { return m_textPlayerName->label(); }

// TODO unused
void PlayerIndicator::refresh(std::string_view playerName) const {
  LOG.debug<"PlayerIndicator refresh={}">(playerName);
  m_textPlayerName->copy_label(playerName.data());
}

PlayerIndicator::~PlayerIndicator() = default;
