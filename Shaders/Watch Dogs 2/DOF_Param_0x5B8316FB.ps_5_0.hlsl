#include "Includes/Common.hlsl"

cbuffer HDREffects : register(b0)
{
  float4 AdditionalBlurParams : packoffset(c0);
  float4 BokehParams : packoffset(c1);
  float4 CoCComputation : packoffset(c2);
  float4 DirectionalBlurUVScaleBias : packoffset(c3);
  float4 SourceSize : packoffset(c4);
  float2 BokehContrast : packoffset(c5);
  float CoCTexelBlurRange : packoffset(c5.z);
  float2 CoCRange : packoffset(c6);
}

SamplerState ColorClamp_s : register(s0);
SamplerState ColorPointClamp2D_s : register(s1);
Texture2D<float4> HDREffects__CoCBuffer__TexObj__ : register(t0);
Texture2D<float4> HDREffects__OriginalSceneTexture__TexObj__ : register(t1);


// 3Dmigoto declarations
#define cmp -


void main(
  linear centroid float2 v0 : TEXCOORD0,
  float4 v1 : SV_Position0,
  out float4 o0 : SV_Target0,
  out float4 o1 : SV_Target1)
{
  float4 r0,r1;
  uint4 bitmask, uiDest;
  float4 fDest;

  r0.xy = v0.xy * float2(0.5,-0.5) + float2(0.5,0.5) + LumaData.GameData.CurrJitters * SourceSize.zw * float2(1, -1);
  float2 uv = v0.xy * float2(0.5,-0.5) + float2(0.5,0.5);
  r0.z = HDREffects__CoCBuffer__TexObj__.SampleLevel(ColorClamp_s, r0.xy, 0, int2(0, -5)).x;
  r0.z = saturate(r0.z);
  r0.z = 0.300000012 * r0.z;
  r0.w = HDREffects__CoCBuffer__TexObj__.SampleLevel(ColorClamp_s, r0.xy, 0, int2(0, -4)).x;
  r0.w = saturate(r0.w);
  r0.w = 0.5 * r0.w;
  r0.z = max(r0.z, r0.w);
  r0.w = HDREffects__CoCBuffer__TexObj__.SampleLevel(ColorClamp_s, r0.xy, 0, int2(0, -3)).x;
  r0.w = saturate(r0.w);
  r0.w = 0.600000024 * r0.w;
  r0.z = max(r0.z, r0.w);
  r0.w = HDREffects__CoCBuffer__TexObj__.SampleLevel(ColorClamp_s, r0.xy, 0, int2(0, -2)).x;
  r0.w = saturate(r0.w);
  r0.w = 0.699999988 * r0.w;
  r0.z = max(r0.z, r0.w);
  r0.w = HDREffects__CoCBuffer__TexObj__.SampleLevel(ColorClamp_s, r0.xy, 0, int2(0, -1)).x;
  r0.w = saturate(r0.w);
  r0.w = 0.899999976 * r0.w;
  r0.z = max(r0.z, r0.w);
  r0.w = HDREffects__CoCBuffer__TexObj__.SampleLevel(ColorClamp_s, r0.xy, 0, int2(0, 1)).x;
  r0.w = saturate(r0.w);
  r0.w = 0.899999976 * r0.w;
  r0.z = max(r0.z, r0.w);
  r0.w = HDREffects__CoCBuffer__TexObj__.SampleLevel(ColorClamp_s, r0.xy, 0, int2(0, 2)).x;
  r0.w = saturate(r0.w);
  r0.w = 0.699999988 * r0.w;
  r0.z = max(r0.z, r0.w);
  r0.w = HDREffects__CoCBuffer__TexObj__.SampleLevel(ColorClamp_s, r0.xy, 0, int2(0, 3)).x;
  r0.w = saturate(r0.w);
  r0.w = 0.600000024 * r0.w;
  r0.z = max(r0.z, r0.w);
  r0.w = HDREffects__CoCBuffer__TexObj__.SampleLevel(ColorClamp_s, r0.xy, 0, int2(0, 4)).x;
  r0.w = saturate(r0.w);
  r0.w = 0.5 * r0.w;
  r0.z = max(r0.z, r0.w);
  r0.w = HDREffects__CoCBuffer__TexObj__.SampleLevel(ColorClamp_s, r0.xy, 0, int2(0, 5)).x;
  r0.w = saturate(r0.w);
  r0.w = 0.300000012 * r0.w;
  r0.z = max(r0.z, r0.w);
  r0.w = HDREffects__CoCBuffer__TexObj__.SampleLevel(ColorClamp_s, r0.xy, 0).x;
  r0.z = max(abs(r0.w), r0.z);
  r0.z = saturate(AdditionalBlurParams.w + r0.z);
  r0.w = BokehParams.z * r0.z;
  o1.z = r0.z;
  r1.xy = uv.xy * DirectionalBlurUVScaleBias.xy + DirectionalBlurUVScaleBias.zw;
  r0.xyz = HDREffects__OriginalSceneTexture__TexObj__.SampleLevel(ColorClamp_s, uv.xy, 0).xyz;
  r0.xyz = log2(abs(r0.xyz));
  r0.xyz = BokehContrast.xxx * r0.xyz;
  o0.xyz = exp2(r0.xyz);
  r0.x = dot(abs(r1.xy), SourceSize.xy);
  o1.xy = float2(8,8) * r1.xy;
  r0.y = max(r0.x, r0.w);
  r0.x = max(r0.x, r0.y);
  r0.x = -0.899999976 + r0.x;
  r0.x = max(0, r0.x);
  o0.w = BokehParams.w + r0.x;
  o1.w = 0;
  return;
}