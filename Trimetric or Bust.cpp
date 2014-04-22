#include "stdafx.h"
#include "Graphics.h"
#include "cModel.h"

int main(){
	Graphics graphics;
	cModel player("stalker.iqm");
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