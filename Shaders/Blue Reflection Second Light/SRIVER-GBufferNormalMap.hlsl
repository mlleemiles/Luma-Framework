// ---- Created with 3Dmigoto v1.3.16 on Mon May 11 10:21:01 2026

cbuffer _Globals : register(b0)
{
  float4 vEye : packoffset(c0);
  float4 mWV[3] : packoffset(c1);
  float4 vAlbedoColor : packoffset(c4);
  float4 vReflectance : packoffset(c5);
  float4 vAmbParam : packoffset(c6);
  float4 mRrW2T[3] : packoffset(c7);
  float fRrRefractiveIndex : packoffset(c10);
  float fRrTransparency : packoffset(c10.y);
  float4 vRrParam : packoffset(c11);
  float2 vD2Z : packoffset(c12);
  float4 vRrCameraPos : packoffset(c13);
  float4 vRrCameraDir : packoffset(c14);
}

SamplerState __smpsRiverNormalMap_s : register(s0);
Texture2D<float4> sRiverNormalMap : register(t0);


// 3Dmigoto declarations
#define cmp -


void main(
  float4 v0 : SV_Position0,
  float4 v1 : TEXCOORD0,
  float4 v2 : TEXCOORD1,
  float2 v3 : TEXCOORD2,
  out float4 o0 : SV_Target0,
  out float4 o1 : SV_Target1,
  out float4 o2 : SV_Target2,
  out float4 o3 : SV_Target3,
  out float4 o4 : SV_Target4,
  out float2 o5 : SV_Target5)
{
  float4 r0,r1,r2;
  uint4 bitmask, uiDest;
  float4 fDest;

  o1.xyz = vReflectance.xyz;
  o1.w = 0.125489995;
  r0.xy = sRiverNormalMap.Sample(__smpsRiverNormalMap_s, v3.xy).xy;
  r0.xy = r0.xy * float2(2,2) + v2.xz;
  r0.xz = float2(-1,-1) + r0.xy;
  r0.y = v2.y;
  r0.w = dot(r0.xyz, r0.xyz);
  r0.w = rsqrt(r0.w);
  r0.xyz = r0.xyz * r0.www;
  r1.y = dot(mWV[0].xyz, r0.xyz);
  r1.z = dot(mWV[1].xyz, r0.xyz);
  r1.x = dot(mWV[2].xyz, r0.xyz);
  r0.x = abs(r1.y) + abs(r1.z);
  r0.x = r0.x + abs(r1.x);
  r0.xyz = r1.xyz / r0.xxx;
  r1.xy = float2(1,1) + -abs(r0.zy);
  r2.xyz = cmp(r0.xyz >= float3(0,0,0));
  r0.xw = r2.yz ? float2(1,1) : float2(-1,-1);
  r0.xw = r1.xy * r0.xw;
  r0.xy = r2.xx ? r0.yz : r0.xw;
  o2.xy = r0.xy * float2(0.5,0.5) + float2(0.5,0.5);
  r0.x = 255 * vReflectance.w;
  r0.x = (uint)r0.x;
  r0.x = (uint)r0.x << 8;
  r0.x = (uint)r0.x;
  o2.w = 1.52590219e-005 * r0.x;
  o2.z = 0;
  o3.xy = fRrRefractiveIndex;
  o3.zw = float2(1,0);
  o4.xyzw = float4(1,1,1,1);
  o0.xyz = vAlbedoColor.xyz;
  o0.w = 1;
  
  // Clear velocity on water
  o5.xy = 0.f;
  return;
}