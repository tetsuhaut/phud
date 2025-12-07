#pragma once

#include "TestInfrastructure.hpp"
#include "db/Database.hpp"
#include "entities/Site.hpp"
#include "history/PokerSiteHistory.hpp"
#include "log/Logger.hpp"
#include <memory>

namespace pt = phud::test;

/**
 * Shared test fixture for sabre_laser test data.
 * Loads the large sabre_laser directory once and reuses it across multiple tests.
 * This significantly improves test performance by avoiding repeated parsing
 * of hundreds of history files and database population.
 */
struct SabreLaserFixture {
  // Shared Site instance loaded once for all tests using this fixture
  inline static std::unique_ptr<Site> pSabreLaserSite {nullptr};
  // Shared Database instance populated once for all tests
  inline static std::unique_ptr<Database> pDatabase {nullptr};
  inline static bool isLoaded = false;

  SabreLaserFixture() {
    if (!isLoaded) {
      static Logger& LOG {[]() -> Logger& {
        static Logger logger {"SabreLaserFixture"};
        return logger;
      }()};

      LOG.info<"Loading sabre_laser test data (one-time setup)...">();
      const auto dir = pt::getDirFromTestResources("Winamax/sabre_laser");
      pSabreLaserSite = PokerSiteHistory::load(dir);

      // Create and populate database once
      pDatabase = std::make_unique<Database>();
      pDatabase->save(*pSabreLaserSite);

      isLoaded = true;
      LOG.info<"sabre_laser test data loaded and saved to database successfully.">();
    }
  }

  // Accessor for the shared Site
  const Site& getSite() const { return *pSabreLaserSite; }

  // Accessor for the shared Database
  Database& getDatabase() const { return *pDatabase; }

  // Prevent accidental modifications
  Site* operator->() const { return pSabreLaserSite.get(); }
};
