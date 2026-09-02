#include <catch2/catch_test_macros.hpp>

#include "core/LineEditor.hpp"

using itsme::core::LineEditor;

TEST_CASE("insert and cursor movement over code points") {
  LineEditor ed;
  ed.insert("ab");
  CHECK(ed.text() == "ab");
  CHECK(ed.cursor() == 2);
  ed.left();
  ed.insert("❯");
  CHECK(ed.text() == "a❯b");
  CHECK(ed.cursor() == 2);
  ed.home();
  CHECK(ed.cursor() == 0);
  ed.end();
  CHECK(ed.cursor() == 3);
  ed.right();
  CHECK(ed.cursor() == 3);
}

TEST_CASE("backspace and delete") {
  LineEditor ed;
  ed.set("héllo");
  CHECK(ed.cursor() == 5);
  ed.backspace();
  CHECK(ed.text() == "héll");
  ed.home();
  ed.backspace();
  CHECK(ed.text() == "héll");
  ed.right();
  ed.del();
  CHECK(ed.text() == "hll");
  CHECK(ed.cursor() == 1);
  ed.clear();
  CHECK(ed.text().empty());
  CHECK(ed.cursor() == 0);
}

TEST_CASE("split around cursor") {
  LineEditor ed;
  ed.set("abc");
  ed.left();
  CHECK(ed.before() == "ab");
  CHECK(ed.at() == "c");
  CHECK(ed.after() == "");
  ed.end();
  CHECK(ed.at() == "");
}
