#ifndef THE1_CONTROLPOINTS_H
#define THE1_CONTROLPOINTS_H
#include <vector>
#include <iostream>
#include <iomanip>
#include <GL/glew.h>
//#include <OpenGL/gl3.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp> // GL Math library header
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
static double margin = 0.00001f;

class ControlPoints {
public:
    vector<glm::vec3> CPV;
    double n, m;
    explicit ControlPoints(vector<vector<double>> m);
};
#endif //THE1_CONTROLPOINTS_H
