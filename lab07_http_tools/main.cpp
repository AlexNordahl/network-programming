#include <iostream>
#include <memory>
#include <curl/curl.h>

int main()
{
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), curl_easy_cleanup);

    if (curl)
    {
        CURLcode res;
        curl_easy_setopt(curl.get(), CURLOPT_URL, "http://example.com");
        res = curl_easy_perform(curl.get());
    }
}