#include "gui/HistoryService.hpp"
#include "log/Logger.hpp"

static Logger LOG { CURRENT_FILE_NAME };

bool HistoryService::historyDirectoryIsValid(const fs::path& dir) {
  LOG.debug<"Validating history directory: '{}'">(dir.string());
  
  bool isValid = isValidHistory(dir);
  
  if (isValid) {
    LOG.info<"History directory '{}' is valid">(dir.string());
  } else {
    LOG.info<"History directory '{}' is not valid">(dir.string());
  }
  
  return isValid;
}

void HistoryService::startImport(const fs::path& dir, ImportCallbacks callbacks) {
  LOG.info<"Starting history import from directory: '{}'">(dir.string());
  
  importHistory(dir, callbacks.onProgress, callbacks.onSetNbFiles, callbacks.onDone);
}

std::string HistoryService::getDisplayName(const fs::path& dir) {
  return dir.filename().string();
}