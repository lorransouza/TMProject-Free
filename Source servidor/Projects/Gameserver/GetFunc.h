#include <Windows.h>
#include "cServer.h"

#ifndef __GETFUNC_H__
#define __GETFUNC_H__

#include <tuple>
#include <array>
#include <functional>

void GetCreateMob(int clientId, BYTE *bufPak);
void GetCreateMobTrade(int clientId, BYTE *bufPak);

int GetAnctCode(st_Item *item, bool usingCostume);
int GetItemIDAndEffect(st_Item *Item, int mnt, bool usingCostume);

// Procura um lugar vazio dentro de uma área para o usuário
bool GetEmptyMobGrid(int Index, unsigned int *posX, unsigned int *posY);
bool GetEmptyItemGrid(int *posX, int *posY);

// Cria o pacote de movimento
void GetAction(int clientId, short posX, short posY, void *buf);

// Pega a refinação atual do item
int GetItemSanc(st_Item *item);

// Pega a classe do usuário baseado na face
int GetInfoClass (int face) ;


bool IsImpossibleToRefine(st_Item* item);

// Pega o valor do efeito baseado no Index 
short GetEffectValueByIndex(int ItemID, int Index);

MapAttribute GetAttribute(int posX, int posY);

// Pega a velocidade atual do mob
int GetSpeed(st_Status *status);

// Pega a distância do usuário
int GetDistance(int x1,int y1,int x2,int y2);

// Pega a rota do usuário
INT32 GetRoute(unsigned int x, unsigned int y, unsigned int *targetX, unsigned int *targetY, char *route, int distance);
//INT32 GetRoute(int arg1,int arg2,int *arg3,int *arg4,char *arg5,int arg6,char *arg7);
// Dano do usuário
int GetDamage(int dam, int ac, int combat) ;
INT32 GetSkillDamage_PvP(INT32 skillId, CMob*mob, INT32 weather, INT32 weaponDamage);
INT32 GetSkillDamage_PvM(INT32 skillId, CMob*mob, INT32 weather, INT32 weaponDamage);
INT32 GetSkillDamage_2(INT32 damage, INT32 defenserAC, INT32 master);

// Evasão do usuário
INT32 GetParryRate(int clientId, int mobId, int type);

// Procura o slot vago nos affects
int GetEmptyAffect(int mobId, int buffId);

// Pk Point do usuário
int GetPKPoint(int Index);

// Pega a cidade e a arena
int GetVillage(unsigned int x, unsigned int y);
int GetArena(unsigned int x, unsigned int y);


int GetTotKill(int Index);
int GetGuilty(int Index);
int GetCurKill(int Index);

void GetHitPosition(int arg1,int arg2,int *arg3,int *arg4);

void GetAttack(int clientId, int mobId, p39D* p);
void GetMultiAttack(int attackerId, int *target, p367 *p);

st_Item *GetItemPointer(int clientIndex, int invType, int invSlot);

void GetCreateItem(int Index, p26E *p);

INT32 GetUserByName(char *name);

INT32 GetEmptyItem();

long long GetExpApply(long long exp, int attackerId, int targetId);
long long GetExpApply_2(long long exp, int receiver, int attackerId, bool useBoxExp);
//long long GetExpApplyMerida(long long exp, int ownerId, int meridaLevel, int targetId);

INT32 GetManaSpent(INT32 skillId, INT32 saveMana, INT32 mastery);

unsigned long long GetFairyExpBonus(unsigned long long exp, INT32 fadaId);

INT32 GetCompounderDeal(INT32 clientId);
INT32 getWeek(INT32 day, INT32 month, INT32 year) ;

std::tuple<int, int> GetPriceAndImpost(sItemData* item, int city);
int GetOverPower(const CMob& mob);
int GetStaticItemAbility(st_Item *item, unsigned char Type);
int GetDamageByJewel(int clientId, int damage);

INT32 GetDoubleCritical(CMob *arg1, short *arg2, short *arg3, unsigned char *arg4);
INT32 GetItemAmount(st_Item *item);

int GetUserInArea(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, char* first);

INT32 GetInventoryAmount(int clientId, int itemId);
void GetGuild(int clientId);
UINT8 DayOfWeek();
unsigned int GetWeekNumber();

INT32 GetInHalf(INT32 clientId, INT32 mobId);
INT32 GetInView(INT32 a, INT32 b);
INT32 GetBonusSet(st_Mob *player, INT32 defense);

int ReturnChance(st_Item *item);
int GetMaxAmountItem(const st_Item* item);

bool IsWarTime();

int GetSpiritRessBonus(int sanc);
int GetSpiritHPMPBonus(int sanc);

bool IsCostume(const st_Item* item);

std::vector<CUser*> GetSameMACUsers(const CUser& user, std::function<bool(CUser& user)> function = nullptr);

TOD_Valley GetValleyWithMinimum();

std::string GetEvolutionName(TOD_Class evolution);

#endif