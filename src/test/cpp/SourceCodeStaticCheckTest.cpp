#include "TestInfrastructure.hpp" // std::filesystem::path, fs::*, phud::*
#include "containers/algorithms.hpp" // phud::algorithms::*
#include "db/sqliteQueries.hpp"
#include "filesystem/TextFile.hpp" // Span
#include "language/assert.hpp" // phudAssert
#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "mainLib/ProgramInfos.hpp" // ProgramInfos::*
#include <frozen/string.h>
#include <frozen/unordered_map.h>
#include <numeric> // std::accumulate
#include <unordered_map>

namespace fs = std::filesystem;
namespace pa = phud::algorithms;
namespace pf = phud::filesystem;
namespace ps = phud::strings;
namespace pt = phud::test;

static Logger LOG { CURRENT_FILE_NAME };

/* algorithms to be moved to containers/algorithms.hpp if we use them elsewhere */
namespace phud::algorithms {
template<typename CONTAINER, typename T>
[[nodiscard]] auto count(const CONTAINER& c, const T& value) {
  return std::count(std::begin(c), std::end(c), value);
}

template<typename MAP, typename KEY>
[[nodiscard]] auto findOrDefault(const MAP& m, const KEY& k) {
  using V = typename MAP::mapped_type;
  const auto& ret { m.find(k) };
  return (std::end(m) == ret) ? V() : ret->second;
}

template<typename CONTAINER, typename T>
void eraseValueFrom(CONTAINER& c, const T& value) {
  c.erase(std::remove(std::begin(c), std::end(c), value), std::end(c));
}

template<typename CONTAINER_SOURCE, typename CONTAINER_TARGET>
void append(CONTAINER_TARGET& target, const CONTAINER_SOURCE& source) {
  target.insert(std::end(target), std::begin(source), std::end(source));
}

template<typename CONTAINER, typename... Args>
/*[[nodiscard]]*/ CONTAINER merge(const CONTAINER& c, Args&& ... otherContainers) {
  CONTAINER ret { c };
  const auto& allOtherContainers = { std::forward<Args>(otherContainers)... };
  forEach(allOtherContainers, [&ret](const auto & other) {
    ret.insert(std::end(ret), std::begin(other), std::end(other));
  });
  return ret;
}

template<typename CONTAINER_SOURCE, typename CONTAINER_TARGET, typename PREDICATE>
void copyIf(const CONTAINER_SOURCE& source, CONTAINER_TARGET& target, PREDICATE p) {
  std::copy_if(std::begin(source), std::end(source), std::back_inserter(target), p);
}
}; // namespace phud::algorithms

static std::array<fs::path, 2> SRC_DIRS { pt::getMainCppDir(), pt::getTestCppDir() };

/** all of the cpp and hpp (and potentialy other) files in src/main/cpp and src/test/cpp directories */
static auto SRC_FILES {
  []() {
    return std::accumulate(SRC_DIRS.begin(), SRC_DIRS.end(), std::vector<fs::path> {}, [](std::vector<fs::path>&& v,
    const fs::path & dir) {
      pa::append(v, pf::listRecursiveFiles(dir));
      return v;
    });
  }()
};

/**
 * Changes '#include <a/b> // c' into 'a/b'
 *         '#include "a/b" // c' into 'a/b'
 */
[[nodiscard]] static inline std::string_view extractInclude(std::string_view line) {
  using namespace pa;
  phudAssert((1 < count(line, '"')) or (contains(line, '<') and contains(line, '>')), "bad line");
  const auto& startPos { line.find_first_of("\"<") + 1 };
  const auto& endPos { line.find_first_of("\">", startPos) - 1 };
  return line.substr(startPos, 1 + endPos - startPos);
}

/**
 * @returns either an std::filesystem::path from SRC_FILES representing the given file name if found,
 * or an std::filesystem::path formed from the the given file name.
 */
[[nodiscard]] static inline fs::path extractAbsolutePathIncludeIfPossible(std::string_view line) {
  const auto& file { extractInclude(line) };
  using namespace phud;
  using namespace pf;
  const auto& f { pa::findIf(SRC_DIRS, [&file](const auto & dir) { return pa::contains(SRC_FILES, dir / file); }) };
  return (SRC_DIRS.end() == f) ? file : std::filesystem::canonical(*f / file);
}

/** @returns all of the files that are nor in the src/main/cpp/thirdparty directory */
[[nodiscard]] static inline std::vector<fs::path> getMySrcFiles(std::span<const fs::path> files) {
  return pa::removeCopyIf<std::vector<fs::path>>(files, [](const auto & f) {
    return ps::contains(f.string(), "thirdParty");
  });
}

namespace {
/**
 * @returns a vector containing all of the files in the given @param files that end with the given @param fileExtension.
 */
[[nodiscard]] inline std::vector<fs::path> getSrcFiles(std::span<const fs::path> files, std::string_view fileExtension) {
  std::vector<fs::path> ret;
  pa::copyIf(files, ret, [&](const auto & p) { return p.string().ends_with(fileExtension); });
  return ret;
}

const auto& CPP_FILES { getSrcFiles(SRC_FILES, ".cpp") };
const auto& HPP_FILES { getSrcFiles(SRC_FILES, ".hpp") };
const auto& H_FILES { getSrcFiles(SRC_FILES, ".h") };
auto MY_SRC_FILES { getMySrcFiles(SRC_FILES) };
const auto& H_HPP_FILES { getMySrcFiles(pa::merge(HPP_FILES, H_FILES)) };
const auto& MY_SRC_FILES_WITH_EXCEPTION {
  pa::removeCopyIf<std::vector<fs::path>>(MY_SRC_FILES, [](const auto & file) {
    return file.string().ends_with("SourceCodeStaticCheckTest.cpp");
  })
};

/**
* Map of file <-> vector of its included files
*/
const auto& FILE_INCLUSIONS = []() {
  // std::unordered_map doesn't accept fs::path as a key
  std::map<fs::path, std::vector<fs::path>, pf::PathComparator> ret;
  std::ranges::for_each(SRC_FILES, [&ret](const auto & file) {
    auto& includes { ret[file] };
    /* we do not check includes from STL, or from thirdParty */
    TextFile tfl { file };

    while (tfl.next()) {
      if (tfl.trim().startsWith("#include ")) {
        includes.push_back(extractAbsolutePathIncludeIfPossible(tfl.getLine()));
      }
    }
  });
  return ret;
}();

class [[nodiscard]] BeforeClass final {
public:
  BeforeClass() {
    BOOST_REQUIRE(pt::isSet(CPP_FILES));
    BOOST_REQUIRE(pt::isSet(HPP_FILES));
    BOOST_REQUIRE(pt::isSet(H_FILES));
    BOOST_REQUIRE(pt::isSet(SRC_FILES));
    BOOST_REQUIRE(pt::isSet(MY_SRC_FILES));
    BOOST_REQUIRE(pt::isSet(H_HPP_FILES));
  }
} beforeClass;

enum class LineType : short { none, pragmaOnce, other };
} // namespace

[[nodiscard]] static inline bool isAComment(const TextFile& tfl) {
  return tfl.startsWith("//") or tfl.startsWith("/*");
}

[[nodiscard]] static inline LineType getFirstCodeLineType(const fs::path& file) {
  TextFile tfl { file };

  while (tfl.next()) { /*find the first code line*/
    if (!tfl.trim().lineIsEmpty() and !isAComment(tfl)) {
      return tfl.startsWith("#pragma once") ? LineType::pragmaOnce : LineType::other;
    }
  }

  return LineType::none; // the file is either empty or commented
}

[[nodiscard]] static inline std::vector<std::string> getAllQueryNames(std::string_view
    sourceFileContainingQueries) {
  const auto& sourceFileWithFullPath { *pa::findIf(MY_SRC_FILES, [&](const auto & file) {
    return file.string().ends_with(sourceFileContainingQueries);
  })
                                     };
  TextFile tfl { sourceFileWithFullPath };
  std::vector<std::string> ret;

  while (tfl.next()) {
    if (tfl.startsWith("static constexpr std::string_view ") and tfl.endsWith("[] {")) {
      const auto& str { tfl.getLine() };
      const auto& pos { ps::length("static constexpr std::string_view ") };
      ret.push_back(str.substr(pos, str.size() - pos - ps::length("[] {")));
    }
  }

  phudAssert(!ret.empty(), "did not detect any query");
  return ret;
}

[[nodiscard]] static inline bool sourceFileContains(std::string_view sourceFile,
    std::string_view sqlQueryName) {
  const auto& sourceFileWithFullPath { *pa::findIf(MY_SRC_FILES, [&](auto & file) {
    return file.string().ends_with(sourceFile);
  })
                                     };
  TextFile tfl { sourceFileWithFullPath };

  while (tfl.next()) {
    if (tfl.contains(sqlQueryName)) {
      return true;
    }
  }

  return false;
}

/*[[nodiscard]]*/ static inline void logIfMySrcFilesContainToken(std::span<const fs::path> files,
    std::string_view token, std::string_view replacement) {
  std::ranges::for_each(files, [&](const auto & file) {
    TextFile tfl { file };

    while (tfl.next()) {
      if (tfl.trim().contains(token)) {
        LOG.warn<"The file {} contains the token '{}' instead of using '{}' at line {}.">(
          file.string(), token, replacement, tfl.getLineIndex());
      }
    }
  });
}

/*[[nodiscard]]*/ static inline void logIfMySrcFilesContainToken(std::string_view token,
    std::string_view replacement) {
  logIfMySrcFilesContainToken(::MY_SRC_FILES_WITH_EXCEPTION, token, replacement);
}
/**
 * Beginning of tests
 */
BOOST_AUTO_TEST_SUITE(SourceStaticCheckTest)

BOOST_AUTO_TEST_CASE(SourceStaticCheckTest_globalsAreCorrect) {
  BOOST_TEST(!std::string_view(ProgramInfos::APP_VERSION).empty());
  BOOST_TEST(!std::string_view(ProgramInfos::APP_SHORT_NAME).empty());
  BOOST_TEST(!std::string_view(ProgramInfos::APP_LONG_NAME).empty());
  BOOST_TEST(!std::string_view(ProgramInfos::WINAMAX_EXECUTABLE_STEM).empty());
  BOOST_TEST(!phud::test::getMainCppDir().empty());
  BOOST_TEST(!phud::test::getTestResourcesDir().empty());
  BOOST_TEST(!phud::test::getTestCppDir().empty());
}

BOOST_AUTO_TEST_CASE(SourceStaticCheckTest_allHppHeaderFilesMustStartWithParagmaOnce) {
  std::ranges::for_each(H_HPP_FILES, [](const auto & file) {
    if (LineType::other == getFirstCodeLineType(file)) {
      LOG.warn<"The header '{}' should start with '#pragma once'">(file.string());
    }
  });
}

BOOST_AUTO_TEST_CASE(SourceStaticCheckTest_noCppFilesShouldStartWithParagmaOnce) {
  std::ranges::for_each(CPP_FILES, [](const auto & file) {
    if (LineType::pragmaOnce == getFirstCodeLineType(file)) {
      LOG.warn<"The cpp file '{}' should not contain '#pragma once'">(file.string());
    }
  });
}

BOOST_AUTO_TEST_CASE(SourceStaticCheckTest_allHeaderFilesShouldBeIncluded) {
  /* list all the h/hpp files, remove one each time we find it included. */
  /* Those remaining are never included. */
  auto headers { ::H_HPP_FILES };
  std::ranges::for_each(::MY_SRC_FILES, [&](const auto & file) {
    if (!headers.empty()) {
      const auto& inclusions { pa::findOrDefault(FILE_INCLUSIONS, file.string()) };
      std::ranges::for_each(inclusions, [&](const auto & h) { pa::eraseValueFrom(headers, h); });
    }
  });
  std::ranges::for_each(headers, [](const auto & h) { LOG.warn<"Unused header:\n{}">(h.string()); });
}

static inline std::vector<fs::path> getIncludes(const fs::path& p) {
  if (const auto & entry { ::FILE_INCLUSIONS.find(p) }; ::FILE_INCLUSIONS.end() != entry) {
    return entry->second;
  }

  return {};
}

BOOST_AUTO_TEST_CASE(SourceStaticCheckTest_noFileShouldBeIncludedInAFileAndOneOfItsIncludedFiles) {
  std::ranges::for_each(::FILE_INCLUSIONS, [&](const auto & fileToIncludes) {
    const auto& f { fileToIncludes.first };
    const auto& currentIncludes { fileToIncludes.second };
    std::ranges::for_each(currentIncludes, [&](const auto & incl) {
      const auto& others { getIncludes(incl) };
      std::ranges::for_each(others, [&](const auto & inclincl) {
        if (pa::contains(currentIncludes, inclincl)) {
          LOG.warn<"\nis in\n{}\nand in\n{}\n">(inclincl.string(), f.string(), incl.string());
        }
      });
    });
  });
}

BOOST_AUTO_TEST_CASE(SourceStaticCheckTest_nonDeletedConstructorsTakingArgumentsShouldBeExplicit) {
  /* explicit is for forbidding implicit conversions for constructors callable with a single parameter */
  std::ranges::for_each(::H_HPP_FILES, [](const auto & file) {
    TextFile tfl { file };
    const auto& constructor { tfl.getFileStem() };

    while (tfl.next()) {
      if (const auto & trimmedLine { tfl.trim().getLine() };
          /* it's not a deleted constructor */
          trimmedLine.starts_with(constructor + "(") and !ps::contains(trimmedLine, " delete") and
          /* it's not a copy constructor */
          !trimmedLine.starts_with(constructor + "(const " + constructor + "&") and
          /* it's not an empty constructor */
          !trimmedLine.starts_with(constructor + "()")
          /* it's a one parameter callable constructor */
          and pa::count(trimmedLine, ',') > pa::count(trimmedLine, '=')
          /* it's not an explicit constructor */
          and trimmedLine.starts_with("explicit ")) {
        LOG.warn<"In {} at line {} the constructor should be explicit: {}.">(
          file.string(), tfl.getLineIndex(), tfl.getLine());
      }
    }
  });
}

BOOST_AUTO_TEST_CASE(SourceStaticCheckTest_nonDeletedConstructorsTakingNoArgShouldNotBeExplicit) {
  /* explicit is for forbidding constructor arguments conversion */
  std::ranges::for_each(::H_HPP_FILES, [](const auto & file) {
    TextFile tfl { file };
    const auto& constructor { tfl.getFileStem() };

    while (tfl.next()) {
      tfl.trim();

      if (const auto & pos { tfl.find(constructor + "(") }; ( std::string::npos != pos) and
          tfl.startsWith("explicit ") and !tfl.contains(" delete") and
          (')' == tfl.getLine()[pos + constructor.size() + 2])) {
        LOG.warn<"In {} at line {} the default constructor should not be explicit:{}">(
          file.string(), tfl.getLineIndex(), tfl.getLine());
      }
    }
  });
}

BOOST_AUTO_TEST_CASE(SourceStaticCheckTest_destructorShouldNotBeNoExcept) {
  /* C++11 made destructors noexcept by default, unless stated differently */
  std::ranges::for_each(::MY_SRC_FILES, [](const auto & file) {
    TextFile tfl { file };

    while (tfl.next()) {
      if (tfl.trim().startsWith('~') and tfl.contains(") noexcept")) {
        LOG.warn<"The file '{}' contains a noexcept destructor at line {}.">(file.string(),
            tfl.getLineIndex());
      }
    }
  });
}

BOOST_AUTO_TEST_CASE(SourceStaticCheckTest_simpleGettersShouldBeNoExcept) {
  std::ranges::for_each(::H_HPP_FILES, [](const auto & file) {
    TextFile tfl { file };

    while (tfl.next()) {
      if (!isAComment(tfl.trim()) and tfl.contains(" get") and !tfl.contains('.')
          and !tfl.contains("->") and !tfl.contains('[') and tfl.contains(") const { return m_")) {
        LOG.warn<"The file '{}' contains a simple getter that should be noexcept at line {}.">(
          file.string(), tfl.getLineIndex());
      }
    }
  });
}

BOOST_AUTO_TEST_CASE(SourceStaticCheckTest_useChevronsForLibraryHeadersOnly) {
  std::ranges::for_each(::MY_SRC_FILES, [](const auto & file) {
    TextFile tfl { file };

    while (tfl.next()) {
      if (tfl.trim().startsWith("#include ")) {
        const auto& hasChevrons { tfl.startsWith("#include <") };
        const auto& include { extractAbsolutePathIncludeIfPossible(tfl.getLine()) };
        const auto& isOneOfMine { pa::contains(MY_SRC_FILES, include) };

        if (hasChevrons and isOneOfMine) {
          LOG.warn<"The non library file '{}' is included in file {} with chevrons instead of double quotes at line {}.">
          (
            include.string(), file.string(), tfl.getLineIndex());
        } else if (!hasChevrons and !isOneOfMine) {
          LOG.warn<"The library file '{}' is included in file '{}' with double quotes instead of chevrons at line {}.">
          (
            include.string(), file.string(), tfl.getLineIndex());
        }
      }
    }
  });
}

BOOST_AUTO_TEST_CASE(SourceStaticCheckTest_noFileShouldBeIncludedTwice) {
  std::ranges::for_each(::FILE_INCLUSIONS, [](const auto & fileToIncludes) {
    if (!pa::isSet(fileToIncludes.second)) {
      LOG.warn<"some files are included more than once in '{}'">(fileToIncludes.first.string());
    }
  });
}

BOOST_AUTO_TEST_CASE(SourceStaticCheckTest_noIncludePathShouldHaveAntiSlashPathSeparator) {
  std::ranges::for_each(::MY_SRC_FILES, [](const auto & file) {
    TextFile tfl { file };

    while (tfl.next()) {
      if (!tfl.trim().startsWith("#include ")) { continue; }

      if (const std::string includePath { extractInclude(tfl.getLine()) }; ps::contains(includePath, '\\')) {
        LOG.warn<"The file {} contains an include path '{}' with anti slashes.">(
          file.string(), includePath);
      }
    }
  });
}

BOOST_AUTO_TEST_CASE(SourceStaticCheckTest_exceptionsShouldBeThrownByValue) {
  logIfMySrcFilesContainToken("throw new ", "throw ");
}

BOOST_AUTO_TEST_CASE(SourceStaticCheckTest_noNoDiscardVoid) {
  logIfMySrcFilesContainToken("[[nodiscard]] void ", "void ");
  logIfMySrcFilesContainToken("[[nodiscard]] static void ", "static void ");
  logIfMySrcFilesContainToken("[[nodiscard]] static inline void ", "static inline void ");
  logIfMySrcFilesContainToken("[[nodiscard]] inline void ", "inline void ");
}

BOOST_AUTO_TEST_CASE(SourceStaticCheckTest_allSqlQueriesAreUsed) {
  const auto& queryNames { getAllQueryNames("sqliteQueries.hpp") };
  std::ranges::for_each(queryNames, [](std::string_view queryName) {
    if (!sourceFileContains("Database.cpp", queryName)) {
      LOG.warn<"The SQL query {} is not used in Database.cpp">(queryName);
    } else {
      LOG.info<"The SQL query {} is used in Database.cpp">(queryName);
    }
  });
}

BOOST_AUTO_TEST_CASE(SourceStaticCheckTest_allSqlQueriesContainVariables) {
  static_assert(ps::contains(phud::sql::GET_PREFLOP_STATS_BY_SITE_AND_TABLE_NAME, '?'),
                "ill-formed SQL template");
  static_assert(ps::contains(phud::sql::GET_MAX_SEATS_BY_SITE_AND_TABLE_NAME, '?'),
                "ill-formed SQL template");
  static_assert(ps::contains(phud::sql::GET_STATS_BY_SITE_AND_PLAYER_NAME, '?'),
                "ill-formed SQL template");
  static_assert(ps::contains(phud::sql::INSERT_ACTION, '?'), "ill-formed SQL template");
  static_assert(ps::contains(phud::sql::INSERT_CASHGAME, '?'), "ill-formed SQL template");
  static_assert(ps::contains(phud::sql::INSERT_TOURNAMENT, '?'), "ill-formed SQL template");
  static_assert(ps::contains(phud::sql::INSERT_GAME, '?'), "ill-formed SQL template");
  static_assert(ps::contains(phud::sql::INSERT_CASHGAME_HAND, '?'), "ill-formed SQL template");
  static_assert(ps::contains(phud::sql::INSERT_TOURNAMENT_HAND, '?'), "ill-formed SQL template");
}

BOOST_AUTO_TEST_CASE(SourceStaticCheckTest_noConstRefToIntAsFunctionParameter) {
  /* function(const int& i) is slower than function(int i) */
  logIfMySrcFilesContainToken("const int&", "int");
  logIfMySrcFilesContainToken("const char&", "char");
  logIfMySrcFilesContainToken("const std::size_t&", "std::size_t");
}

BOOST_AUTO_TEST_CASE(SourceStaticCheckTest_useStdSizeT) {
  logIfMySrcFilesContainToken(" size_t", " std::size_t");
}

BOOST_AUTO_TEST_CASE(SourceStaticCheckTest_noNULLInSourceCode) {
  /* NULL can only be used in SQL queries */
  const auto& MY_SRC_FILES_EXCEPTED_CURRENT_AND_SQL {
    pa::removeCopyIf<std::vector<fs::path>>(::MY_SRC_FILES_WITH_EXCEPTION, [](const auto & file) {
      return file.string().ends_with("sqliteQueries.hpp");
    })
  };
  logIfMySrcFilesContainToken(MY_SRC_FILES_EXCEPTED_CURRENT_AND_SQL, "NULL", "nullptr");
}

BOOST_AUTO_TEST_CASE(SourceStaticCheckTest_useAndAndOr) {
  logIfMySrcFilesContainToken(" || ", " or ");
  logIfMySrcFilesContainToken(" && ", " and ");
}

// commenté car souci sur Either.hpp
//BOOST_AUTO_TEST_CASE(SourceStaticCheckTest_constexprImpliesConst) {
//  logIfMySrcFilesContainToken("constexpr const", "constexpr");
//}

BOOST_AUTO_TEST_CASE(SourceStaticCheckTest_constexprImpliesInline) {
  std::ranges::for_each(::MY_SRC_FILES_WITH_EXCEPTION, [&](const auto & file) {
    TextFile tfl { file };

    while (tfl.next()) {
      if (const auto & line { tfl.trim().getLine() };
          ps::contains(line, "constexpr ") and ps::contains(line, "inline ")) {
        LOG.warn<"The file {} at line {} contains both the tokens 'constexpr' and 'inline', whereas constexpr implies inline.">
        (
          file.string(), tfl.getLineIndex());
      }
    }
  });
}

/* TODO : pas de const sur les attributs */

BOOST_AUTO_TEST_SUITE_END()