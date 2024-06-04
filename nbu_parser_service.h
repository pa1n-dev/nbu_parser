#pragma once

class nbu_service_parser : public service
{
	using service::service;

protected:
	virtual DWORD WINAPI worker(LPVOID) 
	{
		auto last_execution_time = chrono::steady_clock::now() - chrono::milliseconds(settings::update_rate);

		while (WaitForSingleObject(stop_event, 0) != WAIT_OBJECT_0)
		{
			if (WaitForSingleObject(pause_event, 0) != WAIT_OBJECT_0)
			{
				auto current_time = chrono::steady_clock::now();
				auto elapsed_since_last_execution = chrono::duration_cast<chrono::milliseconds>(current_time - last_execution_time);

				if (elapsed_since_last_execution.count() >= settings::update_rate)
				{
					string data;
					CURLcode code = network::get_website_data("https://bank.gov.ua/NBUStatService/v1/statdirectory/exchange?json", data);
					if (code == CURLE_OK)
					{
						json json_data;

						fs::path dir(settings::path);

						if (!fs::exists(dir)) //��������� ���������� �� ����, ���� ��� �� ������
							fs::create_directories(dir);

						//���� ���� ������ ������ � �����, �� ��������� �� � json
						ifstream infile(dir / settings::rates_filename);
						if (infile.is_open())
						{
							infile >> json_data;
							infile.close();
						}

						for (const auto& item : json::parse(data))
						{
							for (const auto& currency : settings::currencies)
							{
								//��������� ������ � ���������� �� � json
								if (item["cc"] == currency)
									json_data.push_back({ {"currency", item["cc"]}, {"price", item["rate"]} });
							}
						}

						//����� ������ ���������� � ����
						ofstream outfile(dir / settings::rates_filename);
						if (outfile.is_open())
						{
							outfile << json_data.dump(4);
							outfile.close();
						}
					}

					last_execution_time = current_time;
				}

				//������������� ����� �� �����-�� ����� (��� ���������� �������� �� ��)
				std::this_thread::sleep_for(chrono::milliseconds(100));
			}
			else 
			{
				confirm_pause();
				WaitForSingleObject(continue_event, INFINITE);
				confirm_continue();
			}
		}

		return ERROR_SUCCESS;
	}

	//���������� �� ����� �������
	virtual void on_startup()
	{
		logger::write_log("on_startup");

		settings::load();
	}

	//����������, ����� ������������ �����
	virtual void on_pause()
	{
		logger::write_log("on_pause");
	}

	//����������, ����� ������������ �����������, ����� �����
	virtual void on_continue()
	{
		logger::write_log("on_continue");
	}

	//����������, ����� windows �������� � ������������� ���������
	virtual void on_stop()
	{
		logger::write_log("on_stop");
	}

	//���������� � ����� ���������� �����
	virtual void on_exit()
	{
		logger::write_log("on_exit");
	}
};