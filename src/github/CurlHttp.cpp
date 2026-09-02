#include "github/CurlHttp.hpp"

#include <curl/curl.h>

#include <mutex>

namespace itsme::github {

namespace {
std::size_t writeBody(char* ptr, std::size_t size, std::size_t nmemb, void* userdata) {
  auto* out = static_cast<std::string*>(userdata);
  out->append(ptr, size * nmemb);
  return size * nmemb;
}

void ensureGlobalInit() {
  static std::once_flag once;
  std::call_once(once, [] { curl_global_init(CURL_GLOBAL_DEFAULT); });
}
}  // namespace

HttpFn curlHttp(long timeoutSeconds) {
  ensureGlobalInit();
  return [timeoutSeconds](const std::string& url, const std::vector<std::string>& headers,
                          const std::optional<std::string>& postBody) -> std::optional<HttpResponse> {
    CURL* curl = curl_easy_init();
    if (!curl) return std::nullopt;

    struct curl_slist* list = nullptr;
    for (const auto& h : headers) list = curl_slist_append(list, h.c_str());

    std::string body;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeBody);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeoutSeconds);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeoutSeconds);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    if (postBody) {
      curl_easy_setopt(curl, CURLOPT_POST, 1L);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postBody->c_str());
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(postBody->size()));
    }

    const CURLcode rc = curl_easy_perform(curl);
    long status = 0;
    if (rc == CURLE_OK) curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_slist_free_all(list);
    curl_easy_cleanup(curl);
    if (rc != CURLE_OK) return std::nullopt;
    return HttpResponse{status, std::move(body)};
  };
}

}  // namespace itsme::github
