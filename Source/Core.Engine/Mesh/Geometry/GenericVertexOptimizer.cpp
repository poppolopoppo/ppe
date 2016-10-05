#include "stdafx.h"

#include "GenericVertexOptimizer.h"

#include "GenericVertex.h"

#include "Core/Container/Hash.h"
#include "Core/Memory/UniqueView.h"
#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"

#include <algorithm>

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// http://fgiesen.wordpress.com/2013/12/14/simple-lossless-index-buffer-compression/
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct FVertexCache {
    static const size_t Size = 32;
    static const size_t MaxValence = 15;

    struct FEntry {
        u32 CachePos;           // its position in the cache (UINT32_MAX if not in)
        u32 Score;              // its score (higher=better)
        u32 TrianglesLeft;      // # of not-yet-used tris
        u32 *TriangleList;      // list of triangle indices
        u32 OpenPos;            // position in "open vertex" list
    };

    struct FTriangle {
        u32 Score;              // current score (UINT32_MAX if already done)
        u32 Indices[3];         // vertex indices
    };
};
//----------------------------------------------------------------------------
}//!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void MergeDuplicateVertices(FGenericVertex& vertices, const TMemoryView<u32>& indices) {
    Assert(0 == (indices.size() % 3));

    const size_t vertexCount = vertices.VertexCountWritten();
    if (0 == vertexCount)
        return;
    Assert(indices.size() >= 3);

    const size_t vertexSizeInBytes = vertices.VertexDeclaration()->SizeInBytes();

    const auto vertexHashes = MALLOCA_VIEW(size_t, vertexCount);
    const auto vertexSorted = MALLOCA_VIEW(u32, vertexCount);
    {
        const u8 *vertexPtr = vertices.Destination().Pointer();
        for (u32 i = 0; i < vertexCount; ++i, vertexPtr += vertexSizeInBytes) {
            vertexHashes[i] = hash_value_as_memory(vertexPtr, vertexSizeInBytes);
            vertexSorted[i] = i;
        }
    }

    std::stable_sort(vertexSorted.begin(), vertexSorted.end(), [&vertexHashes](u32 lhs, u32 rhs) {
        return vertexHashes[lhs] < vertexHashes[rhs];
    });

    u8 *const vertexNewData = vertices.Destination().Pointer();
    const auto vertexOldData = MALLOCA_VIEW(u8, vertexCount * vertexSizeInBytes);
    memcpy(vertexOldData.Pointer(), vertexNewData, vertexOldData.SizeInBytes());

    u32 mergedVertexCount = 0;
    const auto vertexReindexation = MALLOCA_VIEW(u32, vertexCount);
    for (u32 i = 0; i < vertexCount; ) {
        const u32 mergedIndex = mergedVertexCount++;
        const u32 originalIndex = vertexSorted[i];
        vertexReindexation[originalIndex] = mergedIndex;

        const size_t mergedHash = vertexHashes[originalIndex];

        memcpy( vertexNewData + vertexSizeInBytes * mergedIndex, 
                vertexOldData.Pointer() + vertexSizeInBytes * originalIndex,
                vertexSizeInBytes );

        ++i;
        while (i < vertexCount && mergedHash == vertexHashes[vertexSorted[i]]) {
            
            if (0 == memcmp(vertexOldData.Pointer() + vertexSizeInBytes * originalIndex,
                            vertexOldData.Pointer() + vertexSizeInBytes * vertexSorted[i],
                            vertexSizeInBytes )) {
                vertexReindexation[vertexSorted[i]] = mergedIndex;
                ++i;
            }
            else {
                break;
            }
        }
    }

    vertices.SeekVertex(mergedVertexCount);

    for (u32& index : indices)
        index = vertexReindexation[index];
}
//----------------------------------------------------------------------------
void OptimizeIndicesOrder(const TMemoryView<u32>& indices, size_t vertexCount) {
    Assert(indices.size() >= 3);
    Assert(0 == (indices.size() % 3));

    // prepare triangles
    const auto entries = MALLOCA_VIEW(FVertexCache::FEntry, vertexCount);
    for (FVertexCache::FEntry& entry : entries) {
        entry.CachePos = UINT32_MAX;
        entry.Score = 0;
        entry.TrianglesLeft = 0;
        entry.TriangleList = nullptr;
        entry.OpenPos = UINT32_MAX;
    }

    // alloc space for entry triangle indices
    const size_t triangleCount = indices.size() / 3;
    const auto triangles = MALLOCA_VIEW(FVertexCache::FTriangle, triangleCount);
    {
        const u32 *pIndex = &indices[0];
        for (size_t i = 0; i < triangleCount; ++i) {
            FVertexCache::FTriangle& triangle = triangles[i];
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
            FVertexCache::FEntry& entry = entries[i];
            entry.TriangleList = pTriList;
            pTriList += entry.TrianglesLeft;
            Assert(pTriList <= pTriListEnd);
            entry.TrianglesLeft = 0;
        }

        for (size_t i = 0; i < triangleCount; ++i) {
            const FVertexCache::FTriangle& triangle = triangles[i];
            for (size_t j = 0; j < 3; ++j) {
                const u32 index = triangle.Indices[j];
                FVertexCache::FEntry& entry = entries[index];
                entry.TriangleList[entry.TrianglesLeft++] = u32(i);
            }
        }
    }

    // open vertices
    const auto openVertices = MALLOCA_VIEW(u32, vertexCount);
    size_t openCount = 0;

    // the cache
    u32 cache[FVertexCache::Size + 3] = {UINT32_MAX};
    u32 pos2score[FVertexCache::Size];
    u32 val2score[FVertexCache::Size + 1];

    for (size_t i = 0; i < FVertexCache::Size; ++i) {
        const float score = (i < 3) ? 0.75f : std::pow(1.0f - (i - 3)/float(FVertexCache::Size - 3), 1.5f);
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

    for (;;) {
        u32 seedScore = 0;
        u32 seedTriangle = UINT32_MAX;

        // if there are open vertices, search them for the seed triangle
        // which maximum score.
        for (size_t i = 0; i < openCount; ++i) {
            const FVertexCache::FEntry &entry = entries[openVertices[i]];

            for (size_t j = 0; j < entry.TrianglesLeft; ++j) {
                const u32 index = entry.TriangleList[j];
                const FVertexCache::FTriangle& triangle = triangles[index];

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
            FVertexCache::FTriangle& triangle = triangles[bestTriangle];

            // mark this triangle as used, remove it from the "remaining tris"
            // list of the vertices it uses, and add it to the index buffer.
            triangle.Score = UINT32_MAX;

            for (size_t j = 0; j < 3; ++j) {
                const u32 index = triangle.Indices[j];
                *pIndex++ = index;

                FVertexCache::FEntry& entry = entries[index];

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
            cache[FVertexCache::Size] = cache[FVertexCache::Size + 1] = cache[FVertexCache::Size + 2] = UINT32_MAX;

            for(size_t j = 0; j < 3 ; ++j) {
                const u32 index = triangle.Indices[j];
                cache[FVertexCache::Size + 2] = index;

                // find vertex index
                u32 pos = 0;
                while (index != cache[pos])
                    ++pos;

                // move to front
                for(int k = pos; k > 0; k--)
                    cache[k] = cache[k - 1];

                cache[0] = index;

                // remove sentinel if it wasn't used
                if(pos != FVertexCache::Size + 2)
                    cache[FVertexCache::Size + 2] = UINT32_MAX;
            }

            // update vertex scores
            for (size_t i = 0; i < FVertexCache::Size + 3; ++i) {
                const u32 index = cache[i];
                if (UINT32_MAX == index)
                    continue;

                FVertexCache::FEntry& entry = entries[index];

                entry.Score = val2score[std::min(entry.TrianglesLeft, u32(FVertexCache::MaxValence))];
                if (i < FVertexCache::Size) {
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

            for (size_t i = 0; i < FVertexCache::Size; ++i) {
                if (cache[i] == UINT32_MAX)
                    continue;

                const FVertexCache::FEntry& entry = entries[cache[i]];

                for (size_t j = 0; j < entry.TrianglesLeft; ++j) {
                    const u32 index = entry.TriangleList[j];
                    FVertexCache::FTriangle& tri = triangles[index];

                    Assert(tri.Score != UINT32_MAX);

                    u32 score = 0;
                    for (size_t k = 0; k < 3; ++k)
                        score += entries[tri.Indices[k]].Score;

                    tri.Score = score;
                    if (score > bestScore) {
                        bestScore = score;
                        bestTriangle = index;
                    }
                }
            }
        } //!while (bestTriangle != UINT32_MAX)
    } //!for (;;)
}
//----------------------------------------------------------------------------
void OptimizeVerticesOrder(FGenericVertex& vertices, const TMemoryView<u32>& indices) {
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
void OptimizeIndicesAndVerticesOrder(FGenericVertex& vertices, const TMemoryView<u32>& indices) {
    OptimizeIndicesOrder(indices, vertices.VertexCountWritten());
    OptimizeVerticesOrder(vertices, indices);
}
//----------------------------------------------------------------------------
float VertexAverageCacheMissRate(const TMemoryView<u32>& indices, bool fifo/* = true */, size_t cacheSize/* = 16 */) {
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
