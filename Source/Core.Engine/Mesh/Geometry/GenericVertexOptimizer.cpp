#include "stdafx.h"

#include "GenericVertexOptimizer.h"

#include "GenericVertex.h"

#include "Core/Container/Hash.h"
#include "Core/Memory/UniqueView.h"
#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// http://fgiesen.wordpress.com/2013/12/14/simple-lossless-index-buffer-compression/
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct VertexCache {
    static const size_t Size = 32;
    static const size_t MaxValence = 15;

    struct Entry {
        u32 CachePos;           // its position in the cache (UINT32_MAX if not in)
        u32 Score;              // its score (higher=better)
        u32 TrianglesLeft;      // # of not-yet-used tris
        u32 *TriangleList;      // list of triangle indices
        u32 OpenPos;            // position in "open vertex" list
    };

    struct Triangle {
        u32 Score;              // current score (UINT32_MAX if already done)
        u32 Indices[3];         // vertex indices
    };
};
//----------------------------------------------------------------------------
}//!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void MergeDuplicateVertices(GenericVertex& vertices, const MemoryView<u32>& indices) {
    Assert(0 == (indices.size() % 3));

    const size_t vertexCount = vertices.VertexCountWritten();
    if (0 == vertexCount)
        return;
    Assert(indices.size() >= 3);

    const u8 *vertexPtr = vertices.Destination().Pointer();
    const size_t vertexSizeInBytes = vertices.VertexDeclaration()->SizeInBytes();
    AssertRelease(0 == (vertexSizeInBytes % sizeof(u32)) );

    const auto vertexHashes = MALLOCA_VIEW(size_t, vertexCount);
    for (size_t i = 0; i < vertexCount; ++i, vertexPtr += vertexSizeInBytes) {

    }
}
//----------------------------------------------------------------------------
void OptimizeIndicesOrder(const MemoryView<u32>& indices, size_t vertexCount) {
    Assert(indices.size() >= 3);
    Assert(0 == (indices.size() % 3));

    // prepare triangles
    const auto entries = MALLOCA_VIEW(VertexCache::Entry, vertexCount);
    for (VertexCache::Entry& entry : entries) {
        entry.CachePos = UINT32_MAX;
        entry.Score = 0;
        entry.TrianglesLeft = 0;
        entry.TriangleList = nullptr;
        entry.OpenPos = UINT32_MAX;
    }

    // alloc space for entry triangle indices
    const size_t triangleCount = indices.size() / 3;
    const auto triangles = MALLOCA_VIEW(VertexCache::Triangle, triangleCount);
    {
        const u32 *pIndex = &indices[0];
        for (size_t i = 0; i < triangleCount; ++i) {
            VertexCache::Triangle& triangle = triangles[i];
            triangle.Score = 0;
            for (size_t j = 0; j < 3; ++j) {
                const u32 index = *pIndex++;
                triangle.Indices[j] = index;
                ++entries[index].TrianglesLeft;
            }
        }
    }

    const auto adjacencies = MALLOCA_VIEW(u32, triangleCount*3);
    {
        u32 *pTriList = adjacencies.Pointer();
        const u32 *pTriListEnd = adjacencies.Pointer() + adjacencies.size();

        for (size_t i = 0; i < vertexCount; ++i) {
            VertexCache::Entry& entry = entries[i];
            entry.TriangleList = pTriList;
            pTriList += entry.TrianglesLeft;
            Assert(pTriList <= pTriListEnd);
            entry.TrianglesLeft = 0;
        }

        for (size_t i = 0; i < triangleCount; ++i) {
            const VertexCache::Triangle& triangle = triangles[i];
            for (size_t j = 0; j < 3; ++j) {
                const u32 index = triangle.Indices[j];
                VertexCache::Entry& entry = entries[index];
                entry.TriangleList[entry.TrianglesLeft++] = u32(i);
            }
        }
    }

    // open vertices
    const auto openVertices = MALLOCA_VIEW(u32, vertexCount);
    size_t openCount = 0;

    // the cache
    u32 cache[VertexCache::Size + 3] = {UINT32_MAX};
    u32 pos2score[VertexCache::Size];
    u32 val2score[VertexCache::Size + 1];

    for (size_t i = 0; i < VertexCache::Size; ++i) {
        const float score = (i < 3) ? 0.75f : std::pow(1.0f - (i - 3)/float(VertexCache::Size - 3), 1.5f);
        pos2score[i] = static_cast<u32>(score * 65536.0f + 0.5f);
    }

    val2score[0] = 0;
    for(int i=1;i<16;i++)
    {
        const float score = 2.0f / std::sqrt(float(i));
        val2score[i] = static_cast<u32>(score * 65536.0f + 0.5f);
    }

    // outer loop: find triangle to start with
    u32 *pIndex = &indices[0];
    u32 seedPos = 0;

    while (1) {
        u32 seedScore = 0;
        u32 seedTriangle = UINT32_MAX;

        // if there are open vertices, search them for the seed triangle
        // which maximum score.
        for (size_t i = 0; i < openCount; ++i) {
            const VertexCache::Entry &entry = entries[openVertices[i]];

            for (size_t j = 0; j < entry.TrianglesLeft; ++j) {
                const u32 index = entry.TriangleList[j];
                const VertexCache::Triangle& triangle = triangles[index];

                if (triangle.Score > seedScore) {
                    seedScore = triangle.Score;
                    seedTriangle = index;
                }
            }
        }

        // if we haven't found a seed triangle yet, there are no open
        // vertices and we can pick any triangle
        if (UINT32_MAX == seedTriangle) {
            while (seedPos < triangleCount && triangles[seedPos].Score == UINT32_MAX)
                ++seedPos;

            if (seedPos == triangleCount) // no triangle left, we're done!
                break;

            seedTriangle = seedPos;
        }

        u32 bestTriangle = seedTriangle;
        while (bestTriangle != UINT32_MAX) {
            VertexCache::Triangle& triangle = triangles[bestTriangle];

            // mark this triangle as used, remove it from the "remaining tris"
            // list of the vertices it uses, and add it to the index buffer.
            triangle.Score = UINT32_MAX;

            for (size_t j = 0; j < 3; ++j) {
                const u32 index = triangle.Indices[j];
                *pIndex++ = index;

                VertexCache::Entry& entry = entries[index];

                // find this triangles' entry
                u32 k = 0;
                while(entry.TriangleList[k] != bestTriangle) {
                    Assert(k < entry.TrianglesLeft);
                    ++k;
                }

                // swap it to the end and decrement # of tris left
                if(--entry.TrianglesLeft) {
                    std::swap(entry.TriangleList[k], entry.TriangleList[entry.TrianglesLeft]);
                }
                else if(entry.OpenPos != UINT32_MAX) {
                    Assert(openCount);
                    std::swap(openVertices[entry.OpenPos], openVertices[--openCount]);
                }
            }

            // update cache status
            cache[VertexCache::Size] = cache[VertexCache::Size + 1] = cache[VertexCache::Size + 2] = UINT32_MAX;

            for(size_t j = 0; j < 3 ; ++j) {
                const u32 index = triangle.Indices[j];
                cache[VertexCache::Size + 2] = index;

                // find vertex index
                u32 pos = 0;
                while (index != cache[pos])
                    ++pos;

                // move to front
                for(int k = pos; k > 0; k--)
                    cache[k] = cache[k - 1];

                cache[0] = index;

                // remove sentinel if it wasn't used
                if(pos != VertexCache::Size + 2)
                    cache[VertexCache::Size + 2] = UINT32_MAX;
            }

            // update vertex scores
            for (size_t i = 0; i < VertexCache::Size + 3; ++i) {
                const u32 index = cache[i];
                if (UINT32_MAX == index)
                    continue;

                VertexCache::Entry& entry = entries[index];

                entry.Score = val2score[std::min(entry.TrianglesLeft, u32(VertexCache::MaxValence))];
                if (i < VertexCache::Size) {
                    entry.CachePos = checked_cast<u32>(i);
                    entry.Score += pos2score[i];
                }
                else {
                    entry.CachePos = UINT32_MAX;

                    // also add to open vertices list if the vertex is indeed open
                    if (UINT32_MAX == entry.OpenPos && entry.TrianglesLeft) {
                        entry.OpenPos = checked_cast<u32>(openCount);
                        openVertices[openCount++] = index;
                    }
                }
            }

            // update triangle scores, find new best triangle
            u32 bestScore = 0;
            bestTriangle = UINT32_MAX;

            for (size_t i = 0; i < VertexCache::Size; ++i) {
                if (cache[i] == UINT32_MAX)
                    continue;

                const VertexCache::Entry& entry = entries[cache[i]];

                for (size_t j = 0; j < entry.TrianglesLeft; ++j) {
                    const u32 index = entry.TriangleList[j];
                    VertexCache::Triangle& triangle = triangles[index];

                    Assert(triangle.Score != UINT32_MAX);

                    u32 score = 0;
                    for (size_t k = 0; k < 3; ++k)
                        score += entries[triangle.Indices[k]].Score;

                    triangle.Score = score;
                    if (score > bestScore) {
                        bestScore = score;
                        bestTriangle = index;
                    }
                }
            }
        } //!while (bestTriangle != UINT32_MAX)
    } //!while (1)
}
//----------------------------------------------------------------------------
void OptimizeVerticesOrder(GenericVertex& vertices, const MemoryView<u32>& indices) {
    Assert(vertices.VertexCountWritten() >= 3);
    Assert(indices.size() >= 3);

    const size_t vertexCount = vertices.VertexCountWritten();
    const size_t vertexSizeInBytes = vertices.VertexDeclaration()->SizeInBytes();

    const auto old2new = MALLOCA_VIEW(u32, vertexCount);
    memset(old2new.Pointer(), 0xFF, old2new.SizeInBytes());

    u32 sortedIndex = 0;
    u8 *const sortedPtr = reinterpret_cast<u8 *>(vertices.Destination().Pointer());

    const size_t vertexAllSizeInBytes = vertices.VertexDeclaration()->SizeInBytes() * vertexCount;
    const auto tmpVertices = MALLOCA_VIEW(u8, vertexAllSizeInBytes);
    memcpy(tmpVertices.Pointer(), sortedPtr, tmpVertices.SizeInBytes());

    // construct mapping and new VB
    for (u32& index : indices) {
        if (UINT32_MAX == old2new[index]) {
            memcpy( sortedPtr + vertexSizeInBytes * sortedIndex,
                    tmpVertices.Pointer() + vertexSizeInBytes * index,
                    vertexSizeInBytes );
            old2new[index] = sortedIndex++;
        }
        index = old2new[index];
    }
}
//----------------------------------------------------------------------------
void OptimizeIndicesAndVerticesOrder(GenericVertex& vertices, const MemoryView<u32>& indices) {
    OptimizeIndicesOrder(indices, vertices.VertexCountWritten());
    OptimizeVerticesOrder(vertices, indices);
}
//----------------------------------------------------------------------------
float VertexAverageCacheMissRate(const MemoryView<u32>& indices, bool fifo/* = true */, size_t cacheSize/* = 16 */) {
    Assert(cacheSize > 1);

    if (indices.empty())
        return 0.0f;

    const size_t indexCount = indices.size();

    size_t misses = 0;
    size_t wrPos = 0;

    const auto cache = MALLOCA_VIEW(u32, cacheSize+1);

    // initialize cache (we simulate a FIFO here)
    for (size_t i = 0; i < cacheSize; ++i)
        cache[i] = UINT32_MAX;

    // simulate
    for (size_t i = 0; i < indexCount; ++i) {
        const u32 index = indices[i];
        cache[cacheSize] = index;

        // find in cache
        size_t cachePos = 0;
        while (cache[cachePos] != index)
            ++cachePos;

        misses += (cachePos == cacheSize);

        if (fifo) {
            cache[wrPos] = index;
            if (++wrPos == cacheSize)
                wrPos = 0;
        }
        else {
            // move to front
            for (size_t j = cachePos; j > 0; --j)
                cache[j] = cache[j - 1];

            cache[0] = index;
        }
    }

    const float ACMR = misses * 3.0f / indexCount;
    return ACMR;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
