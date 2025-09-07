#pragma once

#include <functional> // std::function
#include <memory> // std::unique_ptr
#include <string>

class [[nodiscard]] TableWatcher final {
private:
  struct Implementation;
  std::unique_ptr<Implementation> m_pImpl;
public:
  using TablesChangedCallback = std::function<void(const std::vector<std::string>& tableNames)>;
  
  struct [[nodiscard]] Callbacks {
    TablesChangedCallback onTablesChanged;
  };

  explicit TableWatcher(const Callbacks& callbacks);
  TableWatcher(const TableWatcher&) = delete;
  TableWatcher(TableWatcher&&) = delete;
  TableWatcher& operator=(const TableWatcher&) = delete;
  TableWatcher& operator=(TableWatcher&&) = delete;
  ~TableWatcher();

  void start();
  void stop();
  
  [[nodiscard]] bool isWatching() const noexcept;
  [[nodiscard]] bool hasActiveTable() const noexcept;
  [[nodiscard]] std::vector<std::string> getCurrentTableNames() const;
  [[nodiscard]] std::size_t getTableCount() const noexcept;
}; // class TableWatcher