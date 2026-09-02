#include <catch2/catch_test_macros.hpp>

#include "data/Portfolio.hpp"

using namespace itsme::data;

TEST_CASE("profile basics") {
  CHECK(profile().name == "Leo Felcianas");
  CHECK(profile().githubHandle == "github.com/DFanso");
  CHECK(profile().resumeUrl == "https://itsme.dfanso.dev/resume.pdf");
}

TEST_CASE("companies and roles are complete") {
  REQUIRE(companies().size() == 7);
  CHECK(companies()[0].company == "CD Extreme OPC");
  CHECK(companies()[1].roles.size() == 3);
  CHECK(companies()[1].roles[0].title == "DevOps Engineer");
  CHECK(companies()[1].roles[0].typeTone == itsme::core::Tone::Yellow);
  CHECK(companies()[6].company == "FOSS Community - NSBM");
  CHECK(companies()[0].roles[0].tech.size() == 13);
}

TEST_CASE("education, certifications, skills, projects, contact") {
  REQUIRE(education().size() == 2);
  CHECK(education()[0].grade == "First-Class Honours");
  REQUIRE(certifications().size() == 2);
  CHECK(certifications()[1].expiry.empty());
  REQUIRE(skillCategories().size() == 10);
  CHECK(skillCategories()[0].name == "cloud");
  CHECK(skillCategories()[9].skills.back() == "CQRS / Mediator");
  REQUIRE(projects().size() == 7);
  CHECK(projects()[2].name == "techxeed");
  CHECK_FALSE(projects()[2].github.has_value());
  CHECK(projects()[0].github == std::optional<std::string>{"DFanso/DevOps-Project-001"});
  REQUIRE(contactLinks().size() == 6);
  CHECK(contactLinks()[1].id == "LO");
  CHECK_FALSE(contactLinks()[1].url.has_value());
}
