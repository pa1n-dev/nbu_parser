#pragma once

namespace logger
{
    //запись логов в файл
    template<typename... T>
    void write_log(const string& format, const T&... args)
    {
        if (!settings::logs)
            return;

        fs::path dir(settings::path);

        if (!fs::exists(dir)) //проверяем существует ли путь, если нет то создаём
            fs::create_directories(dir);

        ofstream file(dir / settings::logs_filename, ios::app);
        if (!file.is_open())
            return;

        //форматируем данные и записываем в файл
        file << format;
        ((file << " " << args), ...); //печатаем все аргументы из пакета args
        file << endl;
        file.close();
    }
}