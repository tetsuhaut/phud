#include "db/Database.hpp"
#include "gui/Gui.hpp"
#include "gui/HistoryService.hpp"
#include "gui/TableService.hpp"
#include "log/Logger.hpp"
#include "mainLib/App.hpp"
#include "statistics/PlayerStatistics.hpp"

namespace fs = std::filesystem;

static Logger LOG { CURRENT_FILE_NAME };

struct [[nodiscard]] App::Implementation final {
  std::unique_ptr<Database> m_database;
  std::unique_ptr<TableService> m_tableService;
  std::unique_ptr<HistoryService> m_historyService;
  std::unique_ptr<Gui> m_gui {};

  Implementation(std::string_view databaseName)
    : m_database { std::make_unique<Database>(databaseName) },
      m_tableService { std::make_unique<TableService>(*m_database) },
      m_historyService { std::make_unique<HistoryService>(*m_database) } {}
};

App::App(std::string_view databaseName)
  : m_pImpl {std::make_unique<Implementation>(databaseName)} {}

App::~App() = default;

int App::showGui() {
  m_pImpl->m_gui = std::make_unique<Gui>(*m_pImpl->m_tableService, *m_pImpl->m_historyService);
  return m_pImpl->m_gui->run();
}
