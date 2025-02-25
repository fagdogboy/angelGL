#pragma once

#include "../glad/glad.h" 
  
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

std::string deserialize(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open the file: " << filename << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

class Shader
{
public:

    unsigned int ID;
  

  Shader(const char* vertexPath, const char* fragmentPath)
  {
    
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    
    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);

    vertexCode = deserialize(vertexPath);
    fragmentCode = deserialize(fragmentPath);
    
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    std::cout << vShaderCode[1] << "\n";
    
    //shader compiling
    
    unsigned int vertex, fragment;
    int success;
    char infoLog[512];
    
    // vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    // print compile errors if any
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if(!success)
      {
	glGetShaderInfoLog(vertex, 512, NULL, infoLog);
	std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
      };


    // frag shader    
    vertex = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &vShaderCode, NULL);
    glCompileShader(fragment);
    // print compile errors if any
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if(!success)
      {
	glGetShaderInfoLog(vertex, 512, NULL, infoLog);
	std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
      };

    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    // print linking errors if any
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if(!success)
      {
	glGetProgramInfoLog(ID, 512, NULL, infoLog);
	std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
      }
    
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    
  }


  void use() {
    glUseProgram(ID);
  }
  
  void setBool(const std::string &name, bool value) const
  {         
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
  }
  void setInt(const std::string &name, int value) const
  { 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
  }
  void setFloat(const std::string &name, float value) const
  { 
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
  } 
  
};

