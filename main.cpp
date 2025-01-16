#include<fstream> 
#include<iostream>
#include "./glad/glad.h"
#include<GLFW/glfw3.h>
#include<string>
#include<sstream>
#include<cstring>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define FOV_DEF 50.0f
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

  int frames_drawn = 0;
  
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


  // random shit for render loop
  unsigned int transformLoc = glGetUniformLocation(mainShader.ID, "transform");
  
  while(!glfwWindowShouldClose(window))
    {
      processInput(window);

      float timeValue = glfwGetTime();
      float greenValue = std::abs(sin(timeValue) * 360);

     
      //model matrix
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::rotate(model, glm::radians(greenValue), glm::vec3(1.0f, 0.0f, 0.0f));

      //view matrix
      glm::mat4 view = glm::mat4(1.0f);
      // note that we're translating the scene in the reverse direction of where we want to move
      view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f)); 
      
      //projection matrix
      int height = 0 ,width = 0;
      glfwGetWindowSize(window, &height, &width);
      glm::mat4 proj = glm::perspective(glm::radians(FOV_DEF), (float)width/(float)height, 0.1f, 100.0f);
      
      //end math
      // sending data

      int modelLoc = glGetUniformLocation(mainShader.ID, "model");
      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
      int viewLoc = glGetUniformLocation(mainShader.ID, "view");
      glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
      int projectionLoc = glGetUniformLocation(mainShader.ID, "projection");
      glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(proj));



      
      mainShader.use();
      
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      int vertexColorLocation = glGetUniformLocation(mainShader.ID, "ourColor");
      glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);      
      
      glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
      //for arrays and indices

      //glDrawArrays(GL_TRIANGLES, 0, 36);
      //for arrays
      
      glfwSwapBuffers(window);
      glfwPollEvents();    

      //      frames_drawn++;
    }

  glfwTerminate();
  
  return 0;
  
}
