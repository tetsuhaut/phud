#include "gui/PlayerIndicatorWindowAppearance.hpp" // PlayerIndicatorWindow::surface, phud::Rectangle
#include "gui/PlayerIndicator.hpp" // Fl_Double_Window, String, StringView
#include "log/Logger.hpp" // fmt::*, Logger, CURRENT_FILE_NAME
#include "statistics/PlayerStatistics.hpp"


#if defined(_MSC_VER) // removal of specific msvc warnings due to FLTK
#  pragma warning(push)
#  pragma warning(disable : 4191 4244 4365 4514 4625 4626 4820 5026 5027 )
#elif defined(__MINGW32__) // removal of specific gcc warnings due to FLTK
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wsuggest-override"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wuseless-cast"
#endif  // _MSC_VER

#include <FL/Fl_Box.H>

#if defined(_MSC_VER)  // end of specific msvc warnings removal
#  pragma warning(pop)
#elif defined(__MINGW32__)
#  pragma GCC diagnostic pop
#endif  // _MSC_VER

namespace piws = PlayerIndicatorWindow::surface;

static Logger LOG { CURRENT_FILE_NAME };

PlayerIndicator::PlayerIndicator(const Pair<int, int>& p, StringView playerName)
  : DragAndDropWindow({ .x = p.first, .y = p.second, .w = piws::width, .h = piws::height },
playerName),
m_textStats { mkUptr<Fl_Box>(0, 0, w(), piws::statsHeight) },
m_textPlayerName { mkUptr<Fl_Box>(0, piws::statsHeight + 2, w(), piws::playerNameHeight) } {
  LOG.debug<"creation du PlayerIndicator {}">(playerName);
  box(FL_FLAT_BOX);
  color(FL_WHITE);
  m_textStats->labelsize(piws::statsHeight);
  m_textPlayerName->labelsize(piws::playerNameHeight);
  m_textPlayerName->copy_label(playerName.data());
  clear_border();
  end();
  show();
  LOG.debug<"m_textPlayerName->label()={}">(m_textPlayerName->label());
}

// TODO unused
void PlayerIndicator::setStats(const PlayerStatistics& s) {
  m_textStats->copy_label(
    fmt::format("{}|{}/{}/{}", s.getNbHands(),
                s.getVoluntaryPutMoneyInPot(),
                s.getPreFlopRaise(), s.getAggressionFactor()).c_str());
}

String PlayerIndicator::getPlayerName() const { return m_textPlayerName->label(); }

// TODO unused
void PlayerIndicator::refresh(StringView playerName) {
  LOG.debug<"PlayerIndicator refresh={}">(playerName);
  m_textPlayerName->copy_label(playerName.data());
}

PlayerIndicator::~PlayerIndicator() = default;
