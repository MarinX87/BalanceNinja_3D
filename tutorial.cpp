//=============================================================================
//
// タイトル画面処理 [tutorial.cpp]
// Author : ヨウシゲン
//
//=============================================================================
#include "tutorial.h"
#include "input.h"
#include "fade.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// 背景サイズ
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	// 
#define TEXTURE_MAX					(3)				// テクスチャの数

#define SOME_UPPER_VALUE_MENU       SCREEN_HEIGHT / 2+ 55.0f //上エッジ
#define SOME_LOWER_VALUE_MENU       SCREEN_HEIGHT / 2+ 120.0f//下エッジ
#define SOME_LEFT_VALUE_MENU        SCREEN_WIDTH / 2 - 210.0f //左エッジ
#define SOME_RIGHT_VALUE_MENU       SCREEN_WIDTH / 2 - 120.0f //右エッジ
#define TEXTURE_WIDTH_LOGO			(480)			// ロゴサイズ
#define TEXTURE_HEIGHT_LOGO			(80)			// 

#define TEXTURE_WIDTH_POINT             (80)			     // ポイントサイズ
#define TEXTURE_HEIGHT_POINT            (80)			     // ポイントサイズ

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;				// 頂点情報
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char* g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/BG_Tutorial.png",
};

static BOOL						g_Use;						// TRUE:使っている  FALSE:未使用
static float					g_w, g_h;					// 幅と高さ
static XMFLOAT3					g_Pos;						// ポリゴンの座標
static int						g_TexNo;					// テクスチャ番号

//float	alpha;
//BOOL	flag_alpha;

static BOOL						g_Load = FALSE;

static float	effect_dx;
static float	effect_dy;

static TUTORIAL	g_PonitMenu;                                 // 初期化を行ったかのフラグ
static TUTORIAL	g_Tutorial;
static int      g_mode = M_TUTORIAL;

//// 現在の選択位置を示す状態変数を追加します（0は上の位置を表し、1は下の位置を表します）
//int g_SelectedOption = 0;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitTutorial(void)
{
	ID3D11Device* pDevice = GetDevice();

	// テクスチャ生成
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TexturName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	// TUTORIAL MENUの初期化
	g_Tutorial.w = TEXTURE_WIDTH;
	g_Tutorial.h = TEXTURE_HEIGHT;
	g_Tutorial.pos = XMFLOAT3(0.0f, 0.0f, 0.0f);

	// TUTORIAL POINTの初期化
	g_PonitMenu.w = TEXTURE_WIDTH_POINT;
	g_PonitMenu.h = TEXTURE_HEIGHT_POINT;
	g_PonitMenu.pos = XMFLOAT3(SOME_LEFT_VALUE_MENU, SOME_UPPER_VALUE_MENU, 0.0f);
	g_PonitMenu.speed = 10;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitTutorial(void)
{
	if (g_Load == FALSE) return;

	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		if (g_Texture[i])
		{
			g_Texture[i]->Release();
			g_Texture[i] = NULL;
		}
	}

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateTutorial(void)
{

	if (GetKeyboardTrigger(DIK_RETURN))
	{

			SetFade(FADE_OUT, MODE_GAME);
	
	}
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawTutorial(void)
{
	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// マトリクス設定
	SetWorldViewProjection2D();

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// マテリアル設定
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	// タイトルの背景を描画
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteLeftTop(g_VertexBuffer, 0.0f, 0.0f, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}




	//  ポリゴン描画
	GetDeviceContext()->Draw(4, 0);

	SetBlendState(BLEND_MODE_ALPHABLEND); // 半透明処理を元に戻す
}
