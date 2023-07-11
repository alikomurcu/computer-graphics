#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


using namespace std;

class Shader
{
public:
    GLuint ID;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath, const char* fragmentPath)
    {
        GLenum err = glewInit();
        if (err != GLEW_OK)
            exit(1); // or handle the error in a nicer way
        if (!GLEW_VERSION_2_1)  // check that the machine supports the 2.1 API.
            exit(1); // or handle the error in a nicer way
        GLuint vertex = createVS(vertexPath);
        GLuint fragment = createFS(fragmentPath);
        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID);
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);

    }

    static GLuint createVS(const char* shaderName)
    {
        string shaderSource;

        string filename(shaderName);
        if (!ReadDataFromFile(filename, shaderSource))
        {
            cout << "Cannot find file name: " + filename << endl;
            exit(-1);
        }

        auto length = (GLint) shaderSource.length();
        const auto* shader = (const GLchar*)shaderSource.c_str();

        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &shader, &length);
        glCompileShader(vs);

        char output[1024] = { 0 };
        glGetShaderInfoLog(vs, 1024, &length, output);
        printf("VS compile log: %s\n", output);

        return vs;
    }

    static GLuint createFS(const char* shaderName)
    {
        string shaderSource;

        string filename(shaderName);
        if (!ReadDataFromFile(filename, shaderSource))
        {
            cout << "Cannot find file name: " + filename << endl;
            exit(-1);
        }

        auto length = (GLint) shaderSource.length();
        const auto* shader = (const GLchar*)shaderSource.c_str();

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &shader, &length);
        glCompileShader(fs);

        char output[1024] = { 0 };
        glGetShaderInfoLog(fs, 1024, &length, output);
        printf("FS compile log: %s\n", output);

        return fs;
    }
    static bool ReadDataFromFile(
            const string& fileName, ///< [in]  Name of the shader file
            string& data)     ///< [out] The contents of the file
    {
        fstream my_file;

        // Open the input
        my_file.open(fileName.c_str(), ios::in);

        if (my_file.is_open())
        {
            string curLine;

            while (getline(my_file, curLine))
            {
                data += curLine;
                if (!my_file.eof())
                {
                    data += "\n";
                }
            }

            my_file.close();
        }
        else
        {
            return false;
        }

        return true;
    }

    // activate the shader this part is derived from learnopengl.com
    // ------------------------------------------------------------------------
    void use() const
    {
        glUseProgram(ID);
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const string &name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    // ------------------------------------------------------------------------
    void setInt(const string &name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setFloat(const string &name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setVec2(const string &name, const glm::vec2 &value) const
    {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec2(const string &name, float x, float y) const
    {
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
    }
    // ------------------------------------------------------------------------
    void setVec3(const string &name, const glm::vec3 &value) const
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec3(const string &name, float x, float y, float z) const
    {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }
    // ------------------------------------------------------------------------
    void setVec4(const string &name, const glm::vec4 &value) const
    {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec4(const string &name, float x, float y, float z, float w) const
    {
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    }
    // ------------------------------------------------------------------------
    void setMat2(const string &name, const glm::mat2 &mat) const
    {
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat3(const string &name, const glm::mat3 &mat) const
    {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat4(const string &name, const glm::mat4 &mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    static void checkCompileErrors(GLuint shader)
    {
        GLint success;
        GLchar infoLog[1024];

        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            cout << "ERROR::PROGRAM_LINKING_ERROR of SHADER PROGRAM:\n" << infoLog << "\n -- --------------------------------------------------- -- " << endl;
            exit(-1);
        }
    }
};
#endif

