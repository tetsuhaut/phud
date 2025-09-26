#pragma once

#include <functional> // std::function
#include <memory> // std::unique_ptr
#include <span>
#include <string>
#include <vector>

class [[nodiscard]] TableWatcher final {
private:
  struct Implementation;
  std::unique_ptr<Implementation> m_pImpl;
public:
  using TablesChangedCallback = std::function<void(std::span<const std::string> tableNames)>;

  explicit TableWatcher(const TablesChangedCallback& onTablesChanged);
  TableWatcher(const TableWatcher&) = delete;
  TableWatcher(TableWatcher&&) = delete;
  TableWatcher& operator=(const TableWatcher&) = delete;
  TableWatcher& operator=(TableWatcher&&) = delete;
  ~TableWatcher();

  void start() const;
  void stop() const;

  [[nodiscard]] static bool isPokerTable(std::string_view title);
  [[nodiscard]] bool isWatching() const noexcept;
  [[nodiscard]] std::vector<std::string> getCurrentTableNames() const;
  [[nodiscard]] std::size_t getTableCount() const noexcept;
}; // class TableWatcher
