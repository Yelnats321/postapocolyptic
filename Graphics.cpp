#include "stdafx.h"
#include "Graphics.h"
#include "AssetLoader.h"
#include "Player.h"
#include "IQMFile.h"

Graphics::Graphics(){
	glfwInit();
	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Fallout Kinda", NULL, NULL );

	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental=true; // Needed in core profile
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	glClearColor(0.95f,0.95f,0.95f, 1);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	try{
		program = genShaders("shaders/vert.vert", "shaders/frag.frag");
	}catch(const char * e){
		std::cout<<"Normal shader compile failed: " << e<<std::endl;
	}
	glUseProgram(program);
	glUniform1i(glGetUniformLocation(program, "tex"), 0);
	glUniform1i(glGetUniformLocation(program, "cubemap"), 1);
	float aspect = 1024.f/768.f;
	//matrix = glm::ortho<float>(-aspect, aspect, -1, 1, 0, 100) *glm::lookAt(glm::vec3(0,1,0.001), glm::vec3(0,0,0), glm::vec3(0,1,0));
	//36,34,90 is good so keep it ok?
	projection = glm::ortho<float>(-aspect*5, aspect*5, -5, 5, -20, 20);
	view = glm::lookAt(glm::vec3(-std::sin(M_PI*36/180.f)*5,sin(M_PI*34/180.f)*5,5), glm::vec3(), glm::vec3(0,1,0));
	const GLfloat vertices[] = {0, 0, 0, 0, 0,// 1, 1, 1,
								1, 0, 0, 1, 0,// 1, 0, 0,
								0, 0, 1, 0, 1,// 0, 1, 0,
								1, 0, 1, 1, 1,// 0, 0, 1,

								/*-0.5f, 0,-0.5f, 1, 1, 1,
								 0.5f, 0,-0.5f, 1, 0, 0,
								-0.5f, 1.f,-0.5f, 0, 1, 0,
								 0.5f, 1.f,-0.5f, 0, 0, 1,*/
	};
	const GLuint indices[] = {0,1,2,
							1,3,2,
							/*4,5,6,
							5,7,6,*/};

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
	//3 because we need 3 floats per vertex
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) *5, (void*)(sizeof(GLfloat)*3));
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);
}

Graphics::~Graphics(){
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vao);

	glDeleteProgram(program);

	glfwTerminate();
}

void Graphics::processInput(){
	if(glfwGetKey(window, GLFW_KEY_ESCAPE)==GLFW_PRESS)
		glfwSetWindowShouldClose(window, 1);
}

glm::vec2 Graphics::getMouseTile(Map & map){
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	glm::vec3 win = glm::vec3( mouseX, 768 - mouseY, -20);
	glm::vec4 viewport(0,0,1024,768);
	glm::vec3 unprojectedNearZ = glm::unProject(win,
		view,
		projection,
		viewport);
	win[2] = 20;
	glm::vec3 unprojectedFarZ = glm::unProject(win,
		view,
		projection,
		viewport);


	glm::vec3 dir( unprojectedFarZ-unprojectedNearZ );

	float t = unprojectedNearZ.y/(-dir.y);
	glm::vec3 linePlaneIntersect = unprojectedNearZ + (dir*t); 

	return glm::vec2(std::floor(linePlaneIntersect.x), std::floor(linePlaneIntersect.z));
}



void Graphics::update(double dt){
	int fx;
	while((fx = glGetError())!=GL_NO_ERROR)
		std::cout<<"-OGL ERROR! "<<fx<<std::endl;
	static Map map("test.bin");
	static Model building("building.iqm");
	building.setPosition(glm::vec3(3,0,2.5));
	building.setRotation(0,M_PI,0);
	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS){
		glm::vec2 pos = getMouseTile(map);
		player->setPosition(pos.x, 0, pos.y);
	}
	if(glfwGetKey(window, GLFW_KEY_W)==GLFW_PRESS){
		auto pos = player->getPosition();
		player->setPosition(pos.x, pos.y, pos.z-1*dt);
	}
	if(glfwGetKey(window, GLFW_KEY_S)==GLFW_PRESS){
		auto pos = player->getPosition();
		player->setPosition(pos.x, pos.y, pos.z+1*dt);
	}
	if(glfwGetKey(window, GLFW_KEY_A)==GLFW_PRESS){
		auto pos = player->getPosition();
		player->setPosition(pos.x-1*dt, pos.y, pos.z);
	}
	if(glfwGetKey(window, GLFW_KEY_D)==GLFW_PRESS){
		auto pos = player->getPosition();
		player->setPosition(pos.x+1*dt, pos.y, pos.z);
	}
	glViewport(0,0,400,400);
	glBindFramebuffer(GL_FRAMEBUFFER, player->frameBuffer);
	glUseProgram(player->proggy);
	glEnable(GL_DEPTH_TEST);
	{
		for(int j = 0; j < 6; j++){
			if(j == 2)
				j=4;
			glm::mat4 wuew = glm::perspective<float>(90,1,0.1, 5)*glm::translate(player->sideViews[j], -player->getPosition()-glm::vec3(0.5,0.2,0.5));
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X+j, player->cubemap,0);
			glClear( GL_DEPTH_BUFFER_BIT);
			glBindVertexArray(vao);
			for(unsigned int y = 0; y < map.getHeight(); y++){
				for(unsigned int x = 0; x < map.getWidth(); x++){
					glm::mat4 MVP(glm::translate(wuew, glm::vec3(x, 0, y)));
					glUniformMatrix4fv(glGetUniformLocation(player->proggy, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				}
			}

			glBindVertexArray(building.data->getVao());
			glUniformMatrix4fv(glGetUniformLocation(player->proggy, "MVP"), 1, GL_FALSE, glm::value_ptr(wuew*building.getModelMatrix()));
			for(int i = 0; i < building.data->getNumMeshes(); i++){
				const IQM::iqmmesh &m = building.data->getMeshes()[i];
				glDrawElements(GL_TRIANGLES, m.num_triangles*3, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * m.first_triangle*3));
			}
		}
	}
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(program);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glViewport(0,0,1024, 768);
	glBindVertexArray(vao);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, player->cubemap);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, map.getTexture());
	glUniform3fv(glGetUniformLocation(program, "lightPos"), 1, glm::value_ptr(player->getPosition()+glm::vec3(0.5,0.2,0.5)));
	for(unsigned int y = 0; y < map.getHeight(); y++){
		for(unsigned int x = 0; x < map.getWidth(); x++){
			glm::mat4 MVP = glm::translate(projection * view, glm::vec3(x, 0, y));
			glUniformMatrix4fv(glGetUniformLocation(program, "M"), 1, GL_FALSE, glm::value_ptr(glm::translate(glm::mat4(), glm::vec3(x, 0, y))));
			glUniformMatrix4fv(glGetUniformLocation(program, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
			glm::vec3 color(1);
			/*switch(map(x,y)){
			case 0:
				color = glm::vec3(0.4,0.22,0.6);
				break;
			case 1:
				color = glm::vec3(0,1,0);
				break;
			case 2:
				color = glm::vec3(0,0,1);
				break;
			}*/
			glUniform3fv(glGetUniformLocation(program, "color"), 1, glm::value_ptr(color));
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
	}
	glBindVertexArray(player->data->getVao());
	glUniformMatrix4fv(glGetUniformLocation(program, "M"), 1, GL_FALSE, glm::value_ptr(player->getModelMatrix()));
	glUniformMatrix4fv(glGetUniformLocation(program, "MVP"), 1, GL_FALSE, glm::value_ptr(projection * view*player->getModelMatrix()));
	glUniform3fv(glGetUniformLocation(program, "color"), 1, glm::value_ptr(glm::vec3(1)));
	for(int i = 0; i < player->data->getNumMeshes(); i++){
		const IQM::iqmmesh &m = player->data->getMeshes()[i];
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, player->data->getTexture(i));
		glDrawElements(GL_TRIANGLES, m.num_triangles*3, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * m.first_triangle*3));
	}
	glBindVertexArray(building.data->getVao());
	glUniformMatrix4fv(glGetUniformLocation(program, "M"), 1, GL_FALSE, glm::value_ptr(building.getModelMatrix()));
	glUniformMatrix4fv(glGetUniformLocation(program, "MVP"), 1, GL_FALSE, glm::value_ptr(projection*view*building.getModelMatrix()));
	glm::vec3 colors[5] = {
		glm::vec3(0.5),
		glm::vec3(0,0.8,0.7),
		glm::vec3(0.5),
		glm::vec3(0.5),
		glm::vec3(0.5)};
	for(int i = 0; i < building.data->getNumMeshes(); i++){
		glUniform3fv(glGetUniformLocation(program, "color"), 1, glm::value_ptr(colors[i]));
		const IQM::iqmmesh &m = building.data->getMeshes()[i];
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, building.data->getTexture(i));
		glDrawElements(GL_TRIANGLES, m.num_triangles*3, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * m.first_triangle*3));
	}
	/*{
		glUseProgram(player->rendproggy);
		glClear(GL_DEPTH_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindVertexArray(player->vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, player->cubemap);
		for(int i = 0; i < 4; i++){
			glViewport(i*200,0,200,200);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}*/

	glfwPollEvents();
	glfwSwapBuffers(window);
}

bool Graphics::isOpen(){return !glfwWindowShouldClose(window);}
void Graphics::setPlayer(Player * p){player = p;}