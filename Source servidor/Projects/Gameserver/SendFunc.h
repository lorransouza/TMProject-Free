#include <Windows.h>
#include "cServer.h"

#ifndef __SENDFUNC_H__
#define __SENDFUNC_H__


// Envia uma mensagem ao usuário
bool SendClientMessage(int clientId, const char *msg, ...);

// Envia um sigal
void SendSignalParm(INT32 toClientId, INT32 clientId, INT16 packetId, INT32 value);
void SendSignalParm2(INT32 toClientId, INT32 clientId, INT16 packetId, INT32 value, INT32 value2);
void SendSignalParm3(INT32 toClientId, INT32 clientId, INT16 packetId, INT32 value, INT32 value2, INT32 value3);
void SendSignal(INT32 toClientId, INT32 clientId, INT16 packetId);

// Envia o mob para o usuário
void SendCreateMob(int sendClientID, int createClientID, INT32 send = 0);

// Envia o mob para todos da área
void SendGridMob(int Index);

// Envia os buffs do usuário
void SendAffect(int clientId);

// Atualiza um slot do inventário do usuário
void SendItem(int clientId, SlotType invType, int slotId, st_Item *item);

// Atualiza os itens equipados no usuário
void SendEquip(int clientId);

// Atualiza os status do personagem
void SendScore(int clientIndex);
void SendEtc(int clientId);

// Envia a loja ao usuário
void SendAutoTrade(int sendClientId, int tradeClientId);

// Atualiza o gold do banco do usuário
void SendCargoCoin(int clientId);

// 
void DeleteMob(int clientId, int reason);

//
void SendSay(int clientId, const char *msg, ...);
void SendMobSay(int clientId, int receiverId, int Type, const char* msg, ...);

// 
void SendRemoveMob(UINT16 killerId, UINT16 killedId,  UINT32 mType, INT32 send);
//
void SendEmotion(int clientId, int val1, int val2);

//
void SendNoticeArea(const char *Message, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);

//
void SendSetHpMp(int clientId);

void SendCreateItem(int toClientId, int initId, int unk);
void SendRemoveItem(int dest, int itemid, int bSend);

void SendAddParty(int target, int whom, int leader);
void SendRemoveParty(int target, int whom);
void RemoveSummonerParty(INT32 clientId);

void SendHpMode(int clientId);
void SendCarry(int clientId);
void SendDamage(unsigned int min_x, unsigned int min_y, unsigned int max_x, unsigned int max_y);
void SendEnvEffect(int min_x, int min_y, int max_x, int max_y, int type1, int type2);

void SendNotice(const char*msg, ...);
void SendGuildNotice(INT32 guildId, const char *msg, ...);
void SendServerNotice(const char *msg, ...);
void SendWarInfo(INT32 arg1, INT32 arg2);

void SendGuildList(INT32 clientId);

void SendChatGuild(INT32 clientId, INT32 guildId, const char *msg, ...);

void SendRepurchase(int clientId);
void SendWeather();

void SendCounterMob(int clientId, short value, short total);
void SendCounterMobArea(int value1, unsigned int value2, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);

void SendQuiz(int clientId);
void SendQuiz(int clientId, const char* question, std::array<std::string, 4> answers);

void SendChristmasMission(int clientId, int status);

void SendKingdomBattleInfo(int clientId, int kingdom, bool status);

void SendArenaScoreboard(int clientId, const std::array<int, 4>& points);

void SendAutoPartyInfo(int clientId);
void SendMissionInfo(int clientId);

void MulticastGetCreateMob(int clientId);

void SendChatMessage(int clientId, int color, const char* message, ...);
void SendChatMessage(int color, const char* message, ...);
void SendOverStoreInfo(int clientId);

void BuyItem(int clientId, int type, int page, int slot);
void SendLojaDonate(int clientId, int Type, int Page, int Slot);
void SendLojaDonate(int clientId);
void SendClientPacket(int clientId);

#endif