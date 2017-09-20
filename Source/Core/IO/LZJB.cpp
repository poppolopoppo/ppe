#include "stdafx.h"

#include "LZJB.h"

#include "Diagnostic/Logger.h"
#include "IO/BufferedStreamProvider.h"
#include "IO/FormatHelpers.h"
#include "Memory/MemoryView.h"
#include "Misc/FourCC.h"

// http://stackoverflow.com/questions/1077496/what-is-a-small-and-fast-real-time-compression-technique-like-lz77

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// LZJB  https://hg.java.net/hg/solaris~on-src/file/tip/usr/src/uts/common/os/compress.c
/*
 * This compression algorithm is a derivative of LZRW1, which I'll call
 * LZJB in the classic LZ* spirit.  All LZ* (Lempel-Ziv) algorithms are
 * based on the same basic principle: when a "phrase" (sequences of bytes)
 * is repeated in a data stream, we can save space by storing a reference to
 * the previous instance of that phrase (a "copy item") rather than storing
 * the phrase itself (a "literal item").  The compressor remembers phrases
 * in a simple hash table (the "Lempel history") that maps three-character
 * sequences (the minimum match) to the addresses where they were last seen.
 *
 * A copy item must encode both the length and the location of the matching
 * phrase so that decompress() can reconstruct the original data stream.
 * For example, here's how we'd encode "yadda yadda yadda, blah blah blah"
 * (with "_" replacing spaces for readability):
 *
 * Original:
 *
 * y a d d a _ y a d d a _ y a d d a , _ b l a h _ b l a h _ b l a h
 *
 * Compressed:
 *
 * y a d d a _ 6 11 , _ b l a h 5 10
 *
 * In the compressed output, the "6 11" simply means "to get the original
 * data, execute memmove(ptr, ptr - 6, 11)".  Note that in this example,
 * the match at "6 11" actually extends beyond the current location and
 * overlaps it.  That's OK; like memmove(), decompress() handles overlap.
 *
 * There's still one more thing decompress() needs to know, which is how to
 * distinguish literal items from copy items.  We encode this information
 * in an 8-bit bitmap that precedes each 8 items of output; if the Nth bit
 * is set, then the Nth item is a copy item.  Thus the full encoding for
 * the example above would be:
 *
 * 0x40 y a d d a _ 6 11 , 0x20 _ b l a h 5 10
 *
 * Finally, the "6 11" isn't really encoded as the two byte values 6 and 11
 * in the output stream because, empirically, we get better compression by
 * dedicating more bits to offset, fewer to match length.  LZJB uses 6 bits
 * to encode the match length, 10 bits to encode the offset.  Since copy-item
 * encoding consumes 2 bytes, we don't generate copy items unless the match
 * length is at least 3; therefore, we can store (length - 3) in the 6-bit
 * match length field, which extends the maximum match from 63 to 66 bytes.
 * Thus the 2-byte encoding for a copy item is as follows:
 *
 *  byte[0] = ((length - 3) << 2) | (offset >> 8);
 *  byte[1] = (uint8_t)offset;
 *
 * In our example above, an offset of 6 with length 11 would be encoded as:
 *
 *  byte[0] = ((11 - 3) << 2) | (6 >> 8) = 0x20
 *  byte[1] = (uint8_t)6 = 0x6
 *
 * Similarly, an offset of 5 with length 10 would be encoded as:
 *
 *  byte[0] = ((10 - 3) << 2) | (5 >> 8) = 0x1c
 *  byte[1] = (uint8_t)5 = 0x5
 *
 * Putting it all together, the actual LZJB output for our example is:
 *
 * 0x40 y a d d a _ 0x2006 , 0x20 _ b l a h 0x1c05
 *
 * The main differences between LZRW1 and LZJB are as follows:
 *
 * (1) LZRW1 is sloppy about buffer overruns.  LZJB never reads past the
 *     end of its input, and never writes past the end of its output.
 *
 * (2) LZJB allows a maximum match length of 66 (vs. 18 for LZRW1), with
 *     the trade-off being a shorter look-behind (1K vs. 4K for LZRW1).
 *
 * (3) LZJB records only the low-order 16 bits of pointers in the Lempel
 *     history (which is all we need since the maximum look-behind is 1K),
 *     and uses only 256 hash entries (vs. 4096 for LZRW1).  This makes
 *     the compression hash small enough to allocate on the stack, which
 *     solves two problems: (1) it saves 64K of kernel/cprboot memory,
 *     and (2) it makes the code MT-safe without any locking, since we
 *     don't have multiple threads sharing a common hash table.
 *
 * (4) LZJB is faster at both compression and decompression, has a
 *     better compression ratio, and is somewhat simpler than LZRW1.
 *
 * Finally, note that LZJB is non-deterministic: given the same input,
 * two calls to compress() may produce different output.  This is a
 * general characteristic of most Lempel-Ziv derivatives because there's
 * no need to initialize the Lempel history; not doing so saves time.
 */
//----------------------------------------------------------------------------
namespace LZJB {
//----------------------------------------------------------------------------
static const FFourCC FILE_MAGIC_         ("LZJB");
static const FFourCC FILE_VERSION_       ("1.00");
//----------------------------------------------------------------------------
STATIC_CONST_INTEGRAL(size_t, MATCH_BITS,   6 );
STATIC_CONST_INTEGRAL(size_t, MATCH_MIN,    3 );
STATIC_CONST_INTEGRAL(size_t, MATCH_MAX,    ((1 << MATCH_BITS) + (MATCH_MIN - 1)) );
STATIC_CONST_INTEGRAL(size_t, OFFSET_MASK,  ((1 << (16 - MATCH_BITS)) - 1) );
STATIC_CONST_INTEGRAL(size_t, LEMPEL_SIZE,  1024 );
STATIC_CONST_INTEGRAL(size_t, NBBY,         8 ); // number of bits in a byte
//----------------------------------------------------------------------------
struct FHeader {
    FFourCC  Magic;
    FFourCC  Version;
    u32     SizeInBytes;
};
//----------------------------------------------------------------------------
static void Overwrite_(IBufferedStreamWriter* writer, size_t offset, u8 value) {
    const std::streamoff cur = writer->TellO();
    writer->SeekO(std::streamoff(offset));
    writer->WritePOD(value);
    writer->SeekO(cur);
}
//----------------------------------------------------------------------------
void CompressMemory(IBufferedStreamWriter* dst, const TMemoryView<const u8>& src) {
    const FHeader header = {
        FILE_MAGIC_,
        FILE_VERSION_,
        checked_cast<u32>(src.SizeInBytes())
    };
    dst->WritePOD(header);

    STACKLOCAL_POD_ARRAY(u8, buffer, 8<<10);

    const u8* s_start = src.Pointer();
    const u8* psrc = s_start;
    const size_t s_len = src.SizeInBytes();

    size_t copyoff = 0;
    u8 copyval = 0;
    int copymask = 1 << (NBBY - 1);
    uint16_t lempel[LEMPEL_SIZE] = { 0 };

    while (psrc < s_start + s_len) {
        if ((copymask <<= 1) == (1 << NBBY)) {
            copymask = 1;
            copyoff = dst->TellO();
            copyval = 0;
            dst->WritePOD(u8(0));
        }
        if (psrc > s_start + s_len - MATCH_MAX) {
            dst->WritePOD(*psrc++);
            continue;
        }
        int hash = (psrc[0] << 16) + (psrc[1] << 8) + psrc[2];
        hash += hash >> 9;
        hash += hash >> 5;
        uint16_t *const hp = &lempel[hash & (LEMPEL_SIZE - 1)];
        const int offset = (intptr_t)(psrc - *hp) & OFFSET_MASK;
        *hp = (uint16_t)(uintptr_t)psrc;
        const u8 *const cpy = psrc - offset;
        if (cpy >= s_start && cpy != psrc &&
            psrc[0] == cpy[0] && psrc[1] == cpy[1] && psrc[2] == cpy[2]) {
            copyval |= copymask;
            Overwrite_(dst, copyoff, copyval);
            size_t mlen = MATCH_MIN;
            for (; mlen < MATCH_MAX; mlen++)
                if (psrc[mlen] != cpy[mlen])
                    break;
            dst->WritePOD((u8)(((mlen - MATCH_MIN) << (NBBY - MATCH_BITS)) | (offset >> NBBY)) );
            dst->WritePOD((u8)offset );
            psrc += mlen;
        } else {
            dst->WritePOD(*psrc++);
        }
    }

    LOG(Info, L"[LZJB] Compression ratio : {0} -> {1} = {2:f2}%",
        FSizeInBytes(src.SizeInBytes()), FSizeInBytes(dst->TellO()), dst->TellO()*100.0f/src.SizeInBytes() );
}
//----------------------------------------------------------------------------
template <typename _Allocator>
static bool DecompressMemory_(TRawStorage<u8, _Allocator>* dst, const TMemoryView<const u8>& src) {
    Assert(dst);

    const FHeader& header = *reinterpret_cast<const FHeader*>(src.Pointer());
    if (sizeof(FHeader) > src.size() ||
        FILE_MAGIC_ != header.Magic ||
        FILE_VERSION_ != header.Version )
        return false;

    dst->Resize_DiscardData(header.SizeInBytes);

    const u8* psrc = src.Pointer() + sizeof(FHeader);
    u8* pdst = dst->Pointer();
    u8 *d_end = pdst + dst->SizeInBytes();
    const u8 *cpy = nullptr;
    u8 copymap = 0;
    int copymask = 1 << (NBBY - 1);

    while (pdst < d_end) {
        if ((copymask <<= 1) == (1 << NBBY)) {
            copymask = 1;
            copymap = *psrc++;
        }
        if (0 == (copymap & copymask) ) {
            *pdst++ = *psrc++;
        }
        else {
            int mlen = (psrc[0] >> (NBBY - MATCH_BITS)) + MATCH_MIN;
            int offset = ((psrc[0] << NBBY) | psrc[1]) & OFFSET_MASK;
            psrc += 2;
            if ((cpy = pdst - offset) < dst->Pointer())
                return false;
            while (--mlen >= 0 && pdst < d_end)
                *pdst++ = *cpy++;
        }
    }

    return true;
}
//----------------------------------------------------------------------------
bool DecompressMemory(RAWSTORAGE(Stream, u8)* dst, const TMemoryView<const u8>& src) {
    return DecompressMemory_(dst, src);
}
//----------------------------------------------------------------------------
bool DecompressMemory(RAWSTORAGE_THREAD_LOCAL(Stream, u8)* dst, const TMemoryView<const u8>& src) {
    return DecompressMemory_(dst, src);
}
//----------------------------------------------------------------------------
} //!namespace LZW
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
