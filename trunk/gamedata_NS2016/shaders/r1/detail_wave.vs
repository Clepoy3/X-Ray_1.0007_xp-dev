#include "common.h"
struct vf {
float4 hpos:POSITION;
float4 C:COLOR0;
float2 tc:TEXCOORD0;
};
uniform float4 consts;
uniform float4 wave;
uniform float4 dir2D;
uniform float4 array [200]:register(c10);
vf main (v_detail v) {
vf o;
int i = v.misc.w;
float4 m0 = array[i+0];
float4 m1 = array[i+1];
float4 m2 = array[i+2];
float4 c0 = array[i+3];
float4 pos;
pos.x = dot (m0,v.pos);
pos.y = dot (m1,v.pos);
pos.z = dot (m2,v.pos);
pos.w = 1;
float base = m1.w;
float dp = calc_cyclic(dot(pos,wave));
float H = pos.y-base;
float frac = v.misc.z*consts.x;
float inten = H*dp;
float2 result = calc_xz_wave(dir2D.xz*inten,frac);
pos = float4(pos.x+result.x,pos.y,pos.z+result.y,1);
o.hpos = mul(m_WVP,pos);
float dpc = max (0.f,dp);
o.C = c0*(consts.w+consts.z*dpc*frac);
o.tc.xy = (v.misc*consts).xy;
return o;
}