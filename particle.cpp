//=============================================================================
//
// パーティクル処理 [particle.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "shadow.h"
#include "light.h"
#include "particle.h"
#include "player.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_MAX			(4)			// テクスチャの数

#define	PARTICLE_SIZE_X		(40.0f)		// 頂点サイズ
#define	PARTICLE_SIZE_Y		(40.0f)		// 頂点サイズ
#define	VALUE_MOVE_PARTICLE	(5.0f)		// 移動速度

#define	MAX_PARTICLE		(512)		// パーティクル最大数

#define	DISP_SHADOW						// 影の表示
//#undef DISP_SHADOW

//*****************************************************************************
// 構造体定義
//*****************************************************************************
typedef struct
{
	XMFLOAT3		pos;			// 位置
	XMFLOAT3		rot;			// 回転
	XMFLOAT3		scale;			// スケール
	XMFLOAT3		move;			// 移動量
	MATERIAL		material;		// マテリアル
	float			fSizeX;			// 幅
	float			fSizeY;			// 高さ
	int				nIdxShadow;		// 影ID
	int				nLife;			// 寿命
	BOOL			use;			// 使用しているかどうか

} PARTICLE;

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT MakeVertexParticle(void);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;		// 頂点バッファ

static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報
static int							g_TexNo;					// テクスチャ番号

static PARTICLE					g_aParticle[MAX_PARTICLE];		// パーティクルワーク
static XMFLOAT3					g_posBase;						// ビルボード発生位置
static float					g_fWidthBase = 0.1f;			// 基準の幅
static float					g_fHeightBase = 10.0f;			// 基準の高さ
static float					g_roty = 0.0f;					// 移動方向
static float					g_spd = 0.0f;					// 移動スピード

static char* g_TextureName[] =
{
	"data/TEXTURE/effect000.jpg",
	"data/TEXTURE/wind0.png",
	"data/TEXTURE/wind1.png",
	"data/TEXTURE/wind2.png",

};

static BOOL							g_Load = FALSE;

int windnot;//無風かどうか
int counttime;//パーティクル発生待ち時間
float windpower;//風力
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitParticle(void)
{
	counttime = 0;

	// 頂点情報の作成
	MakeVertexParticle();

	// テクスチャ生成
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TextureName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}

	g_TexNo = 2;

	// パーティクルワークの初期化
	for (int nCntParticle = 0; nCntParticle < MAX_PARTICLE; nCntParticle++)
	{
		g_aParticle[nCntParticle].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aParticle[nCntParticle].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aParticle[nCntParticle].scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
		g_aParticle[nCntParticle].move = XMFLOAT3(1.0f, 1.0f, 1.0f);

		ZeroMemory(&g_aParticle[nCntParticle].material, sizeof(g_aParticle[nCntParticle].material));
		g_aParticle[nCntParticle].material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		g_aParticle[nCntParticle].fSizeX = PARTICLE_SIZE_X;
		g_aParticle[nCntParticle].fSizeY = PARTICLE_SIZE_Y;
		g_aParticle[nCntParticle].nIdxShadow = -1;
		g_aParticle[nCntParticle].nLife = 0;
		g_aParticle[nCntParticle].use = FALSE;
	}

	g_posBase = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_roty = 0.0f;
	g_spd = 0.0f;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitParticle(void)
{
	if (g_Load == FALSE) return;

	//テクスチャの解放
	for (int nCntTex = 0; nCntTex < TEXTURE_MAX; nCntTex++)
	{
		if (g_Texture[nCntTex] != NULL)
		{
			g_Texture[nCntTex]->Release();
			g_Texture[nCntTex] = NULL;
		}
	}

	// 頂点バッファの解放
	if (g_VertexBuffer != NULL)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateParticle(void)
{
	//PLAYER *pPlayer = GetPlayer();
	//g_posBase = pPlayer->pos;

	{
		for (int nCntParticle = 0; nCntParticle < MAX_PARTICLE; nCntParticle++)//nCntParticle = 今のパーティクル
		{
			if (g_aParticle[nCntParticle].use)
			{// 使用中
				if (g_aParticle[nCntParticle].pos.x < -130 || g_aParticle[nCntParticle].pos.x > 130)//無理やり寿命調整
				{
					g_aParticle[nCntParticle].use = FALSE;
					ReleaseShadow(g_aParticle[nCntParticle].nIdxShadow);
					g_aParticle[nCntParticle].nIdxShadow = -1;
				}
				else
				{
					g_aParticle[nCntParticle].move.x = windpower;

					g_aParticle[nCntParticle].pos.x += g_aParticle[nCntParticle].move.x;//moveをposに加算=移動
					g_aParticle[nCntParticle].pos.z += g_aParticle[nCntParticle].move.z;

				}

#ifdef DISP_SHADOW
				if (g_aParticle[nCntParticle].nIdxShadow != -1)//そのパーティクルは影を使っているか
				{// 影使用中
					float colA;

					// 影の位置設定
					SetPositionShadow(g_aParticle[nCntParticle].nIdxShadow, XMFLOAT3(g_aParticle[nCntParticle].pos.x, 0.1f, g_aParticle[nCntParticle].pos.z));

					// 影の色の設定
					colA = g_aParticle[nCntParticle].material.Diffuse.w;
					SetColorShadow(g_aParticle[nCntParticle].nIdxShadow, XMFLOAT4(0.5f, 0.5f, 0.5f, colA));
				}
#endif

				g_aParticle[nCntParticle].nLife--;//フレーム毎に寿命減少

				if (windnot == 0)//無風なら寿命があああ！
				{
					g_aParticle[nCntParticle].nLife -= 10;
				}

				if (g_aParticle[nCntParticle].nLife <= 0)
				{
					g_aParticle[nCntParticle].use = FALSE;
					ReleaseShadow(g_aParticle[nCntParticle].nIdxShadow);
					g_aParticle[nCntParticle].nIdxShadow = -1;
				}
				else
				{
					if (g_aParticle[nCntParticle].nLife <= 80)//RGBカラー
					{
						g_aParticle[nCntParticle].material.Diffuse.x = 0.8f - (float)(80 - g_aParticle[nCntParticle].nLife) / 80 * 0.8f;//R
						g_aParticle[nCntParticle].material.Diffuse.y = 0.7f - (float)(80 - g_aParticle[nCntParticle].nLife) / 80 * 0.7f;//G
						g_aParticle[nCntParticle].material.Diffuse.z = 0.2f - (float)(80 - g_aParticle[nCntParticle].nLife) / 80 * 0.2f;//B
					}

					if (g_aParticle[nCntParticle].nLife <= 20)//ちょっとずつ透明に
					{
						// α値設定
						g_aParticle[nCntParticle].material.Diffuse.w -= 0.05f;
						if (g_aParticle[nCntParticle].material.Diffuse.w < 0.0f)
						{
							g_aParticle[nCntParticle].material.Diffuse.w = 0.0f;
						}
					}
				}
			}
		}

		counttime++;

		// パーティクル発生
		if (counttime >= 8)//発生時間調整
		{
			{//初期移動量と発射向きと寿命を決める
				float windforce = GetWindForce();//風力所得

				XMFLOAT3 pos;
				XMFLOAT3 move;
				float fAngle, fLength;
				int nLife;
				float fSize;

				int winddirection;//風の向き
				float windangle;//風の向き
				//int windlife;//風力に応じた寿命の変化

				windpower = windforce / 8;//風力を調整

				if (windpower > 0)//風力に合わせたパーティクル進行方向
				{
					winddirection = 1;

				}
				else if (windpower < 0)
				{
					winddirection = 2;

				}
				else
				{
					winddirection = 0;

				}


				switch (winddirection)
				{
				case 0://無
					windpower = 0.0f;
					windangle = 0.0f;
					g_posBase.y = -120;//x初期位置向きに合わせた
					windnot = 0;

					break;

				case 1://右へ
					//windpower *= 1.0f;
					windangle = XM_PI / 2;
					windnot = 1;
					g_posBase.x = -120;//x初期位置向きに合わせた

					break;

				case 2://左へ
					//windpower *= -1.0f;
					windangle = -XM_PI / 2;
					windnot = 1;
					g_posBase.x = 120;//x初期位置向きに合わせた

					break;
				}

				pos = g_posBase;//初期位置をベースに合わせる

				fAngle = windangle;//発生向き
				fLength = g_fWidthBase;
				move.x = windnot * (windpower + sinf(fAngle) * fLength);//xの初速度
				move.y = 0;//g_fHeightBaseで大きく変化 yの初速度
				move.z = 0;//zの初速度

				nLife = windnot * (rand() % 100 + 800);//寿命

				fSize = windnot * (float)(rand() % 150 + 2);//yの大きさ

				pos.y = fSize / 2;//大きさに合わせたy位置範囲
				//pos.z = windnot * (rand() % 150 + pos.z);	//z位置範囲


				// ビルボードの設定
				SetParticle(pos, move, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), fSize, fSize, nLife);
			}
			counttime = 0;
		}
	}
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawParticle(void)
{
	//int randamtex;//テクスチャ張替

	XMMATRIX mtxScl, mtxTranslate, mtxWorld, mtxView;
	CAMERA* cam = GetCamera();

	// ライティングを無効に
	SetLightEnable(FALSE);

	// 加算合成に設定
	SetBlendState(BLEND_MODE_ADD);

	// Z比較無し
	SetDepthEnable(FALSE);

	// フォグ無効
	SetFogEnable(FALSE);

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//randamtex = rand() % 11;
	//if (randamtex <= 4)
	//{
	//	randamtex = 1;
	//}
	//else if (randamtex >= 6)
	//{
	//	randamtex = 2;
	//}
	//else
	//{
	//	randamtex
	//}

	// テクスチャ設定
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);

	for (int nCntParticle = 0; nCntParticle < MAX_PARTICLE; nCntParticle++)
	{
		if (g_aParticle[nCntParticle].use)
		{
			// ワールドマトリックスの初期化
			mtxWorld = XMMatrixIdentity();

			// ビューマトリックスを取得
			mtxView = XMLoadFloat4x4(&cam->mtxView);

			//mtxWorld = XMMatrixInverse(nullptr, mtxView);
			//mtxWorld.r[3].m128_f32[0] = 0.0f;
			//mtxWorld.r[3].m128_f32[1] = 0.0f;
			//mtxWorld.r[3].m128_f32[2] = 0.0f;

			// 処理が速いしお勧め
			mtxWorld.r[0].m128_f32[0] = mtxView.r[0].m128_f32[0];
			mtxWorld.r[0].m128_f32[1] = mtxView.r[1].m128_f32[0];
			mtxWorld.r[0].m128_f32[2] = mtxView.r[2].m128_f32[0];

			mtxWorld.r[1].m128_f32[0] = mtxView.r[0].m128_f32[1];
			mtxWorld.r[1].m128_f32[1] = mtxView.r[1].m128_f32[1];
			mtxWorld.r[1].m128_f32[2] = mtxView.r[2].m128_f32[1];

			mtxWorld.r[2].m128_f32[0] = mtxView.r[0].m128_f32[2];
			mtxWorld.r[2].m128_f32[1] = mtxView.r[1].m128_f32[2];
			mtxWorld.r[2].m128_f32[2] = mtxView.r[2].m128_f32[2];

			// スケールを反映
			mtxScl = XMMatrixScaling(g_aParticle[nCntParticle].scale.x, g_aParticle[nCntParticle].scale.y, g_aParticle[nCntParticle].scale.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// 移動を反映
			mtxTranslate = XMMatrixTranslation(g_aParticle[nCntParticle].pos.x, g_aParticle[nCntParticle].pos.y, g_aParticle[nCntParticle].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			// ワールドマトリックスの設定
			SetWorldMatrix(&mtxWorld);

			// マテリアル設定
			SetMaterial(g_aParticle[nCntParticle].material);

			// ポリゴンの描画
			GetDeviceContext()->Draw(4, 0);
		}
	}

	// ライティングを有効に
	SetLightEnable(TRUE);

	// 通常ブレンドに戻す
	SetBlendState(BLEND_MODE_ALPHABLEND);

	// Z比較有効
	SetDepthEnable(TRUE);

	// フォグ有効
	SetFogEnable(GetFogEnable());
	
}

//=============================================================================
// 頂点情報の作成
//=============================================================================
HRESULT MakeVertexParticle(void)
{
	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	{//頂点バッファの中身を埋める
		D3D11_MAPPED_SUBRESOURCE msr;
		GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

		VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

		// 頂点座標の設定
		vertex[0].Position = XMFLOAT3(-PARTICLE_SIZE_X / 2, PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[1].Position = XMFLOAT3(PARTICLE_SIZE_X / 2, PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[2].Position = XMFLOAT3(-PARTICLE_SIZE_X / 2, -PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[3].Position = XMFLOAT3(PARTICLE_SIZE_X / 2, -PARTICLE_SIZE_Y / 2, 0.0f);

		// 法線の設定
		vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
		vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
		vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
		vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

		// 反射光の設定
		vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		// テクスチャ座標の設定
		vertex[0].TexCoord = { 0.0f, 0.0f };
		vertex[1].TexCoord = { 1.0f, 0.0f };
		vertex[2].TexCoord = { 0.0f, 1.0f };
		vertex[3].TexCoord = { 1.0f, 1.0f };

		GetDeviceContext()->Unmap(g_VertexBuffer, 0);
	}

	return S_OK;
}

//=============================================================================
// マテリアルカラーの設定
//=============================================================================
void SetColorParticle(int nIdxParticle, XMFLOAT4 col)
{
	g_aParticle[nIdxParticle].material.Diffuse = col;
}

//=============================================================================
// パーティクルの発生処理
//=============================================================================
int SetParticle(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT4 col, float fSizeX, float fSizeY, int nLife)
{
		int nIdxParticle = -1;

	if (windnot == 1)//風力が0でないなら生成
	{
		for (int nCntParticle = 0; nCntParticle < MAX_PARTICLE; nCntParticle++)
		{
			if (!g_aParticle[nCntParticle].use)
			{
				g_aParticle[nCntParticle].pos = pos;
				g_aParticle[nCntParticle].rot = { 0.0f, 0.0f, 0.0f };
				g_aParticle[nCntParticle].scale = { 0.7f, 0.7f, 0.7f };
				g_aParticle[nCntParticle].move = move;
				g_aParticle[nCntParticle].material.Diffuse = col;
				g_aParticle[nCntParticle].fSizeX = fSizeX;
				g_aParticle[nCntParticle].fSizeY = fSizeY;
				g_aParticle[nCntParticle].nLife = nLife;
				g_aParticle[nCntParticle].use = TRUE;

				nIdxParticle = nCntParticle;

#ifdef DISP_SHADOW
				// 影の設定
				g_aParticle[nCntParticle].nIdxShadow = CreateShadow(XMFLOAT3(pos.x, 0.1f, pos.z), 0.8f, 0.8f);		// 影の設定
				if (g_aParticle[nCntParticle].nIdxShadow != -1)
				{
					SetColorShadow(g_aParticle[nCntParticle].nIdxShadow, XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f));
				}
#endif

				break;
			}
		}
	}
	return nIdxParticle;
}
