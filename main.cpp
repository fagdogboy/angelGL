#include<fstream> 
#include<iostream>
#include "./glad/glad.h"
#include<GLFW/glfw3.h>
#include<string>
#include<sstream>
#include<cstring>
#include<cmath>

#define STB_IMAGE_IMPLEMENTATION

//#include "shader/shader.h"
#include "newshader.h"
#include "stb_image.h"

void load_texture(std::string to_load, unsigned int slot)
{
  printf("trying to load textures into slot :%d\n",slot);
  int width, height, nrChannels;
  unsigned char *data = stbi_load(to_load.c_str(), &width, &height, &nrChannels, 0);
  
  unsigned int texture;
  glGenTextures(1, &texture);
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, texture);
  
  if (data)
    {
      printf("deserialized image successfully.\n");
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
    }
  else
    {
      std::cout << "Failed to load texture" << std::endl;
    }

  stbi_image_free(data);
  glBindTexture(GL_TEXTURE_2D, texture);
  
  return;
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
    0.5f, -0.5f, 0.0f,   0.8f, 0.0f, 0.0f,  1.0f, 1.0f,
    -0.5f, -0.5f, 0.0f,  0.0f, 0.8f, 0.0f,  1.0f, 0.0f,
    0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 0.8f,  0.0f, 0.0f
  };
  
  unsigned int indices[] = {  
    0, 1, 2
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

  //setup
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  //texture loading

  load_texture("texture.jpg", 0);
  load_texture("anothertexture.jpg", 1);
  
  //buffers
  unsigned int VAO;
  glGenVertexArrays(1, &VAO);  
  glBindVertexArray(VAO);

  unsigned int VBO;
  glGenBuffers(1, &VBO);    
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // tell attrib pointers where to read
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); // void pointer typecast cause why not :-)
  glEnableVertexAttribArray(0);  
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3* sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);
 
  unsigned int EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  //GL_FILL for filled GL_LINE for wireframe

  Shader mainShader("default.vert","default.frag");
  mainShader.use();

  mainShader.setInt("texture1", 0);
  mainShader.setInt("texture2", 1);
  
  while(!glfwWindowShouldClose(window))
    {
      processInput(window);

      mainShader.use();
      
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      float timeValue = glfwGetTime();
      float greenValue = (sin(timeValue) / 2.0f) + 0.5f;

      int vertexColorLocation = glGetUniformLocation(mainShader.ID, "ourColor");
      glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);      
      
      glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
      
      glfwSwapBuffers(window);
      glfwPollEvents();    
      
    }

  glfwTerminate();
  
  return 0;
  
}
