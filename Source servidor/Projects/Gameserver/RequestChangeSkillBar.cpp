#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestChangeSkillbar(PacketHeader *Header)
{
	p378 *p = (p378*)Header;

	if(Status != USER_PLAY)
		return false;
	
//#if defined(_DEBUG)	
//	for (int o = 0; o < 4; o++)
//	{
//		auto skillId = p->SkillBar1[o];
//		if (skillId != 255 && skillId >= 105 && skillId < 153)
//			skillId -= 12 * Mob[clientId].Mobs.Player.ClassInfo;
//
//		Mob[clientId].Mobs.Player.SkillBar1[o] = skillId;
//		p->SkillBar1[o] = skillId;
//	}
//
//	for (int o = 0; o < 16; o++)
//	{
//		auto skillId = p->SkillBar2[o];
//		if (skillId != 255 && skillId >= 105 && skillId < 153)
//			skillId -= 12 * Mob[clientId].Mobs.Player.ClassInfo;
//
//		Mob[clientId].Mobs.SkillBar[o] = skillId;
//		p->SkillBar2[o] = skillId;
//	}
//
//	AddMessage((BYTE*)Header, p->Header.Size);
//	return true;
//#endif

	// Anti skillbar hack
	// Checa se existe a skill realmente no personagem para colocá-la em sua barra de skills
	for(INT32 i = 0; i < 4; i++)
	{
		unsigned char skillId = p->SkillBar1[i];
		if (skillId >= 104)
		{
			p->SkillBar1[i] = 255;

			continue;
		}
		 
		if(skillId < 96)
		{
			if(!(Mob[clientId].Mobs.Player.Learn[0] & (1 << (skillId % 24))))
				p->SkillBar1[i] = 255;
		}
		else if(skillId >= 96 && skillId < 105)
		{
			if(!(Mob[clientId].Mobs.Player.Learn[0] & (1 << (24 + skillId - 96))))
				p->SkillBar1[i] = 255;
		}

	}

	for(INT32 i = 0; i < 16; i++)
	{
		unsigned char skillId = p->SkillBar2[i];
		if(skillId < 0 || skillId >= 104)
		{
			p->SkillBar2[i] = 255;

			continue;
		}

		if(skillId < 96)
		{
			if(!(Mob[clientId].Mobs.Player.Learn[0] & (1 << (skillId % 24))))
				p->SkillBar2[i] = 255;
		}
		else if (skillId >= 96 && skillId < 105)
		{
			if(!(Mob[clientId].Mobs.Player.Learn[0] & (1 << (24 + skillId - 96))))
				p->SkillBar2[i] = 255;
		}

	}

	memcpy(Mob[clientId].Mobs.Player.SkillBar1, p->SkillBar1, 4);
	memcpy(Mob[clientId].Mobs.SkillBar, p->SkillBar2, 16);

	AddMessage((BYTE*)Header, p->Header.Size);
	return true;
}