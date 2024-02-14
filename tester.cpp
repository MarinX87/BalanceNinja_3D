//=============================================================================
//
// ���f������ [player.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "light.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "tester.h"
#include "shadow.h"
#include "bullet.h"
#include "debugproc.h"
#include "meshfield.h"
#include "timer.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MODEL_PLAYER			"data/MODEL/body.obj"			// �ǂݍ��ރ��f����

// 9 Parts
#define	MODEL_PLAYER_HEAD		"data/MODEL/head.obj"			// �ǂݍ��ރ��f����
#define	MODEL_PLAYER_LEFT_ARM	"data/MODEL/larm.obj"			// �ǂݍ��ރ��f����
#define	MODEL_PLAYER_LEFT_HAND	"data/MODEL/lhand.obj"			// �ǂݍ��ރ��f����
#define	MODEL_PLAYER_LEFT_LEG	"data/MODEL/lleg.obj"			// �ǂݍ��ރ��f����
#define	MODEL_PLAYER_LEFT_FOOT	"data/MODEL/lfoot.obj"			// �ǂݍ��ރ��f����
#define	MODEL_PLAYER_RIGHT_ARM	"data/MODEL/rarm.obj"			// �ǂݍ��ރ��f����
#define	MODEL_PLAYER_RIGHT_HAND	"data/MODEL/rhand.obj"			// �ǂݍ��ރ��f����
#define	MODEL_PLAYER_RIGHT_LEG	"data/MODEL/rleg.obj"			// �ǂݍ��ރ��f����
#define	MODEL_PLAYER_RIGHT_FOOT	"data/MODEL/rfoot.obj"			// �ǂݍ��ރ��f����


#define	VALUE_MOVE			(2.0f)							// �ړ���
#define	VALUE_ROTATE		(D3DX_PI * 0.02f)				// ��]��

#define PLAYER_SHADOW_SIZE	(0.4f)							// �e�̑傫��
#define PLAYER_OFFSET_Y		(17.0f)							// �v���C���[�̑��������킹��

#define PLAYER_PARTS_MAX	(9)								// �v���C���[�̃p�[�c�̐�



//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static TESTER		g_Tester;						// �v���C���[

static TESTER		g_Parts[PLAYER_PARTS_MAX];		// �v���C���[�̃p�[�c�p


static float		g_RotationForce;

static float		g_WindForce;
static float		g_WindForceMax;


static BOOL			WindChanged;

static LIGHT		g_Light;


// �v���C���[�̊K�w�A�j���[�V�����f�[�^


// �v���C���[�̓������E�ɓ������Ă���A�j���f�[�^
static INTERPOLATION_DATA move_tbl_left_arm[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(-5.5f, 26.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 240 },
	{ XMFLOAT3(-5.5f, 26.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, XM_2PI / 6),      XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },
};
static INTERPOLATION_DATA move_tbl_left_hand[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(-5.5f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, XM_2PI / 6), XMFLOAT3(1.0f, 1.0f, 1.0f), 240 },
	{ XMFLOAT3(-5.5f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),      XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },
};

static INTERPOLATION_DATA move_tbl_right_arm[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(5.5f, 26.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 240 },
	{ XMFLOAT3(5.5f, 26.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, XM_2PI / 6),      XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },

};
static INTERPOLATION_DATA move_tbl_right_hand[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(5.5f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, XM_2PI / 6), XMFLOAT3(1.0f, 1.0f, 1.0f), 240 },
	{ XMFLOAT3(5.5f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),      XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },
};

static INTERPOLATION_DATA* g_MoveTblAdr[] =
{
	move_tbl_left_arm,
	move_tbl_left_hand,
	move_tbl_right_arm,
	move_tbl_right_hand,
};

//=============================================================================
// ����������
//=============================================================================
HRESULT InitTester(void)
{
	g_Tester.load = TRUE;
	LoadModel(MODEL_PLAYER, &g_Tester.model);

	g_Tester.pos = XMFLOAT3(0.0f, PLAYER_OFFSET_Y, 0.0f);
	g_Tester.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Tester.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

	g_Tester.spd = 0.0f;			// �ړ��X�s�[�h�N���A

	g_Tester.use = TRUE;			// TRUE:�����Ă�
	g_Tester.size = PLAYER_SIZE;	// �����蔻��̑傫��

	// �����Ńv���C���[�p�̉e���쐬���Ă���
	XMFLOAT3 pos = g_Tester.pos;
	pos.y -= (PLAYER_OFFSET_Y - 0.1f);
	g_Tester.shadowIdx = CreateShadow(pos, PLAYER_SHADOW_SIZE, PLAYER_SHADOW_SIZE);
	//          ��
	//        ���̃����o�[�ϐ������������e��Index�ԍ�


	g_RotationForce = 0.0f;

	g_WindForce = 0.0f;
	g_WindForceMax = 0.0f;

	g_Tester.parent = NULL;			// �{�́i�e�j�Ȃ̂�NULL������


	// �K�w�A�j���[�V�����̏�����
	for (int i = 0; i < PLAYER_PARTS_MAX; i++)
	{
		g_Parts[i].use = FALSE;

		// �ʒu�E��]�E�X�P�[���̏����ݒ�
		g_Parts[i].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Parts[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Parts[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

		// �e�q�֌W
		g_Parts[i].parent = &g_Tester;		// �� �����ɐe�̃A�h���X������
		//	g_Parts[�r].parent= &g_Tester;		// �r��������e�͖{�́i�v���C���[�j
		//	g_Parts[��].parent= &g_Paerts[�r];	// �w���r�̎q���������ꍇ�̗�

			// �K�w�A�j���[�V�����p�̃����o�[�ϐ��̏�����
		g_Parts[i].time = 0.0f;			// ���`��ԗp�̃^�C�}�[���N���A
		g_Parts[i].tblNo = 0;			// �Đ�����s���f�[�^�e�[�u��No���Z�b�g
		g_Parts[i].tblMax = 0;			// �Đ�����s���f�[�^�e�[�u���̃��R�[�h�����Z�b�g

		// �p�[�c�̓ǂݍ��݂͂܂����Ă��Ȃ�
		g_Parts[i].load = FALSE;
	}

	g_Parts[0].use = TRUE;
	g_Parts[0].parent = &g_Tester;	// �e���Z�b�g
	//g_Parts[0].tblNo = 0;			// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	//g_Parts[0].tblMax = sizeof(move_tbl_left) / sizeof(INTERPOLATION_DATA);	// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
	//g_Parts[0].load = TRUE;
	LoadModel(MODEL_PLAYER_HEAD, &g_Parts[0].model);

	g_Parts[1].use = TRUE;
	g_Parts[1].parent = &g_Tester;	// �e���Z�b�g
	g_Parts[1].tblNo = 0;			// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	g_Parts[1].tblMax = sizeof(move_tbl_left_arm) / sizeof(INTERPOLATION_DATA);	// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
	g_Parts[1].load = TRUE;
	LoadModel(MODEL_PLAYER_LEFT_ARM, &g_Parts[1].model);


	g_Parts[2].use = TRUE;
	g_Parts[2].parent = &g_Parts[1];	// �e���Z�b�g
	g_Parts[2].tblNo = 1;			// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	g_Parts[2].tblMax = sizeof(move_tbl_left_hand) / sizeof(INTERPOLATION_DATA);	// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
	g_Parts[2].load = TRUE;
	LoadModel(MODEL_PLAYER_LEFT_HAND, &g_Parts[2].model);

	g_Parts[3].use = TRUE;
	g_Parts[3].parent = &g_Tester;	// �e���Z�b�g
	LoadModel(MODEL_PLAYER_LEFT_LEG, &g_Parts[3].model);

	g_Parts[4].use = TRUE;
	g_Parts[4].parent = &g_Tester;	// �e���Z�b�g
	LoadModel(MODEL_PLAYER_LEFT_FOOT, &g_Parts[4].model);


	g_Parts[5].use = TRUE;
	g_Parts[5].parent = &g_Tester;	// �e���Z�b�g
	g_Parts[5].tblNo = 2;			// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	g_Parts[5].tblMax = sizeof(move_tbl_right_arm) / sizeof(INTERPOLATION_DATA);	// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
	g_Parts[5].load = TRUE;
	LoadModel(MODEL_PLAYER_RIGHT_ARM, &g_Parts[5].model);

	g_Parts[6].use = TRUE;
	g_Parts[6].parent = &g_Parts[5];	// �e���Z�b�g
	g_Parts[6].tblNo = 3;			// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	g_Parts[6].tblMax = sizeof(move_tbl_right_hand) / sizeof(INTERPOLATION_DATA);	// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
	g_Parts[6].load = TRUE;
	LoadModel(MODEL_PLAYER_RIGHT_HAND, &g_Parts[6].model);

	g_Parts[7].use = TRUE;
	g_Parts[7].parent = &g_Tester;	// �e���Z�b�g
	LoadModel(MODEL_PLAYER_RIGHT_LEG, &g_Parts[7].model);

	g_Parts[8].use = TRUE;
	g_Parts[8].parent = &g_Tester;	// �e���Z�b�g
	LoadModel(MODEL_PLAYER_RIGHT_FOOT, &g_Parts[8].model);



	// �N�H�[�^�j�I���̏�����
	XMStoreFloat4(&g_Tester.Quaternion, XMQuaternionIdentity());



	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitTester(void)
{
	// ���f���̉������
	if (g_Tester.load == TRUE)
	{
		UnloadModel(&g_Tester.model);
		g_Tester.load = FALSE;
	}

	// �p�[�c�̉������
	for (int i = 0; i < PLAYER_PARTS_MAX; i++)
	{
		if (g_Parts[i].load == TRUE)
		{
			// �p�[�c�̉������
			UnloadModel(&g_Parts[i].model);
			g_Parts[i].load = FALSE;
		}
	}



}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateTester(void)
{
	CAMERA* cam = GetCamera();

	//g_Tester.spd *= 0.7f;

	//if (g_RotationForce >= 0.0f && g_RotationForce <  0.001f) g_RotationForce =  0.001f;
	//if (g_RotationForce <= 0.0f && g_RotationForce > -0.001f) g_RotationForce = -0.001f;

	// �ړ�����
	if (GetKeyboardPress(DIK_LEFT))
	{
		g_RotationForce -= 0.002f;
	}
	else if (GetKeyboardPress(DIK_RIGHT))
	{
		g_RotationForce += 0.002f;

	}
	else
	{
		g_RotationForce *= 0.7f;
	}

#ifdef _DEBUG
	if (GetKeyboardPress(DIK_R))
	{
		g_Tester.pos.z = g_Tester.pos.x = 0.0f;
		g_Tester.spd = 0.0f;
		//roty = 0.0f;
	}
#endif


	//Wind����
	{
		if (GetTime() % 2 == 0)
		{
			if (!WindChanged)
			{
				g_WindForce = rand() % 10 * GetTime();
				g_WindForce /= 1000;

				WindChanged = TRUE;
			}
		}
		else
		{
			WindChanged = FALSE;
		}
	}


	//Rotation����
	{
		float playerRotation = g_RotationForce + g_WindForce;

		g_Tester.rot.z += playerRotation;
	}



	//g_RotationForce *= 0.7;

	//// �e���v���C���[�̈ʒu�ɍ��킹��
	//XMFLOAT3 pos = g_Tester.pos;
	//pos.y -= (PLAYER_OFFSET_Y - 0.1f);
	//SetPositionShadow(g_Tester.shadowIdx, pos);


	// �K�w�A�j���[�V����
	for (int i = 0; i < PLAYER_PARTS_MAX; i++)
	{
		// �g���Ă���Ȃ珈������
		if ((g_Parts[i].use == TRUE) && (g_Parts[i].tblMax > 0))
		{	// ���`��Ԃ̏���
			int nowNo = (int)g_Parts[i].time;			// �������ł���e�[�u���ԍ������o���Ă���
			int maxNo = g_Parts[i].tblMax;				// �o�^�e�[�u�����𐔂��Ă���
			int nextNo = (nowNo + 1) % maxNo;			// �ړ���e�[�u���̔ԍ������߂Ă���
			INTERPOLATION_DATA* tbl = g_MoveTblAdr[g_Parts[i].tblNo];	// �s���e�[�u���̃A�h���X���擾

			XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);	// XMVECTOR�֕ϊ�
			XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);	// XMVECTOR�֕ϊ�
			XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);	// XMVECTOR�֕ϊ�

			XMVECTOR Pos = XMLoadFloat3(&tbl[nextNo].pos) - nowPos;	// XYZ�ړ��ʂ��v�Z���Ă���
			XMVECTOR Rot = XMLoadFloat3(&tbl[nextNo].rot) - nowRot;	// XYZ��]�ʂ��v�Z���Ă���
			XMVECTOR Scl = XMLoadFloat3(&tbl[nextNo].scl) - nowScl;	// XYZ�g�嗦���v�Z���Ă���

			float nowTime = g_Parts[i].time - nowNo;	// ���ԕ����ł��鏭�������o���Ă���

			Pos *= nowTime;								// ���݂̈ړ��ʂ��v�Z���Ă���
			Rot *= nowTime;								// ���݂̉�]�ʂ��v�Z���Ă���
			Scl *= nowTime;								// ���݂̊g�嗦���v�Z���Ă���

			// �v�Z���ċ��߂��ړ��ʂ����݂̈ړ��e�[�u��XYZ�ɑ����Ă��遁�\�����W�����߂Ă���
			XMStoreFloat3(&g_Parts[i].pos, nowPos + Pos);

			// �v�Z���ċ��߂���]�ʂ����݂̈ړ��e�[�u���ɑ����Ă���
			XMStoreFloat3(&g_Parts[i].rot, nowRot + Rot);

			// �v�Z���ċ��߂��g�嗦�����݂̈ړ��e�[�u���ɑ����Ă���
			XMStoreFloat3(&g_Parts[i].scl, nowScl + Scl);

			// frame���g�Ď��Ԍo�ߏ���������
			g_Parts[i].time += 1.0f / tbl[nowNo].frame;	// ���Ԃ�i�߂Ă���
			if ((int)g_Parts[i].time >= maxNo)			// �o�^�e�[�u���Ō�܂ňړ��������H
			{
				g_Parts[i].time -= maxNo;				// �O�ԖڂɃ��Z�b�g�������������������p���ł���
			}

		}

	}


	// �|�C���g���C�g�̃e�X�g
	{
		LIGHT* light = GetLightData(1);
		XMFLOAT3 pos = g_Tester.pos;
		pos.y += 60.0f;

		light->Position = pos;
		light->Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		light->Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		light->Type = LIGHT_TYPE_POINT;
		light->Enable = TRUE;
		SetLightData(1, light);
	}


#ifdef _DEBUG
	// �f�o�b�O�\��
	PrintDebugProc("Player X:%f Y:%f Z:% N:%f\n", g_Tester.pos.x, g_Tester.pos.y, g_Tester.pos.z/*, Normal.y*/);
	PrintDebugProc("Wind Force:%f\n", g_WindForce * 1000);
	PrintDebugProc("Rotation Force:%f\n", g_RotationForce * 1000);
#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawTester(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld, quatMatrix;

	// �J�����O����
	SetCullingMode(CULL_MODE_NONE);

	// ���[���h�}�g���b�N�X�̏�����
	mtxWorld = XMMatrixIdentity();

	// �X�P�[���𔽉f
	mtxScl = XMMatrixScaling(g_Tester.scl.x, g_Tester.scl.y, g_Tester.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// ��]�𔽉f
	mtxRot = XMMatrixRotationRollPitchYaw(g_Tester.rot.x, g_Tester.rot.y + XM_PI, g_Tester.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	//// �N�H�[�^�j�I���𔽉f
	//quatMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&g_Tester.Quaternion));
	//mtxWorld = XMMatrixMultiply(mtxWorld, quatMatrix);

	// �ړ��𔽉f
	mtxTranslate = XMMatrixTranslation(g_Tester.pos.x, g_Tester.pos.y, g_Tester.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ���[���h�}�g���b�N�X�̐ݒ�
	SetWorldMatrix(&mtxWorld);

	XMStoreFloat4x4(&g_Tester.mtxWorld, mtxWorld);


	// �����̐ݒ�
	//SetFuchi(1);

	// ���f���`��
	DrawModel(&g_Tester.model);



	// �K�w�A�j���[�V����
	for (int i = 0; i < PLAYER_PARTS_MAX; i++)
	{
		// ���[���h�}�g���b�N�X�̏�����
		mtxWorld = XMMatrixIdentity();

		// �X�P�[���𔽉f
		mtxScl = XMMatrixScaling(g_Parts[i].scl.x, g_Parts[i].scl.y, g_Parts[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// ��]�𔽉f
		mtxRot = XMMatrixRotationRollPitchYaw(g_Parts[i].rot.x, g_Parts[i].rot.y, g_Parts[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// �ړ��𔽉f
		mtxTranslate = XMMatrixTranslation(g_Parts[i].pos.x, g_Parts[i].pos.y, g_Parts[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		if (g_Parts[i].parent != NULL)	// �q����������e�ƌ�������
		{
			mtxWorld = XMMatrixMultiply(mtxWorld, XMLoadFloat4x4(&g_Parts[i].parent->mtxWorld));
			// ��
			// g_Tester.mtxWorld���w���Ă���
		}

		XMStoreFloat4x4(&g_Parts[i].mtxWorld, mtxWorld);

		// �g���Ă���Ȃ珈������
		if (g_Parts[i].use == FALSE) continue;

		// ���[���h�}�g���b�N�X�̐ݒ�
		SetWorldMatrix(&mtxWorld);


		// ���f���`��
		DrawModel(&g_Parts[i].model);

	}

	//SetFuchi(0);

	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);
}


//=============================================================================
// �v���C���[�����擾
//=============================================================================
TESTER* GetTester(void)
{
	return &g_Tester;
}

