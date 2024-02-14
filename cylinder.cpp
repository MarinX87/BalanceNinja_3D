//=============================================================================
//
// モデル処理 [CYLINDER.cpp]
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
// マクロ定義
//*****************************************************************************
#define	MODEL_CYLINDER		"data/MODEL/cylinder.obj"			// 読み込むモデル名


#define	VALUE_MOVE			(2.0f)							// 移動量
#define	VALUE_ROTATE		(D3DX_PI * 0.02f)				// 回転量

#define CYLINDER_SHADOW_SIZE	(0.4f)							// 影の大きさ
#define CYLINDER_OFFSET_Y		(7.0f)							// プレイヤーの足元をあわせる

#define CYLINDER_PARTS_MAX	(2)								// プレイヤーのパーツの数



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static CYLINDER		g_Cylinder;						// プレイヤー

static float		roty = 0.0f;

static float		RotationForce;

static LIGHT		g_Light;

// プレイヤーの階層アニメーションデータ

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitCylinder(void)
{
	g_Cylinder.load = TRUE;
	LoadModel(MODEL_CYLINDER, &g_Cylinder.model);

	g_Cylinder.pos = XMFLOAT3(0.0f, CYLINDER_OFFSET_Y, 0.0f);
	g_Cylinder.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Cylinder.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

	g_Cylinder.spd = 0.0f;			// 移動スピードクリア

	g_Cylinder.use = TRUE;			// TRUE:生きてる
	g_Cylinder.size = CYLINDER_SIZE;	// 当たり判定の大きさ

	// ここでプレイヤー用の影を作成している
	XMFLOAT3 pos = g_Cylinder.pos;
	pos.y -= (CYLINDER_OFFSET_Y - 0.1f);
	g_Cylinder.shadowIdx = CreateShadow(pos, CYLINDER_SHADOW_SIZE, CYLINDER_SHADOW_SIZE);
	//          ↑
	//        このメンバー変数が生成した影のIndex番号


	RotationForce = 0.0f;

	g_Cylinder.parent = NULL;			// 本体（親）なのでNULLを入れる

	// クォータニオンの初期化
	XMStoreFloat4(&g_Cylinder.Quaternion, XMQuaternionIdentity());



	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitCylinder(void)
{
	// モデルの解放処理
	if (g_Cylinder.load == TRUE)
	{
		UnloadModel(&g_Cylinder.model);
		g_Cylinder.load = FALSE;
	}
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateCylinder(void)
{
	CAMERA* cam = GetCamera();

	g_Cylinder.spd *= 0.7f;

	// 移動処理
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



	//// 影もプレイヤーの位置に合わせる
	//XMFLOAT3 pos = g_Cylinder.pos;
	//pos.y -= (CYLINDER_OFFSET_Y - 0.1f);
	//SetPositionShadow(g_Cylinder.shadowIdx, pos);

	// ポイントライトのテスト
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
	// デバッグ表示
	PrintDebugProc("CYLINDER X:%f Y:%f Z:% N:%f\n", g_Cylinder.pos.x, g_Cylinder.pos.y, g_Cylinder.pos.z/*, Normal.y*/);
#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawCylinder(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld, quatMatrix;

	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	// ワールドマトリックスの初期化
	mtxWorld = XMMatrixIdentity();

	// スケールを反映
	mtxScl = XMMatrixScaling(g_Cylinder.scl.x, g_Cylinder.scl.y, g_Cylinder.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// 回転を反映
	mtxRot = XMMatrixRotationRollPitchYaw(g_Cylinder.rot.x, g_Cylinder.rot.y + XM_PI, g_Cylinder.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	//// クォータニオンを反映
	//quatMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&g_Cylinder.Quaternion));
	//mtxWorld = XMMatrixMultiply(mtxWorld, quatMatrix);

	// 移動を反映
	mtxTranslate = XMMatrixTranslation(g_Cylinder.pos.x, g_Cylinder.pos.y, g_Cylinder.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ワールドマトリックスの設定
	SetWorldMatrix(&mtxWorld);

	XMStoreFloat4x4(&g_Cylinder.mtxWorld, mtxWorld);


	// 縁取りの設定
	//SetFuchi(1);

	// モデル描画
	DrawModel(&g_Cylinder.model);


	//SetFuchi(0);

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}


//=============================================================================
// プレイヤー情報を取得
//=============================================================================
CYLINDER* GetCylinder(void)
{
	return &g_Cylinder;
}

