#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <string>

class [[nodiscard]] TableWatcher final {
public:
  using TableFoundCallback = std::function<void(const std::string& tableName)>;
  using TableLostCallback = std::function<void()>;
  
  struct [[nodiscard]] Callbacks {
    TableFoundCallback onTableFound;
    TableLostCallback onTableLost;
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
  [[nodiscard]] std::string getCurrentTableName() const;

private:
  struct Implementation;
  std::unique_ptr<Implementation> m_pImpl;
}; // class TableWatcher