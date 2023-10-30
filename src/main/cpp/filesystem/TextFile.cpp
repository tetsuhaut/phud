#include "containers/algorithms.hpp"
#include "filesystem/TextFile.hpp"  // std::string, std::string_view, std::vector
#include <sstream> // std::stringstream

namespace fs = std::filesystem;
namespace pa = phud::algorithms;
namespace pf = phud::filesystem;

TextFile::TextFile(const fs::path& file)
  : m_file { file },
    m_content { pf::readToString(file) } {}

TextFile::~TextFile() = default;

bool TextFile::next() {
  const auto ret { !std::getline(m_content, m_line).fail() };

  if (ret) { ++m_lineNb; }

  return ret;
}

bool TextFile::containsOneOf(std::span<const std::string_view> patterns) const {
  return pa::containsIf(patterns, [this](const auto & s) noexcept { return containsExact(s); });
}

TextFile& TextFile::trim() {
  m_line = phud::strings::trim(m_line);
  return *this;
}

bool TextFile::containsExact(std::string_view s) const noexcept {
  const auto pos { m_line.find(s) };

  if (std::string_view::npos == pos) { return false; }

  const auto offset { pos + s.size() };

  // the token found is at the end of the string -> match is always exact
  if (m_line.size() <= offset) { return true; }

  // the user passed a token ending by a space -> match is always exact
  if (' ' == m_line.at(offset - 1)) { return true; }

  // look at the character just after the found substring
  const auto c { m_line.at(offset) };
  return ('A' > c) or (c > 'z');
}

int TextFile::getLineIndex() const noexcept { return m_lineNb; }
std::string TextFile::getFileName() const { return m_file.string(); }
std::string TextFile::getFileStem() const { return m_file.stem().string(); }
std::string TextFile::getLine() const noexcept { return m_line; }
std::size_t TextFile::find(std::string_view s) const noexcept { return m_line.find(s); }
std::size_t TextFile::find(char c) const noexcept { return m_line.find(c); }
bool TextFile::lineIsEmpty() const noexcept { return m_line.empty(); }
bool TextFile::startsWith(char c) const noexcept { return m_line.starts_with(c); }
bool TextFile::startsWith(std::string_view s) const noexcept { return m_line.starts_with(s); }
bool TextFile::endsWith(std::string_view s) const noexcept { return m_line.ends_with(s); }
bool TextFile::endsWith(char c) const noexcept  { return m_line.ends_with(c); }