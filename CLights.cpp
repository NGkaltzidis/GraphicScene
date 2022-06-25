#include "CLights.h"

CLights::CLights() {
	Light = new CModel;
	SpotlightConeAngle = 90.0f;
}


CLights::~CLights()
{
	delete Light;
}

// Get "camera-like" view matrix for a spot light
D3DXMATRIXA16 CLights::CalculateLightViewMatrix()
{
	D3DXMATRIXA16 viewMatrix;

	// Get the world matrix of the light model and invert it to get the view matrix (that is more-or-less the definition of a view matrix)
	// We don't always have a physical model for a light, in which case we would need to store this data along with the light colour etc.
	D3DXMATRIXA16 worldMatrix = Light->GetWorldMatrix();
	D3DXMatrixInverse(&viewMatrix, NULL, &worldMatrix);

	return viewMatrix;
}

// Get "camera-like" projection matrix for a spot light
D3DXMATRIXA16 CLights::CalculateLightProjMatrix()
{
	D3DXMATRIXA16 projMatrix;

	// Create a projection matrix for the light. Use the spotlight cone angle as an FOV, just set default values for everything else.
	D3DXMatrixPerspectiveFovLH(&projMatrix, ToRadians(SpotlightConeAngle), 1, 0.1f, 1000.0f);

	return projMatrix;
}

bool CLights::Load(std::string xFile, ID3D10EffectTechnique* technique)
{
	if (!Light->Load(xFile, technique)) return false;
}

void CLights::SetPosition(D3DXVECTOR3 pos)
{
	Light->SetPosition(pos);
}

void CLights::SetScale(float scale)
{
	Light->SetScale(scale);
}

void CLights::SetFacePoint(D3DXVECTOR3 point)
{
	Light->FacePoint(point);
}

void CLights::Update()
{
	Light->UpdateMatrix();
}

void CLights::Render(ID3D10EffectTechnique* technique)
{
	Light->Render(technique);
}
