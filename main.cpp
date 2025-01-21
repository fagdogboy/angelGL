#include <assimp/material.h>
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

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <system_error>

#define STB_IMAGE_IMPLEMENTATION
#define FOV_DEF 90.0f
#define POLY_WIREFRAME false
#define MESH_TO_LOAD "assets/block.obj"

//#include "shader/shader.h"
#include "newshader.h"
#include "stb_image.h"
#include <vector>
#include <strstream>

#include "entity.h"
#include "scene.h"

//camera
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraLookAt = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

glm::vec3 direction = {0.0f,0.0f,0.0f};

// bungie employees hate this one simple trick
float deltaTime = 0.0f;	
float lastFrame = 0.0f;

// input shit
bool firstMouse = true;
double lastX = 0;
double lastY = 0;
double yaw = 0;
double pitch = 0;

float generateRandomFloat() {

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
  
  return dis(gen);  
}

// thies hat wieder zugeschlagen :c
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
  
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraLookAt = glm::normalize(direction);
}

entity_data initialize_model(const std::string& path) {

  printf("starting model creation!\n");
  
  entity_data output_data;
  
  Assimp::Importer importer;
  
  // Load the model
  const aiScene* scene = importer.ReadFile(path,
					   aiProcess_Triangulate | aiProcess_FlipUVs);
  
  // Check for errors
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    // Handle error (e.g., throw an exception or return)
    std::cout << "could not import model!!!" << std::endl;
    return output_data;
  }
  
  // Clear previous data
  output_data.entity_vertices.clear();
  output_data.entity_indices.clear();
  
  // Process each mesh
  for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
    const aiMesh* mesh = scene->mMeshes[i];
    
    // Reserve space in the vertices vector
    output_data.entity_vertices.reserve(mesh->mNumVertices * 5); // 3 pos + 2 tex = 5

    std::cout << "num pre import: " << mesh->mNumVertices << std::endl;
    
    // Process vertices
    for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
      const aiVector3D& vertex = mesh->mVertices[j];
      const aiVector3D& texCoord = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][j] : aiVector3D(0.0f, 0.0f, 0.0f);
      
      // Push back position (x, y, z) and texture coordinates (u, v)
      output_data.entity_vertices.push_back(vertex.x);
      output_data.entity_vertices.push_back(vertex.y);
      output_data.entity_vertices.push_back(vertex.z);
      output_data.entity_vertices.push_back(texCoord.x);
      output_data.entity_vertices.push_back(texCoord.y);
    }
    
    // Process indices
    for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
      const aiFace& face = mesh->mFaces[j];
      for (unsigned int k = 0; k < face.mNumIndices; k++) {
        output_data.entity_indices.push_back(face.mIndices[k]);
      }
    }
  }

  printf("done with vertex shit\n");
  printf("materials in mesh: %d\n",scene->mNumMaterials);

  for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
    const aiMaterial* material = scene->mMaterials[i];

    //unknown textures
    for (unsigned int j = 0; j < material->GetTextureCount(aiTextureType_DIFFUSE); j++) {
      aiString texturePath; // This will hold the texture path
      if (material->GetTexture(aiTextureType_UNKNOWN, j, &texturePath) == AI_SUCCESS) {
	printf("mat contains unknown tex\n");
	output_data.texture_paths.push_back(texturePath.C_Str());
      }
    }
    
    // Get the number of textures of type aiTextureType_DIFFUSE
    for (unsigned int j = 0; j < material->GetTextureCount(aiTextureType_DIFFUSE); j++) {
      aiString texturePath; // This will hold the texture path
      if (material->GetTexture(aiTextureType_DIFFUSE, j, &texturePath) == AI_SUCCESS) {
	// Store the texture path
	output_data.texture_paths.push_back(texturePath.C_Str());
      }
    }
  }
  
  return output_data;
  }


entity_data import_obj_mesh(std::string file_path) {

  entity_data output;
    
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
        output.entity_vertices.push_back(vec.x);
	output.entity_vertices.push_back(vec.y);
	output.entity_vertices.push_back(vec.z);

	//tex coords. its jank ik
	output.entity_vertices.push_back(generateRandomFloat());
	output.entity_vertices.push_back(generateRandomFloat());

      }
      
      if(line[0] == 'f') {

	int f[3];

	s >> junk >> f[0] >> f[1] >> f[2];
	
	output.entity_indices.push_back(f[0]-1);
	output.entity_indices.push_back(f[1]-1);
	output.entity_indices.push_back(f[2]-1);

      }
      
    }

    std::cout << "successfully deserialized mesh. (hopefully)" << std::endl;
      
    return output;
    
}

void printFloatArray(const float* array, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    std::cout << "Vertex " << i << ": ";
    std::cout << "Position: (" 
	      << array[i * 5] << ", " 
	      << array[i * 5 + 1] << ", " 
	      << array[i * 5 + 2] << "), "
	      << "Texture: (" 
	      << array[i * 5 + 3] << ", " 
	      << array[i * 5 + 4] << ")"
	      << std::endl;
  }
}

void bind_texture_to_slot(std::string to_load, unsigned int slot)
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
  
  float cameraSpeed = 10.0f * deltaTime;
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    cameraPos += cameraSpeed * cameraLookAt;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    cameraPos -= cameraSpeed * cameraLookAt;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    cameraPos -= glm::normalize(glm::cross(cameraLookAt, cameraUp)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    cameraPos += glm::normalize(glm::cross(cameraLookAt, cameraUp)) * cameraSpeed;  
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

  //setup
  glEnable(GL_DEPTH_TEST);  
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);    
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  if(POLY_WIREFRAME)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else {glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);}
  
  // model loading

  scene active_scene;
  entity new_entity;
  new_entity.data = initialize_model(MESH_TO_LOAD);

  std::cout << new_entity.data.texture_paths.size() << std::endl;
  
  active_scene.add_entity(new_entity);
 
  //buffers
  int num_tex = 0;
  for(auto& i : active_scene.loaded_entities) {

    printf("trying to import a model...\n");
    
    //vao vbo ebo
    glGenVertexArrays(1, &i.data.entity_VAO);  
    glBindVertexArray(i.data.entity_VAO);
    printf("bound vertex array with id: %d\n", i.data.entity_VAO);
    
    glGenBuffers(1, &i.data.entity_VBO);    
    glBindBuffer(GL_ARRAY_BUFFER, i.data.entity_VBO);
    
    glBufferData(GL_ARRAY_BUFFER, i.data.entity_vertices.size()*sizeof(float), i.data.entity_vertices.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &i.data.entity_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i.data.entity_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, i.data.entity_indices.size()*sizeof(int), i.data.entity_indices.data(), GL_STATIC_DRAW);
    
    // tell attrib pointers where to read
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); 
    glEnableVertexAttribArray(0);
    //texture
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1);

    printf("done importing a model, trying to load: %d textures for this model.\n",i.data.texture_paths.size());
    
    for(auto j : i.data.texture_paths) {
      printf("- model contains entry in texture paths: %d\n", num_tex);
      std::cout << j << std::endl;
      bind_texture_to_slot(i.data.texture_paths[num_tex], num_tex);
      printf("- finished loading attempt in slot %d? \n", num_tex);
      num_tex++;
    }
    
  }
  
  //uhm
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
      
      // clear shit wow comment to look nice
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

      // TEMP      
      float timeValue = glfwGetTime();
      
      float rotate_first = std::abs(sin(timeValue) * 2+2);
      float rotate_second = std::abs(cos(timeValue) * 2+2);

      //setup ============================
      float currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;

      //projection matrix
      int height = 0 ,width = 0;
      glfwGetWindowSize(window, &height, &width);
      glm::mat4 proj = glm::perspective(glm::radians(FOV_DEF), (float)width/(float)height, 0.1f, 100.0f);

      //view matrix
      glm::mat4 view;
      view = glm::lookAt(cameraPos,cameraLookAt+cameraPos,cameraUp); 

      //render loop for all loaded models
      for(auto i : active_scene.loaded_entities) {

	//setup env
	//printf("vao bind, element nr: %d, vertices, %d \n", i , i.data.entity_indices.size());
	glBindVertexArray(i.data.entity_VAO);
	if (glIsVertexArray(i.data.entity_VAO) == GL_FALSE) {
	  std::cout << "ERROR::VAO::INVALID_ID: " << i.data.entity_VAO << std::endl;
	}
	
        mainShader.use();
	
	//model matrix
	glm::mat4 model = glm::mat4(1.0f);
	

	// sending data
	int modelLoc = glGetUniformLocation(mainShader.ID, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	int viewLoc = glGetUniformLocation(mainShader.ID, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	int projectionLoc = glGetUniformLocation(mainShader.ID, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(proj));
	if (modelLoc == -1 || viewLoc == -1 || projectionLoc == -1) {
	  std::cout << "ERROR::UNIFORM::LOCATION_NOT_FOUND" << std::endl;
	}
	
	glDrawElements(GL_TRIANGLES, i.data.entity_indices.size(), GL_UNSIGNED_INT, 0);
	
      }
      
      //glDrawArrays(GL_TRIANGLES, 0, 36);      
      glfwSwapBuffers(window);
      glfwPollEvents();    

    }

  glfwTerminate();
  
  return 0;
  
}
