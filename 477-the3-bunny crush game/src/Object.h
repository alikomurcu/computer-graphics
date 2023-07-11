#ifndef __OBJECT_H__
#define __OBJECT_H__

struct Vec3
{
    double x,y,z;
};

class Object {
public:
    Object();
    Object(int i,int j, int height, int width);
    void reset_scale();
    void enlarge_scale();
    bool onehalf();
    void reshape(int,int,int,int);
    void restart();
    double scaleX, scaleY;
    double rightbottomx;
    double rightbottomy;
    Vec3 position;
    Vec3 destPos;
    Vec3 color;
    int colorIndex;
    double lefttopx;
    double lefttopy;
    bool explode;
private:
    int height, width;
};

#endif