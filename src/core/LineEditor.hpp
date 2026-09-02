#pragma once
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace itsme::core {

// Text + cursor model for the input line. Cursor is an index in code points.
class LineEditor {
 public:
  const std::string& text() const { return text_; }
  std::size_t cursor() const { return cursor_; }
  std::size_t length() const { return chars_.size(); }

  void insert(std::string_view s);
  void backspace();
  void del();
  void left();
  void right();
  void home();
  void end();
  void set(std::string s);  // replaces text, cursor at end
  void clear();

  std::string before() const;  // text before cursor
  std::string at() const;      // code point under cursor ("" at end)
  std::string after() const;   // text after the cursor code point

 private:
  void rebuild();
  std::string text_;
  std::vector<std::string> chars_;
  std::size_t cursor_ = 0;
};

}  // namespace itsme::core
