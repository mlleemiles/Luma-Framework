SamplerState s_clamp_bi_s : register(s6);
Texture2D<float4> t_fx1 : register(t0);
Texture2D<float4> t_fb2 : register(t1);
Texture2D<float4> t_fb4 : register(t2);
Texture2D<float4> t_fb8 : register(t3);
Texture2D<float4> t_fb16 : register(t4);

void main(
  float4 v0 : SV_Position0,
  out float4 o0 : SV_Target0)
{
  float4 r0,r1,r2;
  float4 fDest;
  t_fx1.GetDimensions(0, fDest.x, fDest.y, fDest.z);
  r0.xy = fDest.xy;
  r0.xy = v0.xy / r0.xy;
  r1.xyzw = t_fb16.Sample(s_clamp_bi_s, r0.xy).xyzw;
  r2.xyzw = t_fb8.Sample(s_clamp_bi_s, r0.xy).xyzw;
  r1.xyzw = r2.xyzw + r1.xyzw;
  r2.xyzw = t_fb4.Sample(s_clamp_bi_s, r0.xy).xyzw;
  r0.xyzw = t_fb2.Sample(s_clamp_bi_s, r0.xy).xyzw;
  r1.xyzw = r2.xyzw + r1.xyzw;
  o0.xyzw = r1.xyzw + r0.xyzw;
}