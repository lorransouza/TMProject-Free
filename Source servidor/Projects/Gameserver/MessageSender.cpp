#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "MessageSender.h"
#include "CNPCGener.h"
#include <thread>
#include <chrono>
#include <cstring>

using namespace std::chrono_literals;

TMessageSender::TMessageSender()
{
	Initialize();
}

TMessageSender::~TMessageSender()
{
	disposing = true;

	thread.join();
}

void TMessageSender::Initialize()
{
	thread = std::thread(&TMessageSender::MessageSender, this);
	disposing = false;
}

void TMessageSender::MessageSender()
{
	while (!disposing)
	{
		try
		{
			for (int i = 1; i < MAX_PLAYER; i++)
			{
				if (Users[i].Status == USER_EMPTY)
					continue;

				CUser& user = Users[i];

				{
					std::lock_guard<decltype(CUser::messageMutex)> mutex{ user.messageMutex };
					int sendSize = user.Socket.nSendPosition - user.Socket.nSentPosition;
					if (sendSize == 0)
						continue;
				}

				if (!Users[i].SendMessageA())
					CloseUser(i);
			}

			Sleep(50);
		}
		catch (const std::exception& e)
		{
			Log(SERVER_SIDE, LOG_ERROR, "Exceção ocorrida em MessageSender. Mensagem> %s", e.what());
		}
	}
}