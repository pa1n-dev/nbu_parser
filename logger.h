#pragma once

namespace logger
{
    //������ ����� � ����
    void write_log(const string& format, auto... args)
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
        file << std::format(format, args...) << endl;
        file.close();
    }
}