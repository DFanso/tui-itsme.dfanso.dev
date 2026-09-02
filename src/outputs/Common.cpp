#include "outputs/Common.hpp"

#include <sstream>

#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element heading(const std::string& s) { return text(s) | color(tone(Tone::Purple)) | bold; }

Element branch(bool last) { return text(last ? "└─▶ " : "├─▶ ") | color(tone(Tone::Muted)); }

Element t(const std::string& s, Tone tn) { return text(s) | color(tone(tn)); }

Element para(const std::string& s, Tone tn) { return paragraph(s) | color(tone(tn)); }

Element richPara(const std::vector<std::pair<std::string, Tone>>& segments) {
  Elements words;
  for (const auto& [str, tn] : segments) {
    std::istringstream in(str);
    std::string word;
    while (in >> word) words.push_back(text(word + " ") | color(tone(tn)));
  }
  return hflow(std::move(words));
}

Element tagRow(const std::vector<std::string>& items) {
  Elements cells;
  for (const auto& item : items)
    cells.push_back(hbox({text("│ ") | color(tone(Tone::Muted)), text(item + "  ") | color(tone(Tone::Green))}));
  return hflow(std::move(cells));
}

Element indent(Element e, int cols) {
  return hbox({text(std::string(static_cast<std::size_t>(cols), ' ')), std::move(e)});
}

Element blank() { return text(""); }

}  // namespace itsme::outputs
