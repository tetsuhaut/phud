#pragma once

#include <filesystem>
#include <memory> // std::unique_ptr
#include <span>

/**
 * A text file reader.
 */
class [[nodiscard]] TextFile final {
 private:
  // Memory layout optimized: largest to smallest to minimize padding
  std::stringstream m_content;
  std::filesystem::path m_file;
  std::string m_line {};
  int m_lineNb {0};

 public:
  explicit TextFile(const std::filesystem::path& file);
  explicit TextFile(auto file) = delete; // use only std::filesystem::path

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
  [[nodiscard]] std::string getFileName() const;

  /**
   * @returns the filename without extension.
   */
  [[nodiscard]] std::string getFileStem() const; // filename without extension

  /**
   * @returns the current line of text.
   */
  [[nodiscard]] std::string getLine() const noexcept;
  [[nodiscard]] std::size_t find(std::string_view s) const noexcept;
  [[nodiscard]] std::size_t find(char c) const noexcept;
  [[nodiscard]] bool lineIsEmpty() const noexcept;
  [[nodiscard]] bool startsWith(char c) const noexcept;
  [[nodiscard]] bool startsWith(std::string_view s) const noexcept;
  [[nodiscard]] bool endsWith(std::string_view s) const noexcept;
  [[nodiscard]] bool endsWith(char c) const noexcept;
  [[nodiscard]] bool contains(std::string_view s) const noexcept {
    return std::string_view::npos != find(s);
  }
  [[nodiscard]] bool contains(char c) const noexcept { return std::string::npos != find(c); }
  [[nodiscard]] bool containsExact(std::string_view s) const noexcept;
  [[nodiscard]] bool containsOneOf(std::span<const std::string_view> patterns) const;
  /*[[nodiscard]]*/ TextFile& trim(); // can be discarded
}; // class TextFile
