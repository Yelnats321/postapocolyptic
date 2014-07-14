#include "stdafx.h"
#include "Graphics.h"
#include "EntityManager.h"

int main(){
	Graphics graphics;
	double time = glfwGetTime();
	while(graphics.isOpen()){
		double dt = glfwGetTime() - time;
		time = glfwGetTime();
		graphics.processInput();
		graphics.update(dt);
	}
	return 0;
}