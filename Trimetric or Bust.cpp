#include "stdafx.h"
#include "Graphics.h"
#include "Player.h"

int main(){
	Graphics graphics;
	Player player;
	graphics.setPlayer(&player);
	double time = glfwGetTime();
	while(graphics.isOpen()){
		double dt = glfwGetTime() - time;
		time = glfwGetTime();
		graphics.processInput();
		graphics.update(dt);
	}
	return 0;
}