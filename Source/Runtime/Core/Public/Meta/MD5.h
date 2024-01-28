#pragma once

#include "Meta/Aliases.h"
#include "Meta/Algorithm.h"
#include "IO/StringView.h"

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Result of an md5 calculation - conventionally an md5 string is hex bytes
// from least significant to most significant.
// https://github.com/elbeno/constexpr/blob/master/src/include/cx_md5
//
// This constexpr implementation is here to provide better safety for hash
// collision than hash_XXX_constexpr() which only uses 32 or 64 bits.
//----------------------------------------------------------------------------
struct FMd5sum {
    u32 h[4];

    constexpr operator u128 () const {
        return {
            h[0]|static_cast<u64>(h[1])<<32u,
            h[2]|static_cast<u64>(h[3])<<32u
        };
    }

    constexpr u32& operator[](int i) { return h[i]; }
    constexpr u32 operator[](int i) const { return h[i]; }

    constexpr bool operator ==(const FMd5sum& other) const {
        return (h[0] == other[0] && h[1] == other[1] && h[2] == other[2] && h[3] == other[3]);
    }
    constexpr bool operator !=(const FMd5sum& other) const {
        return (not operator ==(other));
    }

    constexpr bool operator < (const FMd5sum& other) const {
        return LexicographicalCompare(std::begin(h), std::end(h), std::begin(other.h), std::end(other.h));
    }
    constexpr bool operator >=(const FMd5sum& other) const {
        return (not operator <(other));
    }
};
//----------------------------------------------------------------------------
namespace details {
namespace md5 {
// convert char* buffer (fragment) to u32 (little-endian)
constexpr u32 word32le(const char* s, int len) {
    return ((len > 0 ? static_cast<u32>(s[0]) : 0)
      + (len > 1 ? (static_cast<u32>(s[1]) << 8) : 0)
      + (len > 2 ? (static_cast<u32>(s[2]) << 16) : 0)
      + (len > 3 ? (static_cast<u32>(s[3]) << 24) : 0) );
}
// convert char* buffer (complete) to u32 (little-endian)
constexpr u32 word32le(const char* s) {
    return word32le(s, 4);
}
// shift amounts for the 4 rounds of the main function
inline constexpr int r1shift[4] = { 7, 12, 17, 22 };
inline constexpr int r2shift[4] = { 5,  9, 14, 20 };
inline constexpr int r3shift[4] = { 4, 11, 16, 23 };
inline constexpr int r4shift[4] = { 6, 10, 15, 21 };
// magic constants for each round (actually the integer part of
// abs(sin(i)) where i is the step number
inline constexpr u32 r1const[16] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821
};
inline constexpr u32 r2const[16] = {
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a
};
inline constexpr u32 r3const[16] = {
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665
};
inline constexpr u32 r4const[16] = {
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};
// a schedule is the chunk of buffer to work on
struct schedule {
    u32 w[16];
};
// add two md5sums
constexpr FMd5sum sumadd(const FMd5sum& s1, const FMd5sum& s2) {
    return { { s1[0] + s2[0], s1[1] + s2[1], s1[2] + s2[2], s1[3] + s2[3] } };
}
// the basic MD5 operations
constexpr u32 F(u32 X, u32 Y, u32 Z) {
    return (X & Y) | (~X & Z);
}
constexpr u32 G(u32 X, u32 Y, u32 Z) {
    return (X & Z) | (Y & ~Z);
}
constexpr u32 H(u32 X, u32 Y, u32 Z) {
    return X ^ Y ^ Z;
}
constexpr u32 I(u32 X, u32 Y, u32 Z) {
    return Y ^ (X | ~Z);
}
constexpr u32 rotateL(u32 x, int n) {
    return (x << n) | (x >> (32-n));
}
constexpr u32 FF(u32 a, u32 b, u32 c, u32 d, u32 x, int s, u32 ac){
    return rotateL(a + F(b,c,d) + x + ac, s) + b;
}
constexpr u32 GG(u32 a, u32 b, u32 c, u32 d, u32 x, int s, u32 ac) {
    return rotateL(a + G(b,c,d) + x + ac, s) + b;
}
constexpr u32 HH(u32 a, u32 b, u32 c, u32 d, u32 x, int s, u32 ac) {
    return rotateL(a + H(b,c,d) + x + ac, s) + b;
}
constexpr u32 II(u32 a, u32 b, u32 c, u32 d, u32 x, int s, u32 ac) {
    return rotateL(a + I(b,c,d) + x + ac, s) + b;
}
// initial FMd5sum
constexpr FMd5sum init() {
    return {{ 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476 }};
}
// schedule from an existing buffer
constexpr schedule init(const char* buf) {
    return { { word32le(buf), word32le(buf+4), word32le(buf+8), word32le(buf+12),
        word32le(buf+16), word32le(buf+20), word32le(buf+24), word32le(buf+28),
        word32le(buf+32), word32le(buf+36), word32le(buf+40), word32le(buf+44),
        word32le(buf+48), word32le(buf+52), word32le(buf+56), word32le(buf+60) } };
}
// computing leftovers is messy: we need to pad the empty space to a
// multiple of 64 bytes. the first pad byte is 0x80, the rest are 0.
// the original length (in bits) is the last 8 bytes of padding.
constexpr u32 pad(int len) {
    return (len == 0 ? 0x00000080 :
        len == 1 ? 0x00008000 :
        len == 2 ? 0x00800000 :
        len == 3 ? 0x80000000 :
        0 );
}
constexpr u32 origlenbytes(int origlen, int origlenpos) {
    return (origlenpos == 0 ?
        static_cast<uint64_t>(origlen)*8 & 0xffffffff :
        origlenpos == -4 ?
        (static_cast<uint64_t>(origlen) >> 29) :
        0 );
}
constexpr schedule leftover(const char* buf, int len, int origlen, int origlenpos) {
    return { { word32le(buf, len) | pad(len) | origlenbytes(origlen, origlenpos),
        word32le(len >= 4 ? buf+4 : buf, len-4)
          | pad(len-4) | origlenbytes(origlen, origlenpos-4),
        word32le(len >= 8 ? buf+8 : buf, len-8)
          | pad(len-8) | origlenbytes(origlen, origlenpos-8),
        word32le(len >= 12 ? buf+12 : buf, len-12)
          | pad(len-12) | origlenbytes(origlen, origlenpos-12),
        word32le(len >= 16 ? buf+16 : buf, len-16)
          | pad(len-16) | origlenbytes(origlen, origlenpos-16),
        word32le(len >= 20 ? buf+20 : buf, len-20)
          | pad(len-20) | origlenbytes(origlen, origlenpos-20),
        word32le(len >= 24 ? buf+24 : buf, len-24)
          | pad(len-24) | origlenbytes(origlen, origlenpos-24),
        word32le(len >= 28 ? buf+28 : buf, len-28)
          | pad(len-28) | origlenbytes(origlen, origlenpos-28),
        word32le(len >= 32 ? buf+32 : buf, len-32)
          | pad(len-32) | origlenbytes(origlen, origlenpos-32),
        word32le(len >= 36 ? buf+36 : buf, len-36)
          | pad(len-36) | origlenbytes(origlen, origlenpos-36),
        word32le(len >= 40 ? buf+40 : buf, len-40)
          | pad(len-40) | origlenbytes(origlen, origlenpos-40),
        word32le(len >= 44 ? buf+44 : buf, len-44)
          | pad(len-44) | origlenbytes(origlen, origlenpos-44),
        word32le(len >= 48 ? buf+48 : buf, len-48)
          | pad(len-48) | origlenbytes(origlen, origlenpos-48),
        word32le(len >= 52 ? buf+52 : buf, len-52)
          | pad(len-52) | origlenbytes(origlen, origlenpos-52),
        word32le(len >= 56 ? buf+56 : buf, len-56)
          | pad(len-56) | origlenbytes(origlen, origlenpos-56),
        word32le(len >= 60 ? buf+60 : buf, len-60)
          | pad(len-60) | origlenbytes(origlen, origlenpos-60)} };
}
// compute a step of each round
constexpr FMd5sum round1step(const FMd5sum& sum, const u32* block, int step) {
    return { { FF(sum[0], sum[1], sum[2], sum[3],
        block[step], r1shift[step&3], r1const[step]),
        sum[1], sum[2], sum[3]
        } };
}
constexpr FMd5sum round2step(const FMd5sum& sum, const u32* block, int step) {
    return { { GG(sum[0], sum[1], sum[2], sum[3],
        block[(1+step*5)%16], r2shift[step&3], r2const[step]),
        sum[1], sum[2], sum[3]
        } };
}
constexpr FMd5sum round3step(const FMd5sum& sum, const u32* block, int step) {
    return { { HH(sum[0], sum[1], sum[2], sum[3],
        block[(5+step*3)%16], r3shift[step&3], r3const[step]),
        sum[1], sum[2], sum[3]
        } };
}
constexpr FMd5sum round4step(const FMd5sum& sum, const u32* block, int step) {
    return { { II(sum[0], sum[1], sum[2], sum[3],
        block[(step*7)%16], r4shift[step&3], r4const[step]),
        sum[1], sum[2], sum[3]
        } };
}
// rotate md5sums right and left (each round step does this)
constexpr FMd5sum rotateCR(const FMd5sum& sum) {
    return { { sum[3], sum[0], sum[1], sum[2] } };
}
constexpr FMd5sum rotateCL(const FMd5sum& sum) {
    return { { sum[1], sum[2], sum[3], sum[0] } };
}
// the 4 rounds are each the result of recursively running the respective
// round step (16 times for a block of 64 bytes)
constexpr FMd5sum round1(const FMd5sum& sum, const u32* msg, int n) {
    return (n == 16 ? sum :
        rotateCL(round1(rotateCR(round1step(sum, msg, n)), msg, n+1)) );
}
constexpr FMd5sum round2(const FMd5sum& sum, const u32* msg, int n) {
    return (n == 16 ? sum :
        rotateCL(round2(rotateCR(round2step(sum, msg, n)), msg, n+1)) );
}
constexpr FMd5sum round3(const FMd5sum& sum, const u32* msg, int n) {
    return (n == 16 ? sum :
        rotateCL(round3(rotateCR(round3step(sum, msg, n)), msg, n+1)) );
}
constexpr FMd5sum round4(const FMd5sum& sum, const u32* msg, int n) {
    return (n == 16 ? sum :
        rotateCL(round4(rotateCR(round4step(sum, msg, n)), msg, n+1)) );
}
// the complete transform, for a schedule block
constexpr FMd5sum md5transform(const FMd5sum& sum, const schedule& s) {
    return sumadd(sum, round4(round3(round2( round1(sum, s.w, 0), s.w, 0), s.w, 0), s.w, 0));
}
// three conditions:
// 1. as long as we have a 64-byte block to do, we'll recurse on that
// 2. when we have 56 bytes or more, we need to do a whole empty block to
//    fit the 8 bytes of length after padding
// 3. otherwise we have a block that will fit both padding and the length
constexpr FMd5sum md5update(const FMd5sum& sum, const char* msg, int len, int origlen) {
    return (
        len >= 64 ?
        md5update(md5transform(sum, init(msg)), msg+64, len-64, origlen) :
        len >= 56 ?
        md5update(md5transform(sum, leftover(msg, len, origlen, 64)), msg+len, -1, origlen) :
        md5transform(sum, leftover(msg, len, origlen, 56)) );
}
constexpr FMd5sum md5(const char* msg, int len) {
    return md5update(init(), msg, len, len);
}
} //!namespace md5
} //!namespace details
//----------------------------------------------------------------------------
constexpr FMd5sum md5sum(const char* s, int len) {
    return details::md5::md5(s, len);
}
//----------------------------------------------------------------------------
constexpr FMd5sum md5sum(FStringView str) {
    return md5sum(str.data(), static_cast<int>(str.size()));
}
//----------------------------------------------------------------------------
constexpr void md5sum(FMd5sum& sum, const char* s, int len) {
    sum = details::md5::md5update(sum, s, len, len);
}
//----------------------------------------------------------------------------
constexpr void md5sum(FMd5sum& sum, FStringView str) {
    md5sum(sum, str.data(), static_cast<int>(str.size()));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
