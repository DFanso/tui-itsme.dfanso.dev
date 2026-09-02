#include <catch2/catch_test_macros.hpp>

#include "../RenderHelper.hpp"
#include "outputs/Outputs.hpp"

using namespace itsme::outputs;

TEST_CASE("help lists visible commands only") {
  auto s = renderPlain(renderHelp(), 100, 60);
  CHECK(s.find("Terminal Portfolio Help") != std::string::npos);
  CHECK(s.find("AVAILABLE COMMANDS:") != std::string::npos);
  CHECK(s.find("neofetch") != std::string::npos);
  CHECK(s.find("Show GitHub stats and contributions") != std::string::npos);
  CHECK(s.find("sudo") == std::string::npos);
  CHECK(s.find("Ctrl+L") != std::string::npos);
}

TEST_CASE("ls lists directories then files with perms") {
  auto s = renderPlain(renderLs(), 100, 40);
  CHECK(s.find("Directory listing of ~/portfolio") != std::string::npos);
  CHECK(s.find("drwxr-xr-x") != std::string::npos);
  CHECK(s.find("resume.pdf") != std::string::npos);
  CHECK(s.find("about/") < s.find("welcome.txt"));
  CHECK(s.find("curriculum vitae") != std::string::npos);
}

TEST_CASE("skills and experience render trees") {
  auto sk = renderPlain(renderSkills(), 120, 80);
  CHECK(sk.find("Technical Skills") != std::string::npos);
  CHECK(sk.find("~/cloud/") != std::string::npos);
  CHECK(sk.find("CQRS / Mediator") != std::string::npos);
  auto ex = renderPlain(renderExperience(), 120, 200);
  CHECK(ex.find("Experience History") != std::string::npos);
  CHECK(ex.find("CD Extreme OPC") != std::string::npos);
  CHECK(ex.find("[DEV]") != std::string::npos);
  CHECK(ex.find("Associate DevOps Engineer") != std::string::npos);
  CHECK(ex.find("Premiere Pro") != std::string::npos);
}

TEST_CASE("projects with and without stats") {
  auto none = renderPlain(renderProjects(nullptr), 120, 80);
  CHECK(none.find("Featured Projects") != std::string::npos);
  CHECK(none.find("projects/eks-gitops-platform/") != std::string::npos);
  CHECK(none.find("type: WEB") != std::string::npos);
  CHECK(none.find("★") == std::string::npos);
  itsme::github::ProjectStatsMap stats;
  stats["DFanso/k3s"] = itsme::github::ProjectRepoStats{12, 3, 4};
  stats["DFanso/rss"] = std::nullopt;
  auto with = renderPlain(renderProjects(&stats), 120, 80);
  CHECK(with.find("★ 12") != std::string::npos);
  CHECK(with.find("⑂ 3") != std::string::npos);
  CHECK(with.find("👁 4") != std::string::npos);
}

TEST_CASE("neofetch") {
  auto s = renderPlain(renderNeofetch(), 100, 20);
  CHECK(s.find("OS:") != std::string::npos);
  CHECK(s.find("dfanso.dev") != std::string::npos);
  CHECK(s.find("Portfolio-CLI") != std::string::npos);
}
