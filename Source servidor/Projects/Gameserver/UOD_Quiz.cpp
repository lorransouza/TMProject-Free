#include "Basedef.h"
#include "GetFunc.h"
#include "SendFunc.h"
#include "UOD_Quiz.h"

constexpr int szQuizCorrect[20] = { 0, 3, 2, 0, 0, 2, 0, 2, 1, 2, 2, 3, 2, 0, 3, 1, 0, 0, 0, 2 };

bool TOD_Quiz::Register(CUser& user, st_Item* item)
{
	TOD_Quiz_UserInfo info{ &user, std::chrono::steady_clock::now() };
	info.isWaitingResponse = false;
	info.nextIteraction = std::chrono::milliseconds(1000);
	info.firstInteraction = true;

	if (item->Index == 4111)
		info.quest = TOD_Quiz_Quest::LanHouseN;
	else if (item->Index == 4112)
		info.quest = TOD_Quiz_Quest::LanHouseM;
	else if (item->Index == 4113)
		info.quest = TOD_Quiz_Quest::LanHouseA;

	_users.push_back(info);
	return true;
}

bool TOD_Quiz::CanRegister(CUser&)
{
	return true;
}

bool TOD_Quiz::GetStatus() const noexcept
{
	return _status;
}

std::chrono::milliseconds TOD_Quiz::GetNextQuiz() const
{
	return milliseconds(180000 + (Rand() % 60) * 1000);
}

constexpr std::chrono::milliseconds TOD_Quiz::GetWaitResponseTime() const
{
	return 30s;
}

bool TOD_Quiz::HandlePacket(CUser& user, MSG_QUIZ_ANSWER* packet)
{
	auto userInfo = std::find_if(std::begin(_users), std::end(_users), [&](const TOD_Quiz_UserInfo info) {
		return info.User->clientId == user.clientId;
	});

	if (userInfo == std::end(_users))
	{
		Log(user.clientId, LOG_INGAME, "Respondeu o quiz sem estar na lista de usuários presentes");

		return true;
	}

	if (!userInfo->isWaitingResponse)
	{
		Log(user.clientId, LOG_HACK, "Enviou a resposta sem estar aguardando uma resposta");

		return true;
	}

	auto now = std::chrono::steady_clock::now();
	if (now - userInfo->lastInteraction <= 1.5s)
	{
		Log(userInfo->User->clientId, LOG_HACK, "Respondeu o quiz muito rapidamente. Tempo: %lld", std::chrono::duration_cast<std::chrono::milliseconds>(now - userInfo->lastInteraction).count());
		Log(SERVER_SIDE, LOG_HACK, "%s - Respondeu o quiz muito rapidamente. Tempo: %lld", userInfo->User->User.Username, std::chrono::duration_cast<std::chrono::milliseconds>(now - userInfo->lastInteraction).count());
	}

	if (packet->Asw == userInfo->correctIndex)
	{
		Log(userInfo->User->clientId, LOG_INGAME, "Acertou a resposta da pergunta do Quiz. Recebeu gold.");

		if (!userInfo->firstInteraction)
		{
			if (userInfo->quest == TOD_Quiz_Quest::LanHouseN)
				Mob[userInfo->User->clientId].Mobs.Player.Gold += 2000000;
			else if (userInfo->quest == TOD_Quiz_Quest::LanHouseM)
				Mob[userInfo->User->clientId].Mobs.Player.Gold += 3000000;
			else if (userInfo->quest == TOD_Quiz_Quest::LanHouseA)
				Mob[userInfo->User->clientId].Mobs.Player.Gold += 5000000;
		}
		else
			userInfo->firstInteraction = false;

		SendSignalParm(userInfo->User->clientId, userInfo->User->clientId, 0x3AF, Mob[userInfo->User->clientId].Mobs.Player.Gold);
		SendClientMessage(userInfo->User->clientId, "Resposta correta");
	}
	else
	{
		Log(user.clientId, LOG_INGAME, "O usuário foi removido por errar a resposta do Quiz.");
		DoRecall(userInfo->User->clientId);

		if (userInfo->quest == TOD_Quiz_Quest::LanHouseN)
			userInfo->User->Times.LastLanHouseN = now;
		else if (userInfo->quest == TOD_Quiz_Quest::LanHouseM)
			userInfo->User->Times.LastLanHouseM = now;
		else if (userInfo->quest == TOD_Quiz_Quest::LanHouseA)
			userInfo->User->Times.LastLanHouseA = now;

		_users.erase(userInfo);
		return true;
	}

	userInfo->isWaitingResponse = false;
	userInfo->nextIteraction = GetNextQuiz();
	userInfo->lastInteraction = now;
	return true;
}

void TOD_Quiz::Work()
{
	if (sServer.QuizQuestions.size() == 0)
		return;

	if (!_status)
		return;

	_users.erase(std::remove_if(std::begin(_users), std::end(_users), [&](const TOD_Quiz_UserInfo& info) {
		if (info.User->Status != USER_PLAY)
			return true;

		CMob& mob = Mob[info.User->clientId];
		return !((mob.Target.X >= 3600 && mob.Target.X <= 3700 && mob.Target.Y >= 3600 && mob.Target.Y <= 3700) ||
			(mob.Target.X >= 3732 && mob.Target.X <= 3816 && mob.Target.Y >= 3476 && mob.Target.Y <= 3562) || 
			(mob.Target.X >= 3840 && mob.Target.X <= 3967 && mob.Target.Y >= 3584 && mob.Target.Y <= 3712));
	}), std::end(_users));

	auto now = std::chrono::steady_clock::now();
	// Até este ponto, temos todos os usuários ainda ok
	for (auto& userInfo : _users)
	{
		const auto user = userInfo.User;
		if (now - userInfo.lastInteraction < userInfo.nextIteraction)
			continue;

		// O tempo passou e o meliante não respondeu o quiz
		if (userInfo.isWaitingResponse)
		{
			DoRecall(user->clientId);

			SendQuiz(user->clientId);
			Log(user->clientId, LOG_INGAME, "O usuário foi removido da LanHouse por não responder o quiz.");

			if (userInfo.quest == TOD_Quiz_Quest::LanHouseN)
				userInfo.User->Times.LastLanHouseN = now;
			else if (userInfo.quest == TOD_Quiz_Quest::LanHouseM)
				userInfo.User->Times.LastLanHouseM = now;
			else if (userInfo.quest == TOD_Quiz_Quest::LanHouseA)
				userInfo.User->Times.LastLanHouseA = now;

			continue;
		}

		TOD_QuizQuestions question = *select_randomly(std::begin(sServer.QuizQuestions), std::end(sServer.QuizQuestions));
		if (!(Rand() % 7))
		{
			int number1 = Rand() % 20 + 1;
			int number2 = Rand() % 20 + 1;

			question.Question = std::string("Quanto é ").append(std::to_string(number1)).append(" + ").append(std::to_string(number2)).append("?");
			question.Answers[0] = std::to_string(number1 + number2);

			for (int i = 1; i < 4; i++)
			{
				int count = 0;
				int number = (Rand() % number1) + 1 + number2;
				while (count++ < 20 && number == number1 + number2)
					number = Rand() % number1 + 1 + number2;

				if (count >= 20)
					number = Rand() % 100 + 20;
				question.Answers[i] = std::to_string(number);
			}
		}

		std::array<std::string, 4> answers;

		int correctAnswerIndex = Rand() % 4;
		answers[correctAnswerIndex] = question.Answers[0];

		for (int i = 0; i < 4; i++)
		{
			if (i == correctAnswerIndex)
				continue;

			if (i == 0)
				answers[i] = question.Answers[correctAnswerIndex];
			else
				answers[i] = question.Answers[i];
		}

		SendQuiz(user->clientId, question.Question.c_str(), answers);

		userInfo.isWaitingResponse = true;
		userInfo.nextIteraction = GetWaitResponseTime();
		userInfo.lastInteraction = now;
		userInfo.correctIndex = correctAnswerIndex;

		Log(user->clientId, LOG_INGAME, "Enviado pergunta \"%s\"", question.Question.c_str());
	}
}