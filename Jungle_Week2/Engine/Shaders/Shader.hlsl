cbuffer constants : register(b0)
{
    row_major Matrix MVP;
    int UseInstance;
}

cbuffer pickingConstants : register(b1)
{
    row_major Matrix PickingMVP;
    uint ObjectId;
}

// ShaderW0.hlsl
struct VS_INPUT
{
    float4 position : POSITION; // Input position from vertex buffer
    float4 color : COLOR; // Input color from vertex buffer
    
    float4 i0 : INSTANCE0;
    float4 i1 : INSTANCE1;
    float4 i2 : INSTANCE2;
    float4 i3 : INSTANCE3;
};

struct PS_INPUT
{
    float4 position : SV_POSITION; // Transformed position to pass to the pixel shader
    float4 color : COLOR; // Color to pass to the pixel shader
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    
    if (UseInstance)
    {
        float4x4 Model =
        {
            input.i0,
            input.i1,
            input.i2,
            input.i3
        };
        
        output.position = mul(input.position, Model);
        output.position = mul(output.position, MVP);
    }
    else
    {
        output.position = mul(input.position, MVP);
    }
    
    output.color = input.color;
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    // Output the color directly
    return input.color;
}

struct VS_PICKING_OUTPUT
{
    float4 position : SV_POSITION;
};

VS_PICKING_OUTPUT pickingVS(VS_INPUT input)
{
    VS_PICKING_OUTPUT output;
    output.position = mul(input.position, PickingMVP);
    return output;
}

uint pickingPS(VS_PICKING_OUTPUT input) : SV_TARGET
{
    return ObjectId;
}