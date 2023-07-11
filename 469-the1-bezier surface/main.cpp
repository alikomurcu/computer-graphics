#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp> // GL Math library header
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ControlPoints.h"

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace std;

std::vector<unsigned int> indices;

GLuint vao;
GLuint gProgram;
int gWidth, gHeight;
int samples = 10;
ControlPoints *cp;

GLfloat rotZ;
GLfloat rotationAngle = -30.0f;
GLfloat coordMultiplier = 1.0f;
GLfloat startingVal = -0.5f;
GLfloat endingVal;
GLfloat startX, endX, startY, endY;

GLint modelingMatrixLoc;
GLint viewingMatrixLoc;
GLint projectionMatrixLoc;
GLint eyePosLoc;
GLint cpLoc;
GLint startValLoc;
GLint lposLoc, lcolLoc;
GLint samplesLoc;
GLint nLoc, mLoc;

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix;
glm::vec3 eyePos(0, 0, 0);

int activeProgramIndex = 1;

struct Vertex
{
	Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
	GLfloat x, y, z;
};

struct Texture
{
	Texture(GLfloat inU, GLfloat inV) : u(inU), v(inV) { }
	GLfloat u, v;
};

struct Normal
{
	Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
	GLfloat x, y, z;
};

struct Face
{
	Face(int v[], int t[], int n[]) {
		vIndex[0] = v[0];
		vIndex[1] = v[1];
		vIndex[2] = v[2];
		tIndex[0] = t[0];
		tIndex[1] = t[1];
		tIndex[2] = t[2];
		nIndex[0] = n[0];
		nIndex[1] = n[1];
		nIndex[2] = n[2];
	}
	GLuint vIndex[3], tIndex[3], nIndex[3];
};

struct PointLight {
    GLfloat x, y, z;
    GLfloat r, g, b;
};
vector<Vertex> gVertices;
vector<Texture> gTextures;
vector<Normal> gNormals;
vector<Face> gFaces;

vector<PointLight> PL;
vector<glm::vec4> lightPositions(5);
vector<glm::vec4> lightColors(5);
vector<GLfloat> samplesForEachSurface;

GLuint gVertexAttribBuffer, gIndexBuffer;
GLint gInVertexLoc, gInNormalLoc;
int gVertexDataSizeInBytes, gNormalDataSizeInBytes;


bool ReadDataFromFile(
	const string& fileName, ///< [in]  Name of the shader file
	string& data)     ///< [out] The contents of the file
{
	fstream myfile;

	// Open the input
	myfile.open(fileName.c_str(), std::ios::in);

	if (myfile.is_open())
	{
		string curLine;

		while (getline(myfile, curLine))
		{
			data += curLine;
			if (!myfile.eof())
			{
				data += "\n";
			}
		}

		myfile.close();
	}
	else
	{
		return false;
	}

	return true;
}

GLuint createVS(const char* shaderName)
{
	string shaderSource;

	string filename(shaderName);
	if (!ReadDataFromFile(filename, shaderSource))
	{
		cout << "Cannot find file name: " + filename << endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &shader, &length);
	glCompileShader(vs);

	char output[1024] = { 0 };
	glGetShaderInfoLog(vs, 1024, &length, output);
	printf("VS compile log: %s\n", output);

	return vs;
}

GLuint createFS(const char* shaderName)
{
	string shaderSource;

	string filename(shaderName);
	if (!ReadDataFromFile(filename, shaderSource))
	{
		cout << "Cannot find file name: " + filename << endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &shader, &length);
	glCompileShader(fs);

	char output[1024] = { 0 };
	glGetShaderInfoLog(fs, 1024, &length, output);
	printf("FS compile log: %s\n", output);

	return fs;
}

void initShaders()
{
	// Create the programs

	gProgram = glCreateProgram();

	// Create the shaders

	GLuint vs = createVS("vert.glsl");
	GLuint fs = createFS("frag.glsl");

	// Attach the shader to the program

	glAttachShader(gProgram, vs);
	glAttachShader(gProgram, fs);

	// Link the programs

	glLinkProgram(gProgram);
	GLint status;
	glGetProgramiv(gProgram, GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

    // Get the locations of the uniform variables from program
    modelingMatrixLoc = glGetUniformLocation(gProgram, "modelingMatrix");
    viewingMatrixLoc = glGetUniformLocation(gProgram, "viewingMatrix");
    projectionMatrixLoc = glGetUniformLocation(gProgram, "projectionMatrix");
    eyePosLoc = glGetUniformLocation(gProgram, "eyePos");
    cpLoc = glGetUniformLocation(gProgram, "CP");
    startValLoc = glGetUniformLocation(gProgram, "startingVal");
    lposLoc = glGetUniformLocation(gProgram, "lightPos");
    lcolLoc = glGetUniformLocation(gProgram, "lightColor");
    samplesLoc = glGetUniformLocation(gProgram, "samples");
    nLoc = glGetUniformLocation(gProgram, "n");
    mLoc = glGetUniformLocation(gProgram, "m");
}

void initVBO()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &gVertexAttribBuffer);
	glGenBuffers(1, &gIndexBuffer);

	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

	gVertexDataSizeInBytes = gVertices.size() * 3 * sizeof(GLfloat);
	GLfloat* vertexData = new GLfloat[gVertices.size() * 3];

	GLuint* indexData = new GLuint[indices.size()];
    int indexDataSizeInBytes = indices.size() * sizeof(GLuint);


	for (int i = 0; i < gVertices.size(); ++i)
	{
		vertexData[3 * i] = gVertices[i].x;
		vertexData[3 * i + 1] = gVertices[i].y;
		vertexData[3 * i + 2] = gVertices[i].z;
	}


	for (int i = 0; i < indices.size(); ++i)
	{
		indexData[i] = indices[i];
    }

	glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes , vertexData, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

	// done copying; can free now
	delete[] vertexData;
	delete[] indexData;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

}

void initSamples()
{
    startingVal = -0.5f * coordMultiplier;
    endingVal = 0.5f * coordMultiplier;
    int cpN = cp->n / 4, cpM = cp->m / 4;
    int numOfSamples = samples * max(cpN, cpM);
    GLfloat spacing = (endingVal - startingVal) / (numOfSamples - numOfSamples/samples);
    GLfloat lastX = startingVal, lastY = startingVal;
    int tx =0 , ty = 0;
    for(int nc=0; nc<cpN; nc++)
    {
        lastY = startingVal;
        ty = 0;
        for(int mc=0; mc<cpM; mc++)
        {
            for (int i = 0; i < samples; i++) {
                for (int j = 0; j < samples; j++) {
                    GLfloat x = i * spacing + lastX - (spacing*tx);
                    GLfloat y = j * spacing + lastY - (spacing*ty);

                    if(abs(x) < margin) x = 0.0f;
                    if(abs(y) < margin) y = 0.0f;

                    gVertices.push_back(Vertex(x, y, 0.0f));

                }
            }
            ty++;
            lastY += spacing * (samples);
        }
        tx++;
        lastX += spacing * (samples);
    }

// Generate the indices for the grid
    for(int nc=0; nc<cpN; nc++)
    {
        for(int mc=0; mc<cpM; mc++)
        {
            for (int i = 0; i < samples-1; i++)
            {
                for (int j = 0; j < samples-1; j++)
                {
//                    int index = i * (samples) + j;
                    int index = (nc * cpM + mc) * (samples * samples) + i * (samples) + j;

                    indices.push_back(index);
                    indices.push_back(index + 1);
                    indices.push_back(index + samples);

                    indices.push_back(index + 1);
                    indices.push_back(index + samples);
                    indices.push_back(index + samples + 1);
                }
            }
        }
    }

    if (samples == 2)
    {
        for(int nc=0; nc<cpN; nc++)
        {
            for (int mc = 0; mc < cpM; mc++)
            {
                int index = (nc * cpM + mc) * (samples * samples);
                indices.push_back(index);
                indices.push_back(index + 1);
                indices.push_back(index + samples);

                indices.push_back(index + 1);
                indices.push_back(index + samples);
                indices.push_back(index + samples + 1);
            }
        }
    }
}

void reset()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &gVertexAttribBuffer);
    glDeleteBuffers(1, &gIndexBuffer);
    gVertices.clear();
    indices.clear();
    initSamples();
    initVBO();
}

void initLighting()
{
    for(int i=0; i<5; i++)
    {
        if(i< PL.size())
        {
            lightPositions[i] = glm::vec4(PL[i].x, PL[i].y, PL[i].z, 1.0f);
            lightColors[i] = glm::vec4(PL[i].r, PL[i].g, PL[i].b, 1.0f);
        }
        else
        {
            lightPositions[i] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
            lightColors[i] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
        }
    }
}

void init()
{
	glEnable(GL_DEPTH_TEST);
    initSamples();
	initShaders();
	initVBO();
    initLighting();
}

void drawModel()
{
    glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

void display()
{
    glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	static float angle = 0;

	float angleRad = (float)(angle / 180.0) * M_PI;

	// Compute the modeling matrix
	glm::mat4 matT = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, -1.0f));
	glm::mat4 matRx = glm::rotate<float>(glm::mat4(1.0), (rotationAngle / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
	glm::mat4 matRz = glm::rotate<float>(glm::mat4(1.0), (-90 / 180.) * M_PI, glm::vec3(0.0, 0.0, 1.0));
	modelingMatrix = matT * matRx * matRz;

	// Set the active program and the values of its uniform variables
	glUseProgram(gProgram);
	glUniformMatrix4fv(projectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc, 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc, 1, GL_FALSE, glm::value_ptr(modelingMatrix));

	glUniform3fv(eyePosLoc, 1, glm::value_ptr(eyePos));
    glUniform1f(startValLoc, startingVal);



    glUniform3fv(cpLoc, 576,
                 glm::value_ptr(cp->CPV.data()[0]));
    glUniform4fv(lposLoc,
                 5, glm::value_ptr(lightPositions.data()[0]));
    glUniform4fv(lcolLoc,
                 5, glm::value_ptr(lightColors.data()[0]));
    glUniform1i(nLoc, cp->n);
    glUniform1i(mLoc, cp->m);
    glUniform1i(samplesLoc, samples);
	// Draw the scene
	drawModel();

	angle += 0.5;
}

void reshape(GLFWwindow* window, int w, int h)
{
	w = w < 1 ? 1 : w;
	h = h < 1 ? 1 : h;

	gWidth = w;
	gHeight = h;

	glViewport(0, 0, w, h);

	// Use perspective projection

	float fovyRad = (float)(45.0 / 180.0) * M_PI;
	projectionMatrix = glm::perspective(fovyRad, w/(float) h, 1.0f, 100.0f);


    viewingMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	else if (key == GLFW_KEY_G && action == GLFW_PRESS)
	{
		//glShadeModel(GL_SMOOTH);
		activeProgramIndex = 0;
	}
	else if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		//glShadeModel(GL_SMOOTH);
		activeProgramIndex = 1;
	}
    else if (key == GLFW_KEY_W && action == GLFW_PRESS)     // Increase samples
    {
        samples = samples < 80 ? samples + 2 : 80;
        reset();
    }
    else if (key == GLFW_KEY_S && action == GLFW_PRESS)     // Decrease samples
    {
        samples = samples > 2 ? samples - 2 : 2;
        reset();
    }
    else if (key == GLFW_KEY_E && action == GLFW_PRESS)     // Increase coord multiplier
    {
        coordMultiplier += 0.1f;
        reset();
    }
    else if (key == GLFW_KEY_D && action == GLFW_PRESS)     // Decrease coord multiplier
    {
        coordMultiplier = coordMultiplier > 0.1 ? coordMultiplier - 0.1 : 0.1;
        reset();
    }
    else if (key == GLFW_KEY_T && action == GLFW_PRESS)     // Toggle polygon mode
    {
        GLint polygonMode;
        glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
        if ( polygonMode == GL_LINE )
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        if ( polygonMode == GL_FILL )
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else if (key == GLFW_KEY_R && action == GLFW_PRESS)     // Increase coord multiplier
    {
        rotationAngle += 10.0f;
    }
    else if (key == GLFW_KEY_F && action == GLFW_PRESS)     // Decrease coord multiplier
    {
        rotationAngle -= 10.0f;
    }else if (key == GLFW_KEY_K && action == GLFW_PRESS)     // Increase coord multiplier
    {
        rotZ += 10.0f;
    }
    else if (key == GLFW_KEY_M && action == GLFW_PRESS)     // Decrease coord multiplier
    {
        rotZ -= 10.0f;
    }
}

void mainLoop(GLFWwindow* window)
{
	while (!glfwWindowShouldClose(window))
	{
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
    vector<vector<double>> matrix;
    GLFWwindow* window;
	if (!glfwInit())
	{
		exit(-1);
	}

    // parse input.txt from arguments
    if(argv)
    {
        std::fstream fs{ argv[1] };
        float c;
        int countPtLight = 0;
        fs >> countPtLight; // number of point lights
        // parse point lights
        while(countPtLight--)
        {
            PointLight pl = PointLight();
            fs >> pl.x >> pl.y >> pl.z;
            fs >> pl.r >> pl.g >> pl.b;
            PL.push_back(pl);
        }
        // parse the matrix

        int n, m;   // n x m matrix
        fs >> n >> m;
        for(int i=0; i<n; i++)
        {
            vector<double> row;
            for(int j=0; j<m; j++)
            {
                fs >> c;
                row.push_back(c);
            }
            matrix.push_back(row);
        }
    }

    cp = new ControlPoints(matrix);

    if(!argv || !argv[1])
    {
        std::cout << "No input file given" << std::endl;
        exit(-1);
    }
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    cout << "Welcome to Bezier Surface Renderer" << endl;
    cout << "=================================" << endl;
    cout << "------->This program is the first programming assignment "
            "\nof CENG469 Advanced Computer Graphics course." << endl;
    cout << "Press:" << endl;
    cout << "\t 'W': increase samples" << endl;
    cout << "\t 'S': decrease samples" << endl;
    cout << "\t 'R': rotate whole surface" << endl;
    cout << "\t 'F': rotate in reverse" << endl;
    cout << "\t 'E': increase coord multiplier" << endl;
    cout << "\t 'D': decrease coord multiplier" << endl;
    cout << "\t 'T': toggle wireframe mode" << endl;
    cout << endl << endl;
    cout << "\t ' Developed by Ali Komurcu '" << endl;
    cout << endl << endl;
    int width = 800, height = 600;
	window = glfwCreateWindow(width, height, "CENG469-THE1", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		exit(-1);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	char rendererInfo[512] = { 0 };
	strcpy(rendererInfo, (const char*)glGetString(GL_RENDERER));
	strcat(rendererInfo, " - ");
	strcat(rendererInfo, (const char*)glGetString(GL_VERSION));
	glfwSetWindowTitle(window, rendererInfo);

	init();
    int A = 300;
    int i = int(((A/3) / (samples * samples)) / (cp->m * samples));
    int j = int(((A/3) % int(samples*samples*cp->n)) / (samples*samples));
    int index = i*cp->m + j;
	glfwSetKeyCallback(window, keyboard);
	glfwSetWindowSizeCallback(window, reshape);

	reshape(window, width, height); // need to call this once ourselves
	mainLoop(window); // this does not return unless the window is closed

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
