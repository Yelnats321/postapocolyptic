#include "stdafx.h"
#include "Graphics.h"
#include "AssetLoader.h"
#include "Player.h"
#include "IQMFile.h"

Graphics::Graphics():
//For the projection 
Z_DEPTH(20), X_Y_DEPTH(5),
//for the actual window
WINDOW_WIDTH(1024), WINDOW_HEIGHT(768),
//for the shadow cubemap
SHADOW_CUBE_SIZE(800),
//matrix = glm::ortho<float>(-aspect, aspect, -1, 1, 0, 100) *glm::lookAt(glm::vec3(0,1,0.001), glm::vec3(0,0,0), glm::vec3(0,1,0));
//36,34,90 is good so keep it ok?
baseProjection(glm::ortho<float>(-WINDOW_WIDTH/WINDOW_HEIGHT*X_Y_DEPTH, WINDOW_WIDTH/WINDOW_HEIGHT*X_Y_DEPTH, -X_Y_DEPTH, X_Y_DEPTH, -Z_DEPTH, Z_DEPTH)),
baseView(glm::lookAt(glm::vec3(-std::sin(M_PI*36/180.f)*X_Y_DEPTH,sin(M_PI*34/180.f)*X_Y_DEPTH,X_Y_DEPTH), glm::vec3(), glm::vec3(0,1,0)))
{
	glfwInit();
	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( WINDOW_WIDTH, WINDOW_HEIGHT, "Fallout Kinda", NULL, NULL );

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
		baseProgram = genShaders("shaders/vert.vert", "shaders/frag.frag");
	}catch(const char * e){
		std::cout<<"Normal shader compile failed: " << e<<std::endl;
	}
	glUseProgram(baseProgram);
	glUniform1i(glGetUniformLocation(baseProgram, "tex"), 0);
	glUniform1i(glGetUniformLocation(baseProgram, "cubemap"), 1);


	try{
		shadowProgram = genShaders("shaders/passShadow.vert", "shaders/passShadow.frag");
	}catch(const char * e){
		std::cout << e<<std::endl;
	}
	glGenFramebuffers(1, &shadowFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFrameBuffer);
	glDrawBuffer(GL_NONE);

	glGenTextures(1, &shadowCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, shadowCubemap);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

	for(int i = 0; i <6; i++){
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_DEPTH_COMPONENT, SHADOW_CUBE_SIZE, SHADOW_CUBE_SIZE,0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
	}

	shadowMapViews[1] = glm::lookAt(glm::vec3(0), glm::vec3(-1,0,0),glm::vec3(0,-1,0));
	shadowMapViews[3] = glm::lookAt(glm::vec3(0), glm::vec3(0,-1,0),glm::vec3(0,0,1));
	shadowMapViews[5] = glm::lookAt(glm::vec3(0), glm::vec3(0,0,-1),glm::vec3(0,-1,0));

	shadowMapViews[0] = glm::lookAt(glm::vec3(0), glm::vec3(1,0,0),glm::vec3(0,-1,0));
	shadowMapViews[2] = glm::lookAt(glm::vec3(0), glm::vec3(0,1,0),glm::vec3(0,0,-1));
	shadowMapViews[4] = glm::lookAt(glm::vec3(0), glm::vec3(0,0,1),glm::vec3(0,-1,0));

	//projectile test shit
	GLfloat vert [] = {0,0.2,0,
		1,0.2,1};
	glGenVertexArrays(1, &projVao);
	glBindVertexArray(projVao);
	glGenBuffers(1, &projVbo);
	glBindBuffer(GL_ARRAY_BUFFER, projVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
}

Graphics::~Graphics(){
	glDeleteTextures(1, &shadowCubemap);
	glDeleteFramebuffers(1, &shadowFrameBuffer);
	glDeleteProgram(shadowProgram);

	glDeleteProgram(baseProgram);

	glDeleteVertexArrays(1, &projVao);
	glDeleteBuffers(1, &projVbo);

	glfwTerminate();
}

void Graphics::processInput(){
	if(glfwGetKey(window, GLFW_KEY_ESCAPE)==GLFW_PRESS)
		glfwSetWindowShouldClose(window, 1);
}

glm::vec2 Graphics::getMouseTile(Map & map, float height = 0.f){
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	glm::vec3 win = glm::vec3( mouseX, WINDOW_HEIGHT - mouseY, -Z_DEPTH);
	glm::vec4 viewport(0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
	glm::vec3 unprojectedNearZ = glm::unProject(win,
		baseView,
		baseProjection,
		viewport);
	win[2] = Z_DEPTH;
	glm::vec3 unprojectedFarZ = glm::unProject(win,
		baseView,
		baseProjection,
		viewport);


	glm::vec3 dir( unprojectedFarZ-unprojectedNearZ );

	float t = unprojectedNearZ.y/(-dir.y);
	glm::vec3 linePlaneIntersect = unprojectedNearZ + (dir*t); 
	return glm::vec2(linePlaneIntersect.x, linePlaneIntersect.z);
}

void Graphics::drawModel(const Model * model, const glm::mat4 & VP, bool useTex, GLuint prog, const glm::vec3 * colors = nullptr, int amount = 0){
	glBindVertexArray(model->data->getVao());
	glUniformMatrix4fv(glGetUniformLocation(prog, "M"), 1, GL_FALSE, glm::value_ptr(model->getModelMatrix()));
	glUniformMatrix4fv(glGetUniformLocation(prog, "MVP"), 1, GL_FALSE, glm::value_ptr(VP*model->getModelMatrix()));
	for(int i = 0; i < model->data->getNumMeshes(); i++){
		const IQM::iqmmesh &m = model->data->getMeshes()[i];
		if(useTex){
			glUniform3fv(glGetUniformLocation(prog, "color"), 1, glm::value_ptr(colors[std::min(i, amount-1)]));
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, model->data->getTexture(i));
		}
		glDrawElements(GL_TRIANGLES, m.num_triangles*3, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * m.first_triangle*3));
	}
}

void Graphics::drawMap(const Map * map,const glm::mat4 & VP, bool useTex, GLuint prog){
	glBindVertexArray(map->getVao());
	if(useTex){
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, map->getTexture());
	}
	for(unsigned int y = 0; y < map->getHeight(); y++){
		for(unsigned int x = 0; x < map->getWidth(); x++){
			glm::mat4 M = glm::translate(glm::mat4(), glm::vec3(x, 0, y));
			glUniformMatrix4fv(glGetUniformLocation(prog, "M"), 1, GL_FALSE, glm::value_ptr(M));
			glUniformMatrix4fv(glGetUniformLocation(prog, "MVP"), 1, GL_FALSE, glm::value_ptr(VP * M));
			if(useTex){
				glm::vec3 color(1);
				glUniform3fv(glGetUniformLocation(prog, "color"), 1, glm::value_ptr(color));
			}
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
	}
}

void Graphics::update(double dt){
	int fx;
	while((fx = glGetError())!=GL_NO_ERROR)
		std::cout<<"-OGL ERROR! "<<fx<<std::endl;
	static Map map("test.bin");
	static Model building("building.iqm");
	building.setPosition(glm::vec3(1,0,1));
	//building.setRotation(0,M_PI,0);
	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) ==GLFW_PRESS){
		glm::vec2 diff = getMouseTile(map) - glm::vec2(player->getPosition().x+0.5, player->getPosition().z+0.5);
		projOffset = glm::translate(glm::mat4(), player->getPosition() + glm::vec3(0.5, 0, 0.5));
		projOffset = glm::scale(projOffset, glm::vec3(diff.x, 1, diff.y));
		std::cout<<diff.x<< " " <<diff.y<<std::endl;
	}
	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS){
		glm::vec2 pos = getMouseTile(map);
		player->setPosition(std::floor(pos.x), 0, std::floor(pos.y));
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
	//Shadow shit
	glViewport(0,0,SHADOW_CUBE_SIZE,SHADOW_CUBE_SIZE);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFrameBuffer);
	glUseProgram(shadowProgram);
	glEnable(GL_DEPTH_TEST);
	const glm::vec3 lightPos = player->getPosition() + glm::vec3(0.5,0.3, 0.5);
	for(int i = 0; i < 6; i++){
		//	i = i*2;
		//for now, I think I will n eed all sides later
		//if(i == 2)
		//	i=4;
		glm::mat4 wuew = glm::perspective<float>(90,1,0.01, 5)*glm::translate(shadowMapViews[i], -lightPos);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, shadowCubemap,0);
		glClear( GL_DEPTH_BUFFER_BIT);
		//unneeded ATM cuz map doesnt obscure anything(nohting below it)
		drawMap(&map, wuew, false, shadowProgram);
		drawModel(&building, wuew, false, shadowProgram);
	}

	//Normal draw shit
	//glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(baseProgram);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glViewport(0,0,WINDOW_WIDTH, WINDOW_HEIGHT);
	//figure out (later) if I need this here or it can just be done once in init
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, shadowCubemap);
	glUniform3fv(glGetUniformLocation(baseProgram, "lightPos"), 1, glm::value_ptr(lightPos));
	const glm::mat4 VP = baseProjection * baseView;
	drawMap(&map, VP, true, baseProgram);

	drawModel(player,VP, true, baseProgram, &glm::vec3(1), 1);
	glm::vec3 colors[5] = {
		glm::vec3(0.5),
		glm::vec3(0,0.8,0.7),
		glm::vec3(0.5),
		glm::vec3(0.5),
		glm::vec3(0.5)};
	drawModel(&building, VP, true, baseProgram, colors, 5);

	//projectiel test shit
	glBindVertexArray(projVao);
	glUniform3fv(glGetUniformLocation(baseProgram, "color"), 1, glm::value_ptr(glm::vec3(1,0,0)));
	glUniformMatrix4fv(glGetUniformLocation(baseProgram, "M"), 1, GL_FALSE, glm::value_ptr(projOffset));
	glUniformMatrix4fv(glGetUniformLocation(baseProgram, "MVP"), 1, GL_FALSE, glm::value_ptr(VP*projOffset));
	glDrawArrays(GL_LINES, 0, 6);
	glfwPollEvents();
	glfwSwapBuffers(window);
}

bool Graphics::isOpen(){return !glfwWindowShouldClose(window);}
void Graphics::setPlayer(Player * p){player = p;}