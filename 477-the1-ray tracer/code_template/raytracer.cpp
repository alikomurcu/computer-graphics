#include <iostream>
#include <cmath>
#include <algorithm>
#include <cfloat>
#include "parser.h"
#include "ppm.h"
#include "raytracer.h"
#include <thread>

using namespace std;
using namespace parser;

//TODO: add BVH
//TODO: create a threading logic
//TODO: make the project OOP
/************ GLOBAL VARIABLES ************/
Vec3f u, v, w;
//ray curRay;
float epsilon = 0.000001;

HitRecord::HitRecord() : t(-1) {}
HitRecord::HitRecord(Vec3f hP, Vec3f n, int mId, float tP, Type typ)
:hitPoint(hP), normal(n), materialId(mId), t(tP), type(typ) {}
HitRecord& HitRecord::operator=(const HitRecord& h)
{

    hitPoint = h.hitPoint;
    normal = h.normal;
    materialId = h.materialId;
    t = h.t;
    type = h.type;
    return *this;
}

/************ HELPERS MATH ************/
Vec3f add(Vec3f a, Vec3f b)
{
    Vec3f c{};
    c.x = a.x + b.x;
    c.y = a.y + b.y;
    c.z = a.z + b.z;
    return c;
}

void clamp_rgb(int* x)
{
    x[0] = x[0] > 255 ? 255 : x[0];
    x[1] = x[1] > 255 ? 255 : x[1];
    x[2] = x[2] > 255 ? 255 : x[2];
}

Vec3f cross_product(Vec3f a, Vec3f b)
{
    Vec3f c{};
    c.x = a.y*b.z - a.z*b.y;
    c.y = a.z*b.x - a.x*b.z;
    c.z = a.x*b.y - a.y*b.x;
    return c;
}

float det3(matrix m)
{
    float res = m.c1.x*(m.c2.y*m.c3.z - m.c2.z*m.c3.y)
                - m.c2.x*(m.c1.y*m.c3.z - m.c1.z*m.c3.y)
                + m.c3.x*(m.c1.y*m.c2.z - m.c1.z*m.c2.y);
    return res;
}

float dot_product(Vec3f a, Vec3f b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3f init_vec3(float x, float y, float z)
{
    Vec3f ret{};
    ret.x = x;
    ret.y = y;
    ret.z = z;
    return ret;
}

matrix init_matrix(Vec3f c1, Vec3f c2, Vec3f c3)
{
    matrix ret;
    ret.c1 = c1;
    ret.c2 = c2;
    ret.c3 = c3;
    return ret;
}

Vec3f get_otd(ray r, float t)
{
    return add(r.o, scalar_product(t, r.d));
}

Vec3f negate_vec(Vec3f vec)
{
    Vec3f c{};
    c.x = -vec.x;
    c.y = -vec.y;
    c.z = -vec.z;
    return c;
}

Vec3f normalize(Vec3f vec)
{
    return scalar_product(1.0f/magnitude(vec), vec);
}

float magnitude(Vec3f vec)
{
    return sqrt(sqr_vec(vec));
}

Vec3f scalar_product(float s, Vec3f vec)
{
    Vec3f c{};
    c.x = vec.x * s;
    c.y = vec.y * s;
    c.z = vec.z * s;
    return c;
}

float sqr_vec(Vec3f vec)
{
    return dot_product(vec, vec);
}

Vec3f subtract(Vec3f a, Vec3f b)
{
    Vec3f c{};
    c.x = a.x - b.x;
    c.y = a.y - b.y;
    c.z = a.z - b.z;
    return c;
}

/************ HELPERS ALGORITHM ************/
// RGB& calculate_ambient(HitRecord r, RGB& rgb, Scene& scene)
// {
//     rgb[0] = scene.ambient_light.x * getMaterial(r.materialId, scene).x;
//     rgb[1] = scene.ambient_light.y * getMaterial(r.materialId, scene).y;
//     rgb[2] = scene.ambient_light.z * getMaterial(r.materialId, scene).z;
//     return rgb;
// }

HitRecord intersect_with_sphere(Sphere S, Scene& scene, ray& curRay)
{
    float radius = S.radius, delta;
    float t,t1,t2;
    float A,B,C;
    Vec3f c{};
    c = scene.vertex_data[S.center_vertex_id - 1];

    C = sqr_vec(subtract(curRay.o, c)) - radius * radius;

    B = 2 * dot_product(curRay.d, subtract(curRay.o, c));

    A = sqr_vec(curRay.d);

    delta = B*B - 4*A*C;

    if (delta<0){
        return {};     // TODO: return here smt
    }
    else if (delta == 0)
    {
        t = -B / (2*A);
    }
    else
    {
        delta = sqrt(delta);
        A *= 2;
        t1 = (-B + delta) / A;
        t2 = (-B - delta) / A;

        if (t1<t2) t=t1; else t=t2;
    }

    Vec3f hitPoint = get_otd(curRay, t);
//    Vec3f normal = normalize(scalar_product(1.0f/radius, subtract(hitPoint, c)));
    Vec3f normal = normalize(subtract(hitPoint, c));
    return {hitPoint, normal, S.material_id, t, SPHERE};
}

HitRecord intersect_with_triangle(Triangle T, Scene& scene, ray& curRay)
{
    // HANDLE CRAMER'S RULE
    Vec3f a = scene.vertex_data[T.indices.v0_id - 1]; // The reason of the -1 is the indices start from 1 in XML file
    Vec3f b = scene.vertex_data[T.indices.v1_id - 1];
    Vec3f c = scene.vertex_data[T.indices.v2_id - 1];

    Vec3f abg{};      // alpha, beta, gamma

    Vec3f res = subtract(a, curRay.o);

    matrix A = init_matrix(subtract(a, b), subtract(a, c), curRay.d);

    float detA = det3(A);
    if(detA == 0) return {};

    matrix betaMatrix = init_matrix(res, A.c2, A.c3);
    float beta = det3(betaMatrix) / detA;
    if(beta < 0 - epsilon)
    {
        return {};
    }
    matrix gamaMatrix = init_matrix(A.c1, res, A.c3);
    float gama = det3(gamaMatrix) / detA;
    if(gama < 0 - epsilon)
    {
        return {};
    }

    if(beta + gama > 1 + epsilon)
    {
        return {};
    }

    matrix tMatrix = init_matrix(A.c1, A.c2, res);
    float t = det3(tMatrix) / detA;
    if(t <= 0)
    {
        return {};
    }


    Vec3f hitPoint = get_otd(curRay, t);
    Vec3f normal = normalize(cross_product(subtract(b, a), subtract(c, b)));
    return {hitPoint, normal, T.material_id, t, TRIANGLE};
}

Material getMaterial(int id, Scene& scene)
{
    return scene.materials[id-1];
}

/************ BASE FUNCTIONS ************/
void calculate_shadow(Scene& scene, HitRecord& hit)
{

}
int* compute_color(unsigned char* image, int x, int y, int width, Scene& scene, Camera& curCam, ray& curRay, int rCount)
{
    // Intersect the ray
    int* rgb = new int[3];
    HitRecord r = detect_collision(scene, curRay, false);
    Material curMat = getMaterial(r.materialId, scene);

    if((r.t < 0 || r.t == FLT_MAX) && rCount == scene.max_recursion_depth)   // ray does not intersect
    {
        // No intersect
        rgb[0] = scene.background_color.x;
        rgb[1] = scene.background_color.y;
        rgb[2] = scene.background_color.z;

        return rgb;     // There is no such collisions
    }

    if(r.t < 0)
    {
        rgb[0] = rgb[1] = rgb[2] = 0;
        return rgb;
    }

    // Ambient Shading

    rgb[0] = (int)round(scene.ambient_light.x * getMaterial(r.materialId, scene).ambient.x);
    rgb[1] = (int)round(scene.ambient_light.y * getMaterial(r.materialId, scene).ambient.y);
    rgb[2] = (int)round(scene.ambient_light.z * getMaterial(r.materialId, scene).ambient.z);

    // Handle Multiple Lights
    std::vector<PointLight> point_lights = scene.point_lights;
    int sz = (int)point_lights.size();

    for(int i=0; i<sz; i++)      // For each light source
    {
        PointLight curLight = point_lights[i];
        Vec3f point_to_light = subtract(curLight.position, r.hitPoint);
        float d_square = sqr_vec(point_to_light);

    // Shadows

        float distance = magnitude(point_to_light);
        ray shadowRay{};
        float shEpsilon = scene.shadow_ray_epsilon;
        shadowRay.o = add(r.hitPoint, scalar_product(shEpsilon, normalize(point_to_light)));
        shadowRay.d = normalize(point_to_light);
        HitRecord shadowHit = detect_collision(scene, shadowRay, true);
        if(shadowHit.t > 0 && shadowHit.t < distance)
            continue;

    // Diffuse Shading

        float cos_a = dot_product(r.normal, normalize(point_to_light));
        if(cos_a < 0) cos_a = 0;
        // else if(cos_a > 1) cos_a = 1;
        rgb[0] += (int)round((curLight.intensity.x / d_square) * cos_a * getMaterial(r.materialId, scene).diffuse.x);
        rgb[1] += (int)round((curLight.intensity.y / d_square) * cos_a * getMaterial(r.materialId, scene).diffuse.y);
        rgb[2] += (int)round((curLight.intensity.z / d_square) * cos_a * getMaterial(r.materialId, scene).diffuse.z);

    // Specular shading

        Vec3f point_to_eye = normalize(subtract(curCam.position, r.hitPoint));
        Vec3f half_vec = scalar_product(1.0f/2, add(normalize(point_to_light), point_to_eye));
        float cos_b = dot_product(r.normal, normalize(half_vec));
        if(cos_b < 0) cos_b = 0;
        rgb[0] += (int)round((curLight.intensity.x / d_square) * pow(cos_b, curMat.phong_exponent) * curMat.specular.x);
        rgb[1] += (int)round((curLight.intensity.y / d_square) * pow(cos_b, curMat.phong_exponent) * curMat.specular.y);
        rgb[2] += (int)round((curLight.intensity.z / d_square) * pow(cos_b, curMat.phong_exponent) * curMat.specular.z);


    }

    // Mirror
    int* reflectedRGB = new int[3];
    reflectedRGB[0] = reflectedRGB[1] = reflectedRGB[2] = 0;
    if(curMat.is_mirror && rCount > 0)
    {
        ray reflectedRay;

        Vec3f w_0 = curRay.d;
        Vec3f reflectDir = add(w_0, scalar_product(-2* dot_product(r.normal, w_0), r.normal));
        reflectedRay.o = add(r.hitPoint, scalar_product(scene.shadow_ray_epsilon, reflectDir));
        reflectedRay.d = normalize(reflectDir);
        reflectedRGB = compute_color(image, x, y, width, scene, curCam, reflectedRay, (rCount-1));

        rgb[0] += reflectedRGB[0] * curMat.mirror.x;
        rgb[1] += reflectedRGB[1] * curMat.mirror.y;
        rgb[2] += reflectedRGB[2] * curMat.mirror.z;
    }

    clamp_rgb(rgb);
    return rgb;
}


HitRecord detect_collision(Scene& scene, ray& curRay, bool shadow)
{
    // check if ray intersects any object in the scene
    float tMin = FLT_MAX;
    HitRecord hit;
    // SPHERE intersections
    int sCount = (int)scene.spheres.size();
    vector<Sphere> spheres = scene.spheres;
    for(int i=0; i<sCount; i++)
    {
        HitRecord tmp = intersect_with_sphere(spheres[i], scene, curRay);     // TODO:
        if(tmp.t < tMin && tmp.t>=0)
        {
            tMin = tmp.t;
            hit = tmp;
            if(shadow) return hit;
        }
    }

    // TRIANGLE intersections
    int tCount = (int)scene.triangles.size();
    vector<Triangle> triangles = scene.triangles;
    for(int i=0; i<tCount; i++)
    {
        HitRecord tmp = intersect_with_triangle(triangles[i], scene, curRay);
        if(tmp.t < tMin && tmp.t>=0)
        {
            tMin = tmp.t;
            hit = tmp;
            if(shadow) return hit;
        }
    }

    // MESH intersections
    int mCount = (int)scene.meshes.size();
    vector<Mesh> meshes = scene.meshes;
    for(int i=0; i<mCount; i++)
    {
        int fCount = (int)meshes[i].faces.size();
        vector<Face> meshFaces = meshes[i].faces;
        int mId = meshes[i].material_id;

        for(int j=0; j<fCount; j++)
        {
            Triangle T{};
            T.material_id = mId;
            T.indices = meshFaces[j];
            HitRecord tmp = intersect_with_triangle(T, scene, curRay);
            if(tmp.t < tMin && tmp.t>=0)
            {
                tMin = tmp.t;
                hit = tmp;
                hit.type = MESH;
                if(shadow) return hit;
            }
        }
    }
    return hit;
}

ray generate_ray(const Camera& curCam, int i, int j)
{
    // implement ray-formula
    // Camera planes
    float left_plane = curCam.near_plane.x;
    float right_plane = curCam.near_plane.y;
    float bottom_plane = curCam.near_plane.z;
    float top_plane = curCam.near_plane.w;
    float su, sv;
    ray res;

    su = ((float)i+0.5f) * ((right_plane - left_plane) / (float)curCam.image_width);
    sv = ((float)j+0.5f) * ((top_plane - bottom_plane) / (float)curCam.image_height);
    Vec3f m{}, q{}, s{};        // Centre of the image
    m = add(curCam.position, scalar_product(curCam.near_distance, curCam.gaze));        // m = pos + gaze * distance
    q = add(m, add(scalar_product(left_plane, u), scalar_product(top_plane, v)));
    s = add(q, add(scalar_product(su, u), scalar_product(sv, negate_vec(v))));
    res.o = curCam.position;
    res.d = normalize(subtract(s, curCam.position));       // s - camPos  i.e., direction of the ray
    return res;
}

void set_pixel_value(unsigned char* image, int y, int x, int width, const RGB value)
{
    int index = (y*width + x)*3;     // The 1-D equivalent index of 2-D image coordinates
    image[index] = value[0];     // R
    image[index+1] = value[1];     // G
    image[index+2] = value[2];       // B
}

void start_thread(unsigned char* image, int height, int width, Scene* scene, Camera* curCam, int threadNo, int numberOfThreads)
{
    const clock_t begin_time = clock();

    int columnSize = (int) width / numberOfThreads;
    for(int y = 0; y < height; y++)
    {
        for(int x = threadNo*columnSize; x < (threadNo + 1)*columnSize; x++)
        {
            RGB rgb1;
            ray curRay = generate_ray(*curCam, x, y);        // Create a thread logic here
            int* rgb = compute_color(image, y, x, width, *scene, *curCam, curRay, scene->max_recursion_depth);
            set_pixel_value(image, y, x, width, rgb);
        }
    }
}

int main(int argc, char* argv[])
{
    const clock_t begin_time = clock();

    // Sample usage for reading an XML scene file
    Scene scene;

    //scene.loadFromXml("../test_scenes/inputs/horse_and_mug.xml");
    scene.loadFromXml(argv[1]);
    vector<Camera> cameras = scene.cameras;

    unsigned int nThreads = thread::hardware_concurrency();
    std::thread* threads = new std::thread[nThreads];

    /*********** ITERATE OVER EACH CAMERA in XML ***********/
    for(int c = 0; c < cameras.size(); c++)
    {
        Camera curCam = cameras[c];     // Current Camera
        int height = curCam.image_height;
        int width = curCam.image_width;
        unsigned char *image = new unsigned char[height * width * 3];
        v = curCam.up;
        w = negate_vec(curCam.gaze);
        u = normalize(cross_product(v, w));     // TODO: check this vectors
        /*********** ITERATE OVER EACH PIXEL ***********/
        for(int i=0; i<nThreads; i++)
        {
            threads[i] = std::thread(start_thread, image, height, width, &scene, &curCam, i, (int) nThreads);
            //threads[i] = std::thread(&start_thread, image, height, width, &scene);
        }
        for(int i=0; i<nThreads; i++)
        {
            threads[i].join();
        }
        write_ppm(curCam.image_name.c_str(), image, width, height);
    }
    delete[] threads;
}
