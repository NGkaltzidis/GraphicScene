#pragma once
#include "Model.h"
#include "Defines.h"
class CLights
{
public:
	// Constructor:
	CLights();

	~CLights();
	D3DXMATRIXA16 CalculateLightViewMatrix();
	D3DXMATRIXA16 CalculateLightProjMatrix();
	bool Load(std::string xFile, ID3D10EffectTechnique* technique);
	void SetPosition(D3DXVECTOR3 pos);
	void SetScale(float scale);
	void SetFacePoint(D3DXVECTOR3 point);
	void Update();
	void Render(ID3D10EffectTechnique* technique);


	// Getters
	float* GetWorldMatrix() {
		return (float*) Light->GetWorldMatrix();
	}

	D3DXVECTOR3 GetPosition() {
		return Light->GetPosition();
	}

	D3DXVECTOR3 GetFacing() {
		return Light->GetFacing();
	}

private:
	CModel* Light;
	float SpotlightConeAngle;
};

