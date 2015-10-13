#include "textured.fx"


//Try things in an isolated space
float4 PSTestTextured(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 color = txDiffuse.Sample(samClampLinear, input.UV);
    return float4((color*(Tint*Tint.a) + color*(1-Tint.a)).rgb, color.a);
}