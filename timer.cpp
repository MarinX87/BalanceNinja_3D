//=============================================================================
//
// タイム処理 [timer.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "timer.h"
#include "sprite.h"
#include "sound.h"
#include "score.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(32)	// キャラサイズ
#define TEXTURE_HEIGHT				(64)	// 
#define TEXTURE_MAX					(2)		// テクスチャの数


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/number16x32.png",
	"data/TEXTURE/colon.png",
};


static BOOL						g_Use;						// TRUE:使っている  FALSE:未使用
static float					g_w, g_h;					// 幅と高さ
static XMFLOAT3					g_PosMin;					// ポリゴンの座標
static XMFLOAT3					g_PosSec;					// ポリゴンの座標
static XMFLOAT3					g_PosColon;			// ポリゴンの座標
static int						g_TexNo;					// テクスチャ番号

static int						g_Frame;					// フレーム
static int						g_Time;						// タイム

static BOOL						g_Load = FALSE;


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitTime(void)
{
	ID3D11Device *pDevice = GetDevice();

	//テクスチャ生成
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

	g_Use   = TRUE;
	g_w     = TEXTURE_WIDTH;
	g_h     = TEXTURE_HEIGHT;

	g_PosMin = { SCREEN_WIDTH / 2 - TEXTURE_WIDTH * 1, TEXTURE_HEIGHT, 0.0f };
	g_PosSec = { SCREEN_WIDTH / 2 + TEXTURE_WIDTH * 2, TEXTURE_HEIGHT, 0.0f };
	g_PosColon = { SCREEN_WIDTH / 2, TEXTURE_HEIGHT, 0.0f };

	g_TexNo = 0;

	g_Frame = 0;
	g_Time = 0;

	g_Load = TRUE;

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitTime(void)
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
void UpdateTime(void)
{
	g_Frame++;

	if (g_Frame >= 60)
	{
		g_Frame = 0;
		g_Time++;

		if ((g_Time + 1) % 15 == 0 || (g_Time + 2) % 15 == 0)
		{
			PlaySound(SOUND_LABEL_SE_KANG);
		}

		if (g_Time % 15 == 0)
		{
			AddScore(100);
		}
	}
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawTime(void)
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

	// テクスチャ設定
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);

	// 桁数分処理する
	int number = g_Time / 60;
	for (int i = 0; i < TIMER_DIGIT; i++)
	{
		// 今回表示する桁の数字
		float x = (float)(number % 10);

		// スコアの位置やテクスチャー座標を反映
		float px = g_PosMin.x - g_w*i;	// スコアの表示位置X
		float py = g_PosMin.y;			// スコアの表示位置Y
		float pw = g_w;				// スコアの表示幅
		float ph = g_h;				// スコアの表示高さ

		float tw = 1.0f / 10;		// テクスチャの幅
		float th = 1.0f / 1;		// テクスチャの高さ
		float tx = x * tw;			// テクスチャの左上X座標
		float ty = 0.0f;			// テクスチャの左上Y座標

		XMFLOAT4 color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		if (g_Time != 0 && ((((g_Time) % 15 == 0) || ((g_Time + 1) % 15 == 0) || ((g_Time + 2) % 15 == 0))))
		{
			color = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
		}

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
			color);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);

		// 次の桁へ
		number /= 10;
	}

	// 桁数分処理する
	number = g_Time % 60;
	for (int i = 0; i < TIMER_DIGIT; i++)
	{
		// 今回表示する桁の数字
		float x = (float)(number % 10);

		// スコアの位置やテクスチャー座標を反映
		float px = g_PosSec.x - g_w * i;	// スコアの表示位置X
		float py = g_PosSec.y;			// スコアの表示位置Y
		float pw = g_w;				// スコアの表示幅
		float ph = g_h;				// スコアの表示高さ

		float tw = 1.0f / 10;		// テクスチャの幅
		float th = 1.0f / 1;		// テクスチャの高さ
		float tx = x * tw;			// テクスチャの左上X座標
		float ty = 0.0f;			// テクスチャの左上Y座標

		XMFLOAT4 color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		if (g_Time != 0 && ( ( ((g_Time) % 15 == 0) || ((g_Time + 1) % 15 == 0) ||  ((g_Time + 2) % 15 == 0)) ) )
		{
			color = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
		}

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
			color);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);

		// 次の桁へ
		number /= 10;
	}

	// テクスチャ設定
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

	// スコアの位置やテクスチャー座標を反映
	float px = g_PosColon.x;	// スコアの表示位置X
	float py = g_PosColon.y;	// スコアの表示位置Y
	float pw = g_w;				// スコアの表示幅
	float ph = g_h;				// スコアの表示高さ

	float tw = 1.0f;		// テクスチャの幅
	float th = 1.0f;		// テクスチャの高さ
	float tx = 0.0f;			// テクスチャの左上X座標
	float ty = 0.0f;			// テクスチャの左上Y座標

	XMFLOAT4 color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	if (g_Time != 0 && ((((g_Time) % 15 == 0) || ((g_Time + 1) % 15 == 0) || ((g_Time + 2) % 15 == 0))))
	{
		color = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	}

	// １枚のポリゴンの頂点とテクスチャ座標を設定
	SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
		color);

	// ポリゴン描画
	GetDeviceContext()->Draw(4, 0);
}

int GetTime(void)
{
	return g_Time;
}

