SamplerState s_wrap_tri_s : register(s2);
Texture2D<float4> t_base : register(t0);

void main(
  float4 v0 : SV_Position0,
  float4 v1 : COLOR0,
  float3 v2 : TEXCOORD0,
  float4 v3 : TEXCOORD1,
  out float4 o0 : SV_Target0)
{
  float4 r0;
  r0.xy = v2.xy / v2.zz;
  r0.xyz = t_base.Sample(s_wrap_tri_s, r0.xy).xyz;
  r0.w = 1;
  o0.xyzw = v1.xyzw * r0.xyzw;
}