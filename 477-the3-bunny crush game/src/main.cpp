#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>
#include <vector>
#include <GL/glew.h>   // The GL Header File
#include <GL/gl.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include "Object.h"
#include "Matrix.h"
#include FT_FREETYPE_H

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace std;
/*
 TODO: scaling
 TODO: angle
 TODO: windowsize
*/

GLuint gProgram[3];
GLint gIntensityLoc;
float gIntensity = 1000;
int width = 640, height = 600;
int moves = 0, score = 0;
bool done = false;
bool reshaped = false;
bool restart = false;

bool mouseClicked = false;
double mouse_x= -1, mouse_y =-1;

std::chrono::time_point<std::chrono::high_resolution_clock> start;

enum Modes
{
    IDLE, EXPLODE, SLIDE, CHECK, SWAP

};
Modes state = CHECK;
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

vector<Vertex> gVertices;
vector<Texture> gTextures;
vector<Normal> gNormals;
vector<Face> gFaces;

GLuint gVertexAttribBuffer, gTextVBO, gIndexBuffer;
GLint gInVertexLoc, gInNormalLoc;
int gVertexDataSizeInBytes, gNormalDataSizeInBytes;

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
    GLuint Advance;    // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;


bool ParseObj(const string& fileName)
{
    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            stringstream str(curLine);
            GLfloat c1, c2, c3;
            GLuint index[9];
            string tmp;

            if (curLine.length() >= 2)
            {
                if (curLine[0] == '#') // comment
                {
                    continue;
                }
                else if (curLine[0] == 'v')
                {
                    if (curLine[1] == 't') // texture
                    {
                        str >> tmp; // consume "vt"
                        str >> c1 >> c2;
                        gTextures.push_back(Texture(c1, c2));
                    }
                    else if (curLine[1] == 'n') // normal
                    {
                        str >> tmp; // consume "vn"
                        str >> c1 >> c2 >> c3;
                        gNormals.push_back(Normal(c1, c2, c3));
                    }
                    else // vertex
                    {
                        str >> tmp; // consume "v"
                        str >> c1 >> c2 >> c3;
                        gVertices.push_back(Vertex(c1, c2, c3));
                    }
                }
                else if (curLine[0] == 'f') // face
                {
                    str >> tmp; // consume "f"
					char c;
					int vIndex[3],  nIndex[3], tIndex[3];
					str >> vIndex[0]; str >> c >> c; // consume "//"
					str >> nIndex[0]; 
					str >> vIndex[1]; str >> c >> c; // consume "//"
					str >> nIndex[1]; 
					str >> vIndex[2]; str >> c >> c; // consume "//"
					str >> nIndex[2]; 

					assert(vIndex[0] == nIndex[0] &&
						   vIndex[1] == nIndex[1] &&
						   vIndex[2] == nIndex[2]); // a limitation for now

					// make indices start from 0
					for (int c = 0; c < 3; ++c)
					{
						vIndex[c] -= 1;
						nIndex[c] -= 1;
						tIndex[c] -= 1;
					}

                    gFaces.push_back(Face(vIndex, tIndex, nIndex));
                }
                else
                {
                    cout << "Ignoring unidentified line in obj file: " << curLine << endl;
                }
            }

            //data += curLine;
            if (!myfile.eof())
            {
                //data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

	/*
	for (int i = 0; i < gVertices.size(); ++i)
	{
		Vector3 n;

		for (int j = 0; j < gFaces.size(); ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				if (gFaces[j].vIndex[k] == i)
				{
					// face j contains vertex i
					Vector3 a(gVertices[gFaces[j].vIndex[0]].x, 
							  gVertices[gFaces[j].vIndex[0]].y,
							  gVertices[gFaces[j].vIndex[0]].z);

					Vector3 b(gVertices[gFaces[j].vIndex[1]].x, 
							  gVertices[gFaces[j].vIndex[1]].y,
							  gVertices[gFaces[j].vIndex[1]].z);

					Vector3 c(gVertices[gFaces[j].vIndex[2]].x, 
							  gVertices[gFaces[j].vIndex[2]].y,
							  gVertices[gFaces[j].vIndex[2]].z);

					Vector3 ab = b - a;
					Vector3 ac = c - a;
					Vector3 normalFromThisFace = (ab.cross(ac)).getNormalized();
					n += normalFromThisFace;
				}

			}
		}

		n.normalize();

		gNormals.push_back(Normal(n.x, n.y, n.z));
	}
	*/

	assert(gVertices.size() == gNormals.size());

    return true;
}

bool ReadDataFromFile(
    const string& fileName, ///< [in]  Name of the shader file
    string&       data)     ///< [out] The contents of the file
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

void createVS(GLuint& program, const string& filename)
{
    string shaderSource;

    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &shader, &length);
    glCompileShader(vs);

    char output[1024] = {0};
    glGetShaderInfoLog(vs, 1024, &length, output);
    printf("VS compile log: %s\n", output);

    glAttachShader(program, vs);
}

void createFS(GLuint& program, const string& filename)
{
    string shaderSource;

    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &shader, &length);
    glCompileShader(fs);

    char output[1024] = {0};
    glGetShaderInfoLog(fs, 1024, &length, output);
    printf("FS compile log: %s\n", output);

    glAttachShader(program, fs);
}

void initShaders()
{
    gProgram[0] = glCreateProgram();
    gProgram[1] = glCreateProgram();
    gProgram[2] = glCreateProgram();

    createVS(gProgram[0], "vert0.glsl");
    createFS(gProgram[0], "frag0.glsl");

    createVS(gProgram[1], "vert1.glsl");
    createFS(gProgram[1], "frag1.glsl");

    createVS(gProgram[2], "vert_text.glsl");
    createFS(gProgram[2], "frag_text.glsl");

    glBindAttribLocation(gProgram[0], 0, "inVertex");
    glBindAttribLocation(gProgram[0], 1, "inNormal");
    glBindAttribLocation(gProgram[1], 0, "inVertex");
    glBindAttribLocation(gProgram[1], 1, "inNormal");
    glBindAttribLocation(gProgram[2], 2, "vertex");

    glLinkProgram(gProgram[0]);
    glLinkProgram(gProgram[1]);
    glLinkProgram(gProgram[2]);
    glUseProgram(gProgram[0]);

    gIntensityLoc = glGetUniformLocation(gProgram[0], "intensity");
    cout << "gIntensityLoc = " << gIntensityLoc << endl;
    glUniform1f(gIntensityLoc, gIntensity);
}

void initVBO()
{
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    assert(glGetError() == GL_NONE);

    glGenBuffers(1, &gVertexAttribBuffer);
    glGenBuffers(1, &gIndexBuffer);

    assert(gVertexAttribBuffer > 0 && gIndexBuffer > 0);

    glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

    gVertexDataSizeInBytes = gVertices.size() * 3 * sizeof(GLfloat);
    gNormalDataSizeInBytes = gNormals.size() * 3 * sizeof(GLfloat);
    int indexDataSizeInBytes = gFaces.size() * 3 * sizeof(GLuint);
    GLfloat* vertexData = new GLfloat [gVertices.size() * 3];
    GLfloat* normalData = new GLfloat [gNormals.size() * 3];
    GLuint* indexData = new GLuint [gFaces.size() * 3];

    float minX = 1e6, maxX = -1e6;
    float minY = 1e6, maxY = -1e6;
    float minZ = 1e6, maxZ = -1e6;

    for (int i = 0; i < gVertices.size(); ++i)
    {
        vertexData[3*i] = gVertices[i].x;
        vertexData[3*i+1] = gVertices[i].y;
        vertexData[3*i+2] = gVertices[i].z;

        minX = std::min(minX, gVertices[i].x);
        maxX = std::max(maxX, gVertices[i].x);
        minY = std::min(minY, gVertices[i].y);
        maxY = std::max(maxY, gVertices[i].y);
        minZ = std::min(minZ, gVertices[i].z);
        maxZ = std::max(maxZ, gVertices[i].z);
    }

    std::cout << "minX = " << minX << std::endl;
    std::cout << "maxX = " << maxX << std::endl;
    std::cout << "minY = " << minY << std::endl;
    std::cout << "maxY = " << maxY << std::endl;
    std::cout << "minZ = " << minZ << std::endl;
    std::cout << "maxZ = " << maxZ << std::endl;

    for (int i = 0; i < gNormals.size(); ++i)
    {
        normalData[3*i] = gNormals[i].x;
        normalData[3*i+1] = gNormals[i].y;
        normalData[3*i+2] = gNormals[i].z;
    }

    for (int i = 0; i < gFaces.size(); ++i)
    {
        indexData[3*i] = gFaces[i].vIndex[0];
        indexData[3*i+1] = gFaces[i].vIndex[1];
        indexData[3*i+2] = gFaces[i].vIndex[2];
    }


    glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes + gNormalDataSizeInBytes, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes, vertexData);
    glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes, gNormalDataSizeInBytes, normalData);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

    // done copying; can free now
    delete[] vertexData;
    delete[] normalData;
    delete[] indexData;

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));

}

void initFonts(int windowWidth, int windowHeight)
{
    // Set OpenGL options
    //glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(windowWidth), 0.0f, static_cast<GLfloat>(windowHeight));
    glUseProgram(gProgram[2]);
    glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }

    // Load font as face
    FT_Face face;
    if (FT_New_Face(ft, "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    }

    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

    // Load first 128 characters of ASCII set
    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
                );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    //
    // Configure VBO for texture quads
    //
    glGenBuffers(1, &gTextVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void initMatrix(Matrix &m)
{
    for (int i = 0; i < m.rows; ++i)
    {
        for (int j = 0; j < m.cols; ++j)
        {
            m.contents[i][j] = Object(i, j, m.rows, m.cols);
        }
    }
    m.getPosition();
    m.getCursor(height,width);

}
void init(const string& object, Matrix &m, int width, int height)
{
	ParseObj(object);

    glEnable(GL_DEPTH_TEST);
    initShaders();
    initFonts(width, height);
    initVBO();
    initMatrix(m);
}

void drawModel()
{
	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));

	glDrawElements(GL_TRIANGLES, gFaces.size() * 3, GL_UNSIGNED_INT, 0);
}

void renderText(const std::string& text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    // Activate corresponding render state	
    glUseProgram(gProgram[2]);
    glUniform3f(glGetUniformLocation(gProgram[2], "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },            
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }           
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

        //glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)

        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}


void display(Matrix &m)
{
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	static float angle = 0;
    glm::mat4 projMat = glm::ortho( -10.f, 10.f, -10.f, 10.f, -20.f, 20.f);

    for(int i = 0; i < m.rows; ++i)
    {
        for(int j = 0; j < m.cols; ++j)
        {
            glUseProgram(gProgram[0]);

            glm::mat4 T = glm::translate(glm::mat4(1.f), glm::vec3(m.contents[i][j].position.x, m.contents[i][j].position.y, m.contents[i][j].position.z));
            glm::mat4 S = glm::scale(glm::mat4(1.f), glm::vec3(m.contents[i][j].scaleX, m.contents[i][j].scaleY, 1));
            glm::mat4 R = glm::rotate(glm::mat4(1.f), glm::radians(angle), glm::vec3(0, 1, 0));
            glm::mat4 modelMat = T * S * R;
            glm::mat4 modelMatInv = glm::transpose(glm::inverse(modelMat));
            glm::vec3 color = glm::vec3(m.contents[i][j].color.x, m.contents[i][j].color.y, m.contents[i][j].color.z);
            glm::vec3 lightPos = glm::vec3(m.positions[i][j].x, m.positions[i][j].y, 1);
            glUniform3f(glGetUniformLocation(gProgram[0], "lightPos"), lightPos.x, lightPos.y, lightPos.z);            
            glUniform3f(glGetUniformLocation(gProgram[0], "kd"), 0.2*color.x, 0.2*color.y, 0.2*color.z);
            glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
            glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
            glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(projMat));

            drawModel();
        }
    }
    
    assert(glGetError() == GL_NO_ERROR);

    renderText("Moves: " + to_string(moves) + " Score: " + to_string(score) , 0, 0, 1, glm::vec3(1, 1, 0));

    assert(glGetError() == GL_NO_ERROR);

	angle += 0.5;
}

void reshape(GLFWwindow* window, int w, int h)
{
    w = w < 1 ? 1 : w;
    h = h < 1 ? 1 : h;

    width = w;
    height = h;
    
    glViewport(0, 0, w, h);
    reshaped = true;
}
void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && state == IDLE)
    {
        // MOUSE CLICKED
        cout << "Mouse clicked" << endl;
        glfwGetCursorPos(window, &mouse_x, &mouse_y);
        cout << "Cursor Position at (" << mouse_x << " : " << mouse_y << ")" << endl;
        mouseClicked = true;
    }
}
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        cout << "Escape pressed" << endl;
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    else if(state == IDLE && key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        restart = true;
        moves = 0;
        score = 0;
    }
}
void check_match(Matrix&m){
    
    for(int i = 0; i < m.rows; ++i){
        for(int j = 0; j < m.cols; ++j){
            if(j+2 < m.cols && m.contents[i][j].colorIndex == m.contents[i][j+1].colorIndex &&  
                m.contents[i][j+1].colorIndex== m.contents[i][j+2].colorIndex ){
                m.contents[i][j].explode=true;
                m.contents[i][j+1].explode=true;
                m.contents[i][j+2].explode=true;
            }
            if(i+2 < m.rows && m.contents[i][j].colorIndex == m.contents[i+1][j].colorIndex &&  
                m.contents[i+1][j].colorIndex== m.contents[i+2][j].colorIndex ){
                m.contents[i][j].explode=true;
                m.contents[i+1][j].explode=true;
                m.contents[i+2][j].explode=true;
            }
        }
    }
}

void swap_up(Matrix &m, int maxi, int j)
{
    for(int i = maxi; i >= 0; i--)
    {
        for (int k = i-1; k >= 0; k--)
        {
            if(m.contents[i][j].explode && !m.contents[k][j].explode)   // swap true with nearest above false
            {
                Object tmp = m.contents[i][j];
                m.contents[i][j] = m.contents[k][j];
                m.contents[k][j] = tmp;
                break;
            }
        }
    }
    // sort the matrix m.contents
    for(int i = 0; i < m.rows-1; ++i)
    {
        for(int k = 0; k < m.rows-i-1; k++)
        {
            if(m.contents[k][j].position.y < m.contents[k+1][j].position.y)
            {
                Object tmp = m.contents[k+1][j];
                m.contents[k+1][j] = m.contents[k][j];
                m.contents[k][j] = tmp;
            }
        }
    }
}

void mainLoop(GLFWwindow* window, Matrix &m)
{
    while (!glfwWindowShouldClose(window))
    {
        switch (state)
        {
        case CHECK:{
            display(m);
            if(mouseClicked){ //mouse clicked, idledan geldik
                bool found = false;
                for(int i = m.rows - 1; i > -1; --i)
                {
                    for(int j= m.cols - 1; j > -1; --j)
                    {
                        if(m.lefttopx[i][j]<= mouse_x && m.lefttopy[i][j] <= mouse_y &&
                            m.rightbottomx[i][j] >= mouse_x && m.rightbottomy[i][j] >= mouse_y &&
                            !found){
                            m.contents[i][j].explode =true;
                            found = true;
                            state = EXPLODE;
                            moves++;                        
                        }
                    }
                }
                mouseClicked = false;
            }
            else{ //mouse not clicked slidedan geldik
                check_match(m);
                bool found = false;
                for(int i = 0; i < m.rows; ++i){
                    for(int j = 0; j < m.cols; ++j)
                    {
                        if(m.contents[i][j].explode){
                            state = EXPLODE;
                            found = true;
                        }
                    }
                }
                if(!found)
                    state = IDLE;

            }
        }break;
        case IDLE:{
            display(m);
            if(mouseClicked)
            {
                state = CHECK;
            }
        }break;

        case SWAP:{
            display(m);
            // compute the new positions of each object until slide is finished
            // swap the colmuns in contents

            for(int j = 0; j < m.cols; ++j){
                int maxi = -1;
                
                for(int i = 0; i < m.rows; ++i)
                {
                    if(m.contents[i][j].explode){
                        m.contents[i][j].restart();
                        maxi = i;
                    }
                }
                
                // swap up
                swap_up(m,maxi,j);

                maxi = -1;
            }
            state = SLIDE;
        }

        case SLIDE:{
            display(m);
            double decreaseY = 0.05;
            bool stable = true;
            for(int j = 0; j < m.cols; ++j){
                for(int i = 0; i < m.rows; ++i)
                {
                    if(m.contents[i][j].position.y - decreaseY > m.positions[i][j].y)
                    {
                        m.contents[i][j].position.y -= decreaseY;
                        stable = false;
                    }
                    else{
                        m.contents[i][j].position.y = m.positions[i][j].y;
                        m.contents[i][j].explode = false;
                    }
                }
            }

            if (stable) state = CHECK;

            // compute the new positions of each object until slide is finished
            // swap the colmuns in contents
            
        }break;

        case EXPLODE:{
            display(m);

            for(int i = 0; i < m.rows; ++i){
                for(int j = 0; j < m.cols; ++j)
                {   
                    if(m.contents[i][j].explode){
                        m.contents[i][j].enlarge_scale();
                    }
                    if(m.contents[i][j].onehalf()){
                        done = true;
                    }
                }
            }
        
            if (done) {
                double dist;
                if(m.contents.size() > 1){
                    dist = abs(m.contents[0][0].position.y - m.contents[1][0].position.y);
                }

                int count = 0;
                int columnCount = 0;
                for(int j = 0; j < m.cols; ++j){
                    for(int i = 0; i < m.rows; ++i)
                    {
                        if(m.contents[i][j].explode){
                            count++;
                            columnCount++;
                            m.contents[i][j].position.y = m.positions[0][j].y + dist * columnCount;
                        }
                    }
                    columnCount = 0;
                }

                score = count == 1 ? score : score + count;
                state = SWAP;
                done = false;
            }
            // delete the to be deleted ones
        }break;


        default:
            break;
        }
        if(reshaped){
            m.getCursor(height,width);
            reshaped = false;
        }
        if(restart){
            for(int i = 0; i < m.rows; ++i){
                for(int j = 0; j < m.cols; ++j){
                    m.contents[i][j].restart();
                }
            }
            restart = false;
            state = CHECK;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
    GLFWwindow* window;
    if (!glfwInit())
    {
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    string str_grid_width = argv[1], str_grid_height = argv[2], object = argv[3];
    int grid_width = stoi(str_grid_width , nullptr, 10);
    int grid_height = stoi(str_grid_height , nullptr, 10);

    Matrix m(grid_height, grid_width);

    window = glfwCreateWindow(width, height, "CENG 477 - THE3 - Rabbit Crush", NULL, NULL);

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
        return EXIT_FAILURE;
    }

    char rendererInfo[512] = {0};
    strcpy(rendererInfo, (const char*) glGetString(GL_RENDERER));
    strcat(rendererInfo, " - ");
    strcat(rendererInfo, (const char*) glGetString(GL_VERSION));
    glfwSetWindowTitle(window, rendererInfo);


    init(object, m, width, height);

    glfwSetKeyCallback(window, keyboard);
    glfwSetMouseButtonCallback(window, mouse_callback);
    glfwSetWindowSizeCallback(window, reshape);

    reshape(window, width, height); // need to call this once ourselves
    mainLoop(window, m); // this does not return unless the window is closed

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

