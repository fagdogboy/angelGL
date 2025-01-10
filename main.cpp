#include<fstream> 
#include<iostream>
#include "./glad/glad.h"
#include<GLFW/glfw3.h>
#include<string>
#include <sstream>
#include<cstring>

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

void processInput(GLFWwindow *window)
{
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  glViewport(0, 0, width, height);
}  

int main() {

  float vertices[] = {
    0.5f,  0.5f, 0.0f, 
    0.5f, -0.5f, 0.0f, 
    -0.5f, -0.5f, 0.0f,
    -0.5f,  0.5f, 0.0f,
    1.0f, 1.0f, 0.0f,
    1.0f, 0.5f, 0.0f,
    0.5f, 0.5f, 0.0f
    
  };
  
  unsigned int indices[] = {  
    0, 1, 3,  
    1, 2, 3,  
    4, 5, 6   
  };  
  
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
  
  GLFWwindow* window = glfwCreateWindow(800, 600, "wdwion", NULL, NULL);
  if (window == NULL)
    {
      std::cout << "Failed to create GLFW window" << std::endl;
      glfwTerminate();
      return -1;
    }
  glfwMakeContextCurrent(window);

  
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return -1;
    }  

  glViewport(0, 0, 800, 600);

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);    

  //vertex shader
  std::string tmpShaderSrc = deserialize("default.vert");  
  const char* vertexShaderSource = tmpShaderSrc.c_str();
  
  unsigned int vertexShader;
  vertexShader = glCreateShader(GL_VERTEX_SHADER);

  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  printf("vert shader loaded\n");
  
  //fragment shader
  tmpShaderSrc = deserialize("default.fra");  
  const char* fragmentShaderSource = tmpShaderSrc.c_str();

  unsigned int fragmentShader;
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);

  printf("frag shader loaded\n");
  
  //buffers
  unsigned int VAO;
  glGenVertexArrays(1, &VAO);  
  glBindVertexArray(VAO);

  unsigned int VBO;
  glGenBuffers(1, &VBO);  
  
  glBindBuffer(GL_ARRAY_BUFFER, VBO);  
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
 
  unsigned int EBO;
  glGenBuffers(1, &EBO);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);  

  glBindVertexArray(VAO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  
  
  //shader program
  unsigned int shaderProgram;
  shaderProgram = glCreateProgram();
  
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  //error check
  int  success = false;
  char infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  glGetShaderiv(shaderProgram, GL_COMPILE_STATUS, &success);
  
  if(!success)
    {
      glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
  else 
    {
      std::cout << "everything loaded without errors" << std::endl;      
    }

  glUseProgram(shaderProgram);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  //GL_FILL
  
  while(!glfwWindowShouldClose(window))
    {
      processInput(window);

      
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);
      
      glfwSwapBuffers(window);
      glfwPollEvents();    
      
    }

  glfwTerminate();
  
  return 0;
  
}
