#pragma once

namespace logger
{
    //������ ����� � ����
    template<typename... T>
    void write_log(const string& format, const T&... args)
    {
        if (!settings::logs)
            return;

        fs::path dir(settings::path);

        if (!fs::exists(dir)) //��������� ���������� �� ����, ���� ��� �� ������
            fs::create_directories(dir);

        ofstream file(dir / settings::logs_filename, ios::app);
        if (!file.is_open())
            return;

        //����������� ������ � ���������� � ����
        file << format;
        ((file << " " << args), ...); //�������� ��� ��������� �� ������ args
        file << endl;
        file.close();
    }
}