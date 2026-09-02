#include "core/LineEditor.hpp"

#include "core/Strings.hpp"

namespace itsme::core {

void LineEditor::rebuild() {
  text_.clear();
  for (const auto& c : chars_) text_ += c;
  if (cursor_ > chars_.size()) cursor_ = chars_.size();
}

void LineEditor::insert(std::string_view s) {
  auto incoming = utf8Chars(s);
  chars_.insert(chars_.begin() + static_cast<std::ptrdiff_t>(cursor_), incoming.begin(), incoming.end());
  cursor_ += incoming.size();
  rebuild();
}

void LineEditor::backspace() {
  if (cursor_ == 0) return;
  chars_.erase(chars_.begin() + static_cast<std::ptrdiff_t>(cursor_ - 1));
  --cursor_;
  rebuild();
}

void LineEditor::del() {
  if (cursor_ >= chars_.size()) return;
  chars_.erase(chars_.begin() + static_cast<std::ptrdiff_t>(cursor_));
  rebuild();
}

void LineEditor::left() {
  if (cursor_ > 0) --cursor_;
}
void LineEditor::right() {
  if (cursor_ < chars_.size()) ++cursor_;
}
void LineEditor::home() { cursor_ = 0; }
void LineEditor::end() { cursor_ = chars_.size(); }

void LineEditor::set(std::string s) {
  chars_ = utf8Chars(s);
  cursor_ = chars_.size();
  rebuild();
}

void LineEditor::clear() {
  chars_.clear();
  cursor_ = 0;
  rebuild();
}

std::string LineEditor::before() const {
  std::string out;
  for (std::size_t i = 0; i < cursor_ && i < chars_.size(); ++i) out += chars_[i];
  return out;
}

std::string LineEditor::at() const { return cursor_ < chars_.size() ? chars_[cursor_] : std::string(); }

std::string LineEditor::after() const {
  std::string out;
  for (std::size_t i = cursor_ + 1; i < chars_.size(); ++i) out += chars_[i];
  return out;
}

}  // namespace itsme::core
