#ifndef OBJECT_H
#define OBJECT_H
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include "Primitives.h"
#include "Shader.h"

using namespace std;

class Object
{
private:
    vector<Vertex> gVertices;
    vector<Texture> gTextures;
    vector<Normal> gNormals;
    vector<Face> gFaces;
public:
    Transform* transform;
    GLuint vao;
    GLuint gVertexAttribBuffer, gIndexBuffer;
    int gVertexDataSizeInBytes, gNormalDataSizeInBytes;
    Shader* shader;

    Object(const string& fileName) {
        ParseObj(fileName);
        transform = new Transform();
    }

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
                    if (curLine[0] == 'v')
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
                        int vIndex[3], nIndex[3], tIndex[3];
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

    void InitObject()
    {
        glGenVertexArrays(1, &vao);
//        assert(vao > 0);
        glBindVertexArray(vao);
        cout << "vao = " << vao << endl;

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
//        assert(glGetError() == GL_NONE);

        glGenBuffers(1, &gVertexAttribBuffer);
        glGenBuffers(1, &gIndexBuffer);

//        assert(gVertexAttribBuffer > 0 && gIndexBuffer > 0);

        glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

        gVertexDataSizeInBytes = gVertices.size() * 3 * sizeof(GLfloat);
        gNormalDataSizeInBytes = gNormals.size() * 3 * sizeof(GLfloat);
        int indexDataSizeInBytes = gFaces.size() * 3 * sizeof(GLuint);
        GLfloat* vertexData = new GLfloat[gVertices.size() * 3];
        GLfloat* normalData = new GLfloat[gNormals.size() * 3];
        GLuint* indexData = new GLuint[gFaces.size() * 3];

        float minX = 1e6, maxX = -1e6;
        float minY = 1e6, maxY = -1e6;
        float minZ = 1e6, maxZ = -1e6;

        for (int i = 0; i < gVertices.size(); ++i)
        {
            vertexData[3 * i] = gVertices[i].x;
            vertexData[3 * i + 1] = gVertices[i].y;
            vertexData[3 * i + 2] = gVertices[i].z;

            minX = std::min(minX, gVertices[i].x);
            maxX = std::max(maxX, gVertices[i].x);
            minY = std::min(minY, gVertices[i].y);
            maxY = std::max(maxY, gVertices[i].y);
            minZ = std::min(minZ, gVertices[i].z);
            maxZ = std::max(maxZ, gVertices[i].z);
        }
//
//        std::cout << "minX = " << minX << std::endl;
//        std::cout << "maxX = " << maxX << std::endl;
//        std::cout << "minY = " << minY << std::endl;
//        std::cout << "maxY = " << maxY << std::endl;
//        std::cout << "minZ = " << minZ << std::endl;
//        std::cout << "maxZ = " << maxZ << std::endl;

        for (int i = 0; i < gNormals.size(); ++i)
        {
            normalData[3 * i] = gNormals[i].x;
            normalData[3 * i + 1] = gNormals[i].y;
            normalData[3 * i + 2] = gNormals[i].z;
        }

        for (int i = 0; i < gFaces.size(); ++i)
        {
            indexData[3 * i] = gFaces[i].vIndex[0];
            indexData[3 * i + 1] = gFaces[i].vIndex[1];
            indexData[3 * i + 2] = gFaces[i].vIndex[2];
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
//        glBindBuffer(GL_ARRAY_BUFFER, 0);
//        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

    }
    void draw()
    {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));
        glDrawElements(GL_TRIANGLES, gFaces.size() * 3, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    // Shader must be initialized before calling draw or display methods
    void initShader(const char* vertexPath, const char* fragmentPath)
    {
        shader = new Shader(vertexPath, fragmentPath);
    }

    void destroyModel()
    {
        glDeleteBuffers(1, &gVertexAttribBuffer);
        glDeleteBuffers(1, &gIndexBuffer);
        glDeleteVertexArrays(1, &vao);
    }
};
#endif

