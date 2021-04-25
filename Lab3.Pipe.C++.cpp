// Lab3.Pipe.C++.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "Header.h"

#define BASE_PATH_PIPE L"\\\\.\\pipe\\"					//базовый путь канала
#define SIZE_BUFFER 2048								//размер канала

#define MESSAGE L"Message received"						//возврат сообщения
#define SIZE_MSG_BYTES 32								//размер сообщения

DWORD WINAPI InstanceThread(_In_ LPVOID hPipe);			//функция потока

int main(int argc, char* params[])
{
	BOOL isConnected = FALSE;						
	DWORD dwThreadId = 0;			
	HANDLE hPipe = NULL;				//дескриптор на канал
	std::wstring namePipe(BASE_PATH_PIPE);
	std::wstring tmp;
	std::list<HANDLE> threads;		//перечень потоков

	std::cout << "Write name of pipe: " << std::endl;
	std::wcin >> tmp;
	namePipe.append(tmp);		//ввод клиентом названия канала

	while (TRUE)
	{
		std::wcout << L"Main thread awake creating pipe with path: "
			<< namePipe << std::endl;
		std::wcout << L"Wait for client" << std::endl;			

		//создание канала
		hPipe = CreateNamedPipeW(
			namePipe.c_str(),			//название
			PIPE_ACCESS_DUPLEX,			//режим канала
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,		//особенности канала
			PIPE_UNLIMITED_INSTANCES,		//кол-во экзмепляров соединение, которые можно создать(ограничение кол-ва людей на сервере)
			SIZE_BUFFER,		//размер буфера чтения и записи
			SIZE_BUFFER,
			INFINITE,		//ожидание
			NULL);			//указатель на структуру безопасности

		if (hPipe == INVALID_HANDLE_VALUE)
		{
			std::wcout << L"Error of creating pipe! Process will be terminated!" << std::endl;
			return EXIT_FAILURE;
		}

		//подключение к каналу
		isConnected = ConnectNamedPipe(hPipe, NULL);
		if (isConnected)
		{
			std::wcout << L"Client connected. Creating thread for user" << std::endl;

			//создание потока для работы с пользователем
			HANDLE hThread = CreateThread(
				NULL,
				NULL,
				InstanceThread,		
				(LPVOID)hPipe,
				NULL,
				&dwThreadId);
			if (hThread == INVALID_HANDLE_VALUE)
				std::wcout << L"Error of making thread!" << std::endl;
			else
			{
				++dwThreadId;
				threads.push_back(hThread);   //добавление в перечень потоков
			}
		}
		else
		{
			std::wcout << L"Error of connection user" << std::endl;
			CloseHandle(hPipe);		//при ошибке закрываем канал
		}
	}

	std::for_each(threads.cbegin(), threads.cend(),
		[](HANDLE h)
	{
		CloseHandle(h);		//проходит по переченю потоков и закрывает в каждом из элементов его дескриптор
	});
	CloseHandle(hPipe);

	return EXIT_SUCCESS;
}

DWORD WINAPI InstanceThread(_In_ LPVOID hPipe)
{
	LPWSTR strRequest = new WCHAR[SIZE_BUFFER + 1];			//запись и чтение данных
	memset(strRequest, 0, (SIZE_BUFFER + 1) * sizeof(WCHAR));		
	DWORD cntBytesRead = 0;
	DWORD cntBytesWrited = 0;
	BOOL isSucsses = FALSE;

	HANDLE pipe = (HANDLE)hPipe;
	
	std::wcout << L"Instance created, and wait for messages" << std::endl;

	while (TRUE)
	{
		isSucsses = ReadFile(		//чтение
			pipe,
			strRequest,
			SIZE_BUFFER * sizeof(WCHAR),
			&cntBytesRead,
			NULL);

		if (!isSucsses)
		{
			std::wcout << "Error of read user message or user disconnect" << std::endl;
			break;
		}
		else
			std::wcout << strRequest << std::endl;

		isSucsses = WriteFile(		//запись сообщения
			pipe,
			MESSAGE,
			SIZE_MSG_BYTES,
			&cntBytesWrited,
			NULL
		);

		if (!isSucsses || cntBytesWrited != SIZE_MSG_BYTES)
		{
			std::wcout << L"Error of reply!" << std::endl;
			break;
		}
		else
			std::wcout << L"Message will be replied" << std::endl;
	}

	if (strRequest)
		delete[] strRequest;
	CloseHandle(pipe);
	ExitThread(0);
}



