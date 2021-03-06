#include "stdafx.h"
#include "Model.h"
#include "Graphics.h"
#include "AssetLoader.h"
#include "IQMFile.h"
#include "EntityManager.h"
#include <random>

Graphics::Graphics():
//For the projection 
Z_DEPTH(20), X_Y_DEPTH(5),
//for the actual window
WINDOW_WIDTH(1024), WINDOW_HEIGHT(768),
//for the shadow cubemap
SHADOW_CUBE_SIZE(800), SHADOW_NEAR(0.1), SHADOW_FAR(10),
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
	glfwWindowHint(GLFW_ALPHA_BITS, 8);

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
	glColorMask(true, true, true, true);
	glClearColor(0.95f,0.95f,0.95f, 1);
	glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	try{
		firstPassProgram = AssetLoader::genShaders("shaders/vert.vert", "shaders/firstPass.frag");
	}catch(const char * e){
		std::cout<<"Normal shader compile failed: " << e<<std::endl;
	}
	glUseProgram(firstPassProgram);
	glUniform1i(glGetUniformLocation(firstPassProgram, "tex"), 0);
	glUniform1i(glGetUniformLocation(firstPassProgram, "cubemap"), 1);
	glUniform1f(glGetUniformLocation(firstPassProgram, "SHADOW_NEAR"), SHADOW_NEAR);
	glUniform1f(glGetUniformLocation(firstPassProgram, "SHADOW_FAR"), SHADOW_FAR);

	try{
		secondPassProgram = AssetLoader::genShaders("shaders/vert.vert", "shaders/secondPass.frag");
	}catch(const char * e){
		std::cout<<e<<std::endl;
	}
	try{
		shadowProgram = AssetLoader::genShaders("shaders/passShadow.vert", "shaders/passShadow.frag");
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

	//enable if cubemap is a shadow, disable if not
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

	for(int i = 0; i <6; i++){
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_DEPTH_COMPONENT, SHADOW_CUBE_SIZE, SHADOW_CUBE_SIZE,0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}

	shadowMapViews[1] = glm::lookAt(glm::vec3(0), glm::vec3(-1,0,0),glm::vec3(0,-1,0));
	shadowMapViews[3] = glm::lookAt(glm::vec3(0), glm::vec3(0,-1,0),glm::vec3(0,0,1));
	shadowMapViews[5] = glm::lookAt(glm::vec3(0), glm::vec3(0,0,-1),glm::vec3(0,-1,0));

	shadowMapViews[0] = glm::lookAt(glm::vec3(0), glm::vec3(1,0,0),glm::vec3(0,-1,0));
	shadowMapViews[2] = glm::lookAt(glm::vec3(0), glm::vec3(0,1,0),glm::vec3(0,0,-1));
	shadowMapViews[4] = glm::lookAt(glm::vec3(0), glm::vec3(0,0,1),glm::vec3(0,-1,0));
	entityManager = new EntityManager;
	Projectile::initialize();

	Building building("assets/cube.iqm");
	building.setPosition(glm::vec3(3, 0, 2.5));
	building.setRotation(0, M_PI, 0);
	entityManager->addBuilding(std::move(building));

	map = new Map("assets/test.bin");
}

Graphics::~Graphics(){
	delete map;
	delete entityManager;
	glDeleteTextures(1, &shadowCubemap);
	glDeleteFramebuffers(1, &shadowFrameBuffer);
	glDeleteProgram(shadowProgram);

	glDeleteProgram(firstPassProgram);

	Projectile::close();

	glfwTerminate();
}

void Graphics::processInput(){
	if(glfwGetKey(window, GLFW_KEY_ESCAPE)==GLFW_PRESS)
		glfwSetWindowShouldClose(window, 1);
}

glm::vec2 Graphics::getMouseTile(float height = 0.f){
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	glm::vec3 win = glm::vec3( mouseX, WINDOW_HEIGHT - mouseY, -Z_DEPTH);
	glm::vec4 viewport(0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
	glm::vec3 line0 = glm::unProject(win,
		baseView,
		baseProjection,
		viewport);
	win[2] = Z_DEPTH;
	glm::vec3 line1 = glm::unProject(win,
		baseView,
		baseProjection,
		viewport);


	//from the wikipedia page kinda
	glm::vec3 lineDir(line1 - line0);
	//originally its (point0-line0) dot normal / lineDir dot normal
	//since the point0 is just 0,0,0 and the normal is only up, we only use the y
	float dist = (height-line0.y)/lineDir.y;
	//trace the dist along the line
	glm::vec3 linePlaneIntersect = line0 + (lineDir*dist); 
	//point = near + dir*near.y
	return glm::vec2(linePlaneIntersect.x, linePlaneIntersect.z);
}

bool rayTriangleIntersect(const glm::vec3 & rayStart, const glm::vec3 & rayEnd,const std::array<glm::vec3, 3> & vertices){
	glm::vec3 e1,e2,h,s,q, d(rayEnd-rayStart);
	float a,f,u,v;
	e1 = vertices[1]-vertices[0];
	e2 = vertices[2]-vertices[0];
	h = glm::cross(d, e2);
	//crossProduct(h,d,e2);
	a = glm::dot(e1,h);

	if (a > -0.00001 && a < 0.00001)
		return(false);

	f = 1/a;
	s = rayStart-vertices[0];
//	vector(s,p,v0);
	u = f * (glm::dot(s,h));

	if (u < 0.0 || u > 1.0)
		return(false);
	q = glm::cross(s, e1);
//	crossProduct(q,s,e1);
	v = f * glm::dot(d,q);

	if (v < 0.0 || u + v > 1.0)
		return(false);

	// at this stage we can compute t to find out where
	// the intersection point is on the line
	float t = f * glm::dot(e2,q);

	if (t > 0.00001 && t<=1){ // ray intersection
		//std::cout<<rayEnd.x << " " << rayEnd.z<<std::endl;
		//if((rayStart + d*t) <= glm::vec3(rayEnd))
		return(true);
	}

	else // this means that there is a line intersection
		 // but not a ray intersection
		 return (false);

/*	for(int i = 0; i < 3; i++){
		for(int j = 0; j <3; j++){
			std::cout<< vertices[i*3+j] << " ";
		}
		std::cout<<std::endl;
	}*/
	//return false;
}

bool rayModelIntersect(const Model * model, const glm::vec3 & rayStart, const glm::vec3 & rayEnd){
	const glm::mat4 invModel = glm::inverse(model->getModelMatrix());
	const glm::vec3 invRS(invModel * glm::vec4(rayStart, 1)), invRE(invModel * glm::vec4(rayEnd, 1));
	for(int i = 0; i< model->data->getNumTris(); i++){
		if(rayTriangleIntersect(invRS, invRE, model->data->getTriangle(i))){
			return true;
		}
	}
	return false;
}

glm::vec3 randomVector(const glm::vec3 & refVec, float angle){
	std::mt19937 randomEngine(std::chrono::system_clock::now().time_since_epoch().count());
	std::uniform_real_distribution <> angDist(0, M_PI*2);
	std::uniform_real_distribution <> distDist(0, 1);
	float rad = sqrt(distDist(randomEngine));
	float ang = angDist(randomEngine);
	float x = rad * cos(ang);
	float y = rad * sin(ang);
	float r = tan((angle/2)*M_PI/180);
	glm::vec3 randomVec(x*r,y*r,1);
	randomVec = glm::normalize(randomVec);
	glm::vec3 normRef = glm::normalize(refVec);

	//glm::rotate(normRef, randomVec);
	return glm::rotate(	glm::rotation(glm::vec3(0,0,1),normRef), randomVec)*glm::length(refVec);
}

void Graphics::fire(){
	const glm::vec2 mousePos = getMouseTile();
	auto playerModel = &entityManager->getPlayer();
	glm::vec3 diff = glm::vec3(mousePos.x, 0.2, mousePos.y) - glm::vec3(playerModel->getPosition().x + 0.5, 0.2, playerModel->getPosition().z + 0.5);
	diff = glm::normalize(randomVector(diff, 10));
	glm::mat4 projOffset = glm::translate(glm::mat4(), playerModel->getPosition() + glm::vec3(0.5, 0.2, 0.5));
	projOffset = glm::scale(projOffset, diff);
	glm::vec3 start(projOffset * glm::vec4(0, 0, 0, 1)), end(projOffset*glm::vec4(1, 1, 1, 1));
	entityManager->addProjectile(Projectile(10, std::move(start), std::move(diff), std::move(projOffset)));
}
void Graphics::update(double dt){
	int fx;
	while((fx = glGetError())!=GL_NO_ERROR)
		std::cout<<"-OGL ERROR! "<<fx<<std::endl;
	//change this to be more broad, work using start and end rather than whatever it is now
	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) ==GLFW_PRESS){
		fire();
	}
	auto playerModel = &entityManager->getPlayer();
	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS){
		glm::vec2 pos = getMouseTile();
		playerModel->setPosition(std::floor(pos.x), 0, std::floor(pos.y));
	}
	if(glfwGetKey(window, GLFW_KEY_W)==GLFW_PRESS){
		auto pos = playerModel->getPosition();
		playerModel->setPosition(pos.x, pos.y, pos.z-1*dt);
	}
	if(glfwGetKey(window, GLFW_KEY_S)==GLFW_PRESS){
		auto pos = playerModel->getPosition();
		playerModel->setPosition(pos.x, pos.y, pos.z+1*dt);
	}
	if(glfwGetKey(window, GLFW_KEY_A)==GLFW_PRESS){
		auto pos = playerModel->getPosition();
		playerModel->setPosition(pos.x-1*dt, pos.y, pos.z);
	}
	if(glfwGetKey(window, GLFW_KEY_D)==GLFW_PRESS){
		auto pos = playerModel->getPosition();
		playerModel->setPosition(pos.x+1*dt, pos.y, pos.z);
	}
	//Shadow shit
	//glCullFace(GL_FRONT);
	glClearDepth(1);
	glViewport(0,0,SHADOW_CUBE_SIZE,SHADOW_CUBE_SIZE);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFrameBuffer);
	glUseProgram(shadowProgram);
	const glm::vec3 lightPos = playerModel->getPosition() + glm::vec3(0.5,0.3, 0.5);
	for(int i = 0; i < 6; i++){
		//	i = i*2;
		//for now, I think I will n eed all sides later
		//if(i == 2)
		//	i=4;
		glm::mat4 wuew = glm::perspective<float>(90,1,SHADOW_NEAR, SHADOW_FAR)*glm::translate(shadowMapViews[i], -lightPos);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, shadowCubemap,0);
		glClear( GL_DEPTH_BUFFER_BIT);
		//unneeded ATM cuz map doesnt obscure anything(nohting below it)
		map->draw(wuew, false, shadowProgram);
		for (auto & entity : entityManager->getActors())
			entity.draw(wuew, false, shadowProgram);
		for(auto & entity : entityManager->getBuildings())
			entity.draw(wuew, false, shadowProgram);
	}

	//Normal draw shit
	//glCullFace(GL_BACK);
	glBlendFunc(GL_ONE, GL_ZERO);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(firstPassProgram);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glViewport(0,0,WINDOW_WIDTH, WINDOW_HEIGHT);
	//figure out (later) if I need this here or it can just be done once in init
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, shadowCubemap);
	glUniform3fv(glGetUniformLocation(firstPassProgram, "lightPos"), 1, glm::value_ptr(lightPos));
	const glm::mat4 VP = baseProjection * baseView;
	map->draw(VP, true, firstPassProgram);

	playerModel->draw(VP, true, firstPassProgram, &glm::vec3(1), 1);
	glm::vec3 colors[5] = {
		glm::vec3(0.5),
		glm::vec3(0,0.8,0.7),
		glm::vec3(0.5),
		glm::vec3(0.5),
		glm::vec3(0.5)};

	for(auto & entity : entityManager->getActors())
		entity.draw(VP, true, firstPassProgram, colors, 5);
	for(auto & entity : entityManager->getBuildings())
		entity.draw(VP, true, firstPassProgram, colors, 5);

	
	glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
	glUseProgram(secondPassProgram);
	map->draw(VP, true, secondPassProgram);

	playerModel->draw(VP, true, secondPassProgram, &glm::vec3(1), 1);

	for(auto & entity : entityManager->getActors())
		entity.draw(VP, true, secondPassProgram, colors, 5);
	for(auto & entity : entityManager->getBuildings())
		entity.draw(VP, true, secondPassProgram, colors, 5);

	glUseProgram(firstPassProgram);
	glBindVertexArray(Projectile::getVao());
	glUniform3fv(glGetUniformLocation(firstPassProgram, "color"), 1, glm::value_ptr(glm::vec3(1, 1, 0)));
	for(auto itr = entityManager->getProjectiles().begin(); itr != entityManager->getProjectiles().end();){
		glm::vec3 end = itr->position + itr->direction*itr->speed / 60.f;
		bool deleted = false;
		if(itr->position.x < 0 || itr->position.x > map->getWidth()*Map::SIZE ||
		   itr->position.z < 0 || itr->position.z > map->getHeight()*Map::SIZE){
			deleted = true;
		}
		if(!deleted){
			for(auto & entity : entityManager->getBuildings()){
				if(rayModelIntersect(&entity, itr->position, end)){
					deleted = true;
					break;
				}
			}
		}
		if(deleted){
			itr = entityManager->getProjectiles().erase(itr);
			continue;
		}
		itr->position = end;
		itr->modelMatrix = glm::translate(glm::mat4(), itr->position);
		itr->modelMatrix = glm::scale(itr->modelMatrix, itr->direction);

		itr->draw(VP, true, firstPassProgram);
		++itr;
	}
	glfwPollEvents();
	glfwSwapBuffers(window);
}
