#pragma once

#include "containers/Span.hpp"
#include "filesystem/Filesystem.hpp" // Path, String, StringView, std::stringstream forward declaration
#include "system/memory.hpp" // uptr

/**
 * A text file reader.
 */
class [[nodiscard]] TextFile final {
private:
  Path m_file;
  String m_line {};
  int m_lineNb { 0 };
  std::stringstream m_content;

public:
  explicit TextFile(const Path& file);
  TextFile(auto file) = delete; // use only Path

  TextFile(const TextFile&) = delete;
  TextFile(TextFile&&) = delete;
  TextFile& operator=(const TextFile&) = delete;
  TextFile& operator=(TextFile&&) = delete;
  ~TextFile();

  /**
   * Reads the next line of text in the file.
   * @returns true if there is another line to read, false if not.
   */
  /*[[nodiscard]]*/ bool next(); // can be discarded

  /**
   * @returns the current line index, the first line being 0.
   */
  [[nodiscard]] int getLineIndex() const noexcept;
  [[nodiscard]] String getFileName() const;

  /**
   * @returns the filename without extension.
   */
  [[nodiscard]] String getFileStem() const; // filename without extension

  /**
   * @returns the current line of text.
   */
  [[nodiscard]] String getLine() const noexcept;
  [[nodiscard]] std::size_t find(StringView s) const noexcept;
  [[nodiscard]] std::size_t find(char c) const noexcept;
  [[nodiscard]] bool lineIsEmpty() const noexcept;
  [[nodiscard]] bool startsWith(char c) const noexcept;
  [[nodiscard]] bool startsWith(StringView s) const noexcept;
  [[nodiscard]] bool endsWith(StringView s) const noexcept;
  [[nodiscard]] bool endsWith(char c) const noexcept;
  [[nodiscard]] bool contains(StringView s) const noexcept { return StringView::npos != find(s); }
  [[nodiscard]] bool contains(char c) const noexcept { return String::npos != find(c); }
  [[nodiscard]] bool containsExact(StringView s) const noexcept;
  [[nodiscard]] bool containsOneOf(Span<const StringView> patterns) const;
  /*[[nodiscard]]*/ TextFile& trim(); // can be discarded
}; // class TextFile
