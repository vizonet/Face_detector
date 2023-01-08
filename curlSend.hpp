#include "curl/curl.h"

CURL *curl;
CURLcode res;

void sendHTTP(int box, bool blink, bool stare)
{

    std::cout << "stare " << (stare ? "yes" : "no") << "\n";

    if (curl)
    {
        std::cout << "send";
        // res = curl_easy_perform(curl);
    }
    else
    {
        // std::cout<<"setup";
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        // curl_easy_perform(curl);
    }

    std:: string addrcp = "http://localhost:8080/cpp/look/" + std::to_string(box) + "/stare/" + (stare ? "yes" : "no");
    //char addr[addrcp.length() + 1];
    char *addr = new char(addrcp.length() + 1);
    strcpy(addr, addrcp.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, addr);

    res = curl_easy_perform(curl);
}