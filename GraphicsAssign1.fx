

//--------------------------------------------------------------------------------------
// File: GraphicsAssign1.fx
//
//	Shaders Graphics Assignment
//	Add further models using different shader techniques
//	See assignment specification for details
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------

// Standard input geometry data, more complex techniques (e.g. normal mapping) may need more
struct VS_BASIC_INPUT
{
	float3 Pos    : POSITION;
	float3 Normal : NORMAL;
	float2 UV     : TEXCOORD0;
};

struct VS_DEPTH_INPUT
{
	float3 Pos    : POSITION;
	float3 Normal : NORMAL;
	float2 UV     : TEXCOORD0;
};

struct VS_SHADOW_INPUT
{
	float3 Pos    : POSITION;
	float3 Normal : NORMAL;
	float2 UV     : TEXCOORD0;
};

struct VS_NORMALMAP_INPUT
{
	float3 Pos     : POSITION;
	float3 Normal  : NORMAL;
	float3 Tangent : TANGENT;
	float2 UV      : TEXCOORD0;
};

struct VS_PARALLAXMAP_INPUT
{
	float3 Pos     : POSITION;
	float3 Normal  : NORMAL;
	float3 Tangent : TANGENT;
	float2 UV      : TEXCOORD0;
};

// Data output from vertex shader to pixel shader for simple techniques. Again different techniques have different requirements

struct VS_OUTPUT
{
	float4 ProjPos : SV_POSITION;
	float  Colour : COLOR0;
	float2  UV : TEXCOORD0;
};

struct VS_BASIC_OUTPUT
{
	float4 ProjPos		: SV_POSITION;
	float3 DiffuseLight : COLOR0;
	float3 WorldPos		: POSITION;
	float3 WorldNormal  : NORMAL;
	float2 UV			: TEXCOORD0;
	float  ClipDist : SV_ClipDistance;
};

struct VS_WIGGLE_OUTPUT
{
	float4 ProjPos		: SV_POSITION;
	float3 DiffuseLight : COLOR0;
	float3 WorldPos		: POSITION;
	float3 WorldNormal  : NORMAL;
	float2 UV			: TEXCOORD0;
	float  ClipDist : SV_ClipDistance;
};

struct VS_LIGHTING_OUTPUT
{
	float4 ProjPos		: SV_POSITION;
	float3 WorldPos		: POSITION;
	float3 WorldNormal  : NORMAL;
	float2 UV			: TEXCOORD0;
	float  ClipDist : SV_ClipDistance;
};

struct VS_NORMALMAP_OUTPUT
{
	float4 ProjPos      : SV_POSITION;
	float3 WorldPos     : POSITION;
	float3 ModelNormal  : NORMAL;
	float3 ModelTangent : TANGENT;
	float2 UV           : TEXCOORD0;
	float  ClipDist : SV_ClipDistance;
};

struct VS_PARALLAXMAP_OUTPUT
{
	float4 ProjPos      : SV_POSITION;
	float3 WorldPos     : POSITION;
	float3 ModelNormal  : NORMAL;
	float3 ModelTangent : TANGENT;
	float2 UV           : TEXCOORD0;
	float  ClipDist : SV_ClipDistance;
};

struct VS_TEXONLY_OUTPUT
{
	float4 ProjPos : SV_POSITION;  // 2D "projected" position for vertex (required output for vertex shader)
	float2 UV      : TEXCOORD0;
	float  ClipDist : SV_ClipDistance;
};

struct VS_BLACK_OUTLINE_OUTPUT
{
	float4 ProjPos : SV_POSITION;
	float  ClipDist : SV_ClipDistance;
};

struct VS_SHADOW_OUTPUT
{
	float4 ProjPos		: SV_POSITION;
	float3 WorldPos		: POSITION;
	float3 WorldNormal  : NORMAL;
	float2 UV			: TEXCOORD0;
	float  ClipDist : SV_ClipDistance;
};

struct VS_DEPTH_OUTPUT
{
	float4 ProjPos		: SV_POSITION;
	float3 DiffuseLight : COLOR0;
	float3 WorldPos		: POSITION;
	float3 WorldNormal  : NORMAL;
	float2 UV			: TEXCOORD0;
	float  ClipDist : SV_ClipDistance;
};

struct VS_MIRROR_OUTPUT
{
	float4 ProjPos		: SV_POSITION;
	float3 DiffuseLight : COLOR0;
	float3 WorldPos		: POSITION;
	float3 WorldNormal  : NORMAL;
	float2 UV			: TEXCOORD0;
	float  ClipDist : SV_ClipDistance;
};

struct PS_DEPTH_OUTPUT
{
	float4 Colour : SV_Target;
	float  Depth : SV_Depth;
};



//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
// All these variables are created & manipulated in the C++ code and passed into the shader here

// The matrices (4x4 matrix of floats) for transforming from 3D model to 2D projection (used in vertex shader)
float4x4 WorldMatrix;
float4x4 ViewMatrix;
float4x4 ProjMatrix;
float4x4 ViewProjMatrix; // NG.

//NG. Wiggle effect variable
float Wiggle;

// Variable to tint colours
float3 TintColour;

// A single colour for an entire model - used for light models and the intial basic shader
float3 ModelColour;

// Variable used to for constant colour output (cell shading outline and tint for light models)
float3 ConstantColour;
// Controls thickness of outlines for cell shading
float OutlineThickness;

float4 ClipPlane;

//NG. Lighting information
float3	 Light1Pos;
float3   Light1Colour;
float3   Light1Facing;
float4x4 Light1ViewMatrix;
float4x4 Light1ProjMatrix;
float    Light1CosHalfAngle;

float3   Light2Pos;
float3   Light2Colour;
float3   Light2Facing;
float4x4 Light2ViewMatrix;
float4x4 Light2ProjMatrix;
float    Light2CosHalfAngle;

float3   Light3Pos;
float3   Light3Colour;
float3   Light3Facing;
float4x4 Light3ViewMatrix;
float4x4 Light3ProjMatrix;
float    Light3CosHalfAngle;


float3 AmbientColour;
float  SpecularPower;
float3 CameraPos;

// Diffuse texture map (the main texture colour) - may contain specular map in alpha channel
Texture2D DiffuseMap;
Texture2D NormalMap;
Texture2D ShadowMap1;
Texture2D ShadowMap2;
Texture2D ShadowMap3;
Texture2D CellMap;


float ParallaxDepth;

// NG. Sampler
SamplerState TrilinearWrap
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

SamplerState PointClamp
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Clamp;
	AddressV = Clamp;
};

//--------------------------------------------------------------------------------------
// Vertex Shaders
//--------------------------------------------------------------------------------------

// Basic vertex shader to transform 3D model vertices to 2D and pass UVs to the pixel shader
//
VS_BASIC_OUTPUT BasicTransform(VS_BASIC_INPUT vIn)
{
	//// Use world matrix passed from C++ to transform the input model vertex position into world space
	VS_BASIC_OUTPUT vOut;

	// Use world matrix passed from C++ to transform the input model vertex position into world space
	float4 modelPos = float4(vIn.Pos, 1.0f); // Promote to 1x4 so we can multiply by 4x4 matrix, put 1.0 in 4th element for a point (0.0 for a vector)
	float4 worldPos = mul(modelPos, WorldMatrix);
	float4 viewPos = mul(worldPos, ViewMatrix);
	vOut.ProjPos = mul(viewPos, ProjMatrix);

	// Pass texture coordinates (UVs) on to the pixel shader
	vOut.UV = vIn.UV;

	vOut.ClipDist = dot(worldPos, ClipPlane);

	return vOut;
}

VS_DEPTH_OUTPUT DepthTransform(VS_DEPTH_INPUT vIn)
{
	VS_DEPTH_OUTPUT vOut;

	// Use world matrix passed from C++ to transform the input model vertex position into world space
	float4 modelPos = float4(vIn.Pos, 1.0f); // Promote to 1x4 so we can multiply by 4x4 matrix, put 1.0 in 4th element for a point (0.0 for a vector)
	float4 worldPos = mul(modelPos, WorldMatrix);
	float4 viewPos = mul(worldPos, ViewMatrix);
	vOut.ProjPos = mul(viewPos, ProjMatrix);

	// Pass texture coordinates (UVs) on to the pixel shader
	vOut.UV = vIn.UV;

	vOut.ClipDist = dot(worldPos, ClipPlane);

	return vOut;
}

VS_LIGHTING_OUTPUT VertexLightingTex(VS_BASIC_INPUT vIn)
{
	VS_LIGHTING_OUTPUT vOut;

	// Use world matrix passed from C++ to transform the input model vertex position into world space
	float4 modelPos = float4(vIn.Pos, 1.0f); // Promote to 1x4 so we can multiply by 4x4 matrix, put 1.0 in 4th element for a point (0.0 for a vector)
	float4 worldPos = mul(modelPos, WorldMatrix);
	vOut.WorldPos = worldPos.xyz;

	float4 modelnorm = float4(vIn.Normal, 0.0f); // promote to 1x4 (since normal is a vector we add 0.0f)
	float4 worldNormal = mul(modelnorm, WorldMatrix);
	float4 viewPos = mul(worldPos, ViewMatrix);
	vOut.ProjPos = mul(viewPos, ProjMatrix);

	// Transform the vertex normal from model space into world space (almost same as first lines of code above)
	float4 modelNormal = float4(vIn.Normal, 0.0f); // Set 4th element to 0.0 this time as normals are vectors
	vOut.WorldNormal = mul(modelNormal, WorldMatrix).xyz;

	// Pass texture coordinates (UVs) on to the pixel shader, the vertex shader doesn't need them
	vOut.UV = vIn.UV;

	vOut.ClipDist = dot(worldPos, ClipPlane);

	return vOut;
}

VS_WIGGLE_OUTPUT WiggleTransform(VS_BASIC_INPUT vIn)
{
	VS_WIGGLE_OUTPUT vOut;

	float4 modelPos = float4(vIn.Pos, 1.0f); // Promote to 1x4 so we can multiply by 4x4 matrix, put 1.0 in 4th element for a point (0.0 for a vector)
	float4 worldPos = mul(modelPos, WorldMatrix);
	vOut.WorldPos = worldPos.xyz;
	//NG.
	float4 modelnorm = float4(vIn.Normal, 0.0f); // promote to 1x4 (since normal is a vector we add 0.0f)
	float4 worldNormal = mul(modelnorm, WorldMatrix);

	float4 modelNormal = float4(vIn.Normal, 0.0f);
	vOut.WorldNormal = mul(modelNormal, WorldMatrix).xyz;

	

	worldPos.x += sin(modelPos.y + Wiggle) * 0.1f;
	worldPos.y += sin(modelPos.x + Wiggle) * 0.1f;
	worldPos += worldNormal * (sin(Wiggle) + 1.0f) * 0.1f;

	//vOut.ProjPos = mul(worldPos, ViewProjMatrix);
	// todo?
	float4 viewPos = mul(worldPos, ViewMatrix);
	vOut.ProjPos = mul(viewPos, ProjMatrix);

	// Pass texture coordinates (UVs) on to the pixel shader
	vOut.UV = vIn.UV;

	vOut.ClipDist = dot(worldPos, ClipPlane);

	return vOut;
}

VS_WIGGLE_OUTPUT Teapot2DiffuseSpec(VS_BASIC_INPUT vIn)
{
	VS_WIGGLE_OUTPUT vOut;

	float4 modelPos = float4(vIn.Pos, 1.0f); // Promote to 1x4 so we can multiply by 4x4 matrix, put 1.0 in 4th element for a point (0.0 for a vector)
	float4 worldPos = mul(modelPos, WorldMatrix);
	vOut.WorldPos = worldPos.xyz;
	//NG.
	float4 modelnorm = float4(vIn.Normal, 0.0f); // promote to 1x4 (since normal is a vector we add 0.0f)
	float4 worldNormal = mul(modelnorm, WorldMatrix);

	float4 modelNormal = float4(vIn.Normal, 0.0f);
	vOut.WorldNormal = mul(modelNormal, WorldMatrix).xyz;

	float4 viewPos = mul(worldPos, ViewMatrix);
	vOut.ProjPos = mul(viewPos, ProjMatrix);

	// Pass texture coordinates (UVs) on to the pixel shader
	vOut.UV = vIn.UV;

	vOut.ClipDist = dot(worldPos, ClipPlane);

	return vOut;
}

VS_OUTPUT SphereScroll(VS_BASIC_INPUT vIn)
{
	VS_OUTPUT vOut;

	float3 LightColour = { 1.0f, 0.8f, 0.4f };
	float3 LightDir = { 0.707f, 0.707f, -0.707f };

	// Use matrices to transform the geometry to 2D
	float4 worldPos = mul(float4(vIn.Pos, 1.0f), WorldMatrix);
	float4 viewPos = mul(worldPos, ViewMatrix);
	vOut.ProjPos = mul(viewPos, ProjMatrix);


	// Calculate lighting on this vertex
	float3 worldNormal = (float3)mul(vIn.Normal, (float3x3)WorldMatrix);
	float3 diffuseColour = AmbientColour + LightColour * saturate(dot(normalize(worldNormal), LightDir));

	// Lit the Cube (Nikos Gkaltzidis)
	vOut.Colour = diffuseColour;
	//vOut.Colour = mul(float3(vIn.UV, 1.0f), diffuseColour);

	vOut.UV = vIn.UV;
	vOut.UV.x += Wiggle / 5;

	return vOut;
}

VS_NORMALMAP_OUTPUT NormalMapTransform(VS_NORMALMAP_INPUT vIn)
{
	VS_NORMALMAP_OUTPUT vOut;

	// Use world matrix passed from C++ to transform the input model vertex position into world space
	float4 modelPos = float4(vIn.Pos, 1.0f); // Promote to 1x4 so we can multiply by 4x4 matrix, put 1.0 in 4th element for a point (0.0 for a vector)
	float4 worldPos = mul(modelPos, WorldMatrix);
	vOut.WorldPos = worldPos.xyz;

	// Use camera matrices to further transform the vertex from world space into view space (camera's point of view) and finally into 2D "projection" space for rendering
	float4 viewPos = mul(worldPos, ViewMatrix);
	vOut.ProjPos = mul(viewPos, ProjMatrix);

	// Just send the model's normal and tangent untransformed (in model space). The pixel shader will do the matrix work on normals
	vOut.ModelNormal = vIn.Normal;
	vOut.ModelTangent = vIn.Tangent;

	// Pass texture coordinates (UVs) on to the pixel shader, the vertex shader doesn't need them
	vOut.UV = vIn.UV;

	vOut.ClipDist = dot(worldPos, ClipPlane);

	return vOut;
}

VS_PARALLAXMAP_OUTPUT ParallaxMapTransform(VS_PARALLAXMAP_INPUT vIn)
{
	VS_PARALLAXMAP_OUTPUT vOut;

	// Use world matrix passed from C++ to transform the input model vertex position into world space
	float4 modelPos = float4(vIn.Pos, 1.0f); // Promote to 1x4 so we can multiply by 4x4 matrix, put 1.0 in 4th element for a point (0.0 for a vector)
	float4 worldPos = mul(modelPos, WorldMatrix);
	vOut.WorldPos = worldPos.xyz;

	// Use camera matrices to further transform the vertex from world space into view space (camera's point of view) and finally into 2D "projection" space for rendering
	float4 viewPos = mul(worldPos, ViewMatrix);
	
	vOut.ProjPos = mul(viewPos, ProjMatrix);

	// Just send the model's normal and tangent untransformed (in model space). The pixel shader will do the matrix work on normals
	vOut.ModelNormal = vIn.Normal;
	vOut.ModelTangent = vIn.Tangent;

	// Pass texture coordinates (UVs) on to the pixel shader, the vertex shader doesn't need them
	vOut.UV = vIn.UV;

	vOut.ClipDist = dot(worldPos, ClipPlane);

	return vOut;
}

VS_BLACK_OUTLINE_OUTPUT ExpandOutline(VS_BASIC_INPUT vIn)
{
	VS_BLACK_OUTLINE_OUTPUT vOut;

	// Transform model-space vertex position to world-space
	float4 modelPos = float4(vIn.Pos, 1.0f); // Promote to 1x4 so we can multiply by 4x4 matrix, put 1.0 in 4th element for a point (0.0 for a vector)
	float4 worldPos = mul(modelPos, WorldMatrix);

	// Next the usual transform from world space to camera space - but we don't go any further here - this will be used to help expand the outline
	// The result "viewPos" is the xyz position of the vertex as seen from the camera. The z component is the distance from the camera - that's useful...
	float4 viewPos = mul(worldPos, ViewMatrix);

	// Transform model normal to world space, using the normal to expand the geometry, not for lighting
	float4 modelNormal = float4(vIn.Normal, 0.0f); // Set 4th element to 0.0 this time as normals are vectors
	float4 worldNormal = normalize(mul(modelNormal, WorldMatrix)); // Normalise in case of world matrix scaling

	// Now we return to the world position of this vertex and expand it along the world normal - that will expand the geometry outwards.
	// Use the distance from the camera to decide how much to expand. Use this distance together with a sqrt to creates an outline that
	// gets thinner in the distance, but always remains clear. Overall thickness is also controlled by the constant "OutlineThickness"
	worldPos += OutlineThickness * sqrt(viewPos.z) * worldNormal;

	// Transform new expanded world-space vertex position to viewport-space and output
	viewPos = mul(worldPos, ViewMatrix);
	vOut.ProjPos = mul(viewPos, ProjMatrix);

	vOut.ClipDist = dot(worldPos, ClipPlane);

	return vOut;
}

VS_SHADOW_OUTPUT ShadowTransformTex(VS_SHADOW_INPUT vIn)
{
	VS_SHADOW_OUTPUT vOut;

	// Use world matrix passed from C++ to transform the input model vertex position into world space
	float4 modelPos = float4(vIn.Pos, 1.0f); // Promote to 1x4 so we can multiply by 4x4 matrix, put 1.0 in 4th element for a point (0.0 for a vector)
	float4 worldPos = mul(modelPos, WorldMatrix);
	vOut.WorldPos = worldPos.xyz;

	// Use camera matrices to further transform the vertex from world space into view space (camera's point of view) and finally into 2D "projection" space for rendering
	float4 viewPos = mul(worldPos, ViewMatrix);
	vOut.ProjPos = mul(viewPos, ProjMatrix);

	// Transform the vertex normal from model space into world space (almost same as first lines of code above)
	float4 modelNormal = float4(vIn.Normal, 0.0f); // Set 4th element to 0.0 this time as normals are vectors
	vOut.WorldNormal = mul(modelNormal, WorldMatrix).xyz;

	// Pass texture coordinates (UVs) on to the pixel shader, the vertex shader doesn't need them
	vOut.UV = vIn.UV;

	vOut.ClipDist = dot(worldPos, ClipPlane);

	return vOut;
}

//--------------------------------------------------------------------------------------
// Pixel Shaders
//--------------------------------------------------------------------------------------


// A pixel shader that just outputs a single fixed colour

float4 OneColour(VS_BASIC_OUTPUT vOut) : SV_Target
{
	return float4(ModelColour, 1.0); // Set alpha channel to 1.0 (opaque)
}

float4 ScrollTexturePS(VS_OUTPUT vOut) : SV_Target  // The ": SV_Target" bit just indicates that the returned float4 colour goes to the render target (i.e. it's a colour to render)
{

	/*float SinY = sin(vOut.UV.y * radians(360.0f) + Wiggle);
	vOut.UV.x += 0.1f * SinY;
	float SinX = sin(vOut.UV.x * radians(360.0f) + Wiggle);
	vOut.UV.y += 0.1f * SinX;*/
	/*float SinZ = sin(vOut.UV.z * radians(360.0f) + Wiggle);
	vOut.UV.z += 0.1f * SinZ;*/

	float4 TexColour = DiffuseMap.Sample(TrilinearWrap, vOut.UV);
	TexColour += sin(Wiggle);

	float4 colour;

	colour.rgb = (vOut.Colour + TexColour);
	colour.a = 1.0f;
	return colour;
}

float4 OneColourCell(VS_BLACK_OUTLINE_OUTPUT vOut) : SV_Target
{


	return float4(ConstantColour, 1.0); // Set alpha channel to 1.0 (opaque)
}

float4 DiffuseMapOnly(VS_BASIC_OUTPUT vOut) : SV_Target
{
	// Extract diffuse material colour for this pixel from a texture
	float4 diffuseMapColour = DiffuseMap.Sample(TrilinearWrap, vOut.UV);

	return diffuseMapColour;
}

float4 DiffuseMapOnlyMoodle(VS_BASIC_OUTPUT vOut) : SV_Target
{
	// Extract diffuse material colour for this pixel from a texture
	float4 diffuseMapColour = DiffuseMap.Sample(TrilinearWrap, vOut.UV);
	//NG.
	if (diffuseMapColour.a < 0.5)
		discard;
	return diffuseMapColour;
}

float4 TexturedColour(VS_BASIC_OUTPUT vOut) : SV_Target
{
	// NG. Diffuse material colour from a texture
	float4 DiffuseMaterial = DiffuseMap.Sample(TrilinearWrap, vOut.UV);


	float3 worldNormal = normalize(vOut.WorldNormal);
	///////////////////////
	// Calculate lighting

	// Calculate direction of camera
	float3 CameraDir = normalize(CameraPos - vOut.WorldPos.xyz); // Position of camera - position of current vertex (or pixel) (in world space)
	
	//// LIGHT 1
	float3 Light1Dir = normalize(Light1Pos - vOut.WorldPos.xyz);   // Direction for each light is different
	float3 Light1Distance = length(Light1Pos - vOut.WorldPos.xyz); // Calculate Distance of Light1 NG.
	float3 DiffuseLight1 = Light1Colour * saturate(dot(worldNormal.xyz, Light1Dir)) * 1 / Light1Distance;
	float3 halfway = normalize(Light1Dir + CameraDir);
	float3 SpecularLight1 = DiffuseLight1 * pow(saturate(dot(worldNormal.xyz, halfway)), SpecularPower);

	float3 SpecularMaterial = DiffuseMaterial.a;


	float4 CombinedColour;
	CombinedColour.rgb = DiffuseMaterial * DiffuseLight1 + SpecularMaterial * SpecularLight1;
	CombinedColour.a = 1.0;
	return CombinedColour; // Set alpha channel to 1.0 (opaque)
}

float4 DiffuseSpecular(VS_BASIC_OUTPUT vOut) : SV_Target
{
	// NG. Diffuse material colour from a texture
	float4 DiffuseMaterial = DiffuseMap.Sample(TrilinearWrap, vOut.UV);


	float3 worldNormal = normalize(vOut.WorldNormal);
	///////////////////////
	// Calculate lighting

	// Calculate direction of camera
	float3 CameraDir = normalize(CameraPos - vOut.WorldPos.xyz); // Position of camera - position of current vertex (or pixel) (in world space)

	// LIGHT 1
	float3 Light1Dir = normalize(Light1Pos - vOut.WorldPos.xyz);   // Direction for each light is different
	float3 Light1Distance = length(Light1Pos - vOut.WorldPos.xyz); // Calculate Distance of Light1 NG.
	float3 DiffuseLight1 = Light1Colour * saturate(dot(worldNormal.xyz, Light1Dir)) * 1 / Light1Distance;
	float3 halfway = normalize(Light1Dir + CameraDir);
	float3 SpecularLight1 = DiffuseLight1 * pow(saturate(dot(worldNormal.xyz, halfway)), SpecularPower);

	// LIGHT 2
	float3 Light2Dir = normalize(Light2Pos - vOut.WorldPos.xyz);   // Direction for each light is different
	float3 Light2Distance = length(Light2Pos - vOut.WorldPos.xyz); // Calculate Distance of Light1 NG.
	float3 DiffuseLight2 = Light2Colour * saturate(dot(worldNormal.xyz, Light2Dir)) * 1 / Light2Distance;
	halfway = normalize(Light2Dir + CameraDir);
	float3 SpecularLight2 = DiffuseLight2 * pow(saturate(dot(worldNormal.xyz, halfway)), SpecularPower);

	// LIGHT 3
	float3 Light3Dir = normalize(Light3Pos - vOut.WorldPos.xyz);   // Direction for each light is different
	float3 Light3Distance = length(Light3Pos - vOut.WorldPos.xyz); // Calculate Distance of Light1 NG.
	float3 DiffuseLight3 = Light3Colour * saturate(dot(worldNormal.xyz, Light3Dir)) * 1 / Light3Distance;
	halfway = normalize(Light3Dir + CameraDir);
	float3 SpecularLight3 = DiffuseLight3 * pow(saturate(dot(worldNormal.xyz, halfway)), SpecularPower);

	float3 SpecularMaterial = DiffuseMaterial.a;

	float DiffuseLight = DiffuseLight1 + DiffuseLight2 + DiffuseLight3;
	float SpecularLight = SpecularLight1 + SpecularLight2 + SpecularLight3;


	float4 CombinedColour;
	CombinedColour.rgb = DiffuseMaterial * DiffuseLight  + SpecularMaterial * SpecularLight;
	CombinedColour.a = 1.0;
	return CombinedColour; // Set alpha channel to 1.0 (opaque)
}

float4 LightColourPS(VS_LIGHTING_OUTPUT vOut) : SV_Target
{

	float3 worldNormal = normalize(vOut.WorldNormal);
	///////////////////////
	// Calculate lighting

	// Calculate direction of camera
	float3 CameraDir = normalize(CameraPos - vOut.WorldPos.xyz); // Position of camera - position of current vertex (or pixel) (in world space)

	//// LIGHT 1
	float3 Light1Dir = normalize(Light1Pos - vOut.WorldPos.xyz);   // Direction for each light is different
	float3 Light1Distance = length(Light1Pos - vOut.WorldPos.xyz); // Calculate Distance of Light1 NG.
	float3 DiffuseLight1 = Light1Colour * saturate(dot(worldNormal.xyz, Light1Dir)) * 1 / Light1Distance;
	float3 halfway = normalize(Light1Dir + CameraDir);
	float3 SpecularLight1 = DiffuseLight1 * pow(saturate(dot(worldNormal.xyz, halfway)), SpecularPower);

	//// LIGHT 2
	float3 Light2Dir = normalize(Light2Pos - vOut.WorldPos.xyz);
	float3 Light2Distance = length(Light2Pos - vOut.WorldPos.xyz); // Calculate Distance of Light2 NG.
	float3 DiffuseLight2 = Light2Colour * saturate(dot(worldNormal.xyz, Light2Dir)) * 1 / Light2Distance;
	halfway = normalize(Light2Dir + CameraDir);
	float3 SpecularLight2 = DiffuseLight2 * pow(saturate(dot(worldNormal.xyz, halfway)), SpecularPower);

	//// LIGHT 3
	float3 Light3Dir = normalize(Light3Pos - vOut.WorldPos.xyz);
	float3 Light3Distance = length(Light3Pos - vOut.WorldPos.xyz); // Calculate Distance of Light3 NG.
	float3 DiffuseLight3 = Light3Colour * saturate(dot(worldNormal.xyz, Light3Dir)) * 1 / Light3Distance;
	halfway = normalize(Light3Dir + CameraDir);
	float3 SpecularLight3 = DiffuseLight3 * pow(saturate(dot(worldNormal.xyz, halfway)), SpecularPower);

	// Sum the effect of the two lights - add the ambient at this stage rather than for each light (or we will get twice the ambient level)
	float3 DiffuseLight = AmbientColour + DiffuseLight1 + DiffuseLight2 + DiffuseLight3;
	float3 SpecularLight = SpecularLight1 + SpecularLight2 + SpecularLight3;


	////////////////////
	// Sample texture

	// Extract diffuse material colour for this pixel from a texture (using float3, so we get RGB - i.e. ignore any alpha in the texture)
	float4 DiffuseMaterial = DiffuseMap.Sample(TrilinearWrap, vOut.UV);


	// Assume specular material colour is white (i.e. highlights are a full, untinted reflection of light)
	float3 SpecularMaterial = DiffuseMaterial.a;


	////////////////////
	// Combine colours 

	// Combine maps and lighting for final pixel colour
	float4 combinedColour;
	combinedColour.rgb = DiffuseMaterial * DiffuseLight + SpecularMaterial * SpecularLight;
	combinedColour.a = 1.0f; // No alpha processing in this shader, so just set it to 1

	return combinedColour;
}

//NG.  A pixel shader that just tints a (diffuse) texture with a fixed colour
float4 TintDiffuseMap(VS_BASIC_OUTPUT vOut) : SV_Target
{
	// Extract diffuse material colour for this pixel from a texture
	float4 diffuseMapColour = DiffuseMap.Sample(TrilinearWrap, vOut.UV);

	// Tint by global colour (set from C++)
	diffuseMapColour.rgb *= TintColour / 20;

	return diffuseMapColour;
}

float4 NormalMapLighting(VS_NORMALMAP_OUTPUT vOut) : SV_Target
{
	//************************
	// Normal Map Extraction
	//************************

	// Will use the model normal/tangent to calculate matrix for tangent space. The normals for each pixel are *interpolated* from the
	// vertex normals/tangents. This means they will not be length 1, so they need to be renormalised (same as per-pixel lighting issue)
	float3 modelNormal = normalize(vOut.ModelNormal);
	float3 modelTangent = normalize(vOut.ModelTangent);

	// Calculate bi-tangent to complete the three axes of tangent space - then create the *inverse* tangent matrix to convert *from*
	// tangent space into model space. This is just a matrix built from the three axes (very advanced note - by default shader matrices
	// are stored as columns rather than in rows as in the C++. This means that this matrix is created "transposed" from what we would
	// expect. However, for a 3x3 rotation matrix the transpose is equal to the inverse, which is just what we require)
	float3 modelBiTangent = cross(modelNormal, modelTangent);
	float3x3 invTangentMatrix = float3x3(modelTangent, modelBiTangent, modelNormal);

	// Get the texture normal from the normal map. The r,g,b pixel values actually store x,y,z components of a normal. However, r,g,b
	// values are stored in the range 0->1, whereas the x, y & z components should be in the range -1->1. So some scaling is needed
	float3 textureNormal = 2.0f * NormalMap.Sample(TrilinearWrap, vOut.UV) - 1.0f; // Scale from 0->1 to -1->1
	textureNormal.z = textureNormal.z / 20.0f;

	// Now convert the texture normal into model space using the inverse tangent matrix, and then convert into world space using the world
	// matrix. Normalise, because of the effects of texture filtering and in case the world matrix contains scaling
	float3 worldNormal = normalize(mul(mul(textureNormal, invTangentMatrix), WorldMatrix));

	// Now use this normal for lighting calculations in world space as usual - the remaining code same as per-pixel lighting


	///////////////////////
	// Calculate lighting

	// Calculate direction of camera
	float3 CameraDir = normalize(CameraPos - vOut.WorldPos.xyz); // Position of camera - position of current vertex (or pixel) (in world space)

	//// LIGHT 1
	float3 Light1Dir = normalize(Light1Pos - vOut.WorldPos.xyz);   // Direction for each light is different
	float3 Light1Distance = length(Light1Pos - vOut.WorldPos.xyz); // Calculate Distance of Light1 NG.
	float3 DiffuseLight1 = Light1Colour * saturate(dot(worldNormal.xyz, Light1Dir)) * 1 / Light1Distance;
	float3 halfway = normalize(Light1Dir + CameraDir);
	float3 SpecularLight1 = DiffuseLight1 * pow(saturate(dot(worldNormal.xyz, halfway)), SpecularPower);

	//// LIGHT 2
	float3 Light2Dir = normalize(Light2Pos - vOut.WorldPos.xyz);
	float3 Light2Distance = length(Light2Pos - vOut.WorldPos.xyz); // Calculate Distance of Light2 NG.
	float3 DiffuseLight2 = Light2Colour * saturate(dot(worldNormal.xyz, Light2Dir)) * 1 / Light2Distance;
	halfway = normalize(Light2Dir + CameraDir);
	float3 SpecularLight2 = DiffuseLight2 * pow(saturate(dot(worldNormal.xyz, halfway)), SpecularPower);

	//// LIGHT 3
	float3 Light3Dir = normalize(Light3Pos - vOut.WorldPos.xyz);
	float3 Light3Distance = length(Light3Pos - vOut.WorldPos.xyz); // Calculate Distance of Light3 NG.
	float3 DiffuseLight3 = Light3Colour * saturate(dot(worldNormal.xyz, Light3Dir)) * 1 / Light3Distance;
	halfway = normalize(Light3Dir + CameraDir);
	float3 SpecularLight3 = DiffuseLight3 * pow(saturate(dot(worldNormal.xyz, halfway)), SpecularPower);

	// Sum the effect of the two lights - add the ambient at this stage rather than for each light (or we will get twice the ambient level)
	float3 DiffuseLight = AmbientColour + DiffuseLight1 + DiffuseLight2 + DiffuseLight3;
	float3 SpecularLight = SpecularLight1 + SpecularLight2 + SpecularLight3;


	////////////////////
	// Sample texture

	// Extract diffuse material colour for this pixel from a texture (using float3, so we get RGB - i.e. ignore any alpha in the texture)
	float4 DiffuseMaterial = DiffuseMap.Sample(TrilinearWrap, vOut.UV);


	// Assume specular material colour is white (i.e. highlights are a full, untinted reflection of light)
	float3 SpecularMaterial = DiffuseMaterial.a;


	////////////////////
	// Combine colours 

	// Combine maps and lighting for final pixel colour
	float4 combinedColour;
	combinedColour.rgb = DiffuseMaterial * DiffuseLight + SpecularMaterial * SpecularLight;
	combinedColour.a = 1.0f; // No alpha processing in this shader, so just set it to 1

	return combinedColour;
}

float4 ParallaxMapLighting(VS_PARALLAXMAP_OUTPUT vOut) : SV_Target
{
	//************************
	// Normal Map Extraction
	//************************

	// Will use the model normal/tangent to calculate matrix for tangent space. The normals for each pixel are *interpolated* from the
	// vertex normals/tangents. This means they will not be length 1, so they need to be renormalised (same as per-pixel lighting issue)
	float3 modelNormal = normalize(vOut.ModelNormal);
	float3 modelTangent = normalize(vOut.ModelTangent);

	// Calculate bi-tangent to complete the three axes of tangent space - then create the *inverse* tangent matrix to convert *from*
	// tangent space into model space. This is just a matrix built from the three axes (very advanced note - by default shader matrices
	// are stored as columns rather than in rows as in the C++. This means that this matrix is created "transposed" from what we would
	// expect. However, for a 3x3 rotation matrix the transpose is equal to the inverse, which is just what we require)
	float3 modelBiTangent = cross(modelNormal, modelTangent);
	float3x3 invTangentMatrix = float3x3(modelTangent, modelBiTangent, modelNormal);

	//****| INFO |**********************************************************************************//
	// The following few lines are the parallax mapping. Converts the camera direction into model
	// space and adjusts the UVs based on that and the bump depth of the texel we are looking at
	// Although short, this code involves some intricate matrix work / space transformations
	//**********************************************************************************************//

	// Get normalised vector to camera for parallax mapping and specular equation (this vector was calculated later in previous shaders)
	float3 CameraDir = normalize(CameraPos - vOut.WorldPos.xyz);

	// Transform camera vector from world into model space. Need *inverse* world matrix for this.
	// Only need 3x3 matrix to transform vectors, to invert a 3x3 matrix we transpose it (flip it about its diagonal)
	float3x3 invWorldMatrix = transpose(WorldMatrix);
	float3 cameraModelDir = normalize(mul(CameraDir, invWorldMatrix)); // Normalise in case world matrix is scaled

	// Then transform model-space camera vector into tangent space (texture coordinate space) to give the direction to offset texture
	// coordinate, only interested in x and y components. Calculated inverse tangent matrix above, so invert it back for this step
	float3x3 tangentMatrix = transpose(invTangentMatrix);
	float2 textureOffsetDir = mul(cameraModelDir, tangentMatrix);

	// Get the depth info from the normal map's alpha channel at the given texture coordinate
	// Rescale from 0->1 range to -x->+x range, x determined by ParallaxDepth setting
	float texDepth = ParallaxDepth * (NormalMap.Sample(TrilinearWrap, vOut.UV).a - 0.5f);

	// Use the depth of the texture to offset the given texture coordinate - this corrected texture coordinate will be used from here on
	float2 offsetTexCoord = vOut.UV + texDepth * textureOffsetDir;

	//*******************************************

	//****| INFO |**********************************************************************************//
	// The above chunk of code is used only to calculate "offsetTexCoord", which is the offset in 
	// which part of the texture we see at this pixel due to it being bumpy. The remaining code is 
	// exactly the same as normal mapping, but uses offsetTexCoord instead of the usual vOut.UV
	//**********************************************************************************************//

	// Get the texture normal from the normal map. The r,g,b pixel values actually store x,y,z components of a normal. However, r,g,b
	// values are stored in the range 0->1, whereas the x, y & z components should be in the range -1->1. So some scaling is needed
	float3 textureNormal = 2.0f * NormalMap.Sample(TrilinearWrap, offsetTexCoord) - 1.0f; // Scale from 0->1 to -1->1

	textureNormal.z = textureNormal.z / 5.0f;



	// Now convert the texture normal into model space using the inverse tangent matrix, and then convert into world space using the world
	// matrix. Normalise, because of the effects of texture filtering and in case the world matrix contains scaling
	float3 worldNormal = normalize(mul(mul(textureNormal, invTangentMatrix), WorldMatrix));

	// Now use this normal for lighting calculations in world space as usual - the remaining code same as per-pixel lighting


	///////////////////////
	// Calculate lighting

	// Calculate direction of camera
	//float3 CameraDir = normalize(CameraPos - vOut.WorldPos.xyz); // Position of camera - position of current vertex (or pixel) (in world space)

	//// LIGHT 1
	float3 Light1Dir = normalize(Light1Pos - vOut.WorldPos.xyz);   // Direction for each light is different
	float3 Light1Distance = length(Light1Pos - vOut.WorldPos.xyz); // Calculate Distance of Light1 NG.
	float3 DiffuseLight1 = Light1Colour * saturate(dot(worldNormal.xyz, Light1Dir)) * 1 / Light1Distance;
	float3 halfway = normalize(Light1Dir + CameraDir);
	float3 SpecularLight1 = DiffuseLight1 * pow(saturate(dot(worldNormal.xyz, halfway)), SpecularPower);

	//// LIGHT 2
	float3 Light2Dir = normalize(Light2Pos - vOut.WorldPos.xyz);
	float3 Light2Distance = length(Light2Pos - vOut.WorldPos.xyz); // Calculate Distance of Light2 NG.
	float3 DiffuseLight2 = Light2Colour * saturate(dot(worldNormal.xyz, Light2Dir)) * 1 / Light2Distance;
	halfway = normalize(Light2Dir + CameraDir);
	float3 SpecularLight2 = DiffuseLight2 * pow(saturate(dot(worldNormal.xyz, halfway)), SpecularPower);

	//// LIGHT 3
	float3 Light3Dir = normalize(Light3Pos - vOut.WorldPos.xyz);
	float3 Light3Distance = length(Light3Pos - vOut.WorldPos.xyz); // Calculate Distance of Light3 NG.
	float3 DiffuseLight3 = Light3Colour * saturate(dot(worldNormal.xyz, Light3Dir)) * 1 / Light3Distance;
	halfway = normalize(Light3Dir + CameraDir);
	float3 SpecularLight3 = DiffuseLight3 * pow(saturate(dot(worldNormal.xyz, halfway)), SpecularPower);

	// Sum the effect of the two lights - add the ambient at this stage rather than for each light (or we will get twice the ambient level)
	float3 DiffuseLight = AmbientColour + DiffuseLight1 + DiffuseLight2 + DiffuseLight3;
	float3 SpecularLight = SpecularLight1 + SpecularLight2 + SpecularLight3;


	////////////////////
	// Sample texture

	// Extract diffuse material colour for this pixel from a texture (using float3, so we get RGB - i.e. ignore any alpha in the texture)
	float4 DiffuseMaterial = DiffuseMap.Sample(TrilinearWrap, offsetTexCoord);


	// Assume specular material colour is white (i.e. highlights are a full, untinted reflection of light)
	float3 SpecularMaterial = DiffuseMaterial.a;


	////////////////////
	// Combine colours 

	// Combine maps and lighting for final pixel colour
	float4 combinedColour;
	combinedColour.rgb = DiffuseMaterial * DiffuseLight + SpecularMaterial * SpecularLight;
	combinedColour.a = 1.0f; // No alpha processing in this shader, so just set it to 1

	return combinedColour;
}

float4 VertexLitDiffuseMap(VS_LIGHTING_OUTPUT vOut) : SV_Target  // The ": SV_Target" bit just indicates that the returned float4 colour goes to the render target (i.e. it's a colour to render)
{
	// Can't guarantee the normals are length 1 now (because the world matrix may contain scaling), so renormalise
	// If lighting in the pixel shader, this is also because the interpolation from vertex shader to pixel shader will also rescale normals
	float3 worldNormal = normalize(vOut.WorldNormal);


	///////////////////////
	// Calculate lighting

	// Calculate direction of camera
	float3 CameraDir = normalize(CameraPos - vOut.WorldPos.xyz); // Position of camera - position of current vertex (or pixel) (in world space)

	//// LIGHT 1
	float3 Light1Dir = normalize(Light1Pos - vOut.WorldPos.xyz);   // Direction for each light is different
	float3 Light1Dist = length(Light1Pos - vOut.WorldPos.xyz);

	//****| INFO |*************************************************************************************//
	// To make a cartoon look to the lighting, we clamp the basic light level to just a small range of
	// colours. This is done by using the light level itself as the U texture coordinate to look up
	// a colour in a special 1D texture (a single line). This could be done with if statements, but
	// GPUs are much faster at looking up small textures than if statements
	//*************************************************************************************************//
	float DiffuseLevel1 = max(dot(worldNormal.xyz, Light1Dir), 0);
	float CellDiffuseLevel1 = CellMap.Sample(PointClamp, DiffuseLevel1).r;
	float3 DiffuseLight1 = Light1Colour * CellDiffuseLevel1 / Light1Dist;

	// Do same for specular light and further lights
	float3 halfway = normalize(Light1Dir + CameraDir);
	float SpecularLevel1 = pow(max(dot(worldNormal.xyz, halfway), 0), SpecularPower);
	float CellSpecularLevel1 = CellMap.Sample(PointClamp, SpecularLevel1).r;
	float3 SpecularLight1 = DiffuseLight1 * CellSpecularLevel1;


	//// LIGHT 2
	float3 Light2Dir = normalize(Light2Pos - vOut.WorldPos.xyz);
	float3 Light2Dist = length(Light2Pos - vOut.WorldPos.xyz);
	float DiffuseLevel2 = max(dot(worldNormal.xyz, Light2Dir), 0);
	float CellDiffuseLevel2 = CellMap.Sample(PointClamp, DiffuseLevel2).r;
	float3 DiffuseLight2 = Light2Colour * CellDiffuseLevel2 / Light2Dist;

	halfway = normalize(Light2Dir + CameraDir);
	float SpecularLevel2 = pow(max(dot(worldNormal.xyz, halfway), 0), SpecularPower);
	float CellSpecularLevel2 = CellMap.Sample(PointClamp, SpecularLevel2).r;
	float3 SpecularLight2 = DiffuseLight2 * CellSpecularLevel2;

	//// LIGHT 2
	float3 Light3Dir = normalize(Light3Pos - vOut.WorldPos.xyz);
	float3 Light3Dist = length(Light3Pos - vOut.WorldPos.xyz);
	float DiffuseLevel3= max(dot(worldNormal.xyz, Light3Dir), 0);
	float CellDiffuseLevel3 = CellMap.Sample(PointClamp, DiffuseLevel2).r;
	float3 DiffuseLight3 = Light3Colour * CellDiffuseLevel3 / Light3Dist;

	halfway = normalize(Light3Dir + CameraDir);
	float SpecularLevel3 = pow(max(dot(worldNormal.xyz, halfway), 0), SpecularPower);
	float CellSpecularLevel3 = CellMap.Sample(PointClamp, SpecularLevel3).r;
	float3 SpecularLight3 = DiffuseLight3 * CellSpecularLevel3;


	// Sum the effect of the two lights - add the ambient at this stage rather than for each light (or we will get twice the ambient level)
	float3 DiffuseLight = AmbientColour + DiffuseLight1 + DiffuseLight2 + DiffuseLight3;
	float3 SpecularLight = SpecularLight1 + SpecularLight2 + SpecularLight3;


	////////////////////
	// Sample texture

	// Extract diffuse material colour for this pixel from a texture (using float3, so we get RGB - i.e. ignore any alpha in the texture)
	float4 DiffuseMaterial = DiffuseMap.Sample(TrilinearWrap, vOut.UV);

	// Assume specular material colour is white (i.e. highlights are a full, untinted reflection of light)
	float3 SpecularMaterial = DiffuseMaterial.a;


	////////////////////
	// Combine colours 

	// Combine maps and lighting for final pixel colour
	float4 combinedColour;
	combinedColour.rgb = DiffuseMaterial * DiffuseLight + SpecularMaterial * SpecularLight;
	combinedColour.a = 1.0f; // No alpha processing in this shader, so just set it to 1

	return combinedColour;
}


float4 ShadowMapTex(VS_SHADOW_OUTPUT vOut) : SV_Target  // The ": SV_Target" bit just indicates that the returned float4 colour goes to the render target (i.e. it's a colour to render)
{
	// Slight adjustment to calculated depth of pixels so they don't shadow themselves
	const float DepthAdjust = 0.001f;

	// Can't guarantee the normals are length 1 now (because the world matrix may contain scaling), so renormalise
	// If lighting in the pixel shader, this is also because the interpolation from vertex shader to pixel shader will also rescale normals
	float3 worldNormal = normalize(vOut.WorldNormal);

	///////////////////////
	// Calculate lighting

	// Calculate direction of camera
	float3 cameraDir = normalize(CameraPos - vOut.WorldPos.xyz); // Position of camera - position of current vertex (or pixel) (in world space)

	// ----------
	// LIGHT 1

	// Start with no light contribution from this light
	float3 diffuseLight1 = 0;
	float3 specularLight1 = 0;

	// Using the world position of the current pixel and the matrices of the light (as a camera), find the 2D position of the
	// pixel *as seen from the light*. Will use this to find which part of the shadow map to look at.
	// The usual view / projection matrix multiplies as we would see in a vertex shader (can improve performance by putting these lines in vertex shader)
	float4 light1ViewPos = mul(float4(vOut.WorldPos, 1.0f), Light1ViewMatrix);
	float4 light1ProjPos = mul(light1ViewPos, Light1ProjMatrix);

	// Get direction from pixel to light
	float3 light1Dir = normalize(Light1Pos - vOut.WorldPos.xyz);

	// N.G dot product for cone lighting
	float coneDot = dot(Light1Facing, light1Dir);


	// Check if pixel is within light cone
	// N.G -coneDot > 45degrees.
	if (-coneDot > Light1CosHalfAngle) //**** This condition needs to be written as the first exercise to get spotlights working
	{
		// Convert 2D pixel position as viewed from light into texture coordinates for shadow map - an advanced topic related to the projection step
		// Detail: 2D position x & y get perspective divide, then converted from range -1->1 to UV range 0->1. Also flip V axis
		float2 shadowUV = 0.5f * light1ProjPos.xy / light1ProjPos.w + float2(0.5f, 0.5f);
		shadowUV.y = 1.0f - shadowUV.y;

		// Get depth of this pixel if it were visible from the light (another advanced projection step)
		float depthFromLight = light1ProjPos.z / light1ProjPos.w - DepthAdjust; //*** Adjustment so polygons don't shadow themselves

		// Compare pixel depth from light with depth held in shadow map of the light. If shadow map depth is less than something is nearer
		// to the light than this pixel - so the pixel gets no effect from this light
		if (depthFromLight < ShadowMap1.Sample(PointClamp, shadowUV).r)
		{
			// Remainder of standard per-pixel lighting code is unchanged
			float3 light1Dist = length(Light1Pos - vOut.WorldPos.xyz);
			diffuseLight1 = Light1Colour * max(dot(worldNormal.xyz, light1Dir), 0) / light1Dist;
			float3 halfway = normalize(light1Dir + cameraDir);
			specularLight1 = diffuseLight1 * pow(max(dot(worldNormal.xyz, halfway), 0), SpecularPower);
		}
	}



	//----------
	// LIGHT 2

	// Start with no light contribution from this light
	float3 diffuseLight2 = 0;
	float3 specularLight2 = 0;

	// Using the world position of the current pixel and the matrices of the light (as a camera), find the 2D position of the
	// pixel *as seen from the light*. Will use this to find which part of the shadow map to look at.
	// The usual view / projection matrix multiplies as we would see in a vertex shader (can improve performance by putting these lines in vertex shader)
	float4 light2ViewPos = mul(float4(vOut.WorldPos, 1.0f), Light2ViewMatrix);
	float4 light2ProjPos = mul(light2ViewPos, Light2ProjMatrix);

	// Get direction from pixel to light
	float3 light2Dir = normalize(Light2Pos - vOut.WorldPos.xyz);

	// N.G dot product for cone lighting
	float coneDot2 = dot(Light2Facing, light2Dir);


	// Check if pixel is within light cone
	// N.G -coneDot > 45degrees.
	if (-coneDot2 > Light2CosHalfAngle) //**** This condition needs to be written as the first exercise to get spotlights working
	{
		// Convert 2D pixel position as viewed from light into texture coordinates for shadow map - an advanced topic related to the projection step
		// Detail: 2D position x & y get perspective divide, then converted from range -1->1 to UV range 0->1. Also flip V axis
		float2 shadowUV = 0.5f * light2ProjPos.xy / light2ProjPos.w + float2(0.5f, 0.5f);
		shadowUV.y = 1.0f - shadowUV.y;

		// Get depth of this pixel if it were visible from the light (another advanced projection step)
		float depthFromLight = light2ProjPos.z / light2ProjPos.w - DepthAdjust; //*** Adjustment so polygons don't shadow themselves

		// Compare pixel depth from light with depth held in shadow map of the light. If shadow map depth is less than something is nearer
		// to the light than this pixel - so the pixel gets no effect from this light
		if (depthFromLight < ShadowMap2.Sample(PointClamp, shadowUV).r)
		{
			// Remainder of standard per-pixel lighting code is unchanged
			float3 light2Dist = length(Light2Pos - vOut.WorldPos.xyz);
			diffuseLight2 = Light2Colour * max(dot(worldNormal.xyz, light2Dir), 0) / light2Dist;
			float3 halfway = normalize(light2Dir + cameraDir);
			specularLight2 = diffuseLight2 * pow(max(dot(worldNormal.xyz, halfway), 0), SpecularPower);
		}
	}

	//----------
	// LIGHT 3

	// Start with no light contribution from this light
	float3 diffuseLight3 = 0;
	float3 specularLight3 = 0;

	// Using the world position of the current pixel and the matrices of the light (as a camera), find the 2D position of the
	// pixel *as seen from the light*. Will use this to find which part of the shadow map to look at.
	// The usual view / projection matrix multiplies as we would see in a vertex shader (can improve performance by putting these lines in vertex shader)
	float4 light3ViewPos = mul(float4(vOut.WorldPos, 1.0f), Light3ViewMatrix);
	float4 light3ProjPos = mul(light3ViewPos, Light3ProjMatrix);

	// Get direction from pixel to light
	float3 light3Dir = normalize(Light3Pos - vOut.WorldPos.xyz);

	// N.G dot product for cone lighting
	float coneDot3 = dot(Light3Facing, light3Dir);


	// Check if pixel is within light cone
	// N.G -coneDot > 45degrees.
	if (-coneDot3 > Light3CosHalfAngle) //**** This condition needs to be written as the first exercise to get spotlights working
	{
		// Convert 2D pixel position as viewed from light into texture coordinates for shadow map - an advanced topic related to the projection step
		// Detail: 2D position x & y get perspective divide, then converted from range -1->1 to UV range 0->1. Also flip V axis
		float2 shadowUV = 0.5f * light3ProjPos.xy / light3ProjPos.w + float2(0.5f, 0.5f);
		shadowUV.y = 1.0f - shadowUV.y;

		// Get depth of this pixel if it were visible from the light (another advanced projection step)
		float depthFromLight = light3ProjPos.z / light3ProjPos.w - DepthAdjust; //*** Adjustment so polygons don't shadow themselves

		// Compare pixel depth from light with depth held in shadow map of the light. If shadow map depth is less than something is nearer
		// to the light than this pixel - so the pixel gets no effect from this light
		if (depthFromLight < ShadowMap3.Sample(PointClamp, shadowUV).r)
		{
			// Remainder of standard per-pixel lighting code is unchanged
			float3 light3Dist = length(Light3Pos - vOut.WorldPos.xyz);
			diffuseLight3 = Light3Colour * max(dot(worldNormal.xyz, light3Dir), 0) / light3Dist;
			float3 halfway = normalize(light3Dir + cameraDir);
			specularLight3 = diffuseLight3 * pow(max(dot(worldNormal.xyz, halfway), 0), SpecularPower);
		}
	}



	// Sum the effect of the two lights - add the ambient at this stage rather than for each light (or we will get twice the ambient level)
	float3 diffuseLight = AmbientColour + diffuseLight1 + diffuseLight2 + diffuseLight3;
	float3 specularLight = specularLight1 + specularLight2 + specularLight3;


	////////////////////
	// Sample texture

	// Extract diffuse material colour for this pixel from a texture
	float4 diffuseMaterial = DiffuseMap.Sample(TrilinearWrap, vOut.UV);

	// Get specular material colour from texture alpha
	float3 specularMaterial = diffuseMaterial.a;


	////////////////////
	// Combine colours 

	// Combine maps and lighting for final pixel colour
	float4 combinedColour;
	combinedColour.rgb = diffuseMaterial * diffuseLight + specularMaterial * specularLight;
	combinedColour.a = 1.0f; // No alpha processing in this shader, so just set it to 1

	return combinedColour;
}

float4 PixelDepth(VS_DEPTH_OUTPUT vOut) : SV_Target
{
	// Output the value that would go in the depth puffer to the pixel colour (greyscale)
	return vOut.ProjPos.z / vOut.ProjPos.w;
}

float4 PlainColourMirror(VS_BASIC_OUTPUT vOut) : SV_Target
{
	return float4(ConstantColour, 1.0f);
}

PS_DEPTH_OUTPUT ClearDepth(VS_MIRROR_OUTPUT vOut)
{
	PS_DEPTH_OUTPUT pOut;

	pOut.Colour = float4(ConstantColour, 1.0f);
	pOut.Depth = 1.0f;

	return pOut;
}




//--------------------------------------------------------------------------------------
// States
//--------------------------------------------------------------------------------------
RasterizerState CullNone  // Cull none of the polygons, i.e. show both sides
{
	CullMode = None;
};

RasterizerState CullBack  // Cull back side of polygon - normal behaviour, only show front of polygons
{
	CullMode = Back;
};

RasterizerState CullFront  // Cull back side of polygon - normal behaviour, only show front of polygons
{
	CullMode = Front;
};

DepthStencilState DepthWritesOn  // Write to the depth buffer - normal behaviour 
{
	DepthFunc = Less;  // Must remember reset states that are changed in other techniques - we use the depth buffer normally here
	DepthWriteMask = All;
	StencilEnable = False; // Also switch off states we don't need from other techniques
};

DepthStencilState DepthWritesOff // Don't write to the depth buffer - polygons rendered will not obscure other polygons
{
	DepthFunc = Less;
	DepthWriteMask = Zero;
	StencilEnable = False;
};



//////////////////////////
// Stencil buffer use

// Set a specific stencil value in the rendered area - used to initialise the stencil area of the mirror
DepthStencilState SetStencilValue
{
	// Use depth buffer normally
	DepthFunc = Less;
	DepthWriteMask = All;

	// Enable stencil buffer and replace all pixel stencil values with the reference value (value specified in the technique)
	StencilEnable = True;
	FrontFaceStencilFunc = Always;  // Always...
	FrontFaceStencilPass = Replace; // ...replace the stencil values
	BackFaceStencilFunc = Always;
	BackFaceStencilPass = Replace;
};

// Only render to the area with a given stencil value, used for rendering mirror content (ensures we don't rendered outside area of the mirror)
DepthStencilState AffectStencilArea
{
	// Use depth buffer normally
	DepthFunc = Less;
	DepthWriteMask = All;

	// Only render those pixels whose stencil value is equal to the reference value (value specified in the technique)
	StencilEnable = True;
	FrontFaceStencilFunc = Equal; // Only render on matching stencil
	FrontFaceStencilPass = Keep;  // But don't change the stencil values
	BackFaceStencilFunc = Equal;
	BackFaceStencilPass = Keep;
};

// Only render to the area with a given stencil value but don't write to depth-buffer, used for rendering transparent polygons within the mirror (similar to state above)
DepthStencilState AffectStencilAreaDepthWritesOff
{
	// Test the depth buffer, but don't write anything new to it
	DepthFunc = Less;
	DepthWriteMask = Zero;

	// Only render those pixels whose stencil value is equal to the reference value (value specified in the technique)
	StencilEnable = True;
	FrontFaceStencilFunc = Equal; // Only render on matching stencil
	FrontFaceStencilPass = Keep;  // But don't change the stencil values
	BackFaceStencilFunc = Equal;
	BackFaceStencilPass = Keep;
};

// Only render to the area with a given stencil value and completely ignore the depth buffer - used to clear the mirror before rendering the scene inside it
DepthStencilState AffectStencilAreaIgnoreDepth
{
	// Disable depth buffer - we're going to fill the mirror/portal with distant z-values as we will want to render a new scene in there. So we must
	// ignore the z-values of the mirror surface - or the depth buffer would think the mirror polygon was obscuring our distant z-values
	DepthFunc = Always;

	// Only render those pixels whose stencil value is equal to the reference value (value specified in the technique)
	StencilEnable = True;
	FrontFaceStencilFunc = Equal; // Only render on matching stencil
	FrontFaceStencilPass = Keep;  // But don't change the stencil values
	BackFaceStencilFunc = Equal;
	BackFaceStencilPass = Keep;
};





BlendState NoBlending // Switch off blending - pixels will be opaque
{
	BlendEnable[0] = FALSE;
};

BlendState CullFrontState
{
	BlendEnable[0] = TRUE;
};

BlendState AdditiveBlending // Additive blending is used for lighting effects
{
	BlendEnable[0] = TRUE;
	SrcBlend = ONE;
	DestBlend = ONE;
	BlendOp = ADD;
};

BlendState MultiplicativeBlending
{
	BlendEnable[0] = TRUE;
	SrcBlend = SRC_COLOR;
	DestBlend = DEST_COLOR;
	BlendOp = ADD;
};

BlendState AlphaBlending
{
	BlendEnable[0] = TRUE;
	SrcBlend = SRC_ALPHA;
	DestBlend = INV_SRC_ALPHA;
	BlendOp = ADD;
};

BlendState NoColourOutput // Use blending to prevent drawing pixel colours, but still allow depth/stencil updates
{
	BlendEnable[0] = TRUE;
	SrcBlend = ZERO;
	DestBlend = ONE;
	BlendOp = ADD;
};

//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------

// Techniques are used to render models in our scene. They select a combination of vertex, geometry and pixel shader from those provided above. Can also set states.

// Render models unlit in a single colour
technique10 PlainColour
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, BasicTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, OneColour()));
	}
}

technique10 DiffuseMapOnlyTex
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, BasicTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, DiffuseMapOnly()));

		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullNone);
		SetDepthStencilState(DepthWritesOn, 0);
	}
};

technique10 WiggleTex
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, WiggleTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, TexturedColour()));

		// Switch off blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullNone);
		SetDepthStencilState(DepthWritesOn, 0);
	}
}

technique10 SphereTex
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, SphereScroll()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, ScrollTexturePS()));

		//// Switch off blending states
		//SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		//SetRasterizerState(CullNone);
		//SetDepthStencilState(DepthWritesOn, 0);
	}
}

technique10 DiffuseSpecularTex
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, Teapot2DiffuseSpec()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, DiffuseSpecular()));

		// Switch off blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullNone);
		SetDepthStencilState(DepthWritesOn, 0);
	}
}




technique10 Light1Tex
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, VertexLightingTex()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, LightColourPS()));

		// Switch off blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullBack);
		SetDepthStencilState(DepthWritesOn, 0);
	}
}

technique10 AdditiveTexTint
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, BasicTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, TintDiffuseMap()));

		SetBlendState(AdditiveBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullNone);
		SetDepthStencilState(DepthWritesOff, 0);
	}
}

technique10 ShadowTex
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, ShadowTransformTex()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, ShadowMapTex()));

		// Switch off blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullBack);
		SetDepthStencilState(DepthWritesOn, 0);
	}

}

technique10 NormalMapping
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, NormalMapTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, NormalMapLighting()));

		// Switch off blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullBack);
		SetDepthStencilState(DepthWritesOn, 0);
	}
}

technique10 ParallaxMapping
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, ParallaxMapTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, ParallaxMapLighting()));

		// Switch off blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullBack);
		SetDepthStencilState(DepthWritesOn, 0);
	}

	//pass P1
	//{
	//	SetVertexShader(CompileShader(vs_4_0, ShadowTransformTex()));
	//	SetGeometryShader(NULL);
	//	SetPixelShader(CompileShader(ps_4_0, ShadowMapTex()));

	//	// Switch off blending states
	//	SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	//	SetRasterizerState(CullFront);
	//	SetDepthStencilState(DepthWritesOn, 0);
	//}
}

technique10 CellShading
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, ExpandOutline()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, OneColourCell()));

		// Cull (remove) the polygons facing us - i.e. draw the inside of the model
		SetRasterizerState(CullFront);

		// Switch off other blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetDepthStencilState(DepthWritesOn, 0);
	}
	pass P1
	{
		SetVertexShader(CompileShader(vs_4_0, VertexLightingTex()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, VertexLitDiffuseMap()));

		// Return to standard culling (remove polygons facing away from us)
		SetRasterizerState(CullBack);
	}
}

technique10 DepthOnly
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, DepthTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PixelDepth()));

		// Switch off blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullBack);
		SetDepthStencilState(DepthWritesOn, 0);
	}
}

technique10 BlendedTexture
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, BasicTransform())); 
		SetGeometryShader(NULL);                                    
		SetPixelShader(CompileShader(ps_4_0, DiffuseMapOnlyMoodle()));

		/******************************/
		/* Select states for blending */
		/******************************/
		

		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		//SetBlendState( AdditiveBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		//SetBlendState(CullFrontState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		//SetBlendState(MultiplicativeBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		//SetBlendState(AlphaBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullNone);
		//SetDepthStencilState(DepthWritesOn, 0);
		//SetDepthStencilState(DepthWritesOff, 0);
	}
}


// ------ Mirror Techniques ------ // 
// 
// 
// Vertex lighting with diffuse map

technique10 BlendedMirrorTexture
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, BasicTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, DiffuseMapOnlyMoodle()));

		/******************************/
		/* Select states for blending */
		/******************************/


		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		//SetBlendState( AdditiveBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		//SetBlendState(CullFrontState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		//SetBlendState(MultiplicativeBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		//SetBlendState(AlphaBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullNone);
		//SetDepthStencilState(DepthWritesOn, 0);
		SetDepthStencilState(AffectStencilAreaDepthWritesOff, 1);
	}
}

technique10 NormalMirrorMapping
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, NormalMapTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, NormalMapLighting()));

		// Switch off blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullFront);
		SetDepthStencilState(AffectStencilAreaDepthWritesOff, 1);
	}
}

technique10 ShadowTexMirror
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, ShadowTransformTex()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, ShadowMapTex()));

		// Switch off blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		// Reverse culling - the mirror will reverse the clockwise/anti-clockwise ordering of the polygons
		SetRasterizerState(CullFront);
		// Only render in stencil area (the mirror surface)
		SetDepthStencilState(AffectStencilArea, 1);
	}

}

technique10 WiggleTexMirror
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, WiggleTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, TexturedColour()));

		// Switch off blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullNone);
		SetDepthStencilState(AffectStencilArea, 1);
	}
}

technique10 SphereMirrorTex
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, SphereScroll()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, ScrollTexturePS()));

		//// Switch off blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullNone);
		SetDepthStencilState(AffectStencilArea, 1);
	}
}

technique10 ParallaxMappingMirror
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, ParallaxMapTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, ParallaxMapLighting()));

		// No blending
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);

		// Reverse culling - the mirror will reverse the clockwise/anti-clockwise ordering of the polygons
		SetRasterizerState(CullFront);

		// Only render in stencil area (the mirror surface)
		SetDepthStencilState(AffectStencilArea, 1);
	}
}

// Additive blended texture. No lighting, but uses a global colour tint. Used for light models
technique10 AdditiveTexTintMirror
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, BasicTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, TintDiffuseMap()));

		// Additive blending states plus only rendering to stencil area
		SetBlendState(AdditiveBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullNone);
		SetDepthStencilState(AffectStencilAreaDepthWritesOff, 1);
	}
}



technique10 CellShadingMirror
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, ExpandOutline()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, OneColourCell()));

		// Cull (remove) the polygons facing us - i.e. draw the inside of the model
		SetRasterizerState(CullFront);

		// Switch off other blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetDepthStencilState(AffectStencilAreaDepthWritesOff, 1);
	}
	pass P1
	{
		SetVertexShader(CompileShader(vs_4_0, VertexLightingTex()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, VertexLitDiffuseMap()));

		// Return to standard culling (remove polygons facing away from us)
		SetRasterizerState(CullFront);
	}
}


technique10 MirrorClear
{
	pass P0 // Set the stencil values to 1, do nothing to the pixels
	{
		SetVertexShader(CompileShader(vs_4_0, BasicTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PlainColourMirror()));

		// Switch off colour output (only updating stencil in this pass), normal culling
		SetBlendState(NoColourOutput, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullBack);

		// Set the stencil to 1 in the visible area of this polygon
		SetDepthStencilState(SetStencilValue, 1);
	}
	pass P1 // Set the pixel colour to background colour, set z-value to furthest distance - but do this only where stencil was set to 1 above
	{
		SetVertexShader(CompileShader(vs_4_0, BasicTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, ClearDepth()));

		// Switch off blending, normal culling
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullBack);

		// Only affect the area where the stencil was set to 1 in the pass above
		SetDepthStencilState(AffectStencilAreaIgnoreDepth, 1);
	}
}

technique10 MirrorSurface
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, BasicTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PlainColourMirror()));

		// Use a special blending state to disable any changes to the viewport pixels as we're only updating the stencil/depth
		// buffer in this pass (for now).
		SetBlendState(MultiplicativeBlending, float4(ConstantColour.r, ConstantColour.g, ConstantColour.b, 0.0f), 0xFFFFFFFF);

		// Standard culling
		SetRasterizerState(CullBack);

		// Set the stencil back to 0 in the surface of the polygon
		SetDepthStencilState(SetStencilValue, 0);
	}
}


