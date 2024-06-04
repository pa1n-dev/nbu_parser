#pragma once

class service
{
public:
    service();
    int run();

protected:
    static service* instance;

    HANDLE stop_event;
    HANDLE pause_event;
    HANDLE continue_event;

    HANDLE worker_paused;
    HANDLE worker_continued;

    void confirm_pause() { SetEvent(worker_paused); }
    void confirm_continue() { SetEvent(worker_continued); }

private:
    static void WINAPI service_main();
    static void WINAPI control_handler(DWORD code);
    static DWORD WINAPI worker_thread(LPVOID lpParam);

    SERVICE_STATUS status;
    SERVICE_STATUS_HANDLE status_handle;

    void startup();
    void exit();
    void error();

    void control_stop();
    void control_pause();
    void control_continue();
    void control_stop_on_pause();

    void set_state(DWORD state);
    void set_controls_accepted(bool need);

private:
    virtual DWORD WINAPI worker(LPVOID)
    {
        return ERROR_SUCCESS;
    }

    virtual void on_startup() {};
    virtual void on_pause() {};
    virtual void on_continue() {};
    virtual void on_stop() {};
    virtual void on_exit() {};
};

service* service::instance;

service::service()
{
	instance = this;

	status = {};
	ZeroMemory(&status, sizeof(status));

	status_handle = NULL;

	stop_event = INVALID_HANDLE_VALUE;
	pause_event = INVALID_HANDLE_VALUE;
	continue_event = INVALID_HANDLE_VALUE;
}

wchar_t service_name[] = L"test";

int service::run()
{
	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{service_name, (LPSERVICE_MAIN_FUNCTION)service_main},
		{NULL, NULL}
	};

	//StartServiceCtrlDispatcher - подключает поток main процесса службы к диспетчеру управления службами, что приводит к тому, что поток будет потоком диспетчера управления службой для вызывающего процесса.
	if (!StartServiceCtrlDispatcher(ServiceTable))
		logger::write_log("StartServiceCtrlDispatcher error: ", GetLastError());

	return ERROR_SUCCESS;
}

void WINAPI service::service_main()
{
	//RegisterServiceCtrlHandler - регистрирует функцию для обработки запросов управления службой
	instance->status_handle = RegisterServiceCtrlHandler(service_name, control_handler);
	if (!instance->status_handle)
		return;

	instance->startup();

	//создаём основной поток в котором будем работать
	HANDLE hThread = CreateThread(NULL, 0, worker_thread, NULL, 0, NULL);

	//блокируеv основной поток до тех пор, пока не завершится работа worker_thread
	WaitForSingleObject(hThread, INFINITE);

	instance->exit();
}

void WINAPI service::control_handler(DWORD code)
{
	//обрабатываем коды управления службам, нужно что бы можно было остановить службу итд.
	switch (code)
	{
	case SERVICE_CONTROL_STOP:
	{
		if (instance->status.dwCurrentState == SERVICE_PAUSED)
		{
			instance->control_stop_on_pause();
			break;
		}

		if (instance->status.dwCurrentState != SERVICE_RUNNING)
			break;

		instance->control_stop();
		break;
	}
	case SERVICE_CONTROL_PAUSE:
	{
		if (instance->status.dwCurrentState != SERVICE_RUNNING)
			break;

		instance->control_pause();
		break;
	}
	case SERVICE_CONTROL_CONTINUE:
	{
		if (instance->status.dwCurrentState != SERVICE_PAUSED)
			break;

		instance->control_continue();
		break;
	}
	case SERVICE_CONTROL_SHUTDOWN:
	{
		if (instance->status.dwCurrentState != SERVICE_RUNNING)
			break;

		instance->control_stop();
		break;

	}
	case SERVICE_CONTROL_INTERROGATE: //нужно что бы диспетчер управления службами получал обновленное состояние службы
	{
		SetServiceStatus(instance->status_handle, &instance->status);
		break;
	}
	default:
		break;
	}
}

/*Internal functions*/
void service::startup()
{
	status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	status.dwServiceSpecificExitCode = 0;
	set_controls_accepted(false);
	set_state(SERVICE_START_PENDING);

	on_startup();

	stop_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	pause_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	continue_event = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (!stop_event || !pause_event || !continue_event)
	{
		error();
		return;
	}

	set_controls_accepted(true);
	set_state(SERVICE_RUNNING);
}

void service::exit()
{
	on_exit();
	CloseHandle(stop_event);
	CloseHandle(pause_event);
	CloseHandle(continue_event);
	CloseHandle(worker_paused);
	CloseHandle(worker_continued);
	set_state(SERVICE_STOPPED);
}

void service::error()
{
	logger::write_log("Error: ", GetLastError());

	set_controls_accepted(false);
	status.dwCurrentState = SERVICE_STOPPED;
	status.dwWin32ExitCode = GetLastError();
	status.dwCheckPoint = 0;

	if (!SetServiceStatus(status_handle, &status))
		logger::write_log("SetServiceStatus error: ", GetLastError());
}

/*Controls*/
void service::control_stop()
{
	set_state(SERVICE_STOP_PENDING);
	on_stop();
	set_controls_accepted(false);
	SetEvent(stop_event);
}

void service::control_pause()
{
	set_state(SERVICE_PAUSE_PENDING);
	on_pause();
	SetEvent(pause_event);
	WaitForSingleObject(worker_paused, INFINITE);
	set_state(SERVICE_PAUSED);
}

void service::control_continue()
{
	set_state(SERVICE_CONTINUE_PENDING);
	set_controls_accepted(false);
	ResetEvent(pause_event);
	on_continue();
	SetEvent(continue_event);
	WaitForSingleObject(worker_continued, INFINITE);
	set_controls_accepted(true);
	ResetEvent(continue_event);
	set_state(SERVICE_RUNNING);
}

void service::control_stop_on_pause()
{
	set_controls_accepted(false);
	set_state(SERVICE_STOP_PENDING);
	on_continue();
	SetEvent(stop_event);
	SetEvent(continue_event);
}

void service::set_controls_accepted(bool need)
{
	status.dwControlsAccepted = 0;

	if (need)
		status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

	if (!SetServiceStatus(status_handle, &status))
		logger::write_log("SetServiceStatus error: ", GetLastError());
}

void service::set_state(DWORD state)
{
	status.dwCurrentState = state;
	status.dwWin32ExitCode = 0;
	status.dwCheckPoint = 0;
	status.dwWaitHint = 0;

	if (!SetServiceStatus(status_handle, &status))
		logger::write_log("SetServiceStatus error: ", GetLastError());
}

DWORD WINAPI service::worker_thread(LPVOID lpParam)
{
	return instance->worker(lpParam);
}