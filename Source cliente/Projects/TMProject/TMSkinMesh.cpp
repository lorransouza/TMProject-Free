#include "pch.h"
#include "CFrame.h"
#include "TMObject.h"
#include "TMEffectSWSwing.h"
#include "TMSkinMesh.h"
#include "MeshManager.h"
#include "TMGlobal.h"
#include "TMEffectBillBoard.h"
#include "TMHuman.h"
#include "CMesh.h"
#include "TMLog.h"

int TMSkinMesh::m_nSmooth = 1;

TMSkinMesh::TMSkinMesh(LOOK_INFO* pLook, SANC_INFO* pSanc, int nBoneAniIndex, int bExpand, SANC_INFO* pColor, short nMeshType, short nCos, int mount)
{
	m_nBoneAniIndex = nBoneAniIndex;
	m_bExpand = bExpand;
	m_bMeshGenerated = 0;
	m_bBaseMat = 0;
	m_cDefaultAlpha = 1;

	m_vPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_vAngle = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_vScale = D3DXVECTOR3(1.0f, 1.0f, 1.0f);

	m_pRoot = nullptr;
	m_nAniBaseIndex = 0;
	m_dwLastUsedTime = 0;
	m_dwShowTime = 0;
	m_nAniIndex = 0;
	m_dwFPS = 30;
	m_dwStartOffset = 0;
	m_pOwner = 0;

	memcpy(&m_Look, pLook, sizeof(m_Look));

	if (pSanc)
		memcpy(&m_Sanc, pSanc, sizeof(m_Sanc));
	else
		memset(&m_Sanc, 0, sizeof(m_Sanc));

	for (int i = 0; i < MAX_VALID_FRAME_TO_ANIMATE; ++i)
		m_pframeToAnimate[i] = 0;

	// This is setted to tree, grass, etc
	if (nBoneAniIndex >= MAX_ANI_TYPE)
		m_dwStartTime = 500 * (((m_nBoneAniIndex >> 1) % 5) + rand() % 5);

	memset(&m_materials, 0, sizeof(m_materials));

	m_materials.Diffuse.r = 1.0f;
	m_materials.Diffuse.g = 1.0f;
	m_materials.Diffuse.b = 1.0f;
	m_materials.Specular = m_materials.Diffuse;
	m_materials.Power = 0.0f;
	m_materials.Emissive.r = 0.3f;
	m_materials.Emissive.g = 0.3f;
	m_materials.Emissive.b = 0.3f;
	m_dwOffset = 0;
	m_nAniIndexLast = 0;
	m_pSwingEffect[0] = 0;
	m_pSwingEffect[1] = 0;
	m_cRotate[0] = 0;
	m_cRotate[1] = 0;

	m_cEnableMultiTex = 1;
	if (m_nBoneAniIndex == 1)
		m_fLenDetail = 0.07f;
	else
		m_fLenDetail = 0.1f;

	if (pColor)
		memcpy(&m_Color, pColor, sizeof(m_Color));
	else
		memset(&m_Color, 0, sizeof(m_Color));

	memset(m_matMantua, 0, sizeof(m_matMantua));

	m_dwTexAni = 0;
	m_cTexAni = 0;
	m_nMeshType = nMeshType;
	m_nCosType = nCos;
	m_bMount = mount;

	D3DXMatrixIdentity(&m_BaseMatrix);
	m_Cos = 1;
	m_bRenderEffect = 0;
}

TMSkinMesh::~TMSkinMesh()
{
	if (m_pRoot != m_pframeToAnimate[0] && m_pRoot != nullptr)
	{
		SAFE_DELETE(m_pRoot);
	}

	for (int i = 0; i < MAX_VALID_FRAME_TO_ANIMATE; ++i)
	{
		SAFE_DELETE(m_pframeToAnimate[i]);
	}

	for (int i = 0; i < 2; ++i)
	{
		if (m_pSwingEffect[i] != nullptr)
		{
			g_pObjectManager->DeleteObject(m_pSwingEffect[i]);
			m_pSwingEffect[i] = nullptr;
		}
	}
}

HRESULT TMSkinMesh::InitDeviceObjects()
{
	RestoreDeviceObjects();
	return 0;
}

HRESULT TMSkinMesh::RestoreDeviceObjects()
{
	m_bMeshGenerated = 0;

	SAFE_DELETE(m_pRoot);

	m_pRoot = new CFrame(0);

	if (m_pRoot == nullptr)
		return 0x80004005;

	m_pframeToAnimate[0] = m_pRoot;
	m_pRoot->m_pParentSkin = this;

	for (size_t i = 0; i < MeshManager::m_BoneAnimationList[m_nBoneAniIndex].numBone; ++i)
	{
		unsigned int parent_ID = MeshManager::m_BoneAnimationList[m_nBoneAniIndex].pBone[2 * i];
		unsigned int parent_temp = parent_ID;
		if (parent_ID == -1)
			parent_ID = 0;

		unsigned int my_ID = MeshManager::m_BoneAnimationList[m_nBoneAniIndex].pBone[2 * i + 1];

		CFrame* tmp = new CFrame(my_ID);
		if (tmp == nullptr)
			return 0x80004005;

		tmp->m_dwParentID = parent_temp;
		tmp->m_pParentSkin = this;
		m_pframeToAnimate[my_ID] = tmp;

		CFrame* parent = m_pRoot->FindFrame(parent_ID);
		if (parent != nullptr)
			parent->AddFrame(tmp);
	}

	char szName[32]{};
	char szTexture[64]{};

	unsigned short* look = (unsigned short*)& m_Look;
	unsigned char* sanc = (unsigned char*)& m_Sanc;

	for (unsigned int i = 0; i < MeshManager::m_BoneAnimationList[m_nBoneAniIndex].numParts; ++i)
	{
		bool god2cos = false;
		if (look[2 * i] == '_' || look[2 * i] == 'a' || look[2 * i] == 'N' || look[2 * i] == 'L')
			god2cos = true;

		sprintf(szName, "%s%02d%02d.msh", MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName,
			i + 1,
			look[2 * i] + 20 * m_bExpand + 1);

		if (m_nBoneAniIndex == 45 || m_nBoneAniIndex == 46 || m_nBoneAniIndex == 53 || m_nBoneAniIndex == 54)
		{
			sprintf(szName, "%s%02d%02d.msh", MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName, i + 1, 1);
		}

		// Meshs that use only one texture must be in God2Exception
		if (God2Exception(i))
		{
			sprintf(szTexture, "%s%02d%02d.wyt", MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName,
				1, (look[2 * i + 1] & 0xFFF) + look[2 * i] + 20 * m_bExpand + 1);
			if (m_nBoneAniIndex == 53)
				sprintf(szTexture, "%s%02d%02d.wyt", MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName, i + 1, 1);
		}
		else
		{
			sprintf(szTexture, "%s%02d%02d.wyt", MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName,
				i + 1,
				(look[2 * i + 1] & 0xFFF) + look[2 * i] + 20 * m_bExpand + 1);

			if (m_nBoneAniIndex == 45 || m_nBoneAniIndex == 46 || m_nBoneAniIndex == 53 || m_nBoneAniIndex == 54)
			{
				sprintf(szTexture, "%s%02d%02d.wyt", MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName, i + 1, 1);
			}

			if (!strcmp(szName, "mesh\\ch010218.msh") && !strcmp(szTexture, "mesh\\ch010219.wyt"))
			{
				sprintf(szTexture, "mesh\\ch010214.wyt");
			}
			else if (MantleException(szTexture))
			{
				sprintf(szName, "mesh\\mt010131.msh");
			}
			else if (!strcmp(szTexture, "mesh\\mt010124.wyt"))
			{
				sprintf(szName, "mesh\\mt010124.msh");
			}
			else if (!strcmp(szTexture, "mesh\\mt010132.wyt") || !strcmp(szTexture, "mesh\\mt010133.wyt") ||
				!strcmp(szTexture, "mesh\\mt010134.wyt") || !strcmp(szTexture, "mesh\\mt010135.wyt") ||
				!strcmp(szTexture, "mesh\\mt010136.wyt") || !strcmp(szTexture, "mesh\\mt010137.wyt"))
			{
				sprintf(szName, "mesh\\mt010131.msh");
			}
			else if (!strcmp(szTexture, "mesh\\mt010124.wyt"))
				sprintf(szName, "mesh\\mt010124.msh");
		}

		if (m_nCosType == 100 && m_nBoneAniIndex == 85)
			SetHardcoreMantle(szTexture, szName);

		if (szTexture[5] == 'c' && szTexture[6] == 'h' && szTexture[8] == '2' && szTexture[11] == '1' && szTexture[12] == '3')
		{
			if (szTexture[10] == '1')
				sprintf(szTexture, "mesh\\ch010130.wyt");
			if (szTexture[10] == '4')
				sprintf(szTexture, "mesh\\ch010430.wyt");
			if (szTexture[10] == '5')
				sprintf(szTexture, "mesh\\ch010530.wyt");
		}

		if (!strcmp(szTexture, "mesh\\ch020315.wyt"))
		{
			sprintf(szTexture, "mesh\\ch020314.wyt");
		}
		else if (!strcmp(szTexture, "mesh\\bm010102.wyt"))
		{
			sprintf(szTexture, "mesh\\mi010105.wyt");
		}
		else if (!strncmp(szTexture, "mesh\\tr13", 9))
		{
			sprintf(szTexture, "mesh\\tr130101.wyt");
		}
		else if (!strncmp(szTexture, "mesh\\tr14", 9))
		{
			sprintf(szTexture, "mesh\\tr130101.wyt");
		}
		else if (!strncmp(szTexture, "mesh\\tr15", 9))
		{
			sprintf(szTexture, "mesh\\tr130101.wyt");
		}
		else if (!strncmp(szTexture, "mesh\\tr16", 9))
		{
			sprintf(szTexture, "mesh\\tr130101.wyt");
		}
		else if (!strncmp(szTexture, "mesh\\tr17", 9))
		{
			sprintf(szTexture, "mesh\\tr130101.wyt");
		}
		else if (!strncmp(szTexture, "mesh\\tr190101", 13))
		{
			sprintf(szTexture, "mesh\\tr180101.wyt");
		}
		else if (!strncmp(szTexture, "mesh\\tr190102", 13))
		{
			sprintf(szTexture, "mesh\\tr180102.wyt");
		}
		else if (!strncmp(szTexture, "mesh\\tr200101", 13))
		{
			sprintf(szTexture, "mesh\\tr180101.wyt");
		}
		else if (!strncmp(szTexture, "mesh\\tr200102", 13))
		{
			sprintf(szTexture, "mesh\\tr180102.wyt");
		}
		else if (!strncmp(szTexture, "mesh\\ch010237", 13))
		{
			sprintf(szTexture, "mesh\\ch010137.wyt");
		}
		else if (!strncmp(szTexture, "mesh\\ch010238", 13))
		{
			sprintf(szTexture, "mesh\\ch010138.wyt");
		}
		else if (!strncmp(szTexture, "mesh\\ch020217", 13))
		{
			sprintf(szTexture, "mesh\\ch020117.wyt");
		}

		if (m_nCosType != 0 && m_nCosType != 100)
			SetCostume(m_nCosType, szTexture, szName);


		if ((int)* look < 90 || !i || look[2 * i])
		{
			CMesh* tmpMesh = new CMesh(this);

			if (tmpMesh == nullptr)
				return 0x80004005;

			unsigned char nSanc = (unsigned char)sanc[i];
			unsigned char nLegnd = (unsigned char)sanc[i + 8];

			if (nSanc > 15)
				nSanc = 15;
			if (nLegnd > 15)
				nLegnd = 15;

			if (!strcmp(szTexture, "mesh\\LB010101.wyt"))
			{
				tmpMesh->m_nTextureIndex = 1840;
			}
			else if (!strcmp(szTexture, "mesh\\LB010201.wyt"))
			{
				tmpMesh->m_nTextureIndex = 1840;
			}
			else if (!strcmp(szTexture, "mesh\\LB010301.wyt"))
			{
				tmpMesh->m_nTextureIndex = 1841;
			}
			else if (!strcmp(szTexture, "mesh\\LK010101.wyt"))
			{
				tmpMesh->m_nTextureIndex = 1842;
			}
			else if (!strcmp(szTexture, "mesh\\LK010201.wyt"))
			{
				tmpMesh->m_nTextureIndex = 1842;
			}
			else if (!strcmp(szTexture, "mesh\\LK010301.wyt"))
			{
				tmpMesh->m_nTextureIndex = 1843;
			}
			else
				tmpMesh->m_nTextureIndex = g_pTextureManager->GetModelTextureIndex(szTexture);

			tmpMesh->m_sMultiType = nSanc;
			tmpMesh->m_sLegendType = nLegnd;

			if (i == 1 && m_Look.Mesh1 == 40)
				tmpMesh->m_bHead = 1;
			if (m_bMount)
				tmpMesh->m_bMount = 1;
			if (god2cos)
				tmpMesh->m_god2cos = 1;

			unsigned char nColor = *((unsigned char*)& m_Color.Sanc0 + i);
			if (m_nCosType)
				tmpMesh->m_bHead = 1;

			switch (nColor)
			{
			case 116:
				tmpMesh->m_sLegendType = 116;
				break;
			case 117:
				tmpMesh->m_sLegendType = 117;
				break;
			case 118:
				tmpMesh->m_sLegendType = 118;
				break;
			case 119:
				tmpMesh->m_sLegendType = 119;
				break;
			case 120:
				tmpMesh->m_sLegendType = 120;
				break;
			case 121:
				tmpMesh->m_sLegendType = 121;
				break;
			case 122:
				tmpMesh->m_sLegendType = 122;
				break;
			case 123:
				tmpMesh->m_sLegendType = 123;
				break;
			case 124:
				tmpMesh->m_sLegendType = 124;
				break;
			case 125:
				tmpMesh->m_sLegendType = 125;
				break;
			}

			if (m_nBoneAniIndex < 19 && (i == 6 || i == 7))
			{
				if (i == 6)
				{
					tmpMesh->m_dwID = g_dwHandIndex[m_nBoneAniIndex][0];
				}
				else if (i == 7)
				{
					tmpMesh->m_dwID = g_dwHandIndex[m_nBoneAniIndex][1];
				}

				CFrame* parent = m_pRoot->FindFrame(tmpMesh->m_dwID);
				if (parent != nullptr)
				{
					parent->m_pMesh = tmpMesh;
					tmpMesh->InitEffect();
				}
				else
				{
					LOG_WRITELOG("Can't Find Parent Node in ID : %d, MshName : %s\r\n", tmpMesh->m_dwID, szName);
				}
			}
			else if (tmpMesh->LoadMesh(szName) == 1)
			{
				CFrame* parent = m_pRoot->FindFrame(tmpMesh->m_dwID);

				if (parent)
					parent->m_pMesh = tmpMesh;
				else
					LOG_WRITELOG("Can't Find Parent Node in ID : %d, MshName : %s\r\n", tmpMesh->m_dwID, szName);

			}
			else if (tmpMesh)
			{
				//std::cout << "Can't Load " << szName << " mesh.\n";
				delete tmpMesh;
			}
		}
	}

	m_pRoot->LinkBones(m_pRoot);
	return 0;
}

// NOTE: This function is the mostly ugly function that this project have.
// if you want to remake, be my guest!
// also, i don't know if this really works! XD
void TMSkinMesh::FrameMove(unsigned int dwServerTime)
{
	dwServerTime = m_dwStartTime + g_pTimerManager->GetServerTime();
	unsigned int dwOffset = 0;
	dwServerTime -= m_dwStartOffset;

	if (m_dwFPS == 0)
		m_dwFPS = 30;

	if (m_nBoneAniIndex >= 0 && m_nBoneAniIndex <= MAX_BONE_ANIMATION_LIST)
	{
		unsigned int dwOffset = dwServerTime / m_dwFPS;
		unsigned int dwMod = MeshManager::m_BoneAnimationList[m_nBoneAniIndex].numAniCut[m_nAniIndex];
		if (m_nBoneAniIndex == 49)
			dwMod -= 2;

		if (dwMod == 0)
			return;

		dwOffset %= 4 * dwMod;
		m_dwOffset = dwOffset / 4;
		unsigned int dwTick = m_dwOffset + m_nAniBaseIndex;
		unsigned int numBone = MeshManager::m_BoneAnimationList[m_nBoneAniIndex].numAniFrame;
		unsigned int addr = numBone * dwTick;
		unsigned int numAniFrame = MeshManager::m_BoneAnimationList[m_nBoneAniIndex].numAniFrame;

		if (numAniFrame < 0 || numAniFrame > 100)
			return;

		if (dwMod == 1 || !TMSkinMesh::m_nSmooth || g_pDevice->m_fFPS < 10.0f)
		{
			for (size_t Frame = 0; Frame < numAniFrame; ++Frame)
			{
				if (m_pframeToAnimate[Frame] != nullptr)
				{
					LPD3DXMATRIX matRot = &MeshManager::m_BoneAnimationList[m_nBoneAniIndex].matAnimation[Frame + addr];
					m_pframeToAnimate[Frame]->m_matRot = *matRot;
				}
			}
			m_bMeshGenerated = 1;
			return;
		}

		float* before;
		float* ori;
		unsigned int EndEdge = 4 * dwMod - 3;
		for (size_t j = 0; j < numAniFrame; ++j)
		{
			if (m_pframeToAnimate[j] == nullptr)
				continue;

			LPD3DXMATRIX matRot = &MeshManager::m_BoneAnimationList[m_nBoneAniIndex].matAnimation[j + addr];
			if (m_nAniIndexLast == 0 || dwOffset >= 10)
			{
				m_nAniIndexLast = 0;
				int mod = dwOffset % 4;

				if (mod == 0)
				{
					m_pframeToAnimate[j]->m_matRot = matRot[0];
				}
				else
				{
					D3DXMATRIX NewMat = matRot[0];

					if (dwOffset >= EndEdge)
					{
						before = (float*)&MeshManager::m_BoneAnimationList[m_nBoneAniIndex].matAnimation[j
							+ numBone
							* m_nAniBaseIndex];
					}
					else
					{
						before = (float*)&MeshManager::m_BoneAnimationList[m_nBoneAniIndex].matAnimation[numBone + j + addr];

					}

					ori = (float*)&NewMat;

					if (mod == 1)
					{
						for (int i = 0; i < 16; ++i)
						{
							*ori = (float)((float)((float)(*ori + *ori) + *ori) + *before) / 4.0f;
							++ori;
							++before;
						}
					}
					else if (mod == 2)
					{
						for (int m = 0; m < 16; ++m)
						{
							*ori = (float)(*ori + *before) / 2.0f;
							++ori;
							++before;
						}
					}
					else if (mod == 3)
					{
						for (int n = 0; n < 16; ++n)
						{
							*ori = (float)((float)((float)(*ori + *before) + *before) + *before) / 4.0f;
							++ori;
							++before;
						}
					}

					m_pframeToAnimate[j]->m_matRot = NewMat;
				}
			}
			else
			{
				before = (float*)&MeshManager::m_BoneAnimationList[m_nBoneAniIndex].matAnimation[j + m_dwTickLast];

				D3DXMATRIX NewMat = matRot[0];

				if (m_nBoneAniIndex == 1 || m_nBoneAniIndex == 0)
				{
					int InvTick = 10 - dwOffset;
					D3DXMATRIX QuatMat;
					D3DXQUATERNION NewQuat;

					D3DXQuaternionSlerp(
						&NewQuat,
						&MeshManager::m_BoneAnimationList[m_nBoneAniIndex].matQuaternion[j + addr],
						&MeshManager::m_BoneAnimationList[m_nBoneAniIndex].matQuaternion[j + m_dwTickLast],
						(float)InvTick / 10.0f);

					D3DXMatrixRotationQuaternion(&QuatMat, &NewQuat);

					ori = (float*)&NewMat;
					float* now = (float*)&QuatMat;

					ori += 12;
					before += 12;
					now += 12;

					for (int l = 0; l < 3; ++l)
					{
						*now = (float)((float)((float)dwOffset * *ori) + (float)((float)InvTick * *before)) / 10.0f;
						++ori;
						++before;
						++now;
					}

					m_pframeToAnimate[j]->m_matRot = QuatMat;
				}
				else
				{
					ori = (float*)&NewMat;
					int offset_ = 10 - dwOffset;
					for (int k = 0; k < 16; ++k)
					{
						*ori = (float)((float)((float)dwOffset * *ori) + (float)((float)offset_ * *before)) / 10.0f;
						++ori;
						++before;
					}
					m_pframeToAnimate[j]->m_matRot = NewMat;
				}
			}
		}
		m_bMeshGenerated = 1;
	}
}

void TMSkinMesh::InitMaterial(D3DMATERIAL9 material)
{
	m_materials = material;
}

int TMSkinMesh::Render(float fLen, float fScale, float fLen2)
{
	if (m_bMeshGenerated == 0)
		return 0;

	if (m_pRoot == nullptr)
		return 0;

	if (m_bBaseMat >= 1 && m_bBaseMat <= 5)
	{
		m_pRoot->m_matRot = m_BaseMatrix;
		D3DXMATRIX matPos;
		D3DXMATRIX matScale;
		D3DXMATRIX matTemp;
		D3DXMatrixIdentity(&matTemp);
		D3DXMatrixScaling(&matScale, fScale, fScale, fScale);
		D3DXMatrixTranslation(&matPos, fLen2, fLen, 0);
		D3DXMatrixMultiply(&matTemp, &m_matMantua, &matPos);
		D3DXMatrixMultiply(&matTemp, &matTemp, &matScale);
		D3DXMatrixMultiply(&m_pRoot->m_matRot, &matTemp, &m_pRoot->m_matRot);
	}
	else
	{
		D3DXMATRIX matTemp;
		D3DXMATRIX matScale;
		D3DXMatrixIdentity(&m_pRoot->m_matRot);
		D3DXMatrixIdentity(&matTemp);
		D3DXMatrixTranslation(&m_pRoot->m_matRot, m_vPosition.x, m_vPosition.y, m_vPosition.z);
		if (m_nBoneAniIndex >= 45 && m_nBoneAniIndex <= 57)
		{
			if (m_nBoneAniIndex == 48)
			{
				D3DXMatrixScaling(&matScale, 0.85f, 0.85f, 0.85f);
				D3DXMatrixMultiply(&matTemp, &matTemp, &matScale);
				D3DXMatrixMultiply(&m_pRoot->m_matRot, &matTemp, &m_pRoot->m_matRot);
			}
			if (m_nBoneAniIndex == 47)
			{
				m_vScale.x = 1.5f;
				m_vScale.y = 1.5f;
				m_vScale.z = 1.5f;
			}


			D3DXMatrixTranslation(&m_pRoot->m_matRot, m_vPosition.x, m_vPosition.y, m_vPosition.z);

			if ((m_nBoneAniIndex == 45 && m_nAniIndex != 7 && m_nAniIndex != 8 && m_nAniIndex != 9) 
				|| m_nBoneAniIndex != 45)
			{				
				D3DXMatrixRotationYawPitchRoll(&matTemp, m_vAngle.y + D3DXToRadian(90), m_vAngle.x, m_vAngle.z);
			}
		}
		else
		{		
			D3DXMatrixRotationYawPitchRoll(
				&matTemp,
				m_vAngle.y - D3DXToRadian(90),
				m_vAngle.x - D3DXToRadian(90),
				m_vAngle.z);
		}

		D3DXMatrixScaling(&matScale, m_vScale.x, m_vScale.y, m_vScale.z);
		if (m_pOwner != nullptr)
		{
			if (m_nMeshType == 1)
			{
				TMHuman* pHuman = m_pOwner;
				if (pHuman != nullptr)
				{
					D3DXMATRIX matFlip;
					D3DXMatrixIdentity(&matFlip);
					matFlip._33 = -1.0f;
					D3DXMatrixMultiply(&matScale, &matScale, &matFlip);
				}
			}
		}
		D3DXMatrixMultiply(&matTemp, &matTemp, &matScale);
		D3DXMatrixMultiply(&m_pRoot->m_matRot, &matTemp, &m_pRoot->m_matRot);
	}

	D3DXMATRIX mCur;
	D3DXMatrixIdentity(&mCur);

	if (m_nBoneAniIndex == 44)
	{
		if (m_dwTexAni == 0)
			m_dwTexAni = g_pTimerManager->GetServerTime();

		unsigned int dwNowTime = g_pTimerManager->GetServerTime();
		float fProgress = (float)((m_dwTexAni - dwNowTime) % 3000);
		fProgress = sinf(D3DXToRadian(180) * (float)(fProgress / 3000.0f));
		fProgress = fProgress / 2.0f;

		D3DMATERIAL9 materials{};
		materials.Diffuse.r = 1.0f;
		materials.Diffuse.g = 1.0f;
		materials.Diffuse.b = 1.0f;
		materials.Specular.r = m_materials.Diffuse.r;
		materials.Specular.g = m_materials.Diffuse.g;
		materials.Specular.b = m_materials.Diffuse.b;
		materials.Specular.a = m_materials.Diffuse.a;
		materials.Power = 0.0f;
		materials.Emissive.r = (float)(fProgress) + 0.40000001f;
		materials.Emissive.g = (float)(fProgress) + 0.40000001f;
		materials.Emissive.b = (float)(fProgress) + 0.40000001f;
		InitMaterial(materials);
	}

	g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_FOGENABLE, g_pDevice->m_bFog);
	m_pRoot->UpdateFrames(&mCur);
	m_pRoot->Render();
	g_pDevice->SetTexture(1, nullptr);
	RenderSkinMeshEffect();
	return 1;
}

HRESULT TMSkinMesh::DeleteDeviceObjects()
{
	SAFE_DELETE(m_pRoot);
	return 0;
}

HRESULT TMSkinMesh::InvalidateDeviceObjects()
{
	if (m_pRoot != nullptr)
		m_pRoot->InvalidateDeviceObjects();

	return 0;
}

int TMSkinMesh::SetAnimation(int nIndex)
{
	if (MeshManager::m_BoneAnimationList[m_nBoneAniIndex].numAniTypeCount <= static_cast<unsigned int>(nIndex))
		return 0;

	if (m_nAniIndex == nIndex)
		return 0;

	m_nAniIndexLast = m_nAniIndex;
	m_dwTickLast = MeshManager::m_BoneAnimationList[m_nBoneAniIndex].numAniFrame
		* (m_dwOffset + m_nAniBaseIndex);

	m_dwStartOffset = g_pTimerManager->GetServerTime();
	m_nAniIndex = nIndex;
	m_nAniBaseIndex = 0;

	for (int i = 0; i < m_nAniIndex; ++i)
		m_nAniBaseIndex += MeshManager::m_BoneAnimationList[m_nBoneAniIndex].numAniCut[i];

	if (m_pOwner != nullptr)
	{
		if (m_nBoneAniIndex < 19)
			SetSwingMatrix();
	}

	return 1;
}

void TMSkinMesh::SetPosition(D3DXVECTOR3 vPos)
{
	m_vPosition = vPos;
}

void TMSkinMesh::SetPosition(float fX, float fY, float fZ)
{
	m_vPosition.x = fX;
	m_vPosition.y = fY;
	m_vPosition.z = fZ;
}

void TMSkinMesh::SetAngle(D3DXVECTOR3 vAngle)
{
	m_vAngle = vAngle;
}

void TMSkinMesh::SetAngle(float fYaw, float fPitch, float fRoll)
{
	m_vAngle.x = fYaw;
	m_vAngle.y = fPitch;
	m_vAngle.z = fRoll;
}

void TMSkinMesh::SetSwingMatrix()
{
	TMEffectSWSwing* pSW[2];

	pSW[0] = m_pSwingEffect[0];
	pSW[1] = m_pSwingEffect[1];

	DWORD dwNumBones = MeshManager::m_BoneAnimationList[m_nBoneAniIndex].numAniFrame;
	DWORD addr = dwNumBones * m_nAniBaseIndex;
	DWORD dwNumTick = MeshManager::m_BoneAnimationList[m_nBoneAniIndex].numAniCut[m_nAniIndex];

	DWORD dwHandIndex[2];

	dwHandIndex[0] = g_dwHandIndex[m_nBoneAniIndex][0];
	dwHandIndex[1] = g_dwHandIndex[m_nBoneAniIndex][1];

	D3DXMATRIX matTrans{};
	D3DXMatrixIdentity(&matTrans);

	LPD3DXMATRIX pmatStart = &MeshManager::m_BoneAnimationList[m_nBoneAniIndex].matAnimation[addr];
	if (pSW[0])
	{
		for (size_t i = 0; i < 48; ++i)
			D3DXMatrixIdentity(&pSW[0]->m_matRot[i]);

		for (size_t i = 0; i < dwNumTick && i <= 47; ++i)
		{
			matTrans = pmatStart[dwHandIndex[0] + dwNumBones * i];

			for (size_t j = 0; j < pSW[0]->m_dwNumIndex - 1; ++j)
			{
				matTrans = matTrans * pmatStart[pSW[0]->m_dwIndices[j] + dwNumBones * i];
			}

			pSW[0]->m_matRot[i] = matTrans;
		}
		pSW[0]->m_nNumTicks = dwNumTick;
	}

	if (pSW[1])
	{
		for (size_t i = 0; i < 48; ++i)
			D3DXMatrixIdentity(&pSW[1]->m_matRot[i]);
		for (size_t i = 0; i < dwNumTick && i <= 47; ++i)
		{
			matTrans = pmatStart[dwHandIndex[1] + dwNumBones * i];

			for (size_t j = 0; j < pSW[1]->m_dwNumIndex - 1; ++j)
			{
				matTrans = matTrans * pmatStart[pSW[1]->m_dwIndices[j] + dwNumBones * i];
			}

			pSW[1]->m_matRot[i] = matTrans;
		}
		pSW[1]->m_nNumTicks = dwNumTick;
	}
}

void TMSkinMesh::SetVecMantua(int nType, int nSkinIndex)
{
	m_bBaseMat = nType;
	float fMantuaUp = 0.0f;
	switch (nSkinIndex)
	{
	case 25:
		fMantuaUp = 0.1f;
		break;
	case 28:
		fMantuaUp = 0.15f;
		break;
	case 20:
		fMantuaUp = 0.5f;
		break;
	case 39:
		fMantuaUp = 0.25f;
		break;
	case 29:
		fMantuaUp = 0.18f;
		break;
	case 31:
		fMantuaUp = 0.15f;
		break;
	case 30:
		fMantuaUp = 0.25f;
		break;
	case 38:
		fMantuaUp = 0.26f;
		break;
	case 40:
		fMantuaUp = 0.18f;
		break;
	}

	switch (nType)
	{
	case 1:
		D3DXMatrixRotationYawPitchRoll(&m_matMantua, -D3DXToRadian(90), fMantuaUp + -D3DXToRadian(180), 0);
		break;
	case 2:
		D3DXMatrixRotationYawPitchRoll(&m_matMantua, -D3DXToRadian(90), fMantuaUp + D3DXToRadian(90), 0);
		break;
	case 3:
		D3DXMatrixRotationYawPitchRoll(
			&m_matMantua,
			D3DXToRadian(90),
			(fMantuaUp + -D3DXToRadian(90)) - (fMantuaUp + D3DXToRadian(30)),
			0);
		break;
	case 4:
		D3DXMatrixRotationYawPitchRoll(&m_matMantua, D3DXToRadian(90), -1.3707963f, 0);
		break;
	case 5:
		D3DXMatrixRotationYawPitchRoll(&m_matMantua, -D3DXToRadian(90), 1.9707963f, 0);
		break;
	}
}

void TMSkinMesh::RenderSkinMeshEffect()
{
	if (m_nBoneAniIndex == 31)
	{
		if (m_bRenderEffect != 0)
			RenderEffect_HorseFireLeg();
	}
}

void TMSkinMesh::RenderEffect_HorseFireLeg()
{
	TMEffectBillBoard* mpBill[4];

	for (int i = 0; i < 4; i++)
	{
		int j = rand() % 5;

		mpBill[i] = new TMEffectBillBoard(0, j + 500, 0.0f, 0.0f, 0.0f, 0.001f, 0, 80);

		if (mpBill[i])
		{
			mpBill[i]->m_vecPosition = TMVector3(((rand() % 5 - 2) * 0.05f) + m_pOwner->m_vecTempPos[i].x,
				m_pOwner->m_vecTempPos[i].y, ((rand() % 5 - 2) * 0.05f) + m_pOwner->m_vecTempPos[i].z);

			mpBill[i]->m_vecStartPos = mpBill[i]->m_vecPosition;
			mpBill[i]->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
			mpBill[i]->m_bStickGround = 1;
			mpBill[i]->m_nParticleType = 1;
			mpBill[i]->m_fParticleV = 0.05f;

			mpBill[i]->SetColor(j > 3 ? 0xFFFF1111 : 0xFFFFFFFF);

			g_pCurrentScene->m_pEffectContainer->AddChild((TreeNode*)mpBill[i]);
		}
	}
}

void TMSkinMesh::SetHardcoreMantle(char* szTexture, char* szName)
{
	sprintf(szName, "mesh\\newmt.msh");
	if (m_Look.Skin0 == 0)
	{
		sprintf(szTexture, "mesh\\newmtB000.wys");
	}
	else if (m_Look.Skin0 == 1)
	{
		sprintf(szTexture, "mesh\\newmtR000.wys");
	}
	else
	{
		sprintf(szTexture, "mesh\\newmtW000.wys");
	}
}

void TMSkinMesh::SetOldCostume(int costype, char* szTexture, char* szName)
{
	switch (costype + 1)
	{
	case 0:
		SetRenewOldCostume(costype, szTexture, szName);
		break;
	case 1:
		return;
	case 2:
		switch (m_Cos)
		{
		case 1:
			strcpy(szTexture, "mesh\\ch020161.wyt");
			strcpy(szName, "mesh\\ch020161.msh");
			m_Cos = 2;
			break;
		case 2:
			strcpy(szTexture, "mesh\\ch020261.wyt");
			strcpy(szName, "mesh\\ch020261.msh");
			m_Cos = 3;
			break;
		case 3:
			strcpy(szTexture, "mesh\\ch020357.wyt");
			strcpy(szName, "mesh\\ch020357.msh");
			m_Cos = 4;
			break;
		case 4:
			strcpy(szTexture, "mesh\\ch020457.wyt");
			strcpy(szName, "mesh\\ch020457.msh");
			m_Cos = 5;
			break;
		case 5:
			strcpy(szTexture, "mesh\\ch020557.wyt");
			strcpy(szName, "mesh\\ch020557.msh");
			m_Cos = 6;
			break;
		case 6:
			strcpy(szTexture, "mesh\\ch020657.wyt");
			strcpy(szName, "mesh\\ch020657.msh");
			m_Cos = 1;
			break;
		}
		break;
	case 3:
		if (!strncmp(szTexture, "mesh\\ch0101", 11) || !strncmp(szTexture, "mesh\\ch0201", 11))
		{
			szTexture[strlen(szTexture) - 9] = '1';
			szTexture[strlen(szTexture) - 6] = '3';
			szTexture[strlen(szTexture) - 5] = '0';
		}
		else if (!strncmp(szTexture, "mesh\\ch01", 9) || !strncmp(szTexture, "mesh\\ch02", 9))
		{
			szTexture[strlen(szTexture) - 9] = '1';
			szTexture[strlen(szTexture) - 6] = '3';
			szTexture[strlen(szTexture) - 5] = '1';
		}
		if (!strncmp(szName, "mesh\\ch0101", 11) || !strncmp(szName, "mesh\\ch0201", 11))
		{
			szName[strlen(szName) - 9] = '1';
			szName[strlen(szName) - 6] = '3';
			szName[strlen(szName) - 5] = '0';
		}
		else if (!strncmp(szName, "mesh\\ch01", 9) || !strncmp(szName, "mesh\\ch02", 9))
		{
			szName[strlen(szName) - 9] = '1';
			szName[strlen(szName) - 6] = '3';
			szName[strlen(szName) - 5] = '1';
		}
		break;
	case 4:
		strcpy(szTexture, "mesh\\SpiderCos.wyt");
		switch (m_Cos)
		{
		case 1:
			strcpy(szName, "mesh\\ch020190.msh");
			m_Cos = 2;
			break;
		case 2:
			strcpy(szName, "mesh\\ch020290.msh");
			m_Cos = 3;
			break;
		case 3:
			strcpy(szName, "mesh\\ch020390.msh");
			m_Cos = 4;
			break;
		case 4:
			strcpy(szName, "mesh\\ch020490.msh");
			m_Cos = 5;
			break;
		case 5:
			strcpy(szName, "mesh\\ch020590.msh");
			m_Cos = 6;
			break;
		case 6:
			strcpy(szName, "mesh\\ch020690.msh");
			m_Cos = 1;
			break;
		}
		break;
	case 5:
		if (!strncmp(szTexture, "mesh\\ch0101", 11) || !strncmp(szTexture, "mesh\\ch0201", 11))
		{
			szTexture[strlen(szTexture) - 9] = '1';
			szTexture[strlen(szTexture) - 6] = '3';
			szTexture[strlen(szTexture) - 5] = '7';
		}		
		else if (!strncmp(szTexture, "mesh\\ch0102", 11) || !strncmp(szTexture, "mesh\\ch0201", 11))
		{
			szTexture[strlen(szTexture) - 9] = '1';
			szTexture[strlen(szTexture) - 7] = '1';
			szTexture[strlen(szTexture) - 6] = '3';
			szTexture[strlen(szTexture) - 5] = '7';
		}
		else if (!strncmp(szTexture, "mesh\\ch01", 9) || !strncmp(szTexture, "mesh\\ch02", 9))
		{
			szTexture[strlen(szTexture) - 9] = '1';
			szTexture[strlen(szTexture) - 6] = '3';
			szTexture[strlen(szTexture) - 5] = '7';
		}

		if (!strncmp(szName, "mesh\\ch0101", 11) || !strncmp(szName, "mesh\\ch0201", 11))
		{
			szName[strlen(szName) - 9] = '1';
			szName[strlen(szName) - 6] = '3';
			szName[strlen(szName) - 5] = '7';
		}
		else if (!strncmp(szName, "mesh\\ch01", 9) || !strncmp(szName, "mesh\\ch02", 9u))
		{
			szName[strlen(szName) - 9] = '1';
			szName[strlen(szName) - 6] = '3';
			szName[strlen(szName) - 5] = '7';
		}
		break;
	case 6:
		switch (m_Cos)
		{
		case 1:
			strcpy(szTexture, "mesh\\ch020117.wyt");
			strcpy(szName, "mesh\\ch020117.msh");
			m_Cos = 2;
			break;
		case 2:
			strcpy(szTexture, "mesh\\ch020117.wyt");
			strcpy(szName, "mesh\\ch020217.msh");
			m_Cos = 3;
			break;
		case 3:
			strcpy(szTexture, "mesh\\ch020317.wyt");
			strcpy(szName, "mesh\\ch020317.msh");
			m_Cos = 4;
			break;
		case 4:
			strcpy(szTexture, "mesh\\ch020417.wyt");
			strcpy(szName, "mesh\\ch020417.msh");
			m_Cos = 5;
			break;
		case 5:
			strcpy(szTexture, "mesh\\ch020517.wyt");
			strcpy(szName, "mesh\\ch020517.msh");
			m_Cos = 6;
			break;
		case 6:
			strcpy(szTexture, "mesh\\ch020617.wyt");
			strcpy(szName, "mesh\\ch020617.msh");
			m_Cos = 1;
			break;
		}
		break;
	case 7:
		strcpy(szTexture, "mesh\\ch010195.wyt");
		switch (m_Cos)
		{
		case 1:
			strcpy(szName, "mesh\\ch010195.msh");
			m_Cos = 2;
			break;
		case 2:
			strcpy(szName, "mesh\\ch010295.msh");
			m_Cos = 3;
			break;
		case 3:
			strcpy(szName, "mesh\\ch010395.msh");
			m_Cos = 4;
			break;
		case 4:
			strcpy(szName, "mesh\\ch010495.msh");
			m_Cos = 5;
			break;
		case 5:
			strcpy(szName, "mesh\\ch010595.msh");
			m_Cos = 6;
			break;
		case 6:
			strcpy(szName, "mesh\\ch010695.msh");
			m_Cos = 1;
			break;
		}
		break;
	case 8:
		switch (m_Cos)
		{
		case 1:
			strcpy(szTexture, "mesh\\ch020197.wyt");
			strcpy(szName, "mesh\\ch020197.msh");
			m_Cos = 2;
			break;
		case 2:
			strcpy(szTexture, "mesh\\ch020297.wyt");
			strcpy(szName, "mesh\\ch020297.msh");
			m_Cos = 3;
			break;
		case 3:
			strcpy(szTexture, "mesh\\ch020397.wyt");
			strcpy(szName, "mesh\\ch020397.msh");
			m_Cos = 4;
			break;
		case 4:
			strcpy(szTexture, "mesh\\ch020497.wyt");
			strcpy(szName, "mesh\\ch020497.msh");
			m_Cos = 5;
			break;
		case 5:
			strcpy(szTexture, "mesh\\ch020597.wyt");
			strcpy(szName, "mesh\\ch020597.msh");
			m_Cos = 6;
			break;
		case 6:
			strcpy(szTexture, "mesh\\ch020697.wyt");
			strcpy(szName, "mesh\\ch020697.msh");
			m_Cos = 1;
			break;
		}
		break;
	}
}

void TMSkinMesh::SetRenewOldCostume(int costype, char* szTexture, char* szName)
{
	strcpy(szTexture, "mesh\\ch0101115.wys");
	switch (m_Cos)
	{
	case 1:
		strcpy(szName, "mesh\\ch0101115.msh");
		m_Cos = 2;
		break;
	case 2:
		strcpy(szName, "mesh\\ch0102115.msh");
		m_Cos = 3;
		break;
	case 3:
		strcpy(szName, "mesh\\ch0103115.msh");
		m_Cos = 4;
		break;
	case 4:
		strcpy(szName, "mesh\\ch0104115.msh");
		m_Cos = 5;
		break;
	case 5:
		strcpy(szName, "mesh\\ch0105115.msh");
		m_Cos = 6;
		break;
	case 6:
		strcpy(szName, "mesh\\ch0106115.msh");
		m_Cos = 1;
		break;
	}
}

void TMSkinMesh::SetCostume(int Costype, char* szTexture, char* szName)//controle de trajes
{
	if (Costype == 8 || Costype == 9)
	{
		if (Costype == 9)
			strcpy(szTexture, "mesh\\WhiteSanta.wyt");
		else
			strcpy(szTexture, "mesh\\RedSanta.wyt");
		switch (m_Cos)
		{
		case 1:
			strcpy(szName, "mesh\\ch020196.msh");
			m_Cos = 2;
			break;
		case 2:
			strcpy(szName, "mesh\\ch020296.msh");
			m_Cos = 3;
			break;
		case 3:
			strcpy(szName, "mesh\\ch020396.msh");
			m_Cos = 4;
			break;
		case 4:
			strcpy(szName, "mesh\\ch020496.msh");
			m_Cos = 5;
			break;
		case 5:
			strcpy(szName, "mesh\\ch020596.msh");
			m_Cos = 6;
			break;
		case 6:
			strcpy(szName, "mesh\\ch020696.msh");
			m_Cos = 1;
			break;
		}
	}
	else if (Costype == 10 || Costype == 11)
	{
		if (Costype == 11)
			strcpy(szTexture, "mesh\\BlueRudol.wyt");
		else
			strcpy(szTexture, "mesh\\PurpleRudol.wyt");
		switch (m_Cos)
		{
		case 1:
			strcpy(szName, "mesh\\ch020194.msh");
			m_Cos = 2;
			break;
		case 2:
			strcpy(szName, "mesh\\ch020294.msh");
			m_Cos = 3;
			break;
		case 3:
			strcpy(szName, "mesh\\ch020394.msh");
			m_Cos = 4;
			break;
		case 4:
			strcpy(szName, "mesh\\ch020494.msh");
			m_Cos = 5;
			break;
		case 5:
			strcpy(szName, "mesh\\ch020594.msh");
			m_Cos = 6;
			break;
		case 6:
			strcpy(szName, "mesh\\ch020694.msh");
			m_Cos = 1;
			break;
		}
	}
	else if (Costype == 12 || Costype == 13 || Costype == 34)
	{
		if (Costype == 13)
			strcpy(szTexture, "mesh\\WhitePolice.wys");
		else if (Costype == 34)
			strcpy(szTexture, "mesh\\GreenPolice.wys");
		else
			strcpy(szTexture, "mesh\\BlackPolice.wys");
		switch (m_Cos)
		{
		case 1:
			strcpy(szName, "mesh\\ch020195.msh");
			m_Cos = 2;
			break;
		case 2:
			strcpy(szName, "mesh\\ch020295.msh");
			m_Cos = 3;
			break;
		case 3:
			strcpy(szName, "mesh\\ch020395.msh");
			m_Cos = 4;
			break;
		case 4:
			strcpy(szName, "mesh\\ch020495.msh");
			m_Cos = 5;
			break;
		case 5:
			strcpy(szName, "mesh\\ch020595.msh");
			m_Cos = 6;
			break;
		case 6:
			strcpy(szName, "mesh\\ch020695.msh");
			m_Cos = 1;
			break;
		}
	}
	
	else if (Costype == 19 || Costype == 20)
	{
		if (Costype == 19)
			strcpy(szTexture, "mesh\\ch0101103.wys");
		else
			strcpy(szTexture, "mesh\\ch0102103.wys");
		switch (m_Cos)
		{
		case 1:
			strcpy(szName, "mesh\\ch0101103.msh");
			m_Cos = 2;
			break;
		case 2:
			strcpy(szName, "mesh\\ch0102103.msh");
			m_Cos = 3;
			break;
		case 3:
			strcpy(szName, "mesh\\ch0103103.msh");
			m_Cos = 4;
			break;
		case 4:
			strcpy(szName, "mesh\\ch0104103.msh");
			m_Cos = 5;
			break;
		case 5:
			strcpy(szName, "mesh\\ch0105103.msh");
			m_Cos = 6;
			break;
		case 6:
			strcpy(szName, "mesh\\ch0106103.msh");
			m_Cos = 1;
			break;
		}
	}
	else if (Costype == 21 || Costype == 22)
	{
		strcpy(szTexture, "mesh\\ch0101102.wys");
		switch (m_Cos)
		{
		case 1:
			strcpy(szName, "mesh\\ch0101102.msh");
			m_Cos = 2;
			break;
		case 2:
			strcpy(szName, "mesh\\ch0102102.msh");
			m_Cos = 3;
			break;
		case 3:
			strcpy(szName, "mesh\\ch0103102.msh");
			m_Cos = 4;
			break;
		case 4:
			strcpy(szName, "mesh\\ch0104102.msh");
			m_Cos = 5;
			break;
		case 5:
			strcpy(szName, "mesh\\ch0105102.msh");
			m_Cos = 6;
			break;
		case 6:
			strcpy(szName, "mesh\\ch0106102.msh");
			m_Cos = 1;
			break;
		}
	}
	else
	{
		switch (Costype)
		{
		case 14:
			strcpy(szTexture, "mesh\\DeathCos2.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch010189.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch010289.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch010389.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch010489.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch010589.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch010689.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 15:
			strcpy(szTexture, "mesh\\DeathCos.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch020189.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch020289.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch020389.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch020489.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch020589.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch020689.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 16:
			strcpy(szTexture, "mesh\\ch010199.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch010199.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch010299.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch010399.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch010499.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch010599.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch010699.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 17:
			strcpy(szTexture, "mesh\\ch0101100.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101100.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102100.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103100.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104100.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105100.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106100.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 18:
			strcpy(szTexture, "mesh\\ch0101101.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101101.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102101.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103101.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104101.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105101.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106101.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 23:
			strcpy(szTexture, "mesh\\ch0101104.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101104.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102104.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103104.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104104.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105104.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106104.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 24:
			strcpy(szTexture, "mesh\\ch0101106.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101106.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102106.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103106.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104106.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105106.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106106.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 25:
			strcpy(szTexture, "mesh\\ch0101105.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101105.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102105.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103105.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104105.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105105.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106105.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 26:
			strcpy(szTexture, "mesh\\ch0101107.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101107.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102107.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103107.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104107.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105107.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106107.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 27:
			strcpy(szTexture, "mesh\\ch0101108.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101108.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102108.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103108.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104108.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105108.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106108.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 28:
			strcpy(szTexture, "mesh\\ch0101109.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101109.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102109.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103109.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104109.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105109.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106109.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 29:
			strcpy(szTexture, "mesh\\ch0101110.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101110.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102110.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103110.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104110.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105110.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106110.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 30:
			strcpy(szTexture, "mesh\\ch0101111.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101111.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102111.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103111.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104111.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105111.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106111.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 31:
			strcpy(szTexture, "mesh\\ch0101112.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101112.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102112.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103112.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104112.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105112.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106112.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 32:
			strcpy(szTexture, "mesh\\ch0101113.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101113.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102113.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103113.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104113.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105113.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106113.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 33:
			strcpy(szTexture, "mesh\\ch0101114.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101114.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102114.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103114.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104114.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105114.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106114.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 35:
			strcpy(szTexture, "mesh\\ch0101118.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101118.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102118.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103118.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104118.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105118.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106118.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 36:
			strcpy(szTexture, "mesh\\ch0101119.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101119.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102119.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103119.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104119.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105119.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106119.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 37:
			strcpy(szTexture, "mesh\\ch0101120.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101120.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102120.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103120.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104120.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105120.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106120.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 38:
			strcpy(szTexture, "mesh\\ch0101121.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101121.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102121.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103121.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104121.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105121.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106121.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 39:
			strcpy(szTexture, "mesh\\ch0101122.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101122.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102122.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103122.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104122.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105122.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106122.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 40:
			strcpy(szTexture, "mesh\\ch0101123.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101123.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102123.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103123.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104123.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105123.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106123.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 41:
			strcpy(szTexture, "mesh\\ch0101124.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101124.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102124.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103124.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104124.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105124.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106124.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 42:
			strcpy(szTexture, "mesh\\ch0101125.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101125.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102125.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103125.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104125.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105125.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106125.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 43:
			strcpy(szTexture, "mesh\\ch0101126.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101126.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102126.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103126.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104126.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105126.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106126.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 44:
			strcpy(szTexture, "mesh\\ch0101127.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101127.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102127.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103127.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104127.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105127.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106127.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 45:
			strcpy(szTexture, "mesh\\ch0101128.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101128.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102128.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103128.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104128.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105128.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106128.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 46:
			strcpy(szTexture, "mesh\\ch0101129.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101129.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102129.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103129.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104129.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105129.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106129.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 47:
			strcpy(szTexture, "mesh\\ch0101130.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101130.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102130.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103130.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104130.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105130.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106130.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 48:
			strcpy(szTexture, "mesh\\ch0101131.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101131.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102131.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103131.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104131.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105131.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106131.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 49:
			strcpy(szTexture, "mesh\\ch0101132.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101132.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102132.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103132.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104132.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105132.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106132.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 50:
			strcpy(szTexture, "mesh\\ch0101133.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101133.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102133.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103133.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104133.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105133.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106133.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 51:
			strcpy(szTexture, "mesh\\ch0101138.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101138.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102138.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103138.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104138.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105138.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106138.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 52:
			strcpy(szTexture, "mesh\\ch0101139.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101139.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102139.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103139.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104139.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105139.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106139.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 53:
			strcpy(szTexture, "mesh\\ch0101140.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101140.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102140.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103140.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104140.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105140.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106140.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 54:
			strcpy(szTexture, "mesh\\ch0101141.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101141.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102141.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103141.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104141.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105141.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106141.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 55:
			strcpy(szTexture, "mesh\\ch0101151.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101151.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102151.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103151.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104151.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105151.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106151.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 56:
			strcpy(szTexture, "mesh\\ch0101152.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101152.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102152.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103152.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104152.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105152.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106152.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 57:
			strcpy(szTexture, "mesh\\ch0101154.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101154.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102154.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103154.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104154.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105154.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106154.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 58:
			strcpy(szTexture, "mesh\\ch0101155.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101155.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102155.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103155.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104155.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105155.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106155.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 59:
			strcpy(szTexture, "mesh\\ch0101156.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101156.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102156.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103156.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104156.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105156.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106156.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 60:
			strcpy(szTexture, "mesh\\ch0101157.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101157.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102157.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103157.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104157.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105157.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106157.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 61:
			strcpy(szTexture, "mesh\\ch0101158.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101158.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102158.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103158.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104158.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105158.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106158.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 62:
			strcpy(szTexture, "mesh\\ch0101159.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101159.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102159.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103159.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104159.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105159.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106159.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 63:
			strcpy(szTexture, "mesh\\ch0101160.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101160.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102160.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103160.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104160.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105160.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106160.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 64:
			strcpy(szTexture, "mesh\\ch0101161.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101161.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102161.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103161.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104161.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105161.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106161.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 65:
			strcpy(szTexture, "mesh\\ch0101162.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101162.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102162.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103162.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104162.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105162.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106162.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 66:
			strcpy(szTexture, "mesh\\ch0101163.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101163.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102163.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103163.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104163.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105163.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106163.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 67:
			strcpy(szTexture, "mesh\\ch0101164.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101164.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102164.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103164.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104164.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105164.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106164.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 68:
			strcpy(szTexture, "mesh\\ch0101166.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101166.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102166.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103166.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104166.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105166.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106166.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 69:
			strcpy(szTexture, "mesh\\ch0101167.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101167.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102167.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103167.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104167.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105167.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106167.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 70:
			strcpy(szTexture, "mesh\\ch0101168.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101168.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102168.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103168.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104168.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105168.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106168.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 71:
			strcpy(szTexture, "mesh\\ch0101169.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101169.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102169.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103169.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104169.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105169.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106169.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 72:
			strcpy(szTexture, "mesh\\ch0101170.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101170.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102170.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103170.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104170.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105170.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106170.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 73:
			strcpy(szTexture, "mesh\\ch0101171.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101171.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102171.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103171.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104171.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105171.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106171.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 74:
			strcpy(szTexture, "mesh\\ch0101172.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101172.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102172.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103172.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104172.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105172.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106172.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 75:
			strcpy(szTexture, "mesh\\ch0101173.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101173.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102173.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103173.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104173.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105173.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106173.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 76:
			strcpy(szTexture, "mesh\\ch0101174.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101174.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102174.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103174.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104174.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105174.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106174.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 77:
			strcpy(szTexture, "mesh\\ch0101175.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101175.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102175.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103175.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104175.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105175.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106175.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 78:
			strcpy(szTexture, "mesh\\ch0101176.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101176.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102176.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103176.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104176.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105176.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106176.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 79:
			strcpy(szTexture, "mesh\\ch0101177.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101177.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102177.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103177.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104177.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105177.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106177.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 80:
			strcpy(szTexture, "mesh\\ch0101178.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101178.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102178.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103178.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104178.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105178.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106178.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 81:
			strcpy(szTexture, "mesh\\ch0101179.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101179.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102179.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103179.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104179.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105179.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106179.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 82:
			strcpy(szTexture, "mesh\\ch0101180.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101180.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102180.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103180.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104180.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105180.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106180.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 83:
			strcpy(szTexture, "mesh\\ch0101181.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101181.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102181.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103181.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104181.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105181.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106181.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 84:
			strcpy(szTexture, "mesh\\ch0101182.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101182.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102182.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103182.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104182.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105182.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106182.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 85:
			strcpy(szTexture, "mesh\\ch0101183.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101183.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102183.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103183.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104183.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105183.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106183.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 86:
			strcpy(szTexture, "mesh\\ch0101184.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101184.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102184.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103184.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104184.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105184.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106184.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 87:
			strcpy(szTexture, "mesh\\ch0101185.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101185.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102185.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103185.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104185.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105185.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106185.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 88:
			strcpy(szTexture, "mesh\\ch0101186.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101186.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102186.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103186.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104186.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105186.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106186.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 89:
			strcpy(szTexture, "mesh\\ch0101187.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101187.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102187.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103187.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104187.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105187.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106187.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 90:
			strcpy(szTexture, "mesh\\ch0101188.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101188.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102188.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103188.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104188.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105188.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106188.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 91:
			strcpy(szTexture, "mesh\\ch0101190.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101190.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102190.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103190.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104190.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105190.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106190.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 92:
			strcpy(szTexture, "mesh\\ch0101191.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101191.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102191.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103191.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104191.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105191.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106191.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 93:
			strcpy(szTexture, "mesh\\ch0101193.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101193.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102193.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103193.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104193.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105193.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106193.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 94:
			strcpy(szTexture, "mesh\\ch0101194.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101194.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102194.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103194.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104194.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105194.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106194.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 95:
			strcpy(szTexture, "mesh\\ch0101195.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101195.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102195.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103195.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104195.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105195.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106195.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 96:
			strcpy(szTexture, "mesh\\ch0101196.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101196.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102196.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103196.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104196.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105196.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106196.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 97:
			strcpy(szTexture, "mesh\\ch0101197.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101197.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102197.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103197.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104197.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105197.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106197.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 98:
			strcpy(szTexture, "mesh\\ch0101198.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101198.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102198.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103198.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104198.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105198.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106198.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 99:
			strcpy(szTexture, "mesh\\ch0101199.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101199.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102199.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103199.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104199.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105199.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106199.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 100:
			strcpy(szTexture, "mesh\\ch0101200.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101200.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102200.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103200.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104200.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105200.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106200.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 101:
			strcpy(szTexture, "mesh\\ch0101201.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101201.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102201.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103201.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104201.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105201.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106201.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 102:
			strcpy(szTexture, "mesh\\ch0101202.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101202.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102202.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103202.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104202.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105202.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106202.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 103:
			strcpy(szTexture, "mesh\\ch0101203.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101203.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102203.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103203.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104203.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105203.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106203.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 104:
			strcpy(szTexture, "mesh\\ch0101204.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101204.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102204.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103204.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104204.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105204.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106204.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 105:
			strcpy(szTexture, "mesh\\ch0101205.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101205.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102205.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103205.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104205.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105205.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106205.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 106:
			strcpy(szTexture, "mesh\\ch0101206.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101206.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102206.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103206.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104206.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105206.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106206.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 107:
			strcpy(szTexture, "mesh\\ch0101207.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101207.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102207.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103207.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104207.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105207.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106207.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 108:
			strcpy(szTexture, "mesh\\ch0101208.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101208.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102208.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103208.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104208.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105208.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106208.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 109:
			strcpy(szTexture, "mesh\\ch0101209.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101209.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102209.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103209.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104209.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105209.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106209.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 110:
			strcpy(szTexture, "mesh\\ch0101210.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101210.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102210.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103210.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104210.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105210.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106210.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 111:
			strcpy(szTexture, "mesh\\ch0101211.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101211.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102211.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103211.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104211.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105211.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106211.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 112:
			strcpy(szTexture, "mesh\\ch0101212.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101212.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102212.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103212.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104212.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105212.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106212.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 113:
			strcpy(szTexture, "mesh\\ch0101213.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101213.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102213.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103213.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104213.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105213.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106213.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 114:
			strcpy(szTexture, "mesh\\ch0101214.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101214.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102214.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103214.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104214.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105214.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106214.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 115:
			strcpy(szTexture, "mesh\\ch0101215.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101215.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102215.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103215.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104215.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105215.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106215.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 116:
			strcpy(szTexture, "mesh\\ch0101216.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101216.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102216.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103216.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104216.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105216.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106216.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 117:
			strcpy(szTexture, "mesh\\ch0101221.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101221.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102221.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103221.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104221.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105221.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106221.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 118:
			strcpy(szTexture, "mesh\\ch0101222.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101222.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102222.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103222.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104222.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105222.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106222.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 119:
			strcpy(szTexture, "mesh\\ch0101223.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101223.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102223.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103223.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104223.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105223.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106223.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 120:
			strcpy(szTexture, "mesh\\ch0101224.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101224.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102224.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103224.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104224.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105224.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106224.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 121:
			strcpy(szTexture, "mesh\\ch0101225.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101225.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102225.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103225.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104225.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105225.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106225.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 122:
			strcpy(szTexture, "mesh\\ch0101115.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101115.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102115.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103115.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104115.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105115.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106115.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 123:
			strcpy(szTexture, "mesh\\ch0103134.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szTexture, "mesh\\ch0102134.wys");
				strcpy(szName, "mesh\\ch0102134.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0103134.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0104134.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0105134.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0106134.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106134.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 124:
			strcpy(szTexture, "mesh\\ch0101117.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101117.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102117.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103117.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104117.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105117.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106117.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 125:
			strcpy(szTexture, "mesh\\ch0103135.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szTexture, "mesh\\ch0102135.wys");
				strcpy(szName, "mesh\\ch0102135.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0103135.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0104135.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0105135.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0106135.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106135.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 126:
			strcpy(szTexture, "mesh\\ch0103136.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szTexture, "mesh\\ch0102136.wys");
				strcpy(szName, "mesh\\ch0102136.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0103136.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0104136.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0105136.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0106136.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106136.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 127:
			strcpy(szTexture, "mesh\\ch0103137.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szTexture, "mesh\\ch0102137.wys");
				strcpy(szName, "mesh\\ch0102137.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0103137.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0104137.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0105137.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0106137.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106137.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 128:
			strcpy(szTexture, "mesh\\ch0103103.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101103.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102103.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103103.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104103.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105103.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106103.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 129:
			strcpy(szTexture, "mesh\\ch0104103.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101103.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102103.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103103.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104103.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105103.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106103.msh");
				m_Cos = 1;
				break;
			}
			break;

		case 130:
			strcpy(szTexture, "mesh\\ch0201100.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0201100.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0202100.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0203100.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0204100.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0205100.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0206100.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 131:
			strcpy(szTexture, "mesh\\ch0201101.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0201101.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0202101.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0203101.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0204101.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0205101.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0206101.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 132:
			strcpy(szTexture, "mesh\\ch0201102.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0201102.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0202102.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0203102.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0204102.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0205102.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0206102.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 133:
			strcpy(szTexture, "mesh\\ch0201103.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0201103.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0202103.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0203103.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0204103.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0205103.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0206103.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 134:
			strcpy(szTexture, "mesh\\ch0201108.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0201108.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0202108.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0203108.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0204108.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0205108.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0206108.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 135:
			strcpy(szTexture, "mesh\\ch0201109.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0201109.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0202109.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0203109.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0204109.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0205109.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0206109.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 136:
			strcpy(szTexture, "mesh\\ch0201110.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0201110.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0202110.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0203110.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0204110.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0205110.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0206110.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 137:
			strcpy(szTexture, "mesh\\ch0201111.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0201111.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0202111.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0203111.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0204111.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0205111.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0206111.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 138:
			strcpy(szTexture, "mesh\\ch0201112.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0201112.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0202112.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0203112.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0204112.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0205112.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0206112.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 139:
			strcpy(szTexture, "mesh\\ch0201113.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0201113.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0202113.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0203113.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0204113.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0205113.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0206113.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 140:
			strcpy(szTexture, "mesh\\ch0201114.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0201114.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0202114.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0203114.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0204114.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0205114.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0206114.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 141:
			strcpy(szTexture, "mesh\\ch0201115.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0201115.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0202115.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0203115.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0204115.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0205115.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0206115.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 142:
			strcpy(szTexture, "mesh\\ch0201116.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0201116.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0202116.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0203116.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0204116.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0205116.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0206116.msh");
				m_Cos = 1;
				break;
			}
			break;

		case 143:
			strcpy(szTexture, "mesh\\ch0202101.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0201101.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0202101.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0203101.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0204101.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0205101.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0206101.msh");
				m_Cos = 1;
				break;
			}
			break;

		case 144:
			strcpy(szTexture, "mesh\\ch0202100.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0201100.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0202100.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0203100.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0204100.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0205100.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0206100.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 145:
			strcpy(szTexture, "mesh\\ch0203100.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0201100.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0202100.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0203100.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0204100.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0205100.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0206100.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 146:
			strcpy(szTexture, "mesh\\ch0203101.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0201101.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0202101.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0203101.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0204101.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0205101.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0206101.msh");
				m_Cos = 1;
				break;
			}
			break;

		case 147:
			strcpy(szTexture, "mesh\\ch0202102.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0201102.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0202102.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0203102.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0204102.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0205102.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0206102.msh");
				m_Cos = 1;
				break;
			}
			break;

		case 148:
			strcpy(szTexture, "mesh\\ch0102122.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101122.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102122.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103122.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104122.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105122.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106122.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 149:
			strcpy(szTexture, "mesh\\ch0101164.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101164.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102164.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103164.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104164.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105164.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106164.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 150:
			strcpy(szTexture, "mesh\\ch0101166.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101166.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102166.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103166.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104166.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105166.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106166.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 151:
			strcpy(szTexture, "mesh\\ch0101208.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101127.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102127.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103127.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104127.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105127.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106127.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 152:
			strcpy(szTexture, "mesh\\ch0101209.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101127.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102127.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103127.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104127.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105127.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106127.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 153:
			strcpy(szTexture, "mesh\\ch0101210.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101127.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102127.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103127.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104127.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105127.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106127.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 154:
			strcpy(szTexture, "mesh\\ch0101213.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101161.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102161.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103161.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104161.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105161.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106161.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 155:
			strcpy(szTexture, "mesh\\ch0101214.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101151.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102151.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103151.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104151.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105151.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106151.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 156:
			strcpy(szTexture, "mesh\\ch0203104.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szTexture, "mesh\\ch0202104.wys");
				strcpy(szName, "mesh\\ch0202104.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0203104.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0204104.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0205104.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0206104.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0206104.msh");
				m_Cos = 1;
				break;
			}
			break;

		case 157:
			strcpy(szTexture, "mesh\\ch0203105.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szTexture, "mesh\\ch0202105.wys");
				strcpy(szName, "mesh\\ch0202105.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0203105.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0204105.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0205105.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0206105.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0206105.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 158:
			strcpy(szTexture, "mesh\\ch0203106.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szTexture, "mesh\\ch0202106.wys");
				strcpy(szName, "mesh\\ch0202106.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0203106.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0204106.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0205106.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0206106.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0206106.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 159:
			strcpy(szTexture, "mesh\\ch0203107.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szTexture, "mesh\\ch0202107.wys");
				strcpy(szName, "mesh\\ch0202107.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0203107.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0204107.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0205107.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0206107.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0206107.msh");
				m_Cos = 1;
				break;
			}
			break;
		case 160:
			strcpy(szTexture, "mesh\\ch0101116.wys");
			switch (m_Cos)
			{
			case 1:
				strcpy(szName, "mesh\\ch0101116.msh");
				m_Cos = 2;
				break;
			case 2:
				strcpy(szName, "mesh\\ch0102116.msh");
				m_Cos = 3;
				break;
			case 3:
				strcpy(szName, "mesh\\ch0103116.msh");
				m_Cos = 4;
				break;
			case 4:
				strcpy(szName, "mesh\\ch0104116.msh");
				m_Cos = 5;
				break;
			case 5:
				strcpy(szName, "mesh\\ch0105116.msh");
				m_Cos = 6;
				break;
			case 6:
				strcpy(szName, "mesh\\ch0106116.msh");
				m_Cos = 1;
				break;
			}
			break;
			
		default:
			SetOldCostume(Costype, szTexture, szName);
			break;

		
		}
	}

}

int TMSkinMesh::MantleException(char* texture)
{
	if (!strcmp(texture, "mesh\\mt0101170.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101171.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101172.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101173.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101174.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101175.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101176.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101177.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101178.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101179.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101180.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101181.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101182.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101183.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101184.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101185.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101186.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101187.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101188.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101189.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101190.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101191.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101192.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101193.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101195.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101196.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101197.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101198.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101199.wyt"))
		return 1;
	if (!strcmp(texture, "mesh\\mt0101200.wyt"))
		return 1;

	return 0;
}

BOOL TMSkinMesh::God2Exception(int i)
{
	return MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'g'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'o'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'd'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'r'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[8] == '2'
		&& i == 1
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'd'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'r'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[8] == '1'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'b'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'd'
		&& i == 1
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'b'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'e'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'b'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'o'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'b'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'm'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'h'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'y'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 's'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'p'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'c'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'r'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'w'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'b'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'w'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'f'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'b'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'e'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'c'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'b'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'm'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'i'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'm'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'o'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 't'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'w'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 't'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'r'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'h'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 's'
		&& i == 1
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'e'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 't'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'b'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'n'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'r'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'c'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'f'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'n'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 'b'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'l'
		|| MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[5] == 't'
		&& MeshManager::m_BoneAnimationList[m_nBoneAniIndex].szAniName[6] == 'g';
}
