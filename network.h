#pragma once

namespace network
{
    size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp)
    {
        ((string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    //получаем данные из веб сайта
    CURLcode get_website_data(const string& url, string& data)
    {
        CURL* curl;
        CURLcode code;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl)
        {
            //устанавливаем параметры для curl
            curl_easy_setopt(curl, CURLOPT_URL, "https://bank.gov.ua/NBUStatService/v1/statdirectory/exchange?json");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

            //выполняем сетевой запрос
            code = curl_easy_perform(curl);

            curl_easy_cleanup(curl);
        }

        curl_global_cleanup();

        return code;
    }
}