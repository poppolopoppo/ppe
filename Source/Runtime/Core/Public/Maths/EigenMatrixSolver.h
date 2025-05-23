#pragma once

#include "Core.h"

#include "Container/Vector.h"
#include "HAL/PlatformMemory.h"

#include "Maths/ScalarMatrix_fwd.h"
#include "Maths/ScalarVector_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// http://www.geometrictools.com/Documentation/LeastSquaresFitting.pdf
// http://www.geometrictools.com/GTEngine/Include/Mathematics/GteSymmetricEigensolver3x3.h
//----------------------------------------------------------------------------
// The TSymmetricEigensolver class is an implementation of Algorithm 8.2.3
// (Symmetric QR Algorithm) described in "Matrix Computations, 2nd edition"
// by G. H. Golub and C. F. Van Loan, The Johns Hopkins University Press,
// Baltimore MD, Fourth Printing 1993.  Algorithm 8.2.1 (Householder
// Tridiagonalization) is used to reduce matrix A to tridiagonal T.
// Algorithm 8.2.2 (Implicit Symmetric QR Step with Wilkinson Shift) is
// used for the iterative reduction from tridiagonal to diagonal.  If A is
// the original matrix, D is the diagonal matrix of eigenvalues, and Q is
// the orthogonal matrix of eigenvectors, then theoretically Q^T*A*Q = D.
// Numerically, we have errors E = Q^T*A*Q - D.  Algorithm 8.2.3 mentions
// that one expects |E| is approximately u*|A|, where |M| denotes the
// Frobenius norm of M and where u is the unit roundoff for the
// floating-point arithmetic: 2^{-23} for 'float', which is FLT_EPSILON
// = 1.192092896e-7f, and 2^{-52} for'double', which is DBL_EPSILON
// = 2.2204460492503131e-16.
//
// The condition |a(i,i+1)| <= epsilon*(|a(i,i) + a(i+1,i+1)|) used to
// determine when the reduction decouples to smaller problems is implemented
// as:  sum = |a(i,i)| + |a(i+1,i+1)|; sum + |a(i,i+1)| == sum.  The idea is
// that the superdiagonal term is small relative to its diagonal neighbors,
// and so it is effectively zero.  The unit tests have shown that this
// interpretation of decoupling is effective.
//
// The authors suggest that once you have the tridiagonal matrix, a practical
// implementation will store the diagonal and superdiagonal entries in linear
// arrays, ignoring the theoretically zero values not in the 3-band.  This is
// good for cache coherence.  The authors also suggest storing the Householder
// vectors in the lower-triangular portion of the matrix to save memory.  The
// implementation uses both suggestions.
//
// For matrices with randomly generated values in [0,1], the unit tests
// produce the following information for N-by-N matrices.
//
// N  |A|     |E|        |E|/|A|    iterations
// -------------------------------------------
//  2  1.2332 5.5511e-17 4.5011e-17  1
//  3  2.0024 1.1818e-15 5.9020e-16  5
//  4  2.8708 9.9287e-16 3.4585e-16  7
//  5  2.9040 2.5958e-15 8.9388e-16  9
//  6  4.0427 2.2434e-15 5.5493e-16 12
//  7  5.0276 3.2456e-15 6.4555e-16 15
//  8  5.4468 6.5626e-15 1.2048e-15 15
//  9  6.1510 4.0317e-15 6.5545e-16 17
// 10  6.7523 4.9334e-15 7.3062e-16 21
// 11  7.1322 7.1347e-15 1.0003e-15 22
// 12  7.8324 5.6106e-15 7.1633e-16 24
// 13  8.1073 5.1366e-15 6.3357e-16 25
// 14  8.6257 8.3496e-15 9.6798e-16 29
// 15  9.2603 6.9526e-15 7.5080e-16 31
// 16  9.9853 6.5807e-15 6.5904e-16 32
// 17 10.5388 8.0931e-15 7.6793e-16 35
// 18 11.2377 1.1149e-14 9.9218e-16 38
// 19 11.7105 1.0711e-14 9.1470e-16 42
// 20 12.2227 1.7723e-14 1.4500e-15 42
// 21 12.7411 1.2515e-14 9.8231e-16 47
// 22 13.4462 1.2645e-14 9.4046e-16 50
// 23 13.9541 1.2899e-14 9.2444e-16 47
// 24 14.3298 1.6337e-14 1.1400e-15 53
// 25 14.8050 1.4760e-14 9.9701e-16 54
// 26 15.3914 1.7076e-14 1.1094e-15 57
// 27 15.8430 1.9714e-14 1.2443e-15 60
// 28 16.4771 1.7148e-14 1.0407e-15 60
// 29 16.9909 1.7309e-14 1.0187e-15 60
// 30 17.4456 2.1453e-14 1.2297e-15 64
// 31 17.9909 2.2069e-14 1.2267e-15 68
//
// The eigenvalues and |E|/|A| values were compared to those generated by
// Mathematica Version 9.0; Wolfram Research, Inc., Champaign IL, 2012.
// The results were all comparable with eigenvalues agreeing to a large
// number of decimal places.
//
// Timing on an Intel (R) Core (TM) i7-3930K CPU @ 3.20 GHZ (in seconds):
//
// N    |E|/|A|    iters tridiag QR     evecs    evec[N]  comperr
// --------------------------------------------------------------
//  512 4.4149e-15 1017   0.180  0.005    1.151    0.848    2.166
// 1024 6.1691e-15 1990   1.775  0.031   11.894   12.759   21.179
// 2048 8.5108e-15 3849  16.592  0.107  119.744  116.56   212.227
//
// where iters is the number of QR steps taken, tridiag is the computation
// of the Householder reflection vectors, evecs is the composition of
// Householder reflections and Givens rotations to obtain the matrix of
// eigenvectors, evec[N] is N calls to get the eigenvectors separately, and
// comperr is the computation E = Q^T*A*Q - D.  The construction of the full
// eigenvector matrix is, of course, quite expensive.  If you need only a
// small number of eigenvectors, use function GetEigenvector(int,T*).

template <typename T, size_t N>
class TSymmetricEigensolver {
public:
    // The number N of rows and columns of the matrices to be processed.
    STATIC_CONST_INTEGRAL(size_t, Dim, N);

    typedef TScalarVector<T, N> vector_type;
    typedef TScalarMatrix<T, N, N> matrix_type;

    // The solver processes NxN symmetric matrices, where N > 1 ('size' is N)
    // and the matrix is stored in row-major order.  The maximum number of
    // iterations ('maxIterations') must be specified for the reduction of a
    // tridiagonal matrix to a diagonal matrix.  The goal is to compute
    // NxN orthogonal Q and NxN diagonal D for which Q^T*A*Q = D.
    TSymmetricEigensolver(size_t maxIterations);

    // A copy of the NxN symmetric input is made internally.  The order of
    // the eigenvalues is specified by sortType: -1 (decreasing), 0 (no
    // sorting), or +1 (increasing).  When sorted, the eigenvectors are
    // ordered accordingly.  The return value is the number of iterations
    // consumed when convergence occurred, 0xFFFFFFFF when convergence did
    // not occur, or 0 when N <= 1 was passed to the constructor.
    size_t Solve(const matrix_type& input, int sortType);

    // Get the eigenvalues of the matrix passed to Solve(...).  The input
    // 'eigenvalues' must have N elements.
    void GetEigenvalues(vector_type& eigenvalues) const;

    // Accumulate the Householder reflections and Givens rotations to produce
    // the orthogonal matrix Q for which Q^T*A*Q = D.  The input
    // 'eigenvectors' must be NxN and stored in row-major order.
    void GetEigenvectors(matrix_type& eigenvectors) const;

    // With no sorting, when N is odd the matrix returned by GetEigenvectors
    // is a reflection and when N is even it is a rotation.  With sorting
    // enabled, the type of matrix returned depends on the permutation of
    // columns.  If the permutation has C cycles, the minimum number of column
    // transpositions is T = N-C.  Thus, when C is odd the matrix is a
    // reflection and when C is even the matrix is a rotation.
    bool IsRotation() const;

    // Compute a single eigenvector, which amounts to computing column c
    // of matrix Q.  The reflections and rotations are applied incrementally.
    // This is useful when you want only a small number of the eigenvectors.
    void GetEigenvector(int c, vector_type& eigenvector) const;
    T GetEigenvalue(int c) const;

private:
    // Tridiagonalize using Householder reflections.  On input, _matrix is a
    // copy of the input matrix.  On output, the upper-triangular part of
    // _matrix including the diagonal stores the tridiagonalization.  The
    // lower-triangular part contains 2/Dot(v,v) that are used in computing
    // eigenvectors and the part below the subdiagonal stores the essential
    // parts of the Householder vectors v (the elements of v after the
    // leading 1-valued component).
    void Tridiagonalize();

    // A helper for generating Givens rotation sine and cosine robustly.
    void GetSinCos(T u, T v, T& cs, T& sn);

    // The QR step with implicit shift.  Generally, the initial T is unreduced
    // tridiagonal (all subdiagonal entries are nonzero).  If a QR step causes
    // a superdiagonal entry to become zero, the matrix decouples into a block
    // diagonal matrix with two tridiagonal blocks.  These blocks can be
    // reduced independently of each other, which allows for parallelization
    // of the algorithm.  The inputs imin and imax identify the subblock of T
    // to be processed.   That block has upper-left element T(imin,imin) and
    // lower-right element T(imax,imax).
    void DoQRImplicitShift(int imin, int imax);

    // Sort the eigenvalues and compute the corresponding permutation of the
    // indices of the array storing the eigenvalues.  The permutation is used
    // for reordering the eigenvalues and eigenvectors in the calls to
    // GetEigenvalues(...) and GetEigenvectors(...).
    void ComputePermutation(int sortType);

    // The maximum number of iterations for reducing the tridiagonal mtarix
    // to a diagonal matrix.
    size_t _maxIterations;

    // The internal copy of a matrix passed to the solver.  See the comments
    // about function Tridiagonalize() about what is stored in the matrix.
    matrix_type _matrix;  // NxN elements

    // TAfter the initial tridiagonalization by Householder reflections, we no
    // longer need the full _matrix.  Copy the diagonal and superdiagonal
    // entries to linear arrays in order to be cache friendly.
    vector_type mDiagonal;  // N elements
    TScalarVector<T, N-1> _superdiagonal; // N-1 elements

    // The Givens rotations used to reduce the initial tridiagonal matrix to
    // a diagonal matrix.  A rotation is the identity with the following
    // replacement entries:  R(index,index) = cs, R(index,index+1) = sn,
    // R(index+1,index) = -sn, and R(index+1,index+1) = cs.  If N is the
    // matrix size and K is the maximum number of iterations, the maximum
    // number of Givens rotations is K*(N-1).  The maximum amount of memory
    // is allocated to store these.
    struct FGivensRotation
    {
        FGivensRotation();
        FGivensRotation(int inIndex, T inCs, T inSn);
        int index;
        T cs, sn;
    };

    VECTOR(Maths, FGivensRotation) _givens;  // K*(N-1) elements

    // When sorting is requested, the permutation associated with the sort is
    // stored in _permutation.  When sorting is not requested, _permutation[0]
    // is set to -1.  mVisited is used for finding cycles in the permutation.
    TScalarVector<int, N> _permutation;  // N elements
    mutable TScalarVector<int, N> _visited;  // N elements
    mutable int _isRotation;  // 1 = rotation, 0 = reflection, -1 = unknown

    // Temporary storage to compute Householder reflections and to support
    // sorting of eigenvectors.
    mutable vector_type _vectorP;  // N elements
    mutable vector_type _vectorV;  // N elements
    mutable vector_type _vectorW;  // N elements
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Maths/EigenMatrixSolver-inl.h"
