
//--------------------------------------------------------------------------------------
//	GraphicsAssign1.cpp
//
//	Shaders Graphics Assignment
//	Add further models using different shader techniques
//	See assignment specification for details
//--------------------------------------------------------------------------------------

//***|  INFO  |****************************************************************
// Lights:
//   The initial project shows models for a couple of point lights, but the
//   initial shaders don't actually apply any lighting. Part of the assignment
//   is to add a shader that uses this information to actually light a model.
//   Refer to lab work to determine how best to do this.
// 
// Textures:
//   The models in the initial scene have textures loaded but the initial
//   technique/shaders doesn't show them. Part of the assignment is to add 
//   techniques to show a textured model
//
// Shaders:
//   The initial shaders render plain coloured objects with no lighting:
//   - The vertex shader performs basic transformation of 3D vertices to 2D
//   - The pixel shader colours every pixel the same colour
//   A few shader variables are set up to communicate between C++ and HLSL
//   You will usually need to add further variables to add new techniques
//*****************************************************************************

#include <windows.h>
#include <d3d10.h>
#include <d3dx10.h>
#include <atlbase.h>
#include "resource.h"

#include "Defines.h" // General definitions shared by all source files
#include "Model.h"   // Model class - encapsulates working with vertex/index data and world matrix
#include "Camera.h"  // Camera class - encapsulates the camera's view and projection matrix
#include "Input.h"   // Input functions - not DirectX
#include "CLights.h"


//--------------------------------------------------------------------------------------
// Global Scene Variables
//--------------------------------------------------------------------------------------

// Models and cameras encapsulated in classes for flexibity and convenience
// The CModel class collects together geometry and world matrix, and provides functions to control the model and render it
// The CCamera class handles the view and projections matrice, and provides functions to control the camera
CModel* Cube;
CModel* Cube2;
CModel* Cube3;
CModel* Sphere;
CModel* TeaPot;
CModel* TeaPot2;
CModel* Floor;
CModel* Troll;
CModel* Troll2;
CModel* Amartin;
CModel* Mirror;
CModel* Portal;
CModel* Frame;
CCamera* Camera;
CCamera* PortalCamera;




// Textures - no texture class yet so using DirectX variables
ID3D10ShaderResourceView* CubeDiffuseMap = NULL;
ID3D10ShaderResourceView* Cube2DiffuseMap = NULL;
ID3D10ShaderResourceView* Cube3DiffuseMap = NULL;
ID3D10ShaderResourceView* SphereDiffuseMap = NULL;
ID3D10ShaderResourceView* TrollDiffuseMap = NULL;
ID3D10ShaderResourceView* Troll2DiffuseMap = NULL;
ID3D10ShaderResourceView* CellMap = NULL;
ID3D10ShaderResourceView* TeaPotDiffuseMap = NULL;
ID3D10ShaderResourceView* TeaPot2DiffuseMap = NULL;
ID3D10ShaderResourceView* AmartinDiffuseMap = NULL;
ID3D10ShaderResourceView* FloorDiffuseMap = NULL;
ID3D10ShaderResourceView* FloorNormalMap = NULL;
ID3D10ShaderResourceView* TrollNormalMap = NULL;
ID3D10ShaderResourceView* CubeNormalMap = NULL;
ID3D10ShaderResourceView* LightDiffuseMap = NULL;

// Light data - stored manually as there is no light class
//D3DXVECTOR3 Light1Colour = D3DXVECTOR3(0.8f, 0.8f, 1.0f) * 8;
//D3DXVECTOR3 Light2Colour = D3DXVECTOR3(1.0f, 0.8f, 0.2f) * 60;
//D3DXVECTOR3 Light3Colour = D3DXVECTOR3(1.0f, 0.0f, 0.5f) * 10;

// NG. Portal //
ID3D10Texture2D* PortalTexture = NULL;
ID3D10RenderTargetView* PortalRenderTarget = NULL;
ID3D10ShaderResourceView* PortalMap = NULL;

// NG. Portal2 //
ID3D10Texture2D* PortalTexture2 = NULL;
ID3D10RenderTargetView* PortalRenderTarget2 = NULL;
ID3D10ShaderResourceView* PortalMap2 = NULL;

// NG. Portal Data
// Dimensions of portal texture - controls quality of rendered scene in portal
int PortalWidth = 512;
int PortalHeight = 512;

// Also need a depth/stencil buffer for the portal
// NOTE: ***Can share the depth buffer between multiple portals of the same size***
ID3D10Texture2D* PortalDepthStencil = NULL;
ID3D10DepthStencilView* PortalDepthStencilView = NULL;

ID3D10Texture2D* PortalDepthStencil2 = NULL;
ID3D10DepthStencilView* PortalDepthStencilView2 = NULL;

// NG. SHADOWS //

// Width and height of shadow map - controls resolution/quality of shadows
int ShadowMapSize = 1040;

// The shadow map textures and the view of it as a depth buffer and shader resource (see code comments)
ID3D10Texture2D* ShadowMap1Texture = NULL;
ID3D10DepthStencilView* ShadowMap1DepthView = NULL;
ID3D10ShaderResourceView* ShadowMap1 = NULL;

ID3D10Texture2D* ShadowMap2Texture = NULL;
ID3D10DepthStencilView* ShadowMap2DepthView = NULL;
ID3D10ShaderResourceView* ShadowMap2 = NULL;

ID3D10Texture2D* ShadowMap3Texture = NULL;
ID3D10DepthStencilView* ShadowMap3DepthView = NULL;
ID3D10ShaderResourceView* ShadowMap3 = NULL;

// --------------- //

// Spotlight data - using spotlights in this lab because shadow mapping needs to treat each light as a camera, which is easy with
// spotlights as they are quite camera-like already. Using arrays of data to store lights in this lab for convinience
const int NUM_LIGHTS = 3;
D3DXVECTOR3 LightColours[NUM_LIGHTS] = { D3DXVECTOR3(0.8f, 0.8f, 1.0f) * 8, D3DXVECTOR3(1.0f, 0.8f, 0.2f) * 60, D3DXVECTOR3(1.0f, 0.0f, 0.5f) * 10 }; // Colour * Intensity
//CModel* Lights[NUM_LIGHTS];
float       SpotlightConeAngle = 90.0f; // Spot light cone angle (degrees), like the FOV (field-of-view) of the spot light


CLights* lights[NUM_LIGHTS];


// Cell shading data
D3DXVECTOR3 OutlineColour = D3DXVECTOR3(0, 0, 0); // Black outlines
float       OutlineThickness = 0.015f;

D3DXVECTOR4 BackgroundColour = D3DXVECTOR4(0.2f, 0.2f, 0.3f, 1.0f);
D3DXVECTOR3 AmbientColour = D3DXVECTOR3(0.2f, 0.2f, 0.3f);
float SpecularPower = 256.0f;

float ParallaxDepth = 0.08f; // Overall depth of bumpiness for parallax mapping
bool UseParallax = true;  // Toggle for parallax 

// Display models where the lights are. One of the lights will follow an orbit
//CModel* Light1;
//CModel* Light2;
//CModel* Light3;

const float LightOrbitRadius = 20.0f;
const float MirrorOrbitRadius = 100.0f;
const float LightOrbitSpeed = 0.5f;

// Note: There are move & rotation speed constants in Defines.h
float fWiggle;

// Timer
float timer = 10.0f;
float pulsate = 10.0f;


//--------------------------------------------------------------------------------------
// Shader Variables
//--------------------------------------------------------------------------------------
// Variables to connect C++ code to HLSL shaders

// Effects / techniques
ID3D10Effect* Effect = NULL;
ID3D10EffectTechnique* PlainColourTechnique = NULL;
ID3D10EffectTechnique* FloorTechnique = NULL;
ID3D10EffectTechnique* LightTechnique = NULL;
ID3D10EffectTechnique* DiffuseOnlyTechnique = NULL;
ID3D10EffectTechnique* AdditiveTexTintTechnique = NULL;
ID3D10EffectTechnique* AdditiveTexTintMirrorTechnique = NULL;
ID3D10EffectTechnique* WiggleTechnique = NULL;
ID3D10EffectTechnique* DiffuseSpecularTechnique = NULL;
ID3D10EffectTechnique* WiggleMirrorTechnique = NULL;
ID3D10EffectTechnique* NormalMappingTechnique = NULL;
ID3D10EffectTechnique* NormalMappingMirrorTechnique = NULL;
ID3D10EffectTechnique* ParallaxMappingTechnique = NULL;
ID3D10EffectTechnique* ParallaxMappingMirrorTechnique = NULL;
ID3D10EffectTechnique* CellShadingTechnique = NULL;
ID3D10EffectTechnique* CellShadingMirrorTechnique = NULL;
ID3D10EffectTechnique* ShadowMappingTechnique = NULL;
ID3D10EffectTechnique* ShadowMappingMirrorTechnique = NULL;
ID3D10EffectTechnique* DepthOnlyTechnique = NULL;
ID3D10EffectTechnique* BlendingTechnique = NULL;
ID3D10EffectTechnique* BlendingMirrorTechnique = NULL;
ID3D10EffectTechnique* SphereScrollTechnique = NULL;
ID3D10EffectTechnique* SphereMirrorScrollTechnique = NULL;

// NG Mirror
ID3D10EffectTechnique* MirrorClearTechnique = NULL;
ID3D10EffectTechnique* MirrorSurfaceTechnique = NULL;


// Matrices
ID3D10EffectMatrixVariable* WorldMatrixVar = NULL;
ID3D10EffectMatrixVariable* ViewMatrixVar = NULL;
ID3D10EffectMatrixVariable* ProjMatrixVar = NULL;
ID3D10EffectMatrixVariable* ViewProjMatrixVar = NULL;

// Textures
ID3D10EffectShaderResourceVariable* DiffuseMapVar = NULL;
ID3D10EffectShaderResourceVariable* NormalMapVar = NULL;
ID3D10EffectShaderResourceVariable* ParallaxMapVar = NULL;
ID3D10EffectShaderResourceVariable* CellMapVar = NULL;
ID3D10EffectShaderResourceVariable* ShadowMap1Var = NULL;
ID3D10EffectShaderResourceVariable* ShadowMap2Var = NULL;
ID3D10EffectShaderResourceVariable* ShadowMap3Var = NULL;

// Miscellaneous
ID3D10EffectVectorVariable* CameraPosVar = NULL;
ID3D10EffectVectorVariable* ModelColourVar = NULL;

// Light1 effect variables
ID3D10EffectVectorVariable* Light1PosVar = NULL;
ID3D10EffectVectorVariable* Light1FacingVar = NULL;
ID3D10EffectMatrixVariable* Light1ViewMatrixVar = NULL;
ID3D10EffectMatrixVariable* Light1ProjMatrixVar = NULL;
ID3D10EffectScalarVariable* Light1ConeAngleVar = NULL;
ID3D10EffectVectorVariable* Light1ColourVar = NULL;

ID3D10EffectVectorVariable* Light2PosVar = NULL;
ID3D10EffectVectorVariable* Light2ColourVar = NULL;
ID3D10EffectVectorVariable* Light2FacingVar = NULL;
ID3D10EffectMatrixVariable* Light2ViewMatrixVar = NULL;
ID3D10EffectMatrixVariable* Light2ProjMatrixVar = NULL;
ID3D10EffectScalarVariable* Light2ConeAngleVar = NULL;

ID3D10EffectVectorVariable* Light3PosVar = NULL;
ID3D10EffectVectorVariable* Light3ColourVar = NULL;
ID3D10EffectVectorVariable* Light3FacingVar = NULL;
ID3D10EffectMatrixVariable* Light3ViewMatrixVar = NULL;
ID3D10EffectMatrixVariable* Light3ProjMatrixVar = NULL;
ID3D10EffectScalarVariable* Light3ConeAngleVar = NULL;

ID3D10EffectVectorVariable* AmbientColourVar = NULL;
ID3D10EffectScalarVariable* SpecularPowerVar = NULL;

//Light and EffectVariables
ID3D10EffectScalarVariable* Wiggle = NULL;

// Other
ID3D10EffectVectorVariable* TintColourVar = NULL;
ID3D10EffectScalarVariable* ParallaxDepthVar = NULL;

ID3D10EffectVectorVariable* ConstantColourVar = NULL;
ID3D10EffectScalarVariable* OutlineThicknessVar = NULL;

// OTHER MIRROR
ID3D10EffectVectorVariable* ClipPlaneVar = NULL;




//--------------------------------------------------------------------------------------
// DirectX Variables
//--------------------------------------------------------------------------------------

// The main D3D interface, this pointer is used to access most D3D functions (and is shared across all cpp files through Defines.h)
ID3D10Device* g_pd3dDevice = NULL;

// Width and height of the window viewport
int g_ViewportWidth;
int g_ViewportHeight;

// Variables used to setup D3D
IDXGISwapChain* SwapChain = NULL;
ID3D10Texture2D* DepthStencil = NULL;
ID3D10DepthStencilView* DepthStencilView = NULL;
ID3D10RenderTargetView* RenderTargetView = NULL;


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
bool InitDevice(HWND hWnd)
{
	// Many DirectX functions return a "HRESULT" variable to indicate success or failure. Microsoft code often uses
	// the FAILED macro to test this variable, you'll see it throughout the code - it's fairly self explanatory.
	HRESULT hr = S_OK;


	////////////////////////////////
	// Initialise Direct3D

	// Calculate the visible area the window we are using - the "client rectangle" refered to in the first function is the 
	// size of the interior of the window, i.e. excluding the frame and title
	RECT rc;
	GetClientRect(hWnd, &rc);
	g_ViewportWidth = rc.right - rc.left;
	g_ViewportHeight = rc.bottom - rc.top;


	// Create a Direct3D device (i.e. initialise D3D), and create a swap-chain (create a back buffer to render to)
	DXGI_SWAP_CHAIN_DESC sd;         // Structure to contain all the information needed
	ZeroMemory(&sd, sizeof(sd)); // Clear the structure to 0 - common Microsoft practice, not really good style
	sd.BufferCount = 1;
	sd.BufferDesc.Width = g_ViewportWidth;             // Target window size
	sd.BufferDesc.Height = g_ViewportHeight;           // --"--
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Pixel format of target window
	sd.BufferDesc.RefreshRate.Numerator = 60;          // Refresh rate of monitor
	sd.BufferDesc.RefreshRate.Denominator = 1;         // --"--
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.OutputWindow = hWnd;                            // Target window
	sd.Windowed = TRUE;                                // Whether to render in a window (TRUE) or go fullscreen (FALSE)
	hr = D3D10CreateDeviceAndSwapChain(NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, D3D10_CREATE_DEVICE_DEBUG,
		D3D10_SDK_VERSION, &sd, &SwapChain, &g_pd3dDevice);

	/*
	hr = D3D10CreateDeviceAndSwapChain( NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, D3D10_CREATE_DEVICE_DEBUG,
	D3D10_SDK_VERSION, &sd, &SwapChain, &g_pd3dDevice );
	*/
	if (FAILED(hr)) return false;


	// Specify the render target as the back-buffer - this is an advanced topic. This code almost always occurs in the standard D3D setup
	ID3D10Texture2D* pBackBuffer;
	hr = SwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr)) return false;
	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &RenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr)) return false;


	// Create a texture (bitmap) to use for a depth buffer
	D3D10_TEXTURE2D_DESC descDepth;
	descDepth.Width = g_ViewportWidth;
	descDepth.Height = g_ViewportHeight;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D10_USAGE_DEFAULT;
	descDepth.BindFlags = D3D10_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &DepthStencil);
	if (FAILED(hr)) return false;

	// Create the depth stencil view, i.e. indicate that the texture just created is to be used as a depth buffer
	D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = g_pd3dDevice->CreateDepthStencilView(DepthStencil, &descDSV, &DepthStencilView);
	if (FAILED(hr)) return false;

	// Select the back buffer and depth buffer to use for rendering
	g_pd3dDevice->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);


	D3D10_VIEWPORT vp;
	vp.Width = g_ViewportWidth;
	vp.Height = g_ViewportHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pd3dDevice->RSSetViewports(1, &vp);

	return true;
}


// Release the memory held by all objects created
void ReleaseResources()
{
	// The D3D setup and preparation of the geometry created several objects that use up memory (e.g. textures, vertex/index buffers etc.)
	// Each object that allocates memory (or hardware resources) needs to be "released" when we exit the program
	// There is similar code in every D3D program, but the list of objects that need to be released depends on what was created
	// Test each variable to see if it exists before deletion
	if (g_pd3dDevice)     g_pd3dDevice->ClearState();

	
	delete Floor;
	delete Cube;
	delete Cube2;
	delete Cube3;
	delete Troll;
	delete Troll2;
	delete Amartin;
	delete TeaPot;
	delete TeaPot2;
	delete Camera;
	delete Mirror;
	delete Frame;
	delete PortalCamera;

	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		delete lights[i];
	}


	if (ShadowMap1)             ShadowMap1->Release();
	if (ShadowMap1DepthView)    ShadowMap1DepthView->Release();
	if (ShadowMap1Texture)      ShadowMap1Texture->Release();

	if (ShadowMap2)             ShadowMap2->Release();
	if (ShadowMap2DepthView)    ShadowMap2DepthView->Release();
	if (ShadowMap2Texture)      ShadowMap2Texture->Release();

	if (ShadowMap3)             ShadowMap3->Release();
	if (ShadowMap3DepthView)    ShadowMap3DepthView->Release();
	if (ShadowMap3Texture)      ShadowMap3Texture->Release();

	if (PortalDepthStencilView) PortalDepthStencilView->Release();
	if (PortalDepthStencil)     PortalDepthStencil->Release();
	if (PortalMap)              PortalMap->Release();

	if (PortalDepthStencilView2) PortalDepthStencilView2->Release();
	if (PortalDepthStencil2)     PortalDepthStencil2->Release();
	if (PortalMap2)              PortalMap2->Release();

	if (FloorDiffuseMap)   FloorDiffuseMap->Release();
	if (FloorNormalMap)    FloorNormalMap->Release();
	if (SphereDiffuseMap)  SphereDiffuseMap->Release();
	if (CubeDiffuseMap)    CubeDiffuseMap->Release();
	if (Cube2DiffuseMap)   Cube2DiffuseMap->Release();
	if (Cube3DiffuseMap)   Cube3DiffuseMap->Release();
	if (CubeNormalMap)     CubeNormalMap->Release();
	if (AmartinDiffuseMap) AmartinDiffuseMap->Release();
	if (TrollDiffuseMap)   TrollDiffuseMap->Release();
	if (Troll2DiffuseMap)  Troll2DiffuseMap->Release();
	if (LightDiffuseMap)   LightDiffuseMap->Release();
	if (TeaPotDiffuseMap)  TeaPotDiffuseMap->Release();
	if (TeaPot2DiffuseMap) TeaPot2DiffuseMap->Release();
	if (Effect)            Effect->Release();
	if (DepthStencilView)  DepthStencilView->Release();
	if (RenderTargetView)  RenderTargetView->Release();
	if (DepthStencil)      DepthStencil->Release();
	if (SwapChain)         SwapChain->Release();
	if (g_pd3dDevice)      g_pd3dDevice->Release();

}



//--------------------------------------------------------------------------------------
// Load and compile Effect file (.fx file containing shaders)
//--------------------------------------------------------------------------------------
// An effect file contains a set of "Techniques". A technique is a combination of vertex, geometry and pixel shaders (and some states) used for
// rendering in a particular way. We load the effect file at runtime (it's written in HLSL and has the extension ".fx"). The effect code is compiled
// *at runtime* into low-level GPU language. When rendering a particular model we specify which technique from the effect file that it will use
//
bool LoadEffectFile()
{
	ID3D10Blob* pErrors; // This strangely typed variable collects any errors when compiling the effect file
	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS; // These "flags" are used to set the compiler options

	// Load and compile the effect file
	HRESULT hr = D3DX10CreateEffectFromFile(L"GraphicsAssign1.fx", NULL, NULL, "fx_4_0", dwShaderFlags, 0, g_pd3dDevice, NULL, NULL, &Effect, &pErrors, NULL);
	if (FAILED(hr))
	{
		if (pErrors != 0)  MessageBox(NULL, CA2CT(reinterpret_cast<char*>(pErrors->GetBufferPointer())), L"Error", MB_OK); // Compiler error: display error message
		else               MessageBox(NULL, L"Error loading FX file. Ensure your FX file is in the same folder as this executable.", L"Error", MB_OK);  // No error message - probably file not found
		return false;
	}

	// Now we can select techniques from the compiled effect file
	PlainColourTechnique = Effect->GetTechniqueByName("PlainColour");
	FloorTechnique = Effect->GetTechniqueByName("FloorTechnique");
	WiggleTechnique = Effect->GetTechniqueByName("WiggleTex");
	DiffuseSpecularTechnique = Effect->GetTechniqueByName("DiffuseSpecularTex");
	WiggleMirrorTechnique = Effect->GetTechniqueByName("WiggleTexMirror");
	LightTechnique = Effect->GetTechniqueByName("Light1Tex");
	DiffuseOnlyTechnique = Effect->GetTechniqueByName("DiffuseMapOnlyTex");
	AdditiveTexTintTechnique = Effect->GetTechniqueByName("AdditiveTexTint");
	AdditiveTexTintMirrorTechnique = Effect->GetTechniqueByName("AdditiveTexTintMirror");
	NormalMappingTechnique = Effect->GetTechniqueByName("NormalMapping");
	NormalMappingMirrorTechnique = Effect->GetTechniqueByName("NormalMirrorMapping");
	ParallaxMappingTechnique = Effect->GetTechniqueByName("ParallaxMapping");
	ParallaxMappingMirrorTechnique = Effect->GetTechniqueByName("ParallaxMappingMirror");
	CellShadingTechnique = Effect->GetTechniqueByName("CellShading");
	CellShadingMirrorTechnique = Effect->GetTechniqueByName("CellShadingMirror");
	ShadowMappingTechnique = Effect->GetTechniqueByName("ShadowTex");
	ShadowMappingMirrorTechnique = Effect->GetTechniqueByName("ShadowTexMirror");
	DepthOnlyTechnique = Effect->GetTechniqueByName("DepthOnly");
	BlendingTechnique = Effect->GetTechniqueByName("BlendedTexture");
	BlendingMirrorTechnique = Effect->GetTechniqueByName("BlendedMirrorTexture");
	SphereScrollTechnique = Effect->GetTechniqueByName("SphereTex");
	SphereMirrorScrollTechnique = Effect->GetTechniqueByName("SphereMirrorTex");

	// Mirror
	MirrorClearTechnique = Effect->GetTechniqueByName("MirrorClear");
	MirrorSurfaceTechnique = Effect->GetTechniqueByName("MirrorSurface");


	// Create special variables to allow us to access global variables in the shaders from C++
	WorldMatrixVar = Effect->GetVariableByName("WorldMatrix")->AsMatrix();
	ViewMatrixVar = Effect->GetVariableByName("ViewMatrix")->AsMatrix();
	ProjMatrixVar = Effect->GetVariableByName("ProjMatrix")->AsMatrix();
	//NG.
	ViewProjMatrixVar = Effect->GetVariableByName("ViewProjMatrix")->AsMatrix();
	Wiggle = Effect->GetVariableByName("Wiggle")->AsScalar();

	// We access the texture variable in the shader in the same way as we have before for matrices, light data etc.
	// Only difference is that this variable is a "Shader Resource"
	DiffuseMapVar = Effect->GetVariableByName("DiffuseMap")->AsShaderResource();
	NormalMapVar = Effect->GetVariableByName("NormalMap")->AsShaderResource();
	CellMapVar = Effect->GetVariableByName("CellMap")->AsShaderResource();
	ShadowMap1Var = Effect->GetVariableByName("ShadowMap1")->AsShaderResource();
	ShadowMap2Var = Effect->GetVariableByName("ShadowMap2")->AsShaderResource();
	ShadowMap3Var = Effect->GetVariableByName("ShadowMap3")->AsShaderResource();

	// Other shader variables
	ModelColourVar = Effect->GetVariableByName("ModelColour")->AsVector();
	CameraPosVar = Effect->GetVariableByName("CameraPos")->AsVector();

	Light1PosVar = Effect->GetVariableByName("Light1Pos")->AsVector();
	Light1ColourVar = Effect->GetVariableByName("Light1Colour")->AsVector();
	Light1FacingVar = Effect->GetVariableByName("Light1Facing")->AsVector();
	Light1ViewMatrixVar = Effect->GetVariableByName("Light1ViewMatrix")->AsMatrix();
	Light1ProjMatrixVar = Effect->GetVariableByName("Light1ProjMatrix")->AsMatrix();
	Light1ConeAngleVar = Effect->GetVariableByName("Light1CosHalfAngle")->AsScalar();

	Light2PosVar = Effect->GetVariableByName("Light2Pos")->AsVector();
	Light2ColourVar = Effect->GetVariableByName("Light2Colour")->AsVector();
	Light2FacingVar = Effect->GetVariableByName("Light2Facing")->AsVector();
	Light2ViewMatrixVar = Effect->GetVariableByName("Light2ViewMatrix")->AsMatrix();
	Light2ProjMatrixVar = Effect->GetVariableByName("Light2ProjMatrix")->AsMatrix();
	Light2ConeAngleVar = Effect->GetVariableByName("Light2CosHalfAngle")->AsScalar();

	Light3PosVar = Effect->GetVariableByName("Light3Pos")->AsVector();
	Light3ColourVar = Effect->GetVariableByName("Light3Colour")->AsVector();
	Light3FacingVar = Effect->GetVariableByName("Light3Facing")->AsVector();
	Light3ViewMatrixVar = Effect->GetVariableByName("Light3ViewMatrix")->AsMatrix();
	Light3ProjMatrixVar = Effect->GetVariableByName("Light3ProjMatrix")->AsMatrix();
	Light3ConeAngleVar = Effect->GetVariableByName("Light3CosHalfAngle")->AsScalar();

	SpecularPowerVar = Effect->GetVariableByName("SpecularPower")->AsScalar();
	AmbientColourVar = Effect->GetVariableByName("AmbientColour")->AsVector();

	// Other shader variables
	ConstantColourVar = Effect->GetVariableByName("ConstantColour")->AsVector();
	OutlineThicknessVar = Effect->GetVariableByName("OutlineThickness")->AsScalar();
	TintColourVar = Effect->GetVariableByName("TintColour")->AsVector();
	ParallaxDepthVar = Effect->GetVariableByName("ParallaxDepth")->AsScalar();

	// Mirror
	ClipPlaneVar = Effect->GetVariableByName("ClipPlane")->AsVector();


	return true;
}

//--------------------------------------------------------------------------------------
// Light Helper Functions
//--------------------------------------------------------------------------------------

// Get "camera-like" view matrix for a spot light
// todo
//D3DXMATRIXA16 CalculateLightViewMatrix(int lightNum)
//{
//	D3DXMATRIXA16 viewMatrix;
//
//	// Get the world matrix of the light model and invert it to get the view matrix (that is more-or-less the definition of a view matrix)
//	// We don't always have a physical model for a light, in which case we would need to store this data along with the light colour etc.
//	D3DXMATRIXA16 worldMatrix = Lights[lightNum]->GetWorldMatrix();
//	D3DXMatrixInverse(&viewMatrix, NULL, &worldMatrix);
//
//	return viewMatrix;
//}
//
//// Get "camera-like" projection matrix for a spot light
//D3DXMATRIXA16 CalculateLightProjMatrix(int lightNum)
//{
//	D3DXMATRIXA16 projMatrix;
//
//	// Create a projection matrix for the light. Use the spotlight cone angle as an FOV, just set default values for everything else.
//	D3DXMatrixPerspectiveFovLH(&projMatrix, ToRadians(SpotlightConeAngle), 1, 0.1f, 1000.0f);
//
//	return projMatrix;
//}

//--------------------------------------------------------------------------------------
// Scene Setup / Update / Rendering
//--------------------------------------------------------------------------------------

// Create / load the camera, models and textures for the scene
bool InitScene()
{
	//////////////////
	// Create camera

	Camera = new CCamera();
	Camera->SetPosition(D3DXVECTOR3(0, 70, -140));
	Camera->SetRotation(D3DXVECTOR3(ToRadians(13.0f), ToRadians(0.0f), 0.0f)); // ToRadians is a new helper function to convert degrees to radians
	// Mirror
	Camera->SetNearClip(1.0f);

	// Portal
	PortalCamera = new CCamera();
	PortalCamera->SetPosition(D3DXVECTOR3(45, 45, 85));
	PortalCamera->SetRotation(D3DXVECTOR3(ToRadians(20.0f), ToRadians(215.0f), 0.));

	///////////////////////
	// Load/Create models

	Cube = new CModel;
	Cube2 = new CModel;
	Cube3 = new CModel;
	Sphere = new CModel;
	Amartin = new CModel;
	Troll = new CModel;
	Troll2 = new CModel;
	TeaPot = new CModel;
	TeaPot2 = new CModel;
	Floor = new CModel;
	Mirror = new CModel;
	Portal = new CModel;
	Frame = new CModel;


	// The model class can load ".X" files. It encapsulates (i.e. hides away from this code) the file loading/parsing and creation of vertex/index buffers
	// We must pass an example technique used for each model. We can then only render models with techniques that uses matching vertex input data
	if (!Cube->Load("Cube.x", WiggleTechnique)) return false;
	if (!Cube2->Load("Cube.x", BlendingTechnique)) return false;
	if (!Cube3->Load("Cube.x", NormalMappingTechnique)) return false;
	if (!Sphere->Load("Sphere.x", SphereScrollTechnique)) return false;
	if (!Troll->Load("Troll.x", ShadowMappingTechnique))  return false;
	if (!Troll2->Load("Troll.x", CellShadingTechnique))  return false;
	if (!TeaPot->Load("TeaPot.x", ParallaxMappingTechnique)) return false;
	if (!TeaPot2->Load("TeaPot.x", DiffuseSpecularTechnique)) return false;
	if (!Floor->Load("floor.x", ShadowMappingTechnique)) return false;
	if (!Mirror->Load("Mirror.x", MirrorClearTechnique)) return false;
	if (!Portal->Load("Portal.x", DiffuseOnlyTechnique)) return false;
	if (!Frame->Load("Portal.x", DiffuseOnlyTechnique)) return false;
	


	// Initial positions
	Cube->SetPosition(D3DXVECTOR3(-50, 10, 40));
	Cube2->SetPosition(D3DXVECTOR3(-50, 10, 0));
	Cube3->SetPosition(D3DXVECTOR3(-50, 10, 80));
	Sphere->SetPosition(D3DXVECTOR3(0, 10, 80));
	Sphere->SetScale(0.5f);
	Amartin->SetPosition(D3DXVECTOR3(20, 10, 90));
	Amartin->SetScale(8.0f);
	Troll->SetPosition(D3DXVECTOR3(50, 10, 20));
	Troll->SetScale(5.0f);
	Troll2->SetPosition(D3DXVECTOR3(50, 10, 60));
	Troll2->SetScale(5.0f);
	TeaPot->SetPosition(D3DXVECTOR3(0, 10, 40));
	TeaPot2->SetPosition(D3DXVECTOR3(0, 10, 0));
	Mirror->SetPosition(D3DXVECTOR3(0, 20, 120));
	Mirror->SetRotation(D3DXVECTOR3(0.0f, ToRadians(-180.0f), 0.0f));
	Mirror->SetScale(5.0f);
	Portal->SetPosition(D3DXVECTOR3(90, 50, 80));
	Portal->SetRotation(D3DXVECTOR3(0.0f, ToRadians(-130.0f), 0.0f));
	Frame->SetPosition(D3DXVECTOR3(-90, 50, 80));
	Frame->SetRotation(D3DXVECTOR3(0.0f, ToRadians(130.0f), 0.0f));

	// Using an array of lights in this lab
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		lights[i] = new CLights();
		if (!lights[i]->Load("Light.x", AdditiveTexTintTechnique)) return false;
	}

	// The lights are spotlights - so they need to face in a particular direction too (using the model holds the light's positioning data)
	lights[0]->SetPosition(D3DXVECTOR3(30, 20, 0));
	lights[0]->SetScale(4.0f);
	lights[0]->SetFacePoint(Cube->GetPosition());
	lights[1]->SetPosition(D3DXVECTOR3(-20, 120, 200));
	lights[1]->SetScale(50.0f);
	lights[1]->SetFacePoint(Troll->GetPosition());
	lights[2]->SetPosition(D3DXVECTOR3(30, 40, 40));
	lights[2]->SetScale(10.0f);
	lights[2]->SetFacePoint(Troll->GetPosition());

	//////////////////
	// Load textures

	if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, L"StoneDiffuseSpecular.dds", NULL, NULL, &CubeDiffuseMap, NULL)))
		return false;
	if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, L"moogle.png", NULL, NULL, &Cube2DiffuseMap, NULL)))
		return false;
	if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, L"PatternDiffuseSpecular.dds", NULL, NULL, &Cube3DiffuseMap, NULL)))
		return false;
	if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, L"PatternNormal.dds", NULL, NULL, &CubeNormalMap, NULL)))
		return false;
	if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, L"amartin.jpg", NULL, NULL, &AmartinDiffuseMap, NULL)))
		return false;
	if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, L"Troll3DiffuseSpecular.dds", NULL, NULL, &TrollDiffuseMap, NULL)))
		return false;
	if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, L"PatternNormal.dds", NULL, NULL, &TrollNormalMap, NULL)))
		return false;
	if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, L"Green.png", NULL, NULL, &Troll2DiffuseMap, NULL)))
		return false;
	if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, L"CobbleDiffuseSpecular.dds", NULL, NULL, &TeaPotDiffuseMap, NULL)))
		return false; 
	if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, L"brick1.jpg", NULL, NULL, &TeaPot2DiffuseMap, NULL)))
		return false;
	if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, L"CobbleNormalDepth.dds", NULL, NULL, &FloorNormalMap, NULL)))
		return false;
	if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, L"WoodDiffuseSpecular.dds", NULL, NULL, &FloorDiffuseMap, NULL)))
		return false;
	if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, L"flare.jpg", NULL, NULL, &LightDiffuseMap, NULL)))
		return false;
	if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, L"CellGradient.png", NULL, NULL, &CellMap, NULL)))
		return false;
	if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, L"tiles1.jpg", NULL, NULL, &SphereDiffuseMap, NULL)))
		return false;

	////////////////////////
	//**** Shadow Maps ****//

	// Shadow Map ( NG )

	// Create the shadow map textures, above we used a D3DX... helper function to create basic textures in one line. Here, we need to
	// do things manually as we are creating a special kind of texture (one that we can render to). Many settings to prepare:
	D3D10_TEXTURE2D_DESC texDesc;
	texDesc.Width = ShadowMapSize; // Size of the shadow map determines quality / resolution of shadows
	texDesc.Height = ShadowMapSize;
	texDesc.MipLevels = 1; // 1 level, means just the main texture, no additional mip-maps. Usually don't use mip-maps when rendering to textures (or we would have to render every level)
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS; // The shadow map contains a single 32-bit value [tech gotcha: have to say typeless because depth buffer and texture see things slightly differently]
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D10_USAGE_DEFAULT;
	texDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL | D3D10_BIND_SHADER_RESOURCE; // Indicate we will use texture as render target, and will also pass it to shaders
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	if (FAILED(g_pd3dDevice->CreateTexture2D(&texDesc, NULL, &ShadowMap1Texture))) return false;
	if (FAILED(g_pd3dDevice->CreateTexture2D(&texDesc, NULL, &ShadowMap2Texture))) return false;
	if (FAILED(g_pd3dDevice->CreateTexture2D(&texDesc, NULL, &ShadowMap3Texture))) return false;

	// Create the depth stencil view, i.e. indicate that the texture just created is to be used as a depth buffer
	D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = DXGI_FORMAT_D32_FLOAT; // See "tech gotcha" above
	descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	if (FAILED(g_pd3dDevice->CreateDepthStencilView(ShadowMap1Texture, &descDSV, &ShadowMap1DepthView))) return false;
	if (FAILED(g_pd3dDevice->CreateDepthStencilView(ShadowMap2Texture, &descDSV, &ShadowMap2DepthView))) return false;
	if (FAILED(g_pd3dDevice->CreateDepthStencilView(ShadowMap3Texture, &descDSV, &ShadowMap3DepthView))) return false;

	// We also need to send this texture (a GPU memory resource) to the shaders. To do that we must create a shader-resource "view"	
	D3D10_SHADER_RESOURCE_VIEW_DESC srDesc;
	srDesc.Format = DXGI_FORMAT_R32_FLOAT; // See "tech gotcha" above
	srDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	srDesc.Texture2D.MostDetailedMip = 0;
	srDesc.Texture2D.MipLevels = 1;
	if (FAILED(g_pd3dDevice->CreateShaderResourceView(ShadowMap1Texture, &srDesc, &ShadowMap1))) return false;
	if (FAILED(g_pd3dDevice->CreateShaderResourceView(ShadowMap2Texture, &srDesc, &ShadowMap2))) return false;
	if (FAILED(g_pd3dDevice->CreateShaderResourceView(ShadowMap3Texture, &srDesc, &ShadowMap3))) return false;

	//*****************************//




	// NG. Portal texture

	// Create the portal texture itself, above we used a D3DX... helper function to create a texture in one line. Here, we need to do things manually
	// as we are creating a special kind of texture (one that we can render to). Many settings to prepare:
	D3D10_TEXTURE2D_DESC portalDesc;
	portalDesc.Width = PortalWidth;  // Size of the portal texture determines its quality
	portalDesc.Height = PortalHeight;
	portalDesc.MipLevels = 1; // No mip-maps when rendering to textures (or we would have to render every level)
	portalDesc.ArraySize = 1;
	portalDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // RGBA texture (8-bits each)
	portalDesc.SampleDesc.Count = 1;
	portalDesc.SampleDesc.Quality = 0;
	portalDesc.Usage = D3D10_USAGE_DEFAULT;
	portalDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE; // Indicate we will use texture as render target, and pass it to shaders
	portalDesc.CPUAccessFlags = 0;
	portalDesc.MiscFlags = 0;
	if (FAILED(g_pd3dDevice->CreateTexture2D(&portalDesc, NULL, &PortalTexture))) return false;

	// We created the portal texture above, now we get a "view" of it as a render target, i.e. get a special pointer to the texture that
	// we use when rendering to it (see RenderScene function below)
	if (FAILED(g_pd3dDevice->CreateRenderTargetView(PortalTexture, NULL, &PortalRenderTarget))) return false;

	// We also need to send this texture (resource) to the shaders. To do that we must create a shader-resource "view"
	D3D10_SHADER_RESOURCE_VIEW_DESC srDesc1;
	srDesc1.Format = portalDesc.Format;
	srDesc1.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	srDesc1.Texture2D.MostDetailedMip = 0;
	srDesc1.Texture2D.MipLevels = 1;
	if (FAILED(g_pd3dDevice->CreateShaderResourceView(PortalTexture, &srDesc1, &PortalMap))) return false;


	//**** Portal Depth Buffer ****//

	// We also need a depth buffer to go with our portal
	//**** This depth buffer can be shared with any other portals of the same size
	portalDesc.Width = PortalWidth;
	portalDesc.Height = PortalHeight;
	portalDesc.MipLevels = 1;
	portalDesc.ArraySize = 1;
	portalDesc.Format = DXGI_FORMAT_D32_FLOAT;
	portalDesc.SampleDesc.Count = 1;
	portalDesc.SampleDesc.Quality = 0;
	portalDesc.Usage = D3D10_USAGE_DEFAULT;
	portalDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
	portalDesc.CPUAccessFlags = 0;
	portalDesc.MiscFlags = 0;
	if (FAILED(g_pd3dDevice->CreateTexture2D(&portalDesc, NULL, &PortalDepthStencil))) return false;

	// Create the depth stencil view, i.e. indicate that the texture just created is to be used as a depth buffer
	D3D10_DEPTH_STENCIL_VIEW_DESC portalDescDSV;
	portalDescDSV.Format = portalDesc.Format;
	portalDescDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
	portalDescDSV.Texture2D.MipSlice = 0;
	if (FAILED(g_pd3dDevice->CreateDepthStencilView(PortalDepthStencil, &portalDescDSV, &PortalDepthStencilView))) return false;

	//*****************************//


	// NG. Portal2 texture

	// Create the portal texture itself, above we used a D3DX... helper function to create a texture in one line. Here, we need to do things manually
	// as we are creating a special kind of texture (one that we can render to). Many settings to prepare:
	D3D10_TEXTURE2D_DESC portalDesc2;
	portalDesc2.Width = PortalWidth;  // Size of the portal texture determines its quality
	portalDesc2.Height = PortalHeight;
	portalDesc2.MipLevels = 1; // No mip-maps when rendering to textures (or we would have to render every level)
	portalDesc2.ArraySize = 1;
	portalDesc2.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // RGBA texture (8-bits each)
	portalDesc2.SampleDesc.Count = 1;
	portalDesc2.SampleDesc.Quality = 0;
	portalDesc2.Usage = D3D10_USAGE_DEFAULT;
	portalDesc2.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE; // Indicate we will use texture as render target, and pass it to shaders
	portalDesc2.CPUAccessFlags = 0;
	portalDesc2.MiscFlags = 0;
	if (FAILED(g_pd3dDevice->CreateTexture2D(&portalDesc2, NULL, &PortalTexture2))) return false;

	// We created the portal texture above, now we get a "view" of it as a render target, i.e. get a special pointer to the texture that
	// we use when rendering to it (see RenderScene function below)
	if (FAILED(g_pd3dDevice->CreateRenderTargetView(PortalTexture2, NULL, &PortalRenderTarget2))) return false;

	// We also need to send this texture (resource) to the shaders. To do that we must create a shader-resource "view"
	D3D10_SHADER_RESOURCE_VIEW_DESC srDesc2;
	srDesc2.Format = portalDesc2.Format;
	srDesc2.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	srDesc2.Texture2D.MostDetailedMip = 0;
	srDesc2.Texture2D.MipLevels = 1;
	if (FAILED(g_pd3dDevice->CreateShaderResourceView(PortalTexture2, &srDesc2, &PortalMap2))) return false;


	//**** Portal Depth Buffer ****//

	// We also need a depth buffer to go with our portal
	//**** This depth buffer can be shared with any other portals of the same size
	portalDesc2.Width = PortalWidth;
	portalDesc2.Height = PortalHeight;
	portalDesc2.MipLevels = 1;
	portalDesc2.ArraySize = 1;
	portalDesc2.Format = DXGI_FORMAT_D32_FLOAT;
	portalDesc2.SampleDesc.Count = 1;
	portalDesc2.SampleDesc.Quality = 0;
	portalDesc2.Usage = D3D10_USAGE_DEFAULT;
	portalDesc2.BindFlags = D3D10_BIND_DEPTH_STENCIL;
	portalDesc2.CPUAccessFlags = 0;
	portalDesc2.MiscFlags = 0;
	if (FAILED(g_pd3dDevice->CreateTexture2D(&portalDesc2, NULL, &PortalDepthStencil2))) return false;

	// Create the depth stencil view, i.e. indicate that the texture just created is to be used as a depth buffer
	D3D10_DEPTH_STENCIL_VIEW_DESC portalDescDSV2;
	portalDescDSV2.Format = portalDesc.Format;
	portalDescDSV2.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
	portalDescDSV2.Texture2D.MipSlice = 0;
	if (FAILED(g_pd3dDevice->CreateDepthStencilView(PortalDepthStencil2, &portalDescDSV2, &PortalDepthStencilView2))) return false;

	//*****************************//


	return true;
}


// Update the scene - move/rotate each model and the camera, then update their matrices
void UpdateScene(float frameTime)
{
	// Control camera position and update its matrices (view matrix, projection matrix) each frame
	// Don't be deceived into thinking that this is a new method to control models - the same code we used previously is in the camera class
	Camera->Control(frameTime, Key_Up, Key_Down, Key_Left, Key_Right, Key_W, Key_S, Key_A, Key_D);
	Camera->UpdateMatrices();
	PortalCamera->UpdateMatrices();

	// Control cube position and update its world matrix each frame
	Cube->Control(frameTime, Key_T, Key_G, Key_F, Key_H, Key_R, Key_Y, Key_Period, Key_Comma);
	Cube->UpdateMatrix();

	// NG. Cube 2 Control
	Cube2->Control(frameTime, Key_T, Key_G, Key_F, Key_H, Key_R, Key_Y, Key_Period, Key_Comma);
	Cube2->UpdateMatrix();

	// NG. Sphere Control
	Sphere->Control(frameTime, Key_T, Key_G, Key_F, Key_H, Key_R, Key_Y, Key_Period, Key_Comma);
	Sphere->UpdateMatrix();

	// NG. Cube3 Control
	Cube3->Control(frameTime, Key_T, Key_G, Key_F, Key_H, Key_R, Key_Y, Key_Period, Key_Comma);
	Cube3->UpdateMatrix();

	// Controll Troll
	Troll->Control(frameTime, Key_T, Key_G, Key_F, Key_H, Key_R, Key_Y, Key_Period, Key_Comma);
	Troll->UpdateMatrix();

	Troll2->Control(frameTime, Key_T, Key_G, Key_F, Key_H, Key_R, Key_Y, Key_Period, Key_Comma);
	Troll2->UpdateMatrix();

	// Control Teapot
	TeaPot->Control(frameTime, Key_T, Key_G, Key_F, Key_H, Key_R, Key_Y, Key_Period, Key_Comma);
	TeaPot->UpdateMatrix();

	// Control Teapot2
	TeaPot2->Control(frameTime, Key_T, Key_G, Key_F, Key_H, Key_R, Key_Y, Key_Period, Key_Comma);
	TeaPot2->UpdateMatrix();

	// Control Mirror
	Mirror->Control(frameTime, Key_I, Key_K, Key_J, Key_L, Key_U, Key_O, Key_Period, Key_Comma);
	Mirror->UpdateMatrix();

	// Update Floor
	Floor->UpdateMatrix();

	
	// Update the orbiting light - a bit of a cheat with the static variable [ask the tutor if you want to know what this is]
	static bool Orbiting = true;
	static bool Wiggling = true;
	static bool LightTrigger = true;
	static bool LightPulsate = true;
	static float Rotate = 0.0f;


	//Mirror->SetPosition(TeaPot->GetPosition() + D3DXVECTOR3(cos(Rotate) * MirrorOrbitRadius, 20, sin(Rotate) * MirrorOrbitRadius));
	

	lights[0]->SetPosition(Cube->GetPosition() + D3DXVECTOR3(cos(Rotate) * LightOrbitRadius, 20, sin(Rotate) * LightOrbitRadius));
	if (Orbiting) Rotate -= LightOrbitSpeed * frameTime;


	// NG. Light1 Pointing at Cube
	lights[0]->SetFacePoint(Cube->GetPosition());

	// NG. Light2 Pointing at Troll
	lights[1]->SetFacePoint(Troll->GetPosition());

	// NG .Light3 Pointing at Troll
	lights[2]->SetFacePoint(Troll->GetPosition());

	// NG. Portal
	Portal->UpdateMatrix();

	if (Wiggling)
	{
		fWiggle += 6 * frameTime;
		Wiggle->SetFloat(fWiggle);
	}

	// Stop Wiggle effect
	if (KeyHit(Key_3)) Wiggling = !Wiggling;

	// Stop orbit with key "2" N.G
	if (KeyHit(Key_2)) Orbiting = !Orbiting;

	// Toggle parallax with key "1" N.G
	if (KeyHit(Key_1))
	{
		UseParallax = !UseParallax;
	}

	// Timer calculation per frame
	timer = timer - 15.0f * frameTime;
	pulsate = pulsate - 15.0f * frameTime;

	// NG. Light 3 on & off
	if (timer < 0) {
		if (LightTrigger) {
			LightColours[2] = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
			LightTrigger = false;

		}
		else {
			LightColours[2] = D3DXVECTOR3(1.0f, 0.0f, 0.5f) * 10;
			LightTrigger = true;
		}
		timer = 10.0f;
	}

	// NG. Light 1 - 2 changing colours
	if (pulsate < 0) {
		if (LightPulsate) {
			LightColours[0] = D3DXVECTOR3(0.0f, 0.0f, 0.8f) * 15;
			LightColours[1] = D3DXVECTOR3(1.0f, 1.0f, 1.0f) * 60;
			LightPulsate = false;

		}
		else {
			LightColours[0] = D3DXVECTOR3(0.8f, 0.8f, 1.0f) * 8;
			LightColours[1] = D3DXVECTOR3(1.0f, 0.8f, 0.2f) * 60;
			LightPulsate = true;
		}
		pulsate = 10.0f;
	}

	for (int i = 0; i < NUM_LIGHTS; i++) {
		lights[i]->Update();
	}
	// Update Portal
	Frame->UpdateMatrix();

}

void RenderMirrorModels() {
	// NG. Render TeaPot
	WorldMatrixVar->SetMatrix((float*)TeaPot->GetWorldMatrix());
	DiffuseMapVar->SetResource(TeaPotDiffuseMap);
	NormalMapVar->SetResource(FloorNormalMap);
	TeaPot->Render(ParallaxMappingMirrorTechnique);

	// NG. Render TeaPot2
	WorldMatrixVar->SetMatrix((float*)TeaPot2->GetWorldMatrix());
	DiffuseMapVar->SetResource(TeaPot2DiffuseMap);
	TeaPot2->Render(ParallaxMappingMirrorTechnique);

	// N.G Rener Floor
	WorldMatrixVar->SetMatrix((float*)Floor->GetWorldMatrix());
	DiffuseMapVar->SetResource(FloorDiffuseMap);
	Floor->Render(ShadowMappingMirrorTechnique);

	// N.G Rener Sphere
	WorldMatrixVar->SetMatrix((float*)Sphere->GetWorldMatrix());
	DiffuseMapVar->SetResource(SphereDiffuseMap);
	Sphere->Render(SphereMirrorScrollTechnique);

	// NG. Render cube
	WorldMatrixVar->SetMatrix((float*)Cube->GetWorldMatrix());
	DiffuseMapVar->SetResource(CubeDiffuseMap);
	// todo
	Cube->Render(WiggleMirrorTechnique);

	// NG. Render cube2
	WorldMatrixVar->SetMatrix((float*)Cube2->GetWorldMatrix());
	DiffuseMapVar->SetResource(Cube2DiffuseMap);
	Cube2->Render(BlendingMirrorTechnique);

	// NG. Render cube3
	WorldMatrixVar->SetMatrix((float*)Cube3->GetWorldMatrix());
	NormalMapVar->SetResource(CubeNormalMap);
	DiffuseMapVar->SetResource(Cube3DiffuseMap);
	Cube3->Render(NormalMappingMirrorTechnique);

	// NG. Render Troll
	WorldMatrixVar->SetMatrix((float*)Troll->GetWorldMatrix());
	DiffuseMapVar->SetResource(TrollDiffuseMap);
	//NormalMapVar->SetResource(TrollNormalMap);
	Troll->Render(ShadowMappingMirrorTechnique);

	// NG. Render Troll2
	WorldMatrixVar->SetMatrix((float*)Troll2->GetWorldMatrix());
	ConstantColourVar->SetRawValue(OutlineColour, 0, 12);
	DiffuseMapVar->SetResource(Troll2DiffuseMap);
	Troll2->Render(CellShadingMirrorTechnique);

	// Using an array of lights in this lab
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		WorldMatrixVar->SetMatrix(lights[i]->GetWorldMatrix());
		DiffuseMapVar->SetResource(LightDiffuseMap);
		TintColourVar->SetRawValue(LightColours[i], 0, 12); // Using special shader that tints the light model to match the light colour
		lights[i]->Render(AdditiveTexTintMirrorTechnique);
	}
}


// Render all the models from the point of view of the given camera
void RenderMain(CCamera* camera)
{
	// Pass the camera's matrices to the vertex shader and position to the vertex shader
	ViewMatrixVar->SetMatrix((float*)&camera->GetViewMatrix());
	ProjMatrixVar->SetMatrix((float*)&camera->GetProjectionMatrix());
	CameraPosVar->SetRawValue(camera->GetPosition(), 0, 12);
	
	// Send the shadow maps rendered in the function below to the shader
	ShadowMap1Var->SetResource(ShadowMap1);
	ShadowMap2Var->SetResource(ShadowMap2);
	ShadowMap3Var->SetResource(ShadowMap3);



	// NG. Render cube
	WorldMatrixVar->SetMatrix((float*)Cube->GetWorldMatrix());
	DiffuseMapVar->SetResource(CubeDiffuseMap);
	Cube->Render(WiggleTechnique);

	// NG. Render cube2
	WorldMatrixVar->SetMatrix((float*)Cube2->GetWorldMatrix());
	DiffuseMapVar->SetResource(Cube2DiffuseMap);
	Cube2->Render(BlendingTechnique);

	// N.G Rener Sphere
	WorldMatrixVar->SetMatrix((float*)Sphere->GetWorldMatrix());
	DiffuseMapVar->SetResource(SphereDiffuseMap);
	Sphere->Render(SphereScrollTechnique);

	// NG. Render cube3
	WorldMatrixVar->SetMatrix((float*)Cube3->GetWorldMatrix());
	NormalMapVar->SetResource(CubeNormalMap);
	DiffuseMapVar->SetResource(Cube3DiffuseMap);
	Cube3->Render(NormalMappingTechnique);

	// NG. Render Troll
	WorldMatrixVar->SetMatrix((float*)Troll->GetWorldMatrix());
	DiffuseMapVar->SetResource(TrollDiffuseMap);
	//NormalMapVar->SetResource(TrollNormalMap);
	Troll->Render(ShadowMappingTechnique);

	// NG. Render Troll2
	WorldMatrixVar->SetMatrix((float*)Troll2->GetWorldMatrix());
	ConstantColourVar->SetRawValue(OutlineColour, 0, 12);
	DiffuseMapVar->SetResource(Troll2DiffuseMap);
	Troll2->Render(CellShadingTechnique);

	// NG. Render TeaPot
	WorldMatrixVar->SetMatrix((float*)TeaPot->GetWorldMatrix());
	DiffuseMapVar->SetResource(TeaPotDiffuseMap);
	NormalMapVar->SetResource(FloorNormalMap);
	TeaPot->Render(ParallaxMappingTechnique);

	// NG. Render TeaPot2
	WorldMatrixVar->SetMatrix((float*)TeaPot2->GetWorldMatrix());
	DiffuseMapVar->SetResource(TeaPot2DiffuseMap);
	TeaPot2->Render(DiffuseSpecularTechnique);

	// N.G Rener Floor
	WorldMatrixVar->SetMatrix((float*)Floor->GetWorldMatrix());
	DiffuseMapVar->SetResource(FloorDiffuseMap);
	Floor->Render(ShadowMappingTechnique);

	// N.G Render Portal
	WorldMatrixVar->SetMatrix((float*)Portal->GetWorldMatrix());
	DiffuseMapVar->SetResource(PortalMap);
	Portal->Render(DiffuseOnlyTechnique);
	

	// N.G Render Portal
	WorldMatrixVar->SetMatrix((float*)Frame->GetWorldMatrix());
	DiffuseMapVar->SetResource(PortalMap2);
	Frame->Render(DiffuseOnlyTechnique);


	// Using an array of lights in this lab
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		WorldMatrixVar->SetMatrix(lights[i]->GetWorldMatrix());
		DiffuseMapVar->SetResource(LightDiffuseMap);
		TintColourVar->SetRawValue(LightColours[i], 0, 12); // Using special shader that tints the light model to match the light colour
		lights[i]->Render(AdditiveTexTintTechnique);
	}
}

void RenderMirrors()
{
	//*******************************************************************************************************
	// First set the stencil value for all mirror pixels to 1 and clear the mirror to a fixed colour and 
	// set its depth-buffer values to maximum (so we can render "inside" the mirror)

	D3DXMATRIXA16 mirrorMatrix = Mirror->GetWorldMatrix();
	WorldMatrixVar->SetMatrix((float*)mirrorMatrix);
	ConstantColourVar->SetRawValue(BackgroundColour, 0, 12);
	Mirror->Render(MirrorClearTechnique);


	//*******************************************************************************************************
	// Next reflect the camera in the mirror

	// Some mathematics to get as reflected version of the camera - using DirectX helper functions mostly

	// Create a plane for the mirror
	D3DXPLANE mirrorPlane;
	D3DXVECTOR3 mirrorPoint = D3DXVECTOR3(mirrorMatrix(3, 0), mirrorMatrix(3, 1), mirrorMatrix(3, 2));
	D3DXVECTOR3 mirrorNormal = D3DXVECTOR3(mirrorMatrix(2, 0), mirrorMatrix(2, 1), mirrorMatrix(2, 2));
	D3DXPlaneFromPointNormal(&mirrorPlane, &mirrorPoint, &mirrorNormal);

	// Reflect the camera's view matrix in the mirror plane
	D3DXMATRIXA16 reflectMatrix;
	D3DXMatrixReflect(&reflectMatrix, &mirrorPlane);
	D3DXMATRIXA16 reflectViewMatrix = reflectMatrix * Camera->GetViewMatrix();

	// Reflect the camera's position in the mirror plane
	D3DXVECTOR3 cameraPos = Camera->GetPosition();
	D3DXVECTOR4 reflectCameraPos4; // Initially generate a 4 element vector
	D3DXVec3Transform(&reflectCameraPos4, &cameraPos, &reflectMatrix);
	D3DXVECTOR3 reflectCameraPos = D3DXVECTOR3((float*)reflectCameraPos4); // Drop 4th element


	//*******************************************************************************************************
	// Render all the models "inside" the mirror

	// Set the reflected camera data in the shaders
	ViewMatrixVar->SetMatrix((float*)&reflectViewMatrix);
	CameraPosVar->SetRawValue(reflectCameraPos, 0, 12);
	ClipPlaneVar->SetRawValue(mirrorPlane, 0, 16);

	// Need to use slightly different techniques to avoid mirror rendering being "inside out"
	RenderMirrorModels();

	// Restore main camera data in the shaders
	ViewMatrixVar->SetMatrix((float*)&Camera->GetViewMatrix());
	CameraPosVar->SetRawValue(Camera->GetPosition(), 0, 12);
	ClipPlaneVar->SetRawValue(D3DXVECTOR4(0, 0, 0, 0), 0, 16);


	//*******************************************************************************************************
	// Finally draw a "surface" for the mirror - a transparent layer over the mirror contents. This sets the correct depth-buffer values 
	// for the mirror surface, so further rendering won't go "inside" the mirrored scene

	WorldMatrixVar->SetMatrix((float*)Mirror->GetWorldMatrix());
	Mirror->Render(MirrorSurfaceTechnique);
}



// Render all the models from the point of view of the given camera
void RenderShadowMap(int lightNum)
{
	//---------------------------------
	// Set "camera" matrices in shader

	// Pass the light's "camera" matrices to the vertex shader - use helper functions above to turn spotlight settings into "camera" matrices
	ViewMatrixVar->SetMatrix(lights[lightNum]->CalculateLightViewMatrix());
	ProjMatrixVar->SetMatrix(lights[lightNum]->CalculateLightProjMatrix());

	//-----------------------------------
	// Render each model into shadow map

	WorldMatrixVar->SetMatrix(Floor->GetWorldMatrix());
	Floor->Render(DepthOnlyTechnique);

	WorldMatrixVar->SetMatrix(Troll->GetWorldMatrix());
	Troll->Render(DepthOnlyTechnique);

	WorldMatrixVar->SetMatrix(Cube->GetWorldMatrix());
	Cube->Render(DepthOnlyTechnique);

	WorldMatrixVar->SetMatrix(Cube2->GetWorldMatrix());
	Cube2->Render(DepthOnlyTechnique);

	WorldMatrixVar->SetMatrix(Cube3->GetWorldMatrix());
	Cube3->Render(DepthOnlyTechnique);

	WorldMatrixVar->SetMatrix(Troll2->GetWorldMatrix());
	Troll2->Render(DepthOnlyTechnique);

	WorldMatrixVar->SetMatrix(TeaPot->GetWorldMatrix());
	TeaPot->Render(DepthOnlyTechnique);

	WorldMatrixVar->SetMatrix(TeaPot2->GetWorldMatrix());
	TeaPot2->Render(DepthOnlyTechnique);

	WorldMatrixVar->SetMatrix(Sphere->GetWorldMatrix());
	Sphere->Render(DepthOnlyTechnique);





}


// Render everything in the scene
void RenderScene()
{
	// Clear the back buffer - before drawing the geometry clear the entire window to a fixed colour
	float ClearColor[4] = { 0.2f, 0.2f, 0.3f, 1.0f }; // Good idea to match background to ambient colour
	//g_pd3dDevice->ClearRenderTargetView(RenderTargetView, ClearColor);	
	g_pd3dDevice->ClearRenderTargetView(RenderTargetView, &BackgroundColour[0]);
	//g_pd3dDevice->ClearDepthStencilView(DepthStencilView, D3D10_CLEAR_DEPTH, 1.0f, 0); // Clear the depth buffer too
	g_pd3dDevice->ClearDepthStencilView(DepthStencilView, D3D10_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, 1.0f, 0);

	//---------------------------
	// Common rendering settings

	// Common features for all models, set these once only

	//// Pass the camera's matrices to the vertex shader
	//ViewMatrixVar->SetMatrix( (float*)&Camera->GetViewMatrix() );
	//ProjMatrixVar->SetMatrix( (float*)&Camera->GetProjectionMatrix() );
	////// NG.
	//ViewProjMatrixVar->SetMatrix((float*)&Camera->GetViewProjectionMatrix());


	//---------------------------
	// Render each model
	// todo
	Light1PosVar->SetRawValue(lights[0]->GetPosition(), 0, 12);
	Light1ColourVar->SetRawValue(LightColours[0], 0, 12);
	Light1FacingVar->SetRawValue(lights[0]->GetFacing(), 0, 12);
	Light1ViewMatrixVar->SetMatrix(lights[0]->CalculateLightViewMatrix());
	Light1ProjMatrixVar->SetMatrix(lights[0]->CalculateLightProjMatrix());
	Light1ConeAngleVar->SetFloat(cos(ToRadians(SpotlightConeAngle * 0.5f))); // Shader wants cos(angle/2)

	Light2PosVar->SetRawValue(lights[1]->GetPosition(), 0, 12);
	Light2ColourVar->SetRawValue(LightColours[1], 0, 12);
	Light2FacingVar->SetRawValue(lights[1]->GetFacing(), 0, 12);
	Light2ViewMatrixVar->SetMatrix(lights[1]->CalculateLightViewMatrix());
	Light2ProjMatrixVar->SetMatrix(lights[1]->CalculateLightProjMatrix());
	Light2ConeAngleVar->SetFloat(cos(ToRadians(SpotlightConeAngle * 0.5f))); // Shader wants cos(angle/2)

	Light3PosVar->SetRawValue(lights[2]->GetPosition(), 0, 12);
	Light3ColourVar->SetRawValue(LightColours[2], 0, 12);
	Light3FacingVar->SetRawValue(lights[2]->GetFacing(), 0, 12);
	Light3ViewMatrixVar->SetMatrix(lights[2]->CalculateLightViewMatrix());
	Light3ProjMatrixVar->SetMatrix(lights[2]->CalculateLightProjMatrix());
	Light3ConeAngleVar->SetFloat(cos(ToRadians(SpotlightConeAngle * 0.5f))); // Shader wants cos(angle/2)

	AmbientColourVar->SetRawValue(AmbientColour, 0, 12);
	//CameraPosVar->SetRawValue(Camera->GetPosition(), 0, 12);
	SpecularPowerVar->SetFloat(SpecularPower);

	// Send the "cell map" to the shader, a special 1D texture used to create the cartoon look
	// Also set thickness of outlines. The outline colour is set per-model below
	CellMapVar->SetResource(CellMap);
	OutlineThicknessVar->SetFloat(OutlineThickness);

	// NG. Parallax mapping depth
	ParallaxDepthVar->SetFloat(UseParallax ? ParallaxDepth : 0.0f);



	//---------------------------
	// Render portal scene

	// Setup the viewport - defines which part of the texture we will render to (usually all of it)
	D3D10_VIEWPORT vp1;
	vp1.Width = PortalWidth;
	vp1.Height = PortalHeight;
	vp1.MinDepth = 0.0f;
	vp1.MaxDepth = 1.0f;
	vp1.TopLeftX = 0;
	vp1.TopLeftY = 0;
	g_pd3dDevice->RSSetViewports(1, &vp1);

	// Select the portal texture to use for rendering, will share the depth/stencil buffer with the backbuffer though
	g_pd3dDevice->OMSetRenderTargets(1, &PortalRenderTarget, PortalDepthStencilView);

	// Clear the portal texture and its depth buffer
	g_pd3dDevice->ClearRenderTargetView(PortalRenderTarget, &BackgroundColour[0]);
	g_pd3dDevice->ClearDepthStencilView(PortalDepthStencilView, D3D10_CLEAR_DEPTH, 1.0f, 0);

	// Render everything from the portal camera's point of view (into the portal render target [texture] set above)
	RenderMain(PortalCamera);
	


	//---------------------------


	//---------------------------
	// Render portal2 scene

	// Setup the viewport - defines which part of the texture we will render to (usually all of it)
	D3D10_VIEWPORT vp2;
	vp2.Width = PortalWidth;
	vp2.Height = PortalHeight;
	vp2.MinDepth = 0.0f;
	vp2.MaxDepth = 1.0f;
	vp2.TopLeftX = 0;
	vp2.TopLeftY = 0;
	g_pd3dDevice->RSSetViewports(1, &vp2);

	// Select the portal texture to use for rendering, will share the depth/stencil buffer with the backbuffer though
	g_pd3dDevice->OMSetRenderTargets(1, &PortalRenderTarget2, PortalDepthStencilView2);

	// Clear the portal texture and its depth buffer
	g_pd3dDevice->ClearRenderTargetView(PortalRenderTarget2, &BackgroundColour[0]);
	g_pd3dDevice->ClearDepthStencilView(PortalDepthStencilView2, D3D10_CLEAR_DEPTH, 1.0f, 0);


	// Render everything from the portal camera's point of view (into the portal render target [texture] set above)
	RenderMain(Camera);




	//---------------------------


	// Render Shadow scene

	// Setup the viewport - defines which part of the window we will render to, almost always the whole window
	D3D10_VIEWPORT vp;
	vp.Width = ShadowMapSize;
	vp.Height = ShadowMapSize;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pd3dDevice->RSSetViewports(1, &vp);

	// Rendering a single shadow map for a light
	// 1. Select the shadow map texture as the current depth buffer. We will not be rendering any pixel colours
	// 2. Clear the shadow map texture (as a depth buffer)
	// 3. Render everything from point of view of light 0
	g_pd3dDevice->OMSetRenderTargets(0, 0, ShadowMap1DepthView);
	g_pd3dDevice->ClearDepthStencilView(ShadowMap1DepthView, D3D10_CLEAR_DEPTH, 1.0f, 0);
	RenderShadowMap(0);

	// Rendering shadow map for light2
	g_pd3dDevice->OMSetRenderTargets(0, 0, ShadowMap2DepthView);
	g_pd3dDevice->ClearDepthStencilView(ShadowMap2DepthView, D3D10_CLEAR_DEPTH, 1.0f, 0);
	RenderShadowMap(1);

	// Rendering shadow map for light3
	g_pd3dDevice->OMSetRenderTargets(0, 0, ShadowMap3DepthView);
	g_pd3dDevice->ClearDepthStencilView(ShadowMap3DepthView, D3D10_CLEAR_DEPTH, 1.0f, 0);
	RenderShadowMap(2);


	// Setup the viewport - defines which part of the window we will render to, almost always the whole window
	vp.Width = g_ViewportWidth;
	vp.Height = g_ViewportHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pd3dDevice->RSSetViewports(1, &vp);

	// Select the back buffer and depth buffer to use for rendering
	g_pd3dDevice->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);

	// Clear the back buffer  and its depth buffer
	g_pd3dDevice->ClearRenderTargetView(RenderTargetView, &BackgroundColour[0]);
	g_pd3dDevice->ClearDepthStencilView(DepthStencilView, D3D10_CLEAR_DEPTH, 1.0f, 0);

	ViewMatrixVar->SetMatrix(Camera->GetViewMatrix());
	ProjMatrixVar->SetMatrix(Camera->GetProjectionMatrix());
	ViewProjMatrixVar->SetMatrix(Camera->GetViewProjectionMatrix());
	CameraPosVar->SetRawValue(Camera->GetPosition(), 0, 12);

	RenderMirrors();       // Render the mirror and everything seen within it
	RenderMain(Camera);

	//RenderShadowMap(2);

	//---------------------------
	// Display the Scene

	// After we've finished drawing to the off-screen back buffer, we "present" it to the front buffer (the screen)
	SwapChain->Present(0, 0);
}
