#include "effects/Typewriter.hpp"

namespace itsme::effects {

void Typewriter::advance(int ms) {
  if (done()) return;
  carry_ += ms;
  const int chars = msPerChar_ > 0 ? carry_ / msPerChar_ : total_;
  carry_ -= chars * msPerChar_;
  revealed_ += chars;
  if (revealed_ > total_) revealed_ = total_;
}

}  // namespace itsme::effects
