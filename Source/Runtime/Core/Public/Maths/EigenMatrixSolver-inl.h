#pragma once

#include "Maths/EigenMatrixSolver.h"

#include "Maths/MathHelpers.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t N>
TSymmetricEigensolver<T, N>::TSymmetricEigensolver(size_t maxIterations)
    :
    _maxIterations(0),
    _isRotation(-1)
{
    STATIC_ASSERT(N > 1);
    if (maxIterations > 0)
    {
        _maxIterations = maxIterations;
        _givens.reserve(maxIterations * (N - 1));
    }
}
//----------------------------------------------------------------------------
template <typename T, size_t N>
size_t TSymmetricEigensolver<T, N>::Solve(const matrix_type& input, int sortType)
{
    STATIC_ASSERT(N > 0);

    _matrix = input;
    Tridiagonalize();

    _givens.clear();
    for (size_t j = 0; j < _maxIterations; ++j)
    {
        int imin = -1, imax = -1;
        for (int i = int(N) - 2; i >= 0; --i)
        {
            // When a01 is much smaller than its diagonal neighbors, it is
            // effectively zero.
            T a00 = mDiagonal[i];
            T a01 = _superdiagonal[i];
            T a11 = mDiagonal[i + 1];
            T sum = Abs(a00) + Abs(a11);
            if (sum + Abs(a01) != sum)
            {
                if (imax == -1)
                {
                    imax = i;
                }
                imin = i;
            }
            else
            {
                // The superdiagonal term is effectively zero compared to
                // the neighboring diagonal terms.
                if (imin >= 0)
                {
                    break;
                }
            }
        }

        if (imax == -1)
        {
            // The algorithm has converged.
            ComputePermutation(sortType);
            return j;
        }

        // Process the lower-right-most unreduced tridiagonal block.
        DoQRImplicitShift(imin, imax);
    }

    return 0xFFFFFFFF;
}
//----------------------------------------------------------------------------
template <typename T, size_t N>
void TSymmetricEigensolver<T, N>::GetEigenvalues(vector_type& eigenvalues) const
{
    if (_permutation[0] >= 0)
    {
        // Sorting was requested.
        for (int i = 0; i < int(N); ++i)
        {
            int p = _permutation[i];
            eigenvalues[i] = mDiagonal[p];
        }
    }
    else
    {
        // Sorting was not requested.
        eigenvalues = mDiagonal;
    }
}
//----------------------------------------------------------------------------
template <typename T, size_t N>
void TSymmetricEigensolver<T, N>::GetEigenvectors(matrix_type& eigenvectors) const
{
    // Start with the identity matrix.
    eigenvectors = matrix_type::Identity();

    // Multiply the Householder reflections using backward accumulation.
    int r, c;
    for (int i = int(N) - 3, rmin = i + 1; i >= 0; --i, --rmin)
    {
        // Copy the v vector and 2/Dot(v,v) from the matrix.
        T const* column = &_matrix.data().raw[i];
        T twoinvvdv = column[N*(i + 1)];
        for (r = 0; r < i + 1; ++r)
        {
            _vectorV[r] = (T)0;
        }
        _vectorV[r] = (T)1;
        for (++r; r < int(N); ++r)
        {
            _vectorV[r] = column[N*r];
        }

        // Compute the w vector.
        for (r = 0; r < int(N); ++r)
        {
            _vectorW[r] = (T)0;
            for (c = rmin; c < int(N); ++c)
            {
                _vectorW[r] += _vectorV[c]*eigenvectors.data().raw[r + N*c];
            }
            _vectorW[r] *= twoinvvdv;
        }

        // Update the matrix, Q <- Q - v*w^T.
        for (r = rmin; r < int(N); ++r)
        {
            for (c = 0; c < int(N); ++c)
            {
                eigenvectors.data().raw[c + N*r] -= _vectorV[r]*_vectorW[c];
            }
        }
    }

    // Multiply the Givens rotations.
    for (auto const& givens : _givens)
    {
        for (r = 0; r < int(N); ++r)
        {
            int j = givens.index + N*r;
            T& q0 = eigenvectors.data().raw[j];
            T& q1 = eigenvectors.data().raw[j + 1];
            T prd0 = givens.cs * q0 - givens.sn * q1;
            T prd1 = givens.sn * q0 + givens.cs * q1;
            q0 = prd0;
            q1 = prd1;
        }
    }

    _isRotation = 1 - (N & 1);
    if (_permutation[0] >= 0)
    {
        // Sorting was requested.
        _visited.Broadcast(0);
        for (int i = 0; i < int(N); ++i)
        {
            if (_visited[i] == 0 && _permutation[i] != i)
            {
                // The item starts a cycle with 2 or more elements.
                _isRotation = 1 - _isRotation;
                int start = i, current = i, j, next;
                for (j = 0; j < int(N); ++j)
                {
                    _vectorP[j] = eigenvectors.data().raw[i + N*j];
                }
                while ((next = _permutation[current]) != start)
                {
                    _visited[current] = 1;
                    for (j = 0; j < int(N); ++j)
                    {
                        eigenvectors.data().raw[current + N*j] =
                            eigenvectors.data().raw[next + N*j];
                    }
                    current = next;
                }
                _visited[current] = 1;
                for (j = 0; j < int(N); ++j)
                {
                    eigenvectors.data().raw[current + N*j] = _vectorP[j];
                }
            }
        }
    }
}
//----------------------------------------------------------------------------
template <typename T, size_t N>
bool TSymmetricEigensolver<T, N>::IsRotation() const
{
    IF_CONSTEXPR(N > 0)
    {
        if (_isRotation == -1)
        {
            // Without sorting, the matrix is a rotation when size is even.
            _isRotation = 1 - (N & 1);
            if (_permutation[0] >= 0)
            {
                // With sorting, the matrix is a rotation when the number of
                // cycles in the permutation is even.
                _visited.Broadcast(0);
                for (int i = 0; i < int(N); ++i)
                {
                    if (_visited[i] == 0 && _permutation[i] != i)
                    {
                        // The item starts a cycle with 2 or more elements.
                        int start = i, current = i, next;
                        while ((next = _permutation[current]) != start)
                        {
                            _visited[current] = 1;
                            current = next;
                        }
                        _visited[current] = 1;
                    }
                }
            }
        }
        return _isRotation == 1;
    }
    else
    {
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename T, size_t N>
void TSymmetricEigensolver<T, N>::GetEigenvector(int c, vector_type& eigenvector) const
{
    if (0 <= c && c < int(N))
    {
        // y = H*x, then x and y are swapped for the next H
        T* x = &eigenvector[0];
        T* y = &_vectorP[0];

        // Start with the Euclidean basis vector.
        FPlatformMemory::Memzero(x, N*sizeof(T));
        if (_permutation[c] >= 0)
        {
            x[_permutation[c]] = (T)1;
        }
        else
        {
            x[c] = (T)1;
        }

        // Apply the Givens rotations.
        reverseforeachitem(givens, _givens)
        {
            T& xr = x[givens->index];
            T& xrp1 = x[givens->index + 1];
            T tmp0 = givens->cs * xr + givens->sn * xrp1;
            T tmp1 = -givens->sn * xr + givens->cs * xrp1;
            xr = tmp0;
            xrp1 = tmp1;
        }

        // Apply the Householder reflections.
        for (int i = int(N) - 3; i >= 0; --i)
        {
            // Get the Householder vector v.
            T const* column = &_matrix.data().raw[i];
            T twoinvvdv = column[N*(i + 1)];
            int r;
            for (r = 0; r < i + 1; ++r)
            {
                y[r] = x[r];
            }

            // Compute s = Dot(x,v) * 2/v^T*v.
            T s = x[r];  // r = i+1, v[i+1] = 1
            for (int j = r + 1; j < int(N); ++j)
            {
                s += x[j] * column[N*j];
            }
            s *= twoinvvdv;

            y[r] = x[r] - s;  // v[i+1] = 1

            // Compute the remaining components of y.
            for (++r; r < int(N); ++r)
            {
                y[r] = x[r] - s * column[N*r];
            }

            std::swap(x, y);
        }
        // The final product is stored in x.

        if (x != &eigenvector[0])
        {
            FPlatformMemory::Memcpy(&eigenvector[0], x, N * sizeof(T));
        }
    }
}
//----------------------------------------------------------------------------
template <typename T, size_t N>
T TSymmetricEigensolver<T, N>::GetEigenvalue(int c) const
{
    IF_CONSTEXPR(N > 0)
    {
        if (_permutation[0] >= 0)
        {
            // Sorting was requested.
            return mDiagonal[_permutation[c]];
        }
        else
        {
            // Sorting was not requested.
            return mDiagonal[c];
        }
    }
    else
    {
        return std::numeric_limits<T>::max();
    }
}
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(6295) // Ill-defined for-loop: 'int' values are always of range '-2147483648' to '2147483647'. Loop executes infinitely.
template <typename T, size_t N>
void TSymmetricEigensolver<T, N>::Tridiagonalize()
{
    STATIC_ASSERT(N > 1);
    int r, c;
    for (int i = 0, ip1 = 1; i < int(N) - 2; ++i, ++ip1)
    {
        // Compute the Householder vector.  Read the initial vector from the
        // row of the matrix.
        T length = (T)0;
        for (r = 0; r < ip1; ++r)
        {
            _vectorV[r] = (T)0;
        }
        for (r = ip1; r < int(N); ++r)
        {
            T vr = _matrix.data().raw[r + N*i];
            _vectorV[r] = vr;
            length += vr * vr;
        }
        T vdv = (T)1;
        length = Sqrt(length);
        if (length > (T)0)
        {
            T& v1 = _vectorV[ip1];
            T sgn = (v1 >= (T)0 ? (T)1 : (T)-1);
            T invDenom = ((T)1) / (v1 + sgn * length);
            v1 = (T)1;
            for (r = ip1 + 1; r < int(N); ++r)
            {
                T& vr = _vectorV[r];
                vr *= invDenom;
                vdv += vr * vr;
            }
        }

        // Compute the rank-1 offsets v*w^T and w*v^T.
        T invvdv = (T)1 / vdv;
        T twoinvvdv = invvdv * (T)2;
        T pdvtvdv = (T)0;
        for (r = i; r < int(N); ++r)
        {
            _vectorP[r] = (T)0;
            for (c = i; c < r; ++c)
            {
                _vectorP[r] += _matrix.data().raw[r + N*c] * _vectorV[c];
            }
            for (/**/; c < int(N); ++c)
            {
                _vectorP[r] += _matrix.data().raw[c + N*r] * _vectorV[c];
            }
            _vectorP[r] *= twoinvvdv;
            pdvtvdv += _vectorP[r] * _vectorV[r];
        }

        pdvtvdv *= invvdv;
        for (r = i; r < int(N); ++r)
        {
            _vectorW[r] = _vectorP[r] - pdvtvdv * _vectorV[r];
        }

        // Update the input matrix.
        for (r = i; r < int(N); ++r)
        {
            T vr = _vectorV[r];
            T wr = _vectorW[r];
            T offset = vr * wr * (T)2;
            _matrix.data().raw[r + N*r] -= offset;
            for (c = r + 1; c < int(N); ++c)
            {
                offset = vr * _vectorW[c] + wr * _vectorV[c];
                _matrix.data().raw[c + N*r] -= offset;
            }
        }

        // Copy the vector to column i of the matrix.  The 0-valued components
        // at indices 0 through i are not stored.  The 1-valued component at
        // index i+1 is also not stored; instead, the quantity 2/Dot(v,v) is
        // stored for use in eigenvector construction. That construction must
        // take into account the implied components that are not stored.
        _matrix.data().raw[i + N*ip1] = twoinvvdv;
        for (r = ip1 + 1; r < int(N); ++r)
        {
            _matrix.data().raw[i + N*r] = _vectorV[r];
        }
    }

    // Copy the diagonal and subdiagonal entries for cache coherence in
    // the QR iterations.
    int k, ksup = int(N) - 1, index = 0, delta = N + 1;
    for (k = 0; k < ksup; ++k, index += delta)
    {
        mDiagonal[k] = _matrix.data().raw[index];
        _superdiagonal[k] = _matrix.data().raw[index + 1];
    }
    mDiagonal[k] = _matrix.data().raw[index];
}
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
template <typename T, size_t N>
void TSymmetricEigensolver<T, N>::GetSinCos(T x, T y, T& cs, T& sn)
{
    // Solves sn*x + cs*y = 0 robustly.
    T tau;
    if (y != (T)0)
    {
        if (Abs(y) > Abs(x))
        {
            tau = -x / y;
            sn = ((T)1) / Sqrt(((T)1) + tau*tau);
            cs = sn * tau;
        }
        else
        {
            tau = -y / x;
            cs = ((T)1) / Sqrt(((T)1) + tau*tau);
            sn = cs * tau;
        }
    }
    else
    {
        cs = (T)1;
        sn = (T)0;
    }
}
//----------------------------------------------------------------------------
template <typename T, size_t N>
void TSymmetricEigensolver<T, N>::DoQRImplicitShift(int imin, int imax)
{
    // The implicit shift.  Compute the eigenvalue u of the lower-right 2x2
    // block that is closer to a11.
    T a00 = mDiagonal[imax];
    T a01 = _superdiagonal[imax];
    T a11 = mDiagonal[imax + 1];
    T dif = (a00 - a11) * (T)0.5;
    T sgn = (dif >= (T)0 ? (T)1 : (T)-1);
    T a01sqr = a01 * a01;
    T u = a11 - a01sqr / (dif + sgn*Sqrt(dif*dif + a01sqr));
    T x = mDiagonal[imin] - u;
    T y = _superdiagonal[imin];

    T a12, a22, a23, tmp11, tmp12, tmp21, tmp22, cs, sn;
    T a02 = (T)0;
    int i0 = imin - 1, i1 = imin, i2 = imin + 1;
    for (/**/; i1 <= imax; ++i0, ++i1, ++i2)
    {
        // Compute the Givens rotation and save it for use in computing the
        // eigenvectors.
        GetSinCos(x, y, cs, sn);
        _givens.push_back(FGivensRotation(i1, cs, sn));

        // Update the tridiagonal matrix.  This amounts to updating a 4x4
        // subblock,
        //   b00 b01 b02 b03
        //   b01 b11 b12 b13
        //   b02 b12 b22 b23
        //   b03 b13 b23 b33
        // The four corners (b00, b03, b33) do not change values.  The
        // The interior block {{b11,b12},{b12,b22}} is updated on each pass.
        // For the first pass, the b0c values are out of range, so only
        // the values (b13, b23) change.  For the last pass, the br3 values
        // are out of range, so only the values (b01, b02) change.  For
        // passes between first and last, the values (b01, b02, b13, b23)
        // change.
        if (i1 > imin)
        {
            _superdiagonal[i0] = cs*_superdiagonal[i0] - sn*a02;
        }

        a11 = mDiagonal[i1];
        a12 = _superdiagonal[i1];
        a22 = mDiagonal[i2];
        tmp11 = cs*a11 - sn*a12;
        tmp12 = cs*a12 - sn*a22;
        tmp21 = sn*a11 + cs*a12;
        tmp22 = sn*a12 + cs*a22;
        mDiagonal[i1] = cs*tmp11 - sn*tmp12;
        _superdiagonal[i1] = sn*tmp11 + cs*tmp12;
        mDiagonal[i2] = sn*tmp21 + cs*tmp22;

        if (i1 < imax)
        {
            a23 = _superdiagonal[i2];
            a02 = -sn*a23;
            _superdiagonal[i2] = cs*a23;

            // Update the parameters for the next Givens rotation.
            x = _superdiagonal[i1];
            y = a02;
        }
    }
}
//----------------------------------------------------------------------------
template <typename T, size_t N>
void TSymmetricEigensolver<T, N>::ComputePermutation(int sortType)
{
    _isRotation = -1;

    if (sortType == 0)
    {
        // Set a flag for GetEigenvalues() and GetEigenvectors() to know
        // that sorted output was not requested.
        _permutation[0] = -1;
        return;
    }

    // Compute the permutation induced by sorting.  Initially, we start with
    // the identity permutation I = (0,1,...,N-1).
    struct FSortItem
    {
        T eigenvalue;
        int index;
    };

    FSortItem items[N];
    int i;
    for (i = 0; i < int(N); ++i)
    {
        items[i].eigenvalue = mDiagonal[i];
        items[i].index = i;
    }

    if (sortType > 0)
    {
        std::sort(std::begin(items), std::end(items),
            [](FSortItem const& item0, FSortItem const& item1)
            {
                return item0.eigenvalue < item1.eigenvalue;
            }
        );
    }
    else
    {
        std::sort(std::begin(items), std::end(items),
            [](FSortItem const& item0, FSortItem const& item1)
            {
                return item0.eigenvalue > item1.eigenvalue;
            }
        );
    }

    i = 0;
    for (auto const& item : items)
    {
        _permutation[i++] = item.index;
    }

    // GetEigenvectors() has nontrivial code for computing the orthogonal Q
    // from the reflections and rotations.  To avoid complicating the code
    // further when sorting is requested, Q is computed as in the unsorted
    // case.  We then need to swap columns of Q to be consistent with the
    // sorting of the eigenvalues.  To minimize copying due to column swaps,
    // we use permutation P.  The minimum number of transpositions to obtain
    // P from I is N minus the number of cycles of P.  Each cycle is reordered
    // with a minimum number of transpositions; that is, the eigenitems are
    // cyclically swapped, leading to a minimum amount of copying.  For
    // example, if there is a cycle i0 -> i1 -> i2 -> i3, then the copying is
    //   save = eigenitem[i0];
    //   eigenitem[i1] = eigenitem[i2];
    //   eigenitem[i2] = eigenitem[i3];
    //   eigenitem[i3] = save;
}
//----------------------------------------------------------------------------
template <typename T, size_t N>
TSymmetricEigensolver<T, N>::FGivensRotation::FGivensRotation()
{
    // No default initialization for fast creation of std::vector of objects
    // of this type.
}
//----------------------------------------------------------------------------
template <typename T, size_t N>
TSymmetricEigensolver<T, N>::FGivensRotation::FGivensRotation(int inIndex, T inCs, T inSn)
    :
    index(inIndex),
    cs(inCs),
    sn(inSn)
{
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
