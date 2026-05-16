// GLOBALS //
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

// Texture2D: the texture resource used for rendering the texture on the model
// SamplerState: the sampling(filtering) used for shading the polygon face
Texture2D shaderTexture;
SamplerState SampleType;

// TYPEDEFS //
// TEXCOORD0: thhe semantic for texture coordinates (U, V)
// For multiple texture coordinates, use TEXCOORD1, TEXCOORD2, etc.
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

// Vertex Shader
PixelInputType TextureVertexShader(VertexInputType input)
{
    PixelInputType output;
    
	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;
    
    return output;
}

// Pixel Shader
// The pixel shader has been modified so that it now uses the HLSL sample function. 
// The sample function uses the sampler state we defined above and the texture 
// coordinates for this pixel. It uses these two variables to determine and return 
// the pixel value for this UV location on the polygon face.
float4 TexturePixelShader(PixelInputType input) : SV_TARGET
{
	float4 textureColor;

    // Sample the pixel color from the texture using the sampler at this texture coordinate location.
    textureColor = shaderTexture.Sample(SampleType, input.tex);

    return textureColor;
}
