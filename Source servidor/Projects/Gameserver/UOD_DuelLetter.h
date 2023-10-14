#ifndef UOD_DuelLetterH
#define UOD_DuelLetterH

#include <vector>
#include <map>
#include <tuple>
#include "UOD_Event.h"

class CUser;

enum class TOD_DuelLetter_Room
{
	FirstRoom,
	SecondRoom,
	ThirdRoom,
	FourthRoom,
	BossRoom,
	InvalidRoom
};

enum class TOD_DuelLetter_Type
{
	Normal,
	Mystical,
	Arcane
};

class TOD_DuelLetter : public TOD_EventItem
{
public:
	TOD_DuelLetter(std::chrono::milliseconds interval)
		: TOD_EventItem(interval)
	{
	}
	virtual bool Register(CUser&, st_Item*);
	virtual bool CanRegister(CUser&);
	virtual void Unregister(CUser& user)
	{
	}

protected:
	virtual void Work();

private:
	void Finish(bool outOfTime);
	TOD_DuelLetter_Type GetType(st_Item* item) const;

	void GenerateRoom();
	void GenerateRune();

	bool CanGenerateBoss() const;
	int GetMobCountOnRoom() const;
	int GetInitialIndex() const;
	void CheckUsers();
	void LogGroup(const char* message, ...);
	TOD_DuelLetter_Room GetNextRoom() const;
	TOD_DuelLetter_Room _room{ 0 };

	int _timer{ 0 };
	TOD_DuelLetter_Type _type;
	bool _status{ false };
	bool _haveCourageSymbol{ false };

	std::map<TOD_DuelLetter_Room, int> _runesId;
	std::map<TOD_DuelLetter_Room, std::vector<int>> _initRunes;

	std::vector<CUser*> _users;

	std::tuple<int, int> GetGenerIndexes() const;
};

constexpr int DuelLetter_Normal_Index = 46;
constexpr int DuelLetter_Mystical_Index = 58;
constexpr int DuelLetter_Arcane_Index = 71;
#endif
