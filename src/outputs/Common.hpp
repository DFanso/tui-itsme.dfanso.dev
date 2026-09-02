#pragma once
#include <ftxui/dom/elements.hpp>
#include <string>
#include <utility>
#include <vector>

#include "core/Command.hpp"

namespace itsme::outputs {

ftxui::Element heading(const std::string& s);                // purple bold
ftxui::Element branch(bool last);                            // "├─▶ " or "└─▶ " muted
ftxui::Element t(const std::string& s, core::Tone tone);     // colored text
ftxui::Element para(const std::string& s, core::Tone tone);  // word-wrapped
ftxui::Element richPara(const std::vector<std::pair<std::string, core::Tone>>& segments);
ftxui::Element tagRow(const std::vector<std::string>& items);  // "│ item" flow, green
ftxui::Element indent(ftxui::Element e, int cols);
ftxui::Element blank();

}  // namespace itsme::outputs
