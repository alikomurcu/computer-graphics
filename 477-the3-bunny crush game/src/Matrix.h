#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <vector>
#include "Object.h"



class Matrix {
public:
    Matrix(int rows, int cols);
    Matrix(const Matrix& m);

    Matrix& operator=(const Matrix& m);

    std::vector<std::vector<Object>> contents;
    int rows;
    int cols;
    std::vector<std::vector<Vec3>>  positions;
    std::vector<std::vector<double>>  lefttopx;
    std::vector<std::vector<double>>  rightbottomx;
    std::vector<std::vector<double>>  lefttopy;
    std::vector<std::vector<double>>  rightbottomy;
    void getPosition();
    void getCursor(int,int);
};

#endif