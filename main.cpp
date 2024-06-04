#define CURL_STATICLIB

#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <format>

#include <curl/curl.h> //берем curl для выполнения сетевых запросов, потому что очень удобная и популярная библиотека (https://github.com/curl/curl)
#include "dependencies/nlohmann/json.hpp" //популярная библиотека для работы с json (https://github.com/nlohmann/json) 
#include "dependencies/winreg/WinReg.hpp" //не очень популярная но простая библиотека для работы с реестром (https://github.com/GiovanniDicanio/WinReg)

using namespace std;
using namespace nlohmann;
using namespace winreg;
namespace fs = std::filesystem;

#include "settings.h"
#include "logger.h"
#include "network.h"
#include "service.h"
#include "nbu_parser_service.h"

int main()
{
    logger::write_log("main");

    nbu_service_parser parser = nbu_service_parser();

    return parser.run();
}