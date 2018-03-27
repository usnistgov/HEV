#ifndef rand_glsl
#define rand_glsl

// shader random numbers based on Tiny Encryption Algorithm
//
// Requires #version 130 or later
//
// Externally usable functions:
//   uvec2 tea2(uvec4 key, uvec2 val);
// Two rounds of TEA, sufficient for many visual uses, but not strong
// random numbers

//   uvec2 tea8(uvec4 key, uvec2 val);
// Eight rounds of TEA. Enough to pass testU01 crush randomness test
// suite
//
// Both tea2 and tea8 take two parameters, a 64-bit value to encrypt
// and a 126-bit key. The key is only 126 bits, because the high-order
// bit of key.x and key.z should always be 0. Keys with either of
// these bits set to 1 alias to other key values with them set to 0
// and the high order bits of key.y or key.w flipped. 
//
// The result is 64 bits that are random with respect to changes in
// either the key or value. The key can be used as a seed with val
// used as a counter to generate sequential random numbers, or val can
// be used as a seed with key used as a counter, or key and val can be
// combined together in some other configuration. 
//
// Typical use:
//   randomNumber0 = tea2(uv4seed, uvec2(0,0))
//   randomNumber0 = tea2(uv4seed, uvec2(0,1))
//   randomNumber0 = tea2(uv4seed, uvec2(0,2))
// or
//   randomNumber0 = tea2(uvec4(0,0,0,0),uv2seed)
//   randomNumber0 = tea2(uvec4(0,0,0,1),uv2seed)
//   randomNumber0 = tea2(uvec4(0,0,0,2),uv2seed)
//
// The tea2 function doesn't have enough rounds for full mixing, so
// sequential counter values won't fully propagate to random
// differences in the high-order bits of the result, but is good
// enough for many visual applications where only a few low-order
// random bits are needed. The tea8 function has enough rounds to pass
// the testU01 crush randomness test suite, so should be good enough
// for any purpose.


// two-round TEA
uvec2 tea2(uvec4 key, uvec2 val) {
    uvec2 v = val;
    v.x += ((v.y<<4u)+key.x)^(v.y+0x9E3779B9u)^((v.y>>5u)+key.y);  // 1
    v.y += ((v.x<<4u)+key.z)^(v.x+0x9E3779B9u)^((v.x>>5u)+key.w);
    v.x += ((v.y<<4u)+key.x)^(v.y+0x3C6EF372u)^((v.y>>5u)+key.y);  // 2
    v.y += ((v.x<<4u)+key.z)^(v.x+0x3C6EF372u)^((v.x>>5u)+key.w);
    return v;
}

// eight-round TEA
uvec2 tea8(uvec4 key, uvec2 val) {
    uvec2 v = val;
    v.x += ((v.y<<4u)+key.x)^(v.y+0x9E3779B9u)^((v.y>>5u)+key.y);  // 1
    v.y += ((v.x<<4u)+key.z)^(v.x+0x9E3779B9u)^((v.x>>5u)+key.w);
    v.x += ((v.y<<4u)+key.x)^(v.y+0x3C6EF372u)^((v.y>>5u)+key.y);  // 2
    v.y += ((v.x<<4u)+key.z)^(v.x+0x3C6EF372u)^((v.x>>5u)+key.w);
    v.x += ((v.y<<4u)+key.x)^(v.y+0xDAA66D2Bu)^((v.y>>5u)+key.y);  // 3
    v.y += ((v.x<<4u)+key.z)^(v.x+0xDAA66D2Bu)^((v.x>>5u)+key.w);
    v.x += ((v.y<<4u)+key.x)^(v.y+0x78DDE6E4u)^((v.y>>5u)+key.y);  // 4
    v.y += ((v.x<<4u)+key.z)^(v.x+0x78DDE6E4u)^((v.x>>5u)+key.w);
    v.x += ((v.y<<4u)+key.x)^(v.y+0x1715609Du)^((v.y>>5u)+key.y);  // 5
    v.y += ((v.x<<4u)+key.z)^(v.x+0x1715609Du)^((v.x>>5u)+key.w);
    v.x += ((v.y<<4u)+key.x)^(v.y+0xB54CDA56u)^((v.y>>5u)+key.y);  // 6
    v.y += ((v.x<<4u)+key.z)^(v.x+0xB54CDA56u)^((v.x>>5u)+key.w);
    v.x += ((v.y<<4u)+key.x)^(v.y+0x5384540Fu)^((v.y>>5u)+key.y);  // 7
    v.y += ((v.x<<4u)+key.z)^(v.x+0x5384540Fu)^((v.x>>5u)+key.w);
    v.x += ((v.y<<4u)+key.x)^(v.y+0xF1BBCDC8u)^((v.y>>5u)+key.y);  // 8
    v.y += ((v.x<<4u)+key.z)^(v.x+0xF1BBCDC8u)^((v.x>>5u)+key.w);
    return v;
}

#endif
