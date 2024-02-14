//=============================================================================
//
// ���f������ [CYLINDER.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "light.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "cylinder.h"
#include "shadow.h"
#include "bullet.h"
#include "debugproc.h"
#include "meshfield.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MODEL_CYLINDER		"data/MODEL/cylinder.obj"			// �ǂݍ��ރ��f����


#define	VALUE_MOVE			(2.0f)							// �ړ���
#define	VALUE_ROTATE		(D3DX_PI * 0.02f)				// ��]��

#define CYLINDER_SHADOW_SIZE	(0.4f)							// �e�̑傫��
#define CYLINDER_OFFSET_Y		(7.0f)							// �v���C���[�̑��������킹��

#define CYLINDER_PARTS_MAX	(2)								// �v���C���[�̃p�[�c�̐�



//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static CYLINDER		g_Cylinder;						// �v���C���[

static float		roty = 0.0f;

static float		RotationForce;

static LIGHT		g_Light;

// �v���C���[�̊K�w�A�j���[�V�����f�[�^

//=============================================================================
// ����������
//=============================================================================
HRESULT InitCylinder(void)
{
	g_Cylinder.load = TRUE;
	LoadModel(MODEL_CYLINDER, &g_Cylinder.model);

	g_Cylinder.pos = XMFLOAT3(0.0f, CYLINDER_OFFSET_Y, 0.0f);
	g_Cylinder.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Cylinder.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

	g_Cylinder.spd = 0.0f;			// �ړ��X�s�[�h�N���A

	g_Cylinder.use = TRUE;			// TRUE:�����Ă�
	g_Cylinder.size = CYLINDER_SIZE;	// �����蔻��̑傫��

	// �����Ńv���C���[�p�̉e���쐬���Ă���
	XMFLOAT3 pos = g_Cylinder.pos;
	pos.y -= (CYLINDER_OFFSET_Y - 0.1f);
	g_Cylinder.shadowIdx = CreateShadow(pos, CYLINDER_SHADOW_SIZE, CYLINDER_SHADOW_SIZE);
	//          ��
	//        ���̃����o�[�ϐ������������e��Index�ԍ�


	RotationForce = 0.0f;

	g_Cylinder.parent = NULL;			// �{�́i�e�j�Ȃ̂�NULL������

	// �N�H�[�^�j�I���̏�����
	XMStoreFloat4(&g_Cylinder.Quaternion, XMQuaternionIdentity());



	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitCylinder(void)
{
	// ���f���̉������
	if (g_Cylinder.load == TRUE)
	{
		UnloadModel(&g_Cylinder.model);
		g_Cylinder.load = FALSE;
	}
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateCylinder(void)
{
	CAMERA* cam = GetCamera();

	g_Cylinder.spd *= 0.7f;

	// �ړ�����
	if (GetKeyboardPress(DIK_LEFT))
	{
		//g_Cylinder.spd = VALUE_MOVE;
		////g_Cylinder.pos.x -= g_Cylinder.spd;
		//roty + XM_PI / 2;

		RotationForce += 0.02;
	}
	if (GetKeyboardPress(DIK_RIGHT))
	{
		//g_Cylinder.spd = VALUE_MOVE;
		////g_Cylinder.pos.x += g_Cylinder.spd;
		//roty = -XM_PI / 2;

		RotationForce -= 0.02;
	}
	//if (GetKeyboardPress(DIK_UP))
	//{
	//	g_Cylinder.spd = VALUE_MOVE;
	//	//g_Cylinder.pos.z += g_Cylinder.spd;
	//	roty = XM_PI;
	//}
	//if (GetKeyboardPress(DIK_DOWN))
	//{
	//	g_Cylinder.spd = VALUE_MOVE;
	//	//g_Cylinder.pos.z -= g_Cylinder.spd;
	//	roty = 0.0f;
	//}

#ifdef _DEBUG
	if (GetKeyboardPress(DIK_R))
	{
		g_Cylinder.pos.z = g_Cylinder.pos.x = 0.0f;
		g_Cylinder.spd = 0.0f;
		roty = 0.0f;
	}
#endif



	//// �e���v���C���[�̈ʒu�ɍ��킹��
	//XMFLOAT3 pos = g_Cylinder.pos;
	//pos.y -= (CYLINDER_OFFSET_Y - 0.1f);
	//SetPositionShadow(g_Cylinder.shadowIdx, pos);

	// �|�C���g���C�g�̃e�X�g
	{
		LIGHT* light = GetLightData(2);
		XMFLOAT3 pos = g_Cylinder.pos;
		pos.z -= 20.0f;
		pos.x -= 10.0f;
		light->Position = pos;
		light->Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		light->Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		light->Type = LIGHT_TYPE_POINT;
		light->Enable = TRUE;
		SetLightData(2, light);

		light = GetLightData(3);
		pos.x += 20.0f;
		light->Position = pos;
		light->Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		light->Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		light->Type = LIGHT_TYPE_POINT;
		light->Enable = TRUE;
		SetLightData(3, light);
	}


#ifdef _DEBUG
	// �f�o�b�O�\��
	PrintDebugProc("CYLINDER X:%f Y:%f Z:% N:%f\n", g_Cylinder.pos.x, g_Cylinder.pos.y, g_Cylinder.pos.z/*, Normal.y*/);
#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawCylinder(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld, quatMatrix;

	// �J�����O����
	SetCullingMode(CULL_MODE_NONE);

	// ���[���h�}�g���b�N�X�̏�����
	mtxWorld = XMMatrixIdentity();

	// �X�P�[���𔽉f
	mtxScl = XMMatrixScaling(g_Cylinder.scl.x, g_Cylinder.scl.y, g_Cylinder.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// ��]�𔽉f
	mtxRot = XMMatrixRotationRollPitchYaw(g_Cylinder.rot.x, g_Cylinder.rot.y + XM_PI, g_Cylinder.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	//// �N�H�[�^�j�I���𔽉f
	//quatMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&g_Cylinder.Quaternion));
	//mtxWorld = XMMatrixMultiply(mtxWorld, quatMatrix);

	// �ړ��𔽉f
	mtxTranslate = XMMatrixTranslation(g_Cylinder.pos.x, g_Cylinder.pos.y, g_Cylinder.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ���[���h�}�g���b�N�X�̐ݒ�
	SetWorldMatrix(&mtxWorld);

	XMStoreFloat4x4(&g_Cylinder.mtxWorld, mtxWorld);


	// �����̐ݒ�
	//SetFuchi(1);

	// ���f���`��
	DrawModel(&g_Cylinder.model);


	//SetFuchi(0);

	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);
}


//=============================================================================
// �v���C���[�����擾
//=============================================================================
CYLINDER* GetCylinder(void)
{
	return &g_Cylinder;
}

