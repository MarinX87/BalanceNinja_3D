//=============================================================================
//
// �^�C������ [timer.cpp]
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
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(32)	// �L�����T�C�Y
#define TEXTURE_HEIGHT				(64)	// 
#define TEXTURE_MAX					(2)		// �e�N�X�`���̐�


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// ���_���
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/number16x32.png",
	"data/TEXTURE/colon.png",
};


static BOOL						g_Use;						// TRUE:�g���Ă���  FALSE:���g�p
static float					g_w, g_h;					// ���ƍ���
static XMFLOAT3					g_PosMin;					// �|���S���̍��W
static XMFLOAT3					g_PosSec;					// �|���S���̍��W
static XMFLOAT3					g_PosColon;			// �|���S���̍��W
static int						g_TexNo;					// �e�N�X�`���ԍ�

static int						g_Frame;					// �t���[��
static int						g_Time;						// �^�C��

static BOOL						g_Load = FALSE;


//=============================================================================
// ����������
//=============================================================================
HRESULT InitTime(void)
{
	ID3D11Device *pDevice = GetDevice();

	//�e�N�X�`������
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


	// ���_�o�b�t�@����
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
// �I������
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
// �X�V����
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
// �`�揈��
//=============================================================================
void DrawTime(void)
{
	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// �}�g���N�X�ݒ�
	SetWorldViewProjection2D();

	// �v���~�e�B�u�g�|���W�ݒ�
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// �}�e���A���ݒ�
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	// �e�N�X�`���ݒ�
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);

	// ��������������
	int number = g_Time / 60;
	for (int i = 0; i < TIMER_DIGIT; i++)
	{
		// ����\�����錅�̐���
		float x = (float)(number % 10);

		// �X�R�A�̈ʒu��e�N�X�`���[���W�𔽉f
		float px = g_PosMin.x - g_w*i;	// �X�R�A�̕\���ʒuX
		float py = g_PosMin.y;			// �X�R�A�̕\���ʒuY
		float pw = g_w;				// �X�R�A�̕\����
		float ph = g_h;				// �X�R�A�̕\������

		float tw = 1.0f / 10;		// �e�N�X�`���̕�
		float th = 1.0f / 1;		// �e�N�X�`���̍���
		float tx = x * tw;			// �e�N�X�`���̍���X���W
		float ty = 0.0f;			// �e�N�X�`���̍���Y���W

		XMFLOAT4 color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		if (g_Time != 0 && ((((g_Time) % 15 == 0) || ((g_Time + 1) % 15 == 0) || ((g_Time + 2) % 15 == 0))))
		{
			color = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
		}

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
			color);

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);

		// ���̌���
		number /= 10;
	}

	// ��������������
	number = g_Time % 60;
	for (int i = 0; i < TIMER_DIGIT; i++)
	{
		// ����\�����錅�̐���
		float x = (float)(number % 10);

		// �X�R�A�̈ʒu��e�N�X�`���[���W�𔽉f
		float px = g_PosSec.x - g_w * i;	// �X�R�A�̕\���ʒuX
		float py = g_PosSec.y;			// �X�R�A�̕\���ʒuY
		float pw = g_w;				// �X�R�A�̕\����
		float ph = g_h;				// �X�R�A�̕\������

		float tw = 1.0f / 10;		// �e�N�X�`���̕�
		float th = 1.0f / 1;		// �e�N�X�`���̍���
		float tx = x * tw;			// �e�N�X�`���̍���X���W
		float ty = 0.0f;			// �e�N�X�`���̍���Y���W

		XMFLOAT4 color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		if (g_Time != 0 && ( ( ((g_Time) % 15 == 0) || ((g_Time + 1) % 15 == 0) ||  ((g_Time + 2) % 15 == 0)) ) )
		{
			color = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
		}

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
			color);

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);

		// ���̌���
		number /= 10;
	}

	// �e�N�X�`���ݒ�
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

	// �X�R�A�̈ʒu��e�N�X�`���[���W�𔽉f
	float px = g_PosColon.x;	// �X�R�A�̕\���ʒuX
	float py = g_PosColon.y;	// �X�R�A�̕\���ʒuY
	float pw = g_w;				// �X�R�A�̕\����
	float ph = g_h;				// �X�R�A�̕\������

	float tw = 1.0f;		// �e�N�X�`���̕�
	float th = 1.0f;		// �e�N�X�`���̍���
	float tx = 0.0f;			// �e�N�X�`���̍���X���W
	float ty = 0.0f;			// �e�N�X�`���̍���Y���W

	XMFLOAT4 color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	if (g_Time != 0 && ((((g_Time) % 15 == 0) || ((g_Time + 1) % 15 == 0) || ((g_Time + 2) % 15 == 0))))
	{
		color = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	}

	// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
	SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
		color);

	// �|���S���`��
	GetDeviceContext()->Draw(4, 0);
}

int GetTime(void)
{
	return g_Time;
}

