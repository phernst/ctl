#include "matrix_algorithm.h"
#include <QDebug>

namespace CTL {
namespace mat {

static bool qrdcmp(Matrix3x3 &a, double *d);
static Matrix3x3 mirror(const Matrix3x3 &a);
static void positiveDiag4RQ(PairMat3x3 *qr);

PairMat3x3 QRdecomposition(const Matrix3x3 &A)
{
    Vector3x1 d;
    Matrix3x3 Q,Q1,Q2;
    Matrix3x3 Btmp = A;

    if(qrdcmp(Btmp, d.data())) // check if singular
        qWarning() << "QR decomposition: matrix is close to singular";

    const auto B = Btmp.transposed();
    const Matrix3x3 R(
      { d.get<0>(), B.get<1,0>(), B.get<2,0>(),
               0.0,   d.get<1>(), B.get<2,1>(),
               0.0,          0.0,   d.get<2>()  });

    Vector3x1 u1({ B.get<0,0>(), B.get<0,1>(), B.get<0,2>() });
    Vector3x1 u2({          0.0, B.get<1,1>(), B.get<1,2>() });
    u1 /= u1.norm();
    u2 /= u2.norm();
    Q1 = eye<3>() - 2.0*u1*u1.transposed();
    Q2 = eye<3>() - 2.0*u2*u2.transposed();
    Q = Q1*Q2;

    return {Q,R};
}

/*
* Apply QR to FA'F. Transform result again with Q->FQ'F and R->FR'F.
* Moreover enforce uniqueness with C, so that diagonal of R is positive.
*
* Finally, the total decomposition FA'F = QR w.r.t. A is
*
* A = (F R' F C) (C F Q' F)
*   =    ^R_final     ^Q_final
*
* with F=F', C=C' and FF=CC=1:
*
* F = 0 0 1  ,   C = ±1  0  0
*     0 1 0           0 ±1  0
*     1 0 0           0  0 ±1  .
*
* When setting unique = normalize = true, one obtains the classical pinhole
* camera decomposition with the nomenclature R->K and Q->R (P=K[R|t]) with
* K = fx  s px
*      0 fy py
*      0  0  1
*/
PairMat3x3 RQdecomposition(const Matrix3x3 &A, bool unique, bool normalize)
{
    auto ret = QRdecomposition( mirror(A) );
    ret = { mirror(ret.Q), mirror(ret.R) };

    // reject scale (+ enforce positive determinant)
    if(normalize)
    {
        bool singular = qFuzzyIsNull(ret.R.get<2,2>());
        if(singular) qWarning() << "mat::RQdecomposition: unable to normalize, since R(2,2)==0";
        Q_ASSERT(!singular);
        double det = ret.R.get<0,0>() * ret.R.get<1,1>() * ret.R.get<2,2>();
        ret.R /= std::copysign(ret.R.get<2,2>(), det);
    }

    // enforce uniqueness (diagonal of R positive)
    if(unique)
        positiveDiag4RQ(&ret);

    return ret;
}

double det(const Matrix3x3 & m)
{
    return
        m[0][0]*(m[1][1]*m[2][2] - m[2][1]*m[1][2]) -
        m[0][1]*(m[1][0]*m[2][2] - m[2][0]*m[1][2]) +
        m[0][2]*(m[1][0]*m[2][1] - m[2][0]*m[1][1]) ;
}


// # static functions

bool qrdcmp(Matrix3x3 & a, double *d)
{
    uint i, j;
    constexpr uint n = 3;
    double scale, sigma, sum, tau;
    double c[n-1];
    bool sing = false;
    for(uint k=0; k<n-1; k++)
    {
        scale = 0.0;
        for(i=k; i<n; i++)
            if(scale<fabs(a[i][k]))
                scale=fabs(a[i][k]);
        if(qFuzzyIsNull(scale))
        {
            sing = true;
            c[k] = 0.0;
            d[k] = 0.0;
        }
        else
        {
            for(i=k; i<n; i++)
                a[i][k] /= scale;
            for(sum=0.0, i=k; i<n; i++)
                sum += a[i][k]*a[i][k];

            sigma = std::copysign(sqrt(sum), a[k][k]);
            a[k][k] += sigma;
            c[k] = sigma*a[k][k];
            d[k] = -scale*sigma;

            for(j=k+1; j<n; j++)
            {
                for(sum=0.0, i=k; i<n; i++)
                    sum += a[i][k]*a[i][j];

                tau = sum/c[k];
                for(i=k; i<n; i++)
                    a[i][j] -= tau*a[i][k];
            }
        }
    }
    d[n-1] = a[n-1][n-1];
    if(qFuzzyIsNull(d[n-1]))
        sing = true;
    return sing;
}

Matrix3x3 mirror(const Matrix3x3 &a)
{
    return {{
        a.get<2,2>(), a.get<1,2>(), a.get<0,2>(),
        a.get<2,1>(), a.get<1,1>(), a.get<0,1>(),
        a.get<2,0>(), a.get<1,0>(), a.get<0,0>()  }};
}

void positiveDiag4RQ(PairMat3x3 *qr)
{
    auto & R = qr->R;
    auto & Q = qr->Q;

    for(uint d = 0; d < 3; ++d)
    {
        if(R(d,d) < 0.0)
            for(uint i = 0; i < 3; ++i)
            {
                R(i,d) = -R(i,d);
                Q(d,i) = -Q(d,i);
            }
    }
}

} // namespace mat
} // namespace CTL
