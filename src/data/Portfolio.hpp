#pragma once
#include <optional>
#include <string>
#include <vector>

#include "core/Command.hpp"

namespace itsme::data {

struct Profile {
  std::string name, title, tagline, location, email, linkedin, linkedinHandle, github, githubHandle, portfolio,
      portfolioHandle, resumeUrl, summary;
};

struct Role {
  std::string type;
  core::Tone typeTone;
  std::string title;
  std::string period;
  std::vector<std::string> responsibilities;
  std::vector<std::string> tech;
};

struct Company {
  std::string company;
  std::string totalPeriod;
  std::vector<Role> roles;
};

struct EducationEntry {
  std::string degree, institution, grade, period, location;
};

struct Certification {
  std::string name, issuer, date, expiry, id;
};

struct SkillCategory {
  std::string name;
  std::vector<std::string> skills;
};

struct Project {
  std::string type, name, url;
  std::optional<std::string> github;
  std::string description;
  std::vector<std::string> tech;
};

struct ContactLink {
  std::string id, name;
  std::optional<std::string> url;
};

const Profile& profile();
const std::vector<Company>& companies();
const std::vector<EducationEntry>& education();
const std::vector<Certification>& certifications();
const std::vector<SkillCategory>& skillCategories();
const std::vector<Project>& projects();
const std::vector<ContactLink>& contactLinks();

}  // namespace itsme::data
