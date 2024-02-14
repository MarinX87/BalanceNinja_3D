//=============================================================================
//
// �^�C�g����ʏ��� [title.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "fade.h"
#include "sound.h"
#include "sprite.h"
#include "title.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// �w�i�T�C�Y
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	// 
#define TEXTURE_MAX					(6)				// �e�N�X�`���̐�

#define TEXTURE_WIDTH_LOGO			(480/2)			// ���S�T�C�Y
#define TEXTURE_HEIGHT_LOGO			(80/2)			// 

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// ���_���
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����


static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/BG_Title.png",
	"data/TEXTURE/title_logo.png",
	"data/TEXTURE/anime1.png",
	"data/TEXTURE/anime2.png",
	"data/TEXTURE/effect000.jpg",
	"data/TEXTURE/sunshine.png",
};


static BOOL						g_Use;						// TRUE:�g���Ă���  FALSE:���g�p
static float					g_w, g_h;					// ���ƍ���
static XMFLOAT3					g_Pos;						// �|���S���̍��W
static int						g_TexNo;					// �e�N�X�`���ԍ�

float	alpha;
BOOL	flag_alpha;
static SUN g_Sun;
static BOOL						g_Load = FALSE;
static int                      index = first;
DWORD lastSwitchTime;
float switchInterval = 0.8f;

//=============================================================================
// ����������
//=============================================================================
HRESULT InitTitle(void)
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


	// �ϐ��̏�����
	g_Use   = TRUE;
	g_w     = TEXTURE_WIDTH;
	g_h     = TEXTURE_HEIGHT;
	g_Pos   = XMFLOAT3(g_w/2, g_h/2, 0.0f);
	g_TexNo = 0;

	alpha = 1.0f;
	flag_alpha = TRUE;


	//SUN�̏�����
	g_Sun.x = SCREEN_WIDTH - 100.0f;
	g_Sun.y = 100.0f;
	g_Sun.size = 300.0f;
	g_Sun.breathSpeed = 0.002f;
	g_Sun.maxScale = 1.1f;
	g_Sun.minScale = 0.9f;
	g_Sun.isBreathingIn = TRUE;
	g_Sun.color = XMFLOAT4(1.0f, 0.65f, 0.0f, 1.0f);
	g_Sun.minAlpha = 0.5f;
	g_Sun.maxAlpha = 1.0f;

	// BGM�Đ�
	PlaySound(SOUND_LABEL_BGM_wakizashi);

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitTitle(void)
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
void UpdateTitle(void)
{
	DWORD currentTime = timeGetTime();

	float elapsedTime = (currentTime - lastSwitchTime) * 0.001f;


	if (elapsedTime >= switchInterval)
	{
		if (index == first)
		{
			index = second;
		}
		else
		{
			index = first;
		}

		lastSwitchTime = currentTime;
	}



	//SUN
	if (g_Sun.isBreathingIn)
	{
		g_Sun.size *= (1 + g_Sun.breathSpeed);
		if (g_Sun.size >= g_Sun.maxScale * 800)
		{
			g_Sun.isBreathingIn = FALSE;
		}
	}
	else
	{
		g_Sun.size *= (1 - g_Sun.breathSpeed);
		if (g_Sun.size <= g_Sun.minScale * 800)
		{
			g_Sun.isBreathingIn = TRUE;
		}
	}

	if (g_Sun.isBreathingIn)
	{
		g_Sun.color.x = min(1.0f, g_Sun.color.x + 0.01f);
		g_Sun.color.y = min(0.65f, g_Sun.color.y + 0.005f);
		g_Sun.color.w = min(g_Sun.maxAlpha, g_Sun.color.w + 0.01f);
	}
	else
	{
		g_Sun.color.x = max(0.9f, g_Sun.color.x - 0.01f);
		g_Sun.color.y = max(0.55f, g_Sun.color.y - 0.005f);
		g_Sun.color.w = max(g_Sun.minAlpha, g_Sun.color.w - 0.01f);
	}



	if (GetKeyboardTrigger(DIK_RETURN))
	{// Enter��������A�X�e�[�W��؂�ւ���
		SetFade(FADE_OUT, MODE_TUTORIAL);
	}
	// �Q�[���p�b�h�œ��͏���
	else if (IsButtonTriggered(0, BUTTON_START))
	{
		SetFade(FADE_OUT, MODE_TUTORIAL);
	}
	else if (IsButtonTriggered(0, BUTTON_B))
	{
		SetFade(FADE_OUT, MODE_TUTORIAL);
	}

	//logo
	if (flag_alpha == TRUE)
	{
		alpha -= 0.02f;
		if (alpha <= 0.0f)
		{
			alpha = 0.0f;
			flag_alpha = FALSE;
		}
	}
	else
	{
		alpha += 0.02f;
		if (alpha >= 1.0f)
		{
			alpha = 1.0f;
			flag_alpha = TRUE;
		}
	}



}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawTitle(void)
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



	// �^�C�g���̔w�i��`��
	{
		// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSprite(g_VertexBuffer, g_Pos.x, g_Pos.y, g_w, g_h, 0.0f, 0.0f, 1.0f, 1.0f);

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}



	// �^�C�g���̃��S��`��
	{
		// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSpriteColor(g_VertexBuffer, TEXTURE_WIDTH_LOGO - 30.0f, SCREEN_HEIGHT - TEXTURE_HEIGHT_LOGO - 20.0f, TEXTURE_WIDTH_LOGO, TEXTURE_HEIGHT_LOGO, 0.0f, 0.0f, 1.0f, 1.0f,
			XMFLOAT4(1.0f, 1.0f, 1.0f, alpha));

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}


	// anime1
	if (index == first)
	{
		// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[2]);

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSpriteColor(g_VertexBuffer, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 100.0f, 403.0f * 1.3f, 467.0f * 1.3f, 0.0f, 0.0f, 1.0f, 1.0f,
			XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}


	// anime1
	if (index == second)
	{
		// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[3]);

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSpriteColor(g_VertexBuffer, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 100.0f, 403.0f * 1.3f, 467.0f * 1.3f, 0.0f, 0.0f, 1.0f, 1.0f,
			XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}




	// anime2
	if (index == first)
	{
		// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[2]);

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSpriteColor(g_VertexBuffer, SCREEN_WIDTH / 2 - 300.0f, SCREEN_HEIGHT / 2 + 100.0f, 403.0f * 1.1f, 467.0f * 1.1f, 0.0f, 0.0f, 1.0f, 1.0f,
			XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}


	// anime2
	if (index == second)
	{
		// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[3]);

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSpriteColor(g_VertexBuffer, SCREEN_WIDTH / 2 - 300.0f, SCREEN_HEIGHT / 2 + 100.0f, 403.0f * 1.1f, 467.0f * 1.1f, 0.0f, 0.0f, 1.0f, 1.0f,
			XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}



	// anime3
	if (index == first)
	{
		// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[2]);

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSpriteColor(g_VertexBuffer, SCREEN_WIDTH / 2 + 300.0f, SCREEN_HEIGHT / 2 + 100.0f, 403.0f * 1.1f, 467.0f * 1.1f, 0.0f, 0.0f, 1.0f, 1.0f,
			XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}


	// anime3
	if (index == second)
	{
		// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[3]);

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSpriteColor(g_VertexBuffer, SCREEN_WIDTH / 2 + 300.0f, SCREEN_HEIGHT / 2 + 100.0f, 403.0f * 1.1f, 467.0f * 1.1f, 0.0f, 0.0f, 1.0f, 1.0f,
			XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}

	//Sun
	{
		SetBlendState(BLEND_MODE_ADD);		// ���Z����

		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[4]);

		SetSpriteLTColor(g_VertexBuffer,
			g_Sun.x - g_Sun.size / 2,
			g_Sun.y - g_Sun.size / 2,
			g_Sun.size, g_Sun.size,
			1.0f, 1.0f, 1.0f, 1.0f,
			XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

		GetDeviceContext()->Draw(4, 0);
		SetBlendState(BLEND_MODE_ALPHABLEND);	// ���������������ɖ߂�


		//SUNSHINE
		SetBlendState(BLEND_MODE_ADD);

		float g_Sunshine_x = SCREEN_WIDTH;
		float g_Sunshine_y = 0.0f;
		float g_Sunshine_w = 5000.0f;
		float g_Sunshine_h = 3000.0f;
		XMFLOAT4 g_Sunshine_color;
		g_Sunshine_color = g_Sun.color;
		g_Sunshine_color.w -= 0.35f;


		// sunshine��`��
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[5]);
		SetSpriteColor(g_VertexBuffer, g_Sunshine_x, g_Sunshine_y, g_Sunshine_w, g_Sunshine_h, 0, 0, 1, 1, g_Sunshine_color);
		GetDeviceContext()->Draw(4, 0);

		SetBlendState(BLEND_MODE_ALPHABLEND);	// ���������������ɖ߂�
	}




}





