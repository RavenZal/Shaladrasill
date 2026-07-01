#pragma once
#include <cmath>
#include <vector>
#include <cassert>
#include <iostream>
// struct vec3f
// {
//     float x, y, z;
//     vec3f() : x(0), y(0), z(0) {}
//     vec3f(float _v) : x(_v), y(_v), z(_v) {}
//     vec3f(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
// };

//---------------------------------------------------
//normalization
//---------------------------------------------------
template<int N>
struct vec
{
    double data[N] = {0}; 
    double& operator[](const int i)       {assert(i >= 0 && i < N) ; return data[i];}
    double  operator[](const int i) const {assert(i >= 0 && i < N) ; return data[i];}
    double norm2() const {return (*this)* (*this);}
    double norm() const {return std::sqrt(norm2());}
    vec<N> normalized() const {return (*this) / norm();} 
};

//dot
template<int N> double operator * (const vec<N>& v, const vec<N>& w)
{
    double ans = 0;
    for(int i = 0; i < N ; i++)
    {
     ans = ans + (v[i] * w[i]);
    }
    return  ans;
}

//add
template<int N> vec<N> operator + (const vec<N>& v, const vec<N>& w)
{
    vec<N> tem_vec = v;
    for(int i = 0; i < N ; i++)
    {
     tem_vec[i] += w[i];
    }
    return  tem_vec;
}

//deduce
template<int N> vec<N> operator - (const vec<N>& v, const vec<N>& w)
{
    vec<N> tem_vec = v;
    for(int i = 0; i < N ; i++)
    {
     tem_vec[i] -= w[i];
    }
    return  tem_vec;
}

//·
template<int N> vec<N> operator * (const vec<N>& v, const double& num)
{
    vec<N> tem_vec = v;
    for(int i = 0; i < N ; i++)
    {
     tem_vec[i] *= num;
    }
    return  tem_vec;
}

template<int N> vec<N> operator * (const double& num, const vec<N>& v)
{
    return v * num;
}

//division
template<int N> vec<N> operator / (const vec<N>& v, const double& num)
{
    vec<N> tem_vec = v;
    for(int i = 0; i < N ; i++)
    {
     tem_vec[i] /= num;
    }
    return  tem_vec;
}

//print
template<int N> std::ostream& operator<<(std::ostream& os, const vec<N>& v) {
    os << "vec<" << N << ">:" ;
    for (int i=0; i<N; i++) os << v[i] << ".";
    return os;
}

//Ascending dimensions
template<int totalDimensions, int originDimensions> vec<totalDimensions> embed(const vec<originDimensions>& v, double fill = 1)
{
    vec<totalDimensions> ans_vec;
    for(int i = 0 ; i < totalDimensions ; i++)
    {
       ans_vec[i] = (i < originDimensions ? v[i] : fill) ;
    }  
    return ans_vec;
}

//Deducing dimensions
template<int totalDimensions, int originDimensions> vec<totalDimensions> del(const vec<originDimensions>& v)
{
    vec<totalDimensions> ans_vec;
    for(int i = 0 ; i < totalDimensions ; i++)
    {
       ans_vec[i] = v[i] ; 
    }  
    return ans_vec;
}

//Projection
template<int totalDimensions, int originDimensions> vec<totalDimensions> proj(const vec<originDimensions>& v)
{
    static_assert(totalDimensions == originDimensions - 1, "proj: must reduce dimension by exactly 1");
    vec<totalDimensions> ans_vec;
    for(int i = 0 ; i < totalDimensions ; i++)
    {
        ans_vec[i] = v[i] / v[originDimensions - 1];
    }
    return ans_vec;
}

//---------------------------------------------------
//specialization
//---------------------------------------------------

template <> struct vec<2> 
{
    double x = 0, y = 0;
    double& operator[](const int i)       { assert(i>=0 && i<2); return i ? y : x; }
    double  operator[](const int i) const { assert(i>=0 && i<2); return i ? y : x; }
    double norm2() const { return (*this)*(*this); }
    double norm()  const { return std::sqrt(norm2()); }
    vec<2> normalize() { return (*this)/norm(); }
    
    vec<2>(double _x=0, double _y=0) : x(_x), y(_y) {}
};

template <> struct vec<3> 
{
    double x = 0, y = 0, z = 0;
    double& operator[](const int i)       { assert(i>=0 && i<3); return i ? (1==i ? y : z) : x; }
    double  operator[](const int i) const { assert(i>=0 && i<3); return i ? (1==i ? y : z) : x; }
    double norm2() const { return (*this)*(*this); }
    double norm()  const { return std::sqrt(norm2()); }
    vec<3> normalize() { return (*this)/norm(); }

    vec<3>(double _x=0, double _y=0, double _z=0) : x(_x), y(_y), z(_z) {}
};

//cross product
inline vec<3> cross(vec<3> v1 , vec<3> v2)
{
    return vec<3>(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);
} 

template <> struct vec<4>
{
    double x = 0, y = 0, z = 0, w = 0;
    double& operator[](const int i)       { assert(i>=0 && i<4); return i ? (1!=i ? (2 == i ? z : w) : y) : x; }
    double  operator[](const int i) const { assert(i>=0 && i<4); return i ? (1!=i ? (2 == i ? z : w) : y) : x; }
    double norm2() const { return (*this)*(*this); }
    double norm()  const { return std::sqrt(norm2()); }
    vec<4> normalize() { return (*this)/norm(); }
    vec<2> xy() const {return vec<2>(x,y);}

    vec<4>(double _x=0, double _y=0, double _z=0, double _w=0) : x(_x), y(_y), z(_z), w(_w) {}
};


//---------------------------------------------------
//matrix
//---------------------------------------------------
template<int DimRows, int DimCols> struct mat
{
    vec<DimCols> rows[DimRows];
    //mat() {}

    vec<DimCols>& operator[] (const int idx)       { assert(idx>=0 && idx<DimRows); return rows[idx]; }
    const vec<DimCols>& operator[] (const int idx) const { assert(idx>=0 && idx<DimRows); return rows[idx]; }

    vec<DimRows> col(const int idx) const {
        assert(idx>=0 && idx<DimCols);
        vec<DimRows> ret;
        for (int i=DimRows; i--; ret[i]=rows[i][idx]);
        return ret;
    }
    
    void set_col(const int idx, const vec<DimRows> &v) {
        assert(idx>=0 && idx<DimCols);
        for (int i=DimRows; i--; rows[i][idx]=v[i]);
    }

    double det() const {
    static_assert((DimRows == DimCols) && (DimRows == 3), "det() only for square matrices");
    double det = rows[0][0] * (rows[1][1]*rows[2][2] - rows[1][2]*rows[2][1]) - rows[0][1] * (rows[1][0]*rows[2][2] - rows[1][2]*rows[2][0]) + rows[0][2] * (rows[1][0]*rows[2][1] - rows[1][1]*rows[2][0]); 
    return det;
    }

};

//transpose
template <int Rows, int Cols> mat<Cols, Rows> transpose(const mat<Rows, Cols>& m)
{
    mat<Cols , Rows> ans_mat;
    for(int i = Rows - 1 ; i >= 0 ; i--)
    {
       for(int j = Cols - 1 ; j >= 0 ; j--)
       {
          ans_mat[j][i] = m[i][j];
       }
    }
    return ans_mat;
}

//multiplication
template <int m, int n , int c> mat<m, c> operator * (const mat<m, n>& m1 , const mat<n, c>& m2)
{
    mat<m, c> ans_mat;
    mat<c, n> trans_mat = transpose(m2);
    for(int i = m - 1 ; i >= 0 ; i--)
    {
        for(int j = c - 1 ; j >= 0 ; j--)
        {
            ans_mat[i][j] = m1[i] * trans_mat[j];
        }
    }
    return ans_mat;
}

template <int m, int n> vec<m> operator * (const mat<m, n>& m1 , const vec<n>& v)
{
    vec<m> ans_v;
    for(int i = m - 1 ; i >= 0 ; i--)
    {
        ans_v[i] = m1[i] * v;
    }
    return ans_v;
}

//generate identity matrix
template <int length> mat<length , length> genIdentityMatrix()
{
    mat<length , length> Idmat;
    for(int i = length - 1 ; i >= 0 ; i--)
    {
        Idmat[i][i] = 1 ;
    }
    return Idmat;
}

//inversion
template <int length> mat<length , length> inversion(const mat<length, length>& OriginMat)
{
    mat<length , length> OriginMatCopy = OriginMat;
    mat<length , length> AnsMat = genIdentityMatrix<length>();
    for(int col = length - 1 ; col >= 0 ; col--)
    {
        vec<length> colCopy = OriginMatCopy.col(col);
        // Rows below `col` have already been used as pivots for columns to the right,
        // so we must not pick them again here.
        int MaxIdx = col;
        double maxVarInCol = colCopy[col];
        for(int rowIdx = col - 1 ; rowIdx >=0 ; rowIdx--)
        {
            if (std::abs(maxVarInCol) < std::abs(colCopy[rowIdx]))
            {
                maxVarInCol = colCopy[rowIdx];
                MaxIdx = rowIdx;
            }
        }
        //exchange Row
        if(MaxIdx != col)
        {
            vec<length> rowCopy = OriginMatCopy[MaxIdx];
            vec<length> AnsRowCopy = AnsMat[MaxIdx];
            OriginMatCopy[MaxIdx] = OriginMatCopy[col];
            AnsMat[MaxIdx] = AnsMat[col];
            OriginMatCopy[col] = rowCopy;
            AnsMat[col] = AnsRowCopy;
        }
        //normalization
        if (OriginMatCopy[col][col] != 0)
        {
            double mainVarCopy = OriginMatCopy[col][col];
            for(int i = length - 1 ; i >= 0 ; i--)
            {
                OriginMatCopy[col][i] = OriginMatCopy[col][i] / mainVarCopy;
                AnsMat[col][i] = AnsMat[col][i] / mainVarCopy;
            }
        }
        
        for(int row = length - 1 ; row >= 0 ; row --)
        {
            if (row != col)
            {
                double factor = OriginMatCopy[row][col];
                OriginMatCopy[row] = OriginMatCopy[row] - OriginMatCopy[col] * factor;
                AnsMat[row] = AnsMat[row] - AnsMat[col] * factor;
            }
        }
        
    }
    return AnsMat;
}

template <int length> mat<length , length> invert_transpose(const mat<length, length>& Mat)
{
    return transpose(inversion(Mat));
}

//---------------------------------------------------
//Naming
//---------------------------------------------------
using vec3f = vec<3>;
