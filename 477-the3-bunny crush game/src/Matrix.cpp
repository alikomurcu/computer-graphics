#include "Matrix.h"
#include <iostream>

Matrix::Matrix(int rows, int cols) {
    this->rows = rows;
    this->cols = cols;
    this->contents = std::vector<std::vector<Object>>(rows, std::vector<Object>(cols));
    this->positions = std::vector<std::vector<Vec3>>(rows, std::vector<Vec3>(cols));
    this->lefttopx = std::vector<std::vector<double>>(rows, std::vector<double>(cols));
    this->lefttopy = std::vector<std::vector<double>>(rows, std::vector<double>(cols));
    this->rightbottomx = std::vector<std::vector<double>>(rows, std::vector<double>(cols));
    this->rightbottomy = std::vector<std::vector<double>>(rows, std::vector<double>(cols));
}

void Matrix::getPosition(){
    for(int i = 0; i < this->rows; ++i){
        for(int j = 0; j < this->cols; ++j){
            this->positions[i][j] = this->contents[i][j].position;
        }
    }
}

void Matrix::getCursor(int height, int width){
    double edge_center_i_pixel = height*8.0/(20*this->rows);
    double edge_center_j_pixel = width*9.0/(20*this->cols);
     
    for(int i =0; i < this->rows; ++i){
        for(int j = 0; j < this->cols; ++j){
            this -> lefttopx[i][j] =  0.5*width/20.0 + edge_center_j_pixel + 2*edge_center_j_pixel*j;
            this -> lefttopy[i][j] =  1*height/20.0 + edge_center_i_pixel + 2*edge_center_i_pixel*i;
            this -> rightbottomx[i][j] =  lefttopx[i][j] + 2*edge_center_j_pixel;
            this -> rightbottomy[i][j] =  lefttopy[i][j] + 2*edge_center_i_pixel;

        }
    }
}