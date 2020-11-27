//#define GLFW_INCLUDE_VULKAN

#include<stdlib.h>
#include<stdio.h>
#include<iostream>
#include"math/Vec3.h"
#include"math/Quaternion.h"
#include"Renderer/Camera.h"
#include"Renderer/Renderer.h"
#include"Renderer/VulkanRenderer.h"
#include"Renderer/VkRealTimeRenderer.h"
#include"Renderer/VkForwardRenderer.h"

#include"BVH/BVH.h"
#include"BVH/AABB.h"

#define GL_SILENCE_DEPRECATION
// #include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/gtx/quaternion.hpp>
//#include "main.h"

#define GL_RGBA32F 0x8814
#define GL_RGB32F 0x8815

Camera cam;
Renderer * renderer;

//bool keys[1024];
//bool mouse_keys[64] = {};
//GLfloat deltaTime = 0.0f;
//GLfloat lastFrame = 0.0f;

//void Do_Movement();
//void Do_Rotate();
//double mouse_pos_x = 0, mouse_pos_y = 0;

float rot_x = 0;
float rot_y = 0;


int main(){
	
    cam = Camera(800, 600, Vec3(200.0f, 0.0f, -200.0f));
    //std::cout << "cam init done";
    // Renderer * renderer = new ParallelRenderer();
    renderer = new VkForwardRenderer();
    renderer->StartRender();
    // cam.transform.position = {-0.671992, 0.378616, -30.7013}; // dragon
    // rot_y = 12.3; rot_x = 100;
    //cam.transform.rotation = 
    //    glm::normalize(glm::quat(glm::radians(glm::vec3{rot_y, -rot_x, 180 })));     
    //cam.transform.UpdateMatrix();    


	return 0;
}


