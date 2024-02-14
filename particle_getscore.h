//=============================================================================
//
// パーティクル(スコア加算演出)処理 [particle_getscore.h]
// Author : 
//
//=============================================================================
#pragma once


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitParticleGetScore(void);
void UninitParticleGetScore(void);
void UpdateParticleGetScore(void);
void DrawParticleGetScore(void);

int SetParticleGetScore(void);
void SetColorParticleGetScore(int nIdxParticle, XMFLOAT4 col);

