#include "Object.h"
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <random>

double scale_formula(int parallel,int other){
    if(parallel*other <= 25)
        return 5.0/parallel;
    else if(parallel*other <= 100)
        return 5.4/parallel;
    else if(parallel*other <= 400)
        return 5.6/parallel;
    else
        return 5.7/parallel;
}

int getRandom() 
{
    // std::srand(std::time(NULL)); //use current time as seed for random generator
    // return std::rand() % 8;
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> dist(1, 5);
    return dist(generator);
}

Object::Object()
{
    scaleX = 1.0;
    scaleY = 1.0;
    Vec3 position = {0,0,0};
    Vec3 color = {0,0,0};
    colorIndex = -1;    
}

Object::Object(int i,int j, int height, int width){
    this -> scaleX = scale_formula(height,width);
    this -> scaleY = scale_formula(width,height);

    this -> colorIndex = getRandom();

    this->height = height;
    this->width = width;

    double edge_center_i = 8.0/height;
    double edge_center_j = 9.0/width;

    this -> position.x = -9 + edge_center_j + 2*edge_center_j*j;
    this -> position.y = -(-8 + edge_center_i + 2*edge_center_i*i);
    this -> position.z = -10;

    this -> color.x = ((this->colorIndex>>2)%2); //255 if index = {4,5,6,7}
    this -> color.y = ((this->colorIndex>>1)%2); //255 if index = {2,3,6,7}
    this -> color.z = ((this->colorIndex)%2); //255 if index = {1,3,5,7}

    this -> explode =false;
}

bool Object::onehalf(){
    if(this -> scaleX > 1.5 * scale_formula(height,width)){
        return true;
    }
    return false;
}

void Object::reset_scale(){
    this -> scaleX = scale_formula(height,width);
    this -> scaleY = scale_formula(width,height);
}

void Object::enlarge_scale(){
    this -> scaleX *= 1.01;
    this -> scaleY *= 1.01;
}



void Object::restart(){
    this -> colorIndex = getRandom();
    this -> color.x = (this->colorIndex>>2)%2; //255 if index = {4,5,6,7}
    this -> color.y = (this->colorIndex>>1)%2; //255 if index = {2,3,6,7}
    this -> color.z = (this->colorIndex)%2; //255 if index = {1,3,5,7}
    reset_scale();
}