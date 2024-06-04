#pragma once

namespace settings
{
    wstring path = L"C:\\nbu_parser";	                //путь для сохранения данных
    wstring rates_filename = L"rates.json";		        //название файла с данными о валюте
    wstring logs_filename = L"logs.txt";		        //название файла с логами

    json currencies = json::parse(LR"(["EUR", "USD"])");//какие валюты нужно сохранять

    int update_rate = 60000;		                    //частота сохранения данных о валюте в миллисекундах
    bool logs = true;			                        //нужно ли записывать логи

    //простая загрузка данных из реестра
    void load()
    {
        RegKey key;
        RegResult result = key.TryOpen(HKEY_LOCAL_MACHINE, L"SOFTWARE\\nbu_parser");

        if (!result)
            key.Create(HKEY_LOCAL_MACHINE, L"SOFTWARE\\nbu_parser");

        if (!key.TryGetStringValue(L"path").IsValid())
            key.SetStringValue(L"path", L"C:\\\\nbu_parser");

        path = key.GetStringValue(L"path");

        if (!key.TryGetStringValue(L"rates_filename").IsValid())
            key.SetStringValue(L"rates_filename", L"rates.json");

        rates_filename = key.GetStringValue(L"rates_filename");

        if (!key.TryGetStringValue(L"logs_filename").IsValid())
            key.SetStringValue(L"logs_filename", L"logs.txt");

        logs_filename = key.GetStringValue(L"logs_filename");

        if (!key.TryGetStringValue(L"currencies").IsValid())
            key.SetStringValue(L"currencies", LR"(["EUR", "USD"])");

        currencies = json::parse(key.GetStringValue(L"currencies"));

        if (!key.TryGetDwordValue(L"update_rate").IsValid())
            key.SetDwordValue(L"update_rate", 60000);

        update_rate = key.GetDwordValue(L"update_rate");

        if (!key.TryGetDwordValue(L"logs").IsValid())
            key.SetDwordValue(L"logs", 1);

        logs = (bool)key.GetDwordValue(L"logs");
    }
}