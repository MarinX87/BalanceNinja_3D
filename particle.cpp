//=============================================================================
//
// �p�[�e�B�N������ [particle.cpp]
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
// �}�N����`
//*****************************************************************************
#define TEXTURE_MAX			(4)			// �e�N�X�`���̐�

#define	PARTICLE_SIZE_X		(40.0f)		// ���_�T�C�Y
#define	PARTICLE_SIZE_Y		(40.0f)		// ���_�T�C�Y
#define	VALUE_MOVE_PARTICLE	(5.0f)		// �ړ����x

#define	MAX_PARTICLE		(512)		// �p�[�e�B�N���ő吔

#define	DISP_SHADOW						// �e�̕\��
//#undef DISP_SHADOW

//*****************************************************************************
// �\���̒�`
//*****************************************************************************
typedef struct
{
	XMFLOAT3		pos;			// �ʒu
	XMFLOAT3		rot;			// ��]
	XMFLOAT3		scale;			// �X�P�[��
	XMFLOAT3		move;			// �ړ���
	MATERIAL		material;		// �}�e���A��
	float			fSizeX;			// ��
	float			fSizeY;			// ����
	int				nIdxShadow;		// �eID
	int				nLife;			// ����
	BOOL			use;			// �g�p���Ă��邩�ǂ���

} PARTICLE;

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT MakeVertexParticle(void);

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;		// ���_�o�b�t�@

static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����
static int							g_TexNo;					// �e�N�X�`���ԍ�

static PARTICLE					g_aParticle[MAX_PARTICLE];		// �p�[�e�B�N�����[�N
static XMFLOAT3					g_posBase;						// �r���{�[�h�����ʒu
static float					g_fWidthBase = 0.1f;			// ��̕�
static float					g_fHeightBase = 10.0f;			// ��̍���
static float					g_roty = 0.0f;					// �ړ�����
static float					g_spd = 0.0f;					// �ړ��X�s�[�h

static char* g_TextureName[] =
{
	"data/TEXTURE/effect000.jpg",
	"data/TEXTURE/wind0.png",
	"data/TEXTURE/wind1.png",
	"data/TEXTURE/wind2.png",

};

static BOOL							g_Load = FALSE;

int windnot;//�������ǂ���
int counttime;//�p�[�e�B�N�������҂�����
float windpower;//����
//=============================================================================
// ����������
//=============================================================================
HRESULT InitParticle(void)
{
	counttime = 0;

	// ���_���̍쐬
	MakeVertexParticle();

	// �e�N�X�`������
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

	// �p�[�e�B�N�����[�N�̏�����
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
// �I������
//=============================================================================
void UninitParticle(void)
{
	if (g_Load == FALSE) return;

	//�e�N�X�`���̉��
	for (int nCntTex = 0; nCntTex < TEXTURE_MAX; nCntTex++)
	{
		if (g_Texture[nCntTex] != NULL)
		{
			g_Texture[nCntTex]->Release();
			g_Texture[nCntTex] = NULL;
		}
	}

	// ���_�o�b�t�@�̉��
	if (g_VertexBuffer != NULL)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	g_Load = FALSE;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateParticle(void)
{
	//PLAYER *pPlayer = GetPlayer();
	//g_posBase = pPlayer->pos;

	{
		for (int nCntParticle = 0; nCntParticle < MAX_PARTICLE; nCntParticle++)//nCntParticle = ���̃p�[�e�B�N��
		{
			if (g_aParticle[nCntParticle].use)
			{// �g�p��
				if (g_aParticle[nCntParticle].pos.x < -130 || g_aParticle[nCntParticle].pos.x > 130)//��������������
				{
					g_aParticle[nCntParticle].use = FALSE;
					ReleaseShadow(g_aParticle[nCntParticle].nIdxShadow);
					g_aParticle[nCntParticle].nIdxShadow = -1;
				}
				else
				{
					g_aParticle[nCntParticle].move.x = windpower;

					g_aParticle[nCntParticle].pos.x += g_aParticle[nCntParticle].move.x;//move��pos�ɉ��Z=�ړ�
					g_aParticle[nCntParticle].pos.z += g_aParticle[nCntParticle].move.z;

				}

#ifdef DISP_SHADOW
				if (g_aParticle[nCntParticle].nIdxShadow != -1)//���̃p�[�e�B�N���͉e���g���Ă��邩
				{// �e�g�p��
					float colA;

					// �e�̈ʒu�ݒ�
					SetPositionShadow(g_aParticle[nCntParticle].nIdxShadow, XMFLOAT3(g_aParticle[nCntParticle].pos.x, 0.1f, g_aParticle[nCntParticle].pos.z));

					// �e�̐F�̐ݒ�
					colA = g_aParticle[nCntParticle].material.Diffuse.w;
					SetColorShadow(g_aParticle[nCntParticle].nIdxShadow, XMFLOAT4(0.5f, 0.5f, 0.5f, colA));
				}
#endif

				g_aParticle[nCntParticle].nLife--;//�t���[�����Ɏ�������

				if (windnot == 0)//�����Ȃ�������������I
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
					if (g_aParticle[nCntParticle].nLife <= 80)//RGB�J���[
					{
						g_aParticle[nCntParticle].material.Diffuse.x = 0.8f - (float)(80 - g_aParticle[nCntParticle].nLife) / 80 * 0.8f;//R
						g_aParticle[nCntParticle].material.Diffuse.y = 0.7f - (float)(80 - g_aParticle[nCntParticle].nLife) / 80 * 0.7f;//G
						g_aParticle[nCntParticle].material.Diffuse.z = 0.2f - (float)(80 - g_aParticle[nCntParticle].nLife) / 80 * 0.2f;//B
					}

					if (g_aParticle[nCntParticle].nLife <= 20)//������Ƃ�������
					{
						// ���l�ݒ�
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

		// �p�[�e�B�N������
		if (counttime >= 8)//�������Ԓ���
		{
			{//�����ړ��ʂƔ��ˌ����Ǝ��������߂�
				float windforce = GetWindForce();//���͏���

				XMFLOAT3 pos;
				XMFLOAT3 move;
				float fAngle, fLength;
				int nLife;
				float fSize;

				int winddirection;//���̌���
				float windangle;//���̌���
				//int windlife;//���͂ɉ����������̕ω�

				windpower = windforce / 8;//���͂𒲐�

				if (windpower > 0)//���͂ɍ��킹���p�[�e�B�N���i�s����
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
				case 0://��
					windpower = 0.0f;
					windangle = 0.0f;
					g_posBase.y = -120;//x�����ʒu�����ɍ��킹��
					windnot = 0;

					break;

				case 1://�E��
					//windpower *= 1.0f;
					windangle = XM_PI / 2;
					windnot = 1;
					g_posBase.x = -120;//x�����ʒu�����ɍ��킹��

					break;

				case 2://����
					//windpower *= -1.0f;
					windangle = -XM_PI / 2;
					windnot = 1;
					g_posBase.x = 120;//x�����ʒu�����ɍ��킹��

					break;
				}

				pos = g_posBase;//�����ʒu���x�[�X�ɍ��킹��

				fAngle = windangle;//��������
				fLength = g_fWidthBase;
				move.x = windnot * (windpower + sinf(fAngle) * fLength);//x�̏����x
				move.y = 0;//g_fHeightBase�ő傫���ω� y�̏����x
				move.z = 0;//z�̏����x

				nLife = windnot * (rand() % 100 + 800);//����

				fSize = windnot * (float)(rand() % 150 + 2);//y�̑傫��

				pos.y = fSize / 2;//�傫���ɍ��킹��y�ʒu�͈�
				//pos.z = windnot * (rand() % 150 + pos.z);	//z�ʒu�͈�


				// �r���{�[�h�̐ݒ�
				SetParticle(pos, move, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), fSize, fSize, nLife);
			}
			counttime = 0;
		}
	}
}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawParticle(void)
{
	//int randamtex;//�e�N�X�`������

	XMMATRIX mtxScl, mtxTranslate, mtxWorld, mtxView;
	CAMERA* cam = GetCamera();

	// ���C�e�B���O�𖳌���
	SetLightEnable(FALSE);

	// ���Z�����ɐݒ�
	SetBlendState(BLEND_MODE_ADD);

	// Z��r����
	SetDepthEnable(FALSE);

	// �t�H�O����
	SetFogEnable(FALSE);

	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// �v���~�e�B�u�g�|���W�ݒ�
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

	// �e�N�X�`���ݒ�
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);

	for (int nCntParticle = 0; nCntParticle < MAX_PARTICLE; nCntParticle++)
	{
		if (g_aParticle[nCntParticle].use)
		{
			// ���[���h�}�g���b�N�X�̏�����
			mtxWorld = XMMatrixIdentity();

			// �r���[�}�g���b�N�X���擾
			mtxView = XMLoadFloat4x4(&cam->mtxView);

			//mtxWorld = XMMatrixInverse(nullptr, mtxView);
			//mtxWorld.r[3].m128_f32[0] = 0.0f;
			//mtxWorld.r[3].m128_f32[1] = 0.0f;
			//mtxWorld.r[3].m128_f32[2] = 0.0f;

			// ������������������
			mtxWorld.r[0].m128_f32[0] = mtxView.r[0].m128_f32[0];
			mtxWorld.r[0].m128_f32[1] = mtxView.r[1].m128_f32[0];
			mtxWorld.r[0].m128_f32[2] = mtxView.r[2].m128_f32[0];

			mtxWorld.r[1].m128_f32[0] = mtxView.r[0].m128_f32[1];
			mtxWorld.r[1].m128_f32[1] = mtxView.r[1].m128_f32[1];
			mtxWorld.r[1].m128_f32[2] = mtxView.r[2].m128_f32[1];

			mtxWorld.r[2].m128_f32[0] = mtxView.r[0].m128_f32[2];
			mtxWorld.r[2].m128_f32[1] = mtxView.r[1].m128_f32[2];
			mtxWorld.r[2].m128_f32[2] = mtxView.r[2].m128_f32[2];

			// �X�P�[���𔽉f
			mtxScl = XMMatrixScaling(g_aParticle[nCntParticle].scale.x, g_aParticle[nCntParticle].scale.y, g_aParticle[nCntParticle].scale.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// �ړ��𔽉f
			mtxTranslate = XMMatrixTranslation(g_aParticle[nCntParticle].pos.x, g_aParticle[nCntParticle].pos.y, g_aParticle[nCntParticle].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			// ���[���h�}�g���b�N�X�̐ݒ�
			SetWorldMatrix(&mtxWorld);

			// �}�e���A���ݒ�
			SetMaterial(g_aParticle[nCntParticle].material);

			// �|���S���̕`��
			GetDeviceContext()->Draw(4, 0);
		}
	}

	// ���C�e�B���O��L����
	SetLightEnable(TRUE);

	// �ʏ�u�����h�ɖ߂�
	SetBlendState(BLEND_MODE_ALPHABLEND);

	// Z��r�L��
	SetDepthEnable(TRUE);

	// �t�H�O�L��
	SetFogEnable(GetFogEnable());
	
}

//=============================================================================
// ���_���̍쐬
//=============================================================================
HRESULT MakeVertexParticle(void)
{
	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	{//���_�o�b�t�@�̒��g�𖄂߂�
		D3D11_MAPPED_SUBRESOURCE msr;
		GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

		VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

		// ���_���W�̐ݒ�
		vertex[0].Position = XMFLOAT3(-PARTICLE_SIZE_X / 2, PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[1].Position = XMFLOAT3(PARTICLE_SIZE_X / 2, PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[2].Position = XMFLOAT3(-PARTICLE_SIZE_X / 2, -PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[3].Position = XMFLOAT3(PARTICLE_SIZE_X / 2, -PARTICLE_SIZE_Y / 2, 0.0f);

		// �@���̐ݒ�
		vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
		vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
		vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
		vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

		// ���ˌ��̐ݒ�
		vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		// �e�N�X�`�����W�̐ݒ�
		vertex[0].TexCoord = { 0.0f, 0.0f };
		vertex[1].TexCoord = { 1.0f, 0.0f };
		vertex[2].TexCoord = { 0.0f, 1.0f };
		vertex[3].TexCoord = { 1.0f, 1.0f };

		GetDeviceContext()->Unmap(g_VertexBuffer, 0);
	}

	return S_OK;
}

//=============================================================================
// �}�e���A���J���[�̐ݒ�
//=============================================================================
void SetColorParticle(int nIdxParticle, XMFLOAT4 col)
{
	g_aParticle[nIdxParticle].material.Diffuse = col;
}

//=============================================================================
// �p�[�e�B�N���̔�������
//=============================================================================
int SetParticle(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT4 col, float fSizeX, float fSizeY, int nLife)
{
		int nIdxParticle = -1;

	if (windnot == 1)//���͂�0�łȂ��Ȃ琶��
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
				// �e�̐ݒ�
				g_aParticle[nCntParticle].nIdxShadow = CreateShadow(XMFLOAT3(pos.x, 0.1f, pos.z), 0.8f, 0.8f);		// �e�̐ݒ�
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
