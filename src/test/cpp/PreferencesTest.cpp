#include "TestInfrastructure.hpp"
#include "gui/Preferences.hpp"

BOOST_AUTO_TEST_SUITE(PreferencesTest)

static constexpr auto IN_MEMORY { true };

BOOST_AUTO_TEST_CASE(PreferencesTest_getDefaultValues) {
  const Preferences prefs(IN_MEMORY);
  const auto [x, y] { prefs.getMainWindowPosition() };
  const  auto historyDir { prefs.getPreferredHistoDir() };

  // Vérifier que les valeurs par défaut sont cohérentes
  BOOST_CHECK(x >= 0);
  BOOST_CHECK(y >= 0);
  BOOST_CHECK_NO_THROW(std::ignore = historyDir.string());
}

BOOST_AUTO_TEST_CASE(TestPreferencesTest_saveAndLoad) {
  const Preferences prefs(IN_MEMORY);

  prefs.saveStringPreference("test_key", "test_value");
  prefs.saveIntPreference("test_int", 42);

  BOOST_CHECK_EQUAL(prefs.getStringPreference("test_key"), "test_value");
  BOOST_CHECK_EQUAL(prefs.getIntPreference("test_int"), 42);
}

BOOST_AUTO_TEST_CASE(PreferencesTest_defaultValuesForMissingKeys) {
  const Preferences prefs(IN_MEMORY);

  // Test des valeurs par défaut pour des clés manquantes
  BOOST_CHECK_EQUAL(prefs.getStringPreference("missing_key", "default"), "default");
  BOOST_CHECK_EQUAL(prefs.getIntPreference("missing_int", 100), 100);
  BOOST_CHECK_EQUAL(prefs.getStringPreference("missing_key"), ""); // défaut vide
  BOOST_CHECK_EQUAL(prefs.getIntPreference("missing_int"), 0);     // défaut 0
}

BOOST_AUTO_TEST_CASE(PreferencesTest_windowPositionAndSize) {
  Preferences prefs(IN_MEMORY);

  prefs.saveWindowPosition(150, 200);
  prefs.saveWindowSize(800, 600);

  const auto [x, y] { prefs.getMainWindowPosition() };
  BOOST_CHECK_EQUAL(x, 150);
  BOOST_CHECK_EQUAL(y, 200);
}

BOOST_AUTO_TEST_CASE(PreferencesTest_historyDirectory) {
  Preferences prefs(IN_MEMORY);

  // Test avec un répertoire inexistant (retourne empty path)
  prefs.saveHistoryDirectory("/path/that/does/not/exist");
  const auto& dir { prefs.getPreferredHistoDir() };
  BOOST_CHECK(dir.empty());

  // Test du label par défaut
  const auto& label { prefs.getHistoryDirectoryDisplayLabel() };
  BOOST_CHECK(!label.empty());
}

BOOST_AUTO_TEST_SUITE_END()
