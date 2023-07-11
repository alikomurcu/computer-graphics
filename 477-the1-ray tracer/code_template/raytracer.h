#include <iostream>
#include "parser.h"
#include "ppm.h"

using namespace std;
typedef int RGB[3];

using namespace parser;

/************ HELPERS STRUCTS ************/
typedef struct
{
    Vec3f o,d;
} ray;

typedef struct
{
    Vec3f c1, c2, c3;       // columns of the matrix
} matrix;

enum Type {SPHERE, TRIANGLE, MESH};     // Type of the object
enum sphere_ray_info {OUTSIDE, ON, INSIDE};


class HitRecord
{
    public:
        Vec3f hitPoint{};
        Vec3f normal{};
        int materialId{};
        float t;
        Type type;
        HitRecord(Vec3f hP, Vec3f n, int mId, float tP, Type typ);
        HitRecord();
        HitRecord& operator=(const HitRecord& h);
};


/************ HELPERS MATH ************/
Vec3f add(Vec3f a, Vec3f b);

Vec3f cross_product(Vec3f a, Vec3f b);

float det3(matrix m);

float dot_product(Vec3f a, Vec3f b);

Vec3f negate_vec(Vec3f v);

Vec3f normalize(Vec3f v);

float magnitude(Vec3f v);

Vec3f scalar_product(float s, Vec3f v);

float sqr_vec(Vec3f v);

Vec3f subtract(Vec3f a, Vec3f b);

/************ HELPERS ALGORITHM ************/
float intersect_with_sphere(Sphere S);
/************ BASE FUNCTIONS ************/
int* compute_color(unsigned char* image, int x, int y, int width, Scene& scene, Camera& curCam, ray& curRay, int rCount);

HitRecord detect_collision(Scene& scene, ray& curRay, bool shadow = false);

ray generate_ray(const Camera& curCam, int i, int j);

void set_pixel_value(unsigned char* image, int x, int y, int height, const RGB value);
