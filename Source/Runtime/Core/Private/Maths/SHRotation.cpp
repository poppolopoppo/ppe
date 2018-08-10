#include "stdafx.h"

#include "Maths/SHRotation.h"

#include "Maths/SHVector.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <size_t _Dim>
static void SHRotation_Transform_(size_t bands, const SHScalar *matrices, TSHVector<_Dim> *dst, const TSHVector<_Dim>& src) {
    Assert(dst);
    Assert(dst->Bands() == src.Bands());

    dst->Reset(); // set to zero

    TSHCoefficient<_Dim> *dstCoeffs = dst->Coefficients().Pointer();
    const TSHCoefficient<_Dim> *srcCoeffs = src.Coefficients().Pointer();

    const SHScalar *m = matrices;

    dstCoeffs[0] = srcCoeffs[0];
    for(size_t l = 1; l < bands; ++l) {
        const size_t width = l*2+1;

        for (size_t c = 0; c < width; ++c) {
            for(size_t cm = 0; cm < width; ++cm)
                dstCoeffs[c] += srcCoeffs[cm] * m[c * width + cm];
        }

        //we move the offset to the coefficient on the next band
        dstCoeffs += width;
        srcCoeffs += width;

        m += (width) * (width/* height */); // advance to next band matrix
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FSHRotation::FSHRotation(size_t bands)
:   _bands(bands) {
    Assert(0 < bands);

    size_t scalarCount = 0;
    for (size_t l = 1 /* no matrix needed for first band */; l < _bands; ++l)
        scalarCount += (l*2+1) * (l*2+1);

    _matrices.Resize_DiscardData(scalarCount);
}
//----------------------------------------------------------------------------
FSHRotation::~FSHRotation() {}
//----------------------------------------------------------------------------
void FSHRotation::Transform(TSHVector<1> *dst, const TSHVector<1>& src) const {
    SHRotation_Transform_(_bands, _matrices.Pointer(), dst, src);
}
//----------------------------------------------------------------------------
void FSHRotation::Transform(TSHVector<2> *dst, const TSHVector<2>& src) const {
    SHRotation_Transform_(_bands, _matrices.Pointer(), dst, src);
}
//----------------------------------------------------------------------------
void FSHRotation::Transform(TSHVector<3> *dst, const TSHVector<3>& src) const {
    SHRotation_Transform_(_bands, _matrices.Pointer(), dst, src);
}
//----------------------------------------------------------------------------
void FSHRotation::Transform(TSHVector<4> *dst, const TSHVector<4>& src) const {
    SHRotation_Transform_(_bands, _matrices.Pointer(), dst, src);
}
//----------------------------------------------------------------------------
void FSHRotation::AroundX(FSHRotation *rotation, SHScalar radians) {
    Assert(rotation);
    AssertRelease(rotation->_bands <= 2); // not supported, todo ?

    const SHScalar cos1a = std::cos(radians);
    const SHScalar sin1a = std::sin(radians);

    SHScalar *const m = rotation->_matrices.Pointer();

    m[0] =  cos1a;
    m[1] = -sin1a;
    m[2] =  0;
    m[3] =  sin1a;
    m[4] =  cos1a;
    m[5] =  0;
    m[6] =  0;
    m[7] =  0;
    m[8] =  1;
}
//----------------------------------------------------------------------------
void FSHRotation::AroundY(FSHRotation *rotation, SHScalar radians) {
    Assert(rotation);
    AssertRelease(rotation->_bands <= 2);

    const SHScalar cos1a = std::cos(radians);
    const SHScalar sin1a = std::sin(radians);

    SHScalar *const m = rotation->_matrices.Pointer();

    m[0] =  1;
    m[1] =  0;
    m[2] =  0;
    m[3] =  0;
    m[4] =  cos1a;
    m[5] = -sin1a;
    m[6] =  0;
    m[7] =  sin1a;
    m[8] =  cos1a;
}
//----------------------------------------------------------------------------
void FSHRotation::AroundZ(FSHRotation *rotation, SHScalar radians) {
    Assert(rotation);
    // supports any band count

    SHScalar *m = nullptr;

    if (rotation->_bands > 1) {
        const SHScalar cos1a = std::cos(radians);
        const SHScalar sin1a = std::sin(radians);

        m = rotation->_matrices.Pointer();

        m[0] =  cos1a;
        m[1] =  0;
        m[2] =  sin1a;
        m[3] =  0;
        m[4] =  1;
        m[5] =  0;
        m[6] = -sin1a;
        m[7] =  0;
        m[8] =  cos1a;

        if (rotation->_bands > 2) {
            const SHScalar cos2a = std::cos(2 * radians);
            const SHScalar sin2a = std::sin(2 * radians);

            m = &rotation->_matrices[9];

            m[0] =  cos2a;
            m[1] =  0;
            m[2] =  0;
            m[3] =  0;
            m[4] =  sin2a;
            m[5] =  0;
            m[6] =  cos1a;
            m[7] =  0;
            m[8] =  sin1a;
            m[9] =  0;
            m[10] =  0;
            m[11] =  0;
            m[12] =  1;
            m[13] =  0;
            m[14] =  0;
            m[15] =  0;
            m[16] = -sin1a;
            m[17] =  0;
            m[18] =  cos1a;
            m[19] =  0;
            m[20] = -sin2a;
            m[21] =  0;
            m[22] =  0;
            m[23] =  0;
            m[24] =  cos2a;

            if (rotation->_bands > 3) {    //general computation
                const size_t matrixBand3Offset = 25+9;
                m = &rotation->_matrices[matrixBand3Offset];

                // fill with 0
                for (SHScalar *it = m; it < rotation->_matrices.end(); ++it)
                    *it = 0;

                // fill with rotation value
                size_t matrixOffset = matrixBand3Offset;
                for(int l = 3; l < int(rotation->_bands); ++l) {
                    const int width = l*2+1;
                    const int middle = l+1;

                    m = &rotation->_matrices[matrixOffset];

                    for(int c = 0; c < middle - 1; ++c) {
                        m[c * width + c] = std::cos(std::abs(middle-c-1)*radians);
                        m[c * width + (width - c - 1)] = std::sin(std::abs(middle-c-1)*radians);
                    }

                    m[(middle - 1)*width + middle - 1] = 1;

                    for(int c = middle; c < width; ++c) {
                        //copy instead of computing again sin and cos
                        //TODO, copy preeceding matrix values and compute only corners!
                        int tmp = middle - 1 - (c - middle + 1);
                        m[c * width + c] = m[tmp * width + tmp];
                        tmp = middle - 1 - (c - middle + 1);
                        m[c * width + (width - c - 1)]    = -m[tmp * width + (width - tmp - 1)];
                    }

                    matrixOffset += (width) * (width/* height */); // advance to next band matrix
                }
            }
        }
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
