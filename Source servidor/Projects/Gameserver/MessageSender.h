#ifndef MessageSenderH
#define MessageSenderH

#include <thread>
#include <mutex>

class TMessageSender
{
private:
	std::thread thread;

	volatile bool disposing{ false };

	void Initialize();
public:
	TMessageSender();
	~TMessageSender();

	// Remove os operadores/construtores de movimento/cópia
	TMessageSender(const TMessageSender&&) = delete;
	TMessageSender(TMessageSender&&) = delete;

	TMessageSender&& operator=(TMessageSender&&) = delete;
	TMessageSender&& operator=(const TMessageSender&&) = delete;

	void MessageSender();
};

#endif