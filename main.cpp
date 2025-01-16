#include<fstream> 
#include <glm/ext/vector_float3.hpp>
#include<iostream>
#include "./glad/glad.h"
#include<GLFW/glfw3.h>
#include<string>
#include<sstream>
#include<cstring>
#include <cmath>
#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define FOV_DEF 50.0f
#define POLY_WIREFRAME false

//#include "shader/shader.h"
#include "newshader.h"
#include "stb_image.h"
#include <vector>
#include <strstream>

struct model_data {

  std::vector<float> model_vertices;
  std::vector<int> model_indices;
  
};

float generateRandomFloat() {

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
  
  return dis(gen);  
}

model_data import_obj_mesh(std::string file_path) {

  model_data output;
    
    std::ifstream file(file_path);
    if(!file.is_open())
      std::cout << "imort mesh not found!!!" << std::endl;
    else{  std::cout << "deserializing obj model..." << std::endl;}
    
    std::vector<glm::vec3> vertices;
    
    while(!file.eof()) {

      char line[1024];
      file.getline(line,1024);

      std::strstream s;

      s << line;

      char junk;

      if(line[0] == 'v') {

	glm::vec3 vec = {0.0f,0.0f,0.0f};

	s >> junk >> vec.x >> vec.y >> vec.z;

        //pos data
        output.model_vertices.push_back(vec.x);
	output.model_vertices.push_back(vec.y);
	output.model_vertices.push_back(vec.z);

	//tex coords. its jank ik
	output.model_vertices.push_back(generateRandomFloat());
	output.model_vertices.push_back(generateRandomFloat());

      }
      
      if(line[0] == 'f') {

	int f[3];

	s >> junk >> f[0] >> f[1] >> f[2];
	
	output.model_indices.push_back(f[0]-1);
	output.model_indices.push_back(f[1]-1);
	output.model_indices.push_back(f[2]-1);

      }
      
    }

    std::cout << "successfully deserialized mesh. (hopefully)" << std::endl;
      
    return output;
    
}

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
  glEnable(GL_DEPTH_TEST);  
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // texture loading

  load_texture("texture.jpg", 0);
  load_texture("anothertexture.jpg", 1);

  // model loading

  model_data loaded_model = import_obj_mesh("keyboard.obj");

  float* vertices = loaded_model.model_vertices.data();
  int* indices = loaded_model.model_indices.data();

  int vertices_num = loaded_model.model_vertices.size();
  int indices_num = loaded_model.model_indices.size();

  //buffers
  unsigned int VAO;
  glGenVertexArrays(1, &VAO);  
  glBindVertexArray(VAO);

  unsigned int VBO;
  glGenBuffers(1, &VBO);    
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glBufferData(GL_ARRAY_BUFFER, loaded_model.model_vertices.size()*sizeof(float), vertices, GL_STATIC_DRAW);

  // tell attrib pointers where to read
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); // void pointer typecast cause why not :-)
  glEnableVertexAttribArray(0);
  //texture
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3* sizeof(float)));
  glEnableVertexAttribArray(1);

  //mor buffer 
  unsigned int EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, loaded_model.model_indices.size()*sizeof(int), indices, GL_STATIC_DRAW);

  if(POLY_WIREFRAME)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else {glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);}



  Shader mainShader("default.vert","default.frag");

  mainShader.use();

  mainShader.setInt("texture1", 0);
  mainShader.setInt("texture2", 1);


  // random shit for render loop
  unsigned int transformLoc = glGetUniformLocation(mainShader.ID, "transform");


  // pre debugging space
  
  //====================
  
  while(!glfwWindowShouldClose(window))
    {
      processInput(window);

      float timeValue = glfwGetTime();
      float greenValue = std::abs(sin(timeValue) * 360);

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     
      //model matrix
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::rotate(model, glm::radians(greenValue/5), glm::vec3(1.0f, 0.0f, 0.0f));

      //view matrix
      glm::mat4 view = glm::mat4(1.0f);
      // note that we're translating the scene in the reverse direction of where we want to move
      view = glm::translate(view, glm::vec3(0.0f, 0.0f, -10.0f)); 
      
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

      // w

      mainShader.use();
      
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      
      //glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);      
      
      glDrawElements(GL_TRIANGLES, indices_num, GL_UNSIGNED_INT, 0);
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
