#include "stdafx.h"
#include "Player.h"
#include "AssetLoader.h"
Player::Player():Model("stalker.iqm"){
	try{
		proggy = genShaders("shaders/passShadow.vert", "shaders/passShadow.frag");
	}catch(const char * e){
		std::cout << e<<std::endl;
	}
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glGenTextures(1, &cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
	for(int i = 0; i <6; i++){
		/*if(i==2 || i ==3){
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_DEPTH_COMPONENT, 2, 2,0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, cubemap,0);
			//glClear(GL_DEPTH_BUFFER_BIT);
		}
		else*/
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_DEPTH_COMPONENT, 400, 400,0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
		switch(i){
		case 0://NEGATIVE X
			sideViews[i] = glm::lookAt(glm::vec3(0), glm::vec3(-1,0,0),glm::vec3(0,-1,0));
			break;
		case 2://NEGATIVE Y
			sideViews[i] = glm::lookAt(glm::vec3(0), glm::vec3(0,-1,0),glm::vec3(0,0,-1));
			break;
		case 4://NEGATIVE Z
			sideViews[i] = glm::lookAt(glm::vec3(0), glm::vec3(0,0,-1),glm::vec3(0,-1,0));
			break;
		case 1://POSITIVE X
			sideViews[i] = glm::lookAt(glm::vec3(0), glm::vec3(1,0,0),glm::vec3(0,-1,0));
			break;
		case 3://POSITIVE Y
			sideViews[i] = glm::lookAt(glm::vec3(0), glm::vec3(0,1,0),glm::vec3(0,0,1));
			break;
		case 5://POSITIVE Z
			sideViews[i] = glm::lookAt(glm::vec3(0), glm::vec3(0,0,1),glm::vec3(0,-1,0));
			break;
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

	glDrawBuffer(GL_NONE);
	/*glViewport(0,0,400,400);
	for(int i = 0; i <2; i++){
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_Y+i, cubemap,0);
		glClear(GL_DEPTH_BUFFER_BIT);
	}*/
}

Player::~Player(){
	glDeleteTextures(1, &cubemap);
	glDeleteFramebuffers(1, &frameBuffer);
	glDeleteProgram(proggy);
}