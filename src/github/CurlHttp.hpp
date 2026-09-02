#pragma once
#include "github/Client.hpp"

namespace itsme::github {
HttpFn curlHttp(long timeoutSeconds = 10);
}
