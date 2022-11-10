#include "../libs/sdw/CanvasTriangle.h"
#include "../libs/sdw/DrawingWindow.h"
#include "../libs/sdw/Utils.h"
#include <fstream>
#include <vector>
#include "../libs/glm-0.9.7.2/glm/glm.hpp"
#include <iostream>
#include "../libs/sdw/CanvasPoint.h"
#include "../libs/sdw/Colour.h"
#include "../libs/sdw/CanvasTriangle.h"
#include "../libs/sdw/TextureMap.h"
#include "../libs/sdw/ModelTriangle.h"
#include <sstream>


#define WIDTH 320
#define HEIGHT 240

std::vector<float> interpolateSingleFloats(float from, float to, float numberOfValues) {
	std::vector<float> single; //variable single of type float
  float step = (to-from)/ float(numberOfValues-1); //step is dividing distance of two pixels by number of values
  for(int i=0; i<numberOfValues; i++) { //iterating through number of values
		single.push_back(from + i*step);  // push_back val to end of vector
    }
  return single;
}

std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues) {
	std::vector<glm::vec3> three; // declaring variable
	glm::vec3 step = (to-from)/float(numberOfValues-1); // step is dividing distance of two pixels by number of values
	for(float i=0; i<numberOfValues; i++) {
		three.push_back(from + i*step);
	}
	return three;
}

//takes in r,g,b and makes unsigned int of colours and returns that colour
uint32_t lineColour (int red, int green, int blue) {
	uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
	return colour;
}


void drawLine(CanvasPoint from, CanvasPoint to, Colour colour, DrawingWindow &window) {
	float xDiff = to.x - from.x; // distance from start to end
	float yDiff = to.y - from.y;
	
	float numberOfSteps = std::max(abs(xDiff), abs(yDiff)); // max of distance
	float xStepSize = xDiff/numberOfSteps; 
	float yStepSize = yDiff/numberOfSteps;

	for (float i = 0.0; i<numberOfSteps; i++) {
		float x = from.x + (xStepSize*i);
		float y = from.y + (yStepSize*i);
		window.setPixelColour(round(x), round(y), lineColour(colour.red, colour.green, colour.blue));
	}
}

void drawTexLine(CanvasPoint from, CanvasPoint to, TexturePoint texFrom, TexturePoint texTo,  TextureMap texture, DrawingWindow &window) {
  
	float xDiff = to.x - from.x; // distance from start to end
	float yDiff = to.y - from.y;
	
	float numberOfSteps = std::max(abs(xDiff), abs(yDiff)); // max of distance

	float xStepSize = xDiff/numberOfSteps; 
	float yStepSize = yDiff/numberOfSteps;

	float xTexDiff = texTo.x - texFrom.x; // distance from start to end for texture
	float yTexDiff = texTo.y - texFrom.y;

	float xTexStepSize = xTexDiff/numberOfSteps;
	float yTexStepSize = yTexDiff/numberOfSteps;

	for (float i = 0.0; i<numberOfSteps; i++) {
		float x = from.x + (xStepSize*i);
		float y = from.y + (yStepSize*i);

		float xTex = texFrom.x + (xTexStepSize*i);
		float yTex = texFrom.y + (yTexStepSize*i);

		window.setPixelColour(round(x), round(y), texture.pixels[ round(xTex) + round(yTex)*texture.width ]);
	}
}

void drawTriangle (CanvasTriangle triangle, Colour colour, DrawingWindow &window) {
	drawLine(triangle.v0(), triangle.v1(), colour, window); // drawing line from vertice 0 to vertice 1
	drawLine(triangle.v1(), triangle.v2(), colour, window);
	drawLine(triangle.v2(), triangle.v0(), colour, window);

}

void fillTriangle (CanvasPoint top, CanvasPoint mid, CanvasPoint bot, float height, Colour colour, DrawingWindow &window) {
	// interpolate between v0 and v1
	std::vector<float> topToMidX = interpolateSingleFloats(top.x, mid.x, height + 1);
	std::vector<float> topToMidY = interpolateSingleFloats(top.y, mid.y, height + 1);
	std::vector<float> topToBotX = interpolateSingleFloats(top.x, bot.x, height + 1);
	std::vector<float> topToBotY = interpolateSingleFloats(top.y, bot.y, height + 1);

  for (float i = 0; i < height; i++) {
		CanvasPoint from(topToMidX[i], topToMidY[i]);
		CanvasPoint to(topToBotX[i], topToBotY[i]);
		drawLine(from, to, colour, window);
	}
}

void textureTriangle (CanvasPoint top, CanvasPoint mid, CanvasPoint bot, float height, float textureH, TextureMap texture, DrawingWindow &window) {
	// interpolate between top(v0) and midpoint and top(v0) and bottom(v2)
	std::vector<float> topToMidX = interpolateSingleFloats(top.x, mid.x, height + 1);
	std::vector<float> topToMidY = interpolateSingleFloats(top.y, mid.y, height + 1);
	std::vector<float> topToBotX = interpolateSingleFloats(top.x, bot.x, height + 1);
	std::vector<float> topToBotY = interpolateSingleFloats(top.y, bot.y, height + 1);
  
	// interpolate from top(v0) to midpoint and top(v0) to bottom(v2) for texture map
	std::vector<float> texTopToMidX = interpolateSingleFloats(top.texturePoint.x, mid.texturePoint.x, height + 1);
	std::vector<float> texTopToMidY = interpolateSingleFloats(top.texturePoint.y, mid.texturePoint.y, height + 1);
	std::vector<float> texTopToBotX = interpolateSingleFloats(top.texturePoint.x, bot.texturePoint.x, height + 1);
	std::vector<float> texTopToBotY = interpolateSingleFloats(top.texturePoint.y, bot.texturePoint.y, height + 1);


	for (float i=0; i < height; i++) {
		CanvasPoint from(topToMidX[i], topToMidY[i]);
		CanvasPoint to(topToBotX[i], topToBotY[i]);

		TexturePoint texFrom(texTopToMidX[i], texTopToMidY[i]);
		TexturePoint texTo(texTopToBotX[i], texTopToBotY[i]);
		
		drawTexLine(from, to, texFrom, texTo, texture, window);
	}
}


void fillMapper (CanvasTriangle triangle, Colour colour, DrawingWindow &window) {
	// sorting vertices in order of height
	if (triangle.v0().y > triangle.v1().y) std::swap(triangle.v0().y, triangle.v1().y);
	if (triangle.v0().y > triangle.v2().y) std::swap(triangle.v0().y, triangle.v2().y);
	if (triangle.v1().y > triangle.v2().y) std::swap(triangle.v1().y, triangle.v2().y);
  // finding gradient of line from v0 to v2
	float gradient = ((triangle.v2().y - triangle.v0().y) / (triangle.v2().x - triangle.v0().x));
	// finding value of x on line v0 to v2
	float midX = ((triangle.v1().y - triangle.v0().y) / gradient) + triangle.v0().x;
	// co-ordinates of x on line v0 to v2
	CanvasPoint midPoint(midX, triangle.v1().y);
  // splitting triangle into two by using new x point
	CanvasTriangle topTriangle(triangle.v0(), midPoint, triangle.v1());
	CanvasTriangle bottomTriangle(midPoint, triangle.v1(),  triangle.v2());
	
	// fill in top triangle area
	drawTriangle(topTriangle, colour, window);
	// height of triangle
	float topHeight = midPoint.y - triangle.v0().y;
  // calling func to fill top triangle
  fillTriangle(triangle.v0(), midPoint, triangle.v1(), topHeight, colour, window);
	// fill in bottom triangle area
	drawTriangle(bottomTriangle, colour, window);
	// height of triangle
	float bottomHeight = triangle.v2().y - midPoint.y;

  fillTriangle(triangle.v2(), midPoint, triangle.v1(), bottomHeight, colour, window);
  // white border around triangles
	int red = 255;
	int green = 255;
	int blue = 255;

	Colour whiteBorder(red, green, blue);
	drawTriangle(triangle, whiteBorder, window);
}

void textureMapper(CanvasTriangle triangle, Colour colour, DrawingWindow &window) {
  
	TextureMap texture = TextureMap("texture.ppm");

	// sorting vertices in order of height
	if (triangle.v0().y > triangle.v1().y) std::swap(triangle.v0().y, triangle.v1().y);
	if (triangle.v0().y > triangle.v2().y) std::swap(triangle.v0().y, triangle.v2().y);
	if (triangle.v1().y > triangle.v2().y) std::swap(triangle.v1().y, triangle.v2().y);
  // finding gradient of line from v0 to v2
	float gradient = ((triangle.v2().y - triangle.v0().y) / (triangle.v2().x - triangle.v0().x));
  // finding value of x on line v0 to v2
	float midX = ((triangle.v1().y - triangle.v0().y) / gradient) + triangle.v0().x;
	// co-ordinates of new x point on line v0 to v2
	CanvasPoint midPoint(midX, triangle.v1().y);

	drawTriangle(triangle, colour, window);

	// finding midpoint of texture using ratio of triangles
	// ratio from top to midpoint
	float topToMidX = triangle.v0().x - midPoint.x;
	float topToMidY = triangle.v0().y - midPoint.y;

	// ratio from top to bottom
	float topToBotX = triangle.v0().x - triangle.v2().x;
	float topToBotY = triangle.v0().y - triangle.v2().y;

	float topToMidRatioX = topToMidX/topToBotX;
	float topToMidRatioY = topToMidY/topToBotY;

	// ratio from top to bottom of x and y co-ordinates for texture
	float texTopToBotX = triangle.v0().texturePoint.x - triangle.v2().texturePoint.x;
	float texTopToBotY = triangle.v0().texturePoint.y - triangle.v2().texturePoint.y;

	float texTopToMidX = topToMidRatioX * texTopToBotX;
	float texTopToMidY = topToMidRatioY * texTopToBotY;

  // co-ordinates of texture middle point
	midPoint.texturePoint.x = triangle.v0().texturePoint.x - texTopToMidX;
	midPoint.texturePoint.y = triangle.v0().texturePoint.y - texTopToMidY;

	// filling top triangle
	float topHeight = midPoint.y - triangle.v0().y;
	float texTopHeight = midPoint.texturePoint.y - triangle.v0().texturePoint.y;
	textureTriangle(triangle.v0(), midPoint, triangle.v1(), topHeight, texTopHeight, texture, window);

	// filling bottom triangle
	float botHeight = triangle.v2().y - midPoint.y ;
	float texBotHeight = triangle.v2().texturePoint.y - midPoint.texturePoint.y ;
	textureTriangle(triangle.v2(), midPoint, triangle.v1(), botHeight, texBotHeight, texture, window);


}


void keyPressU (DrawingWindow &window) {
	CanvasPoint ver0 = CanvasPoint(rand()%256, rand()%256); // making random value for each vertice point
	CanvasPoint ver1 = CanvasPoint(rand()%256, rand()%256);
	CanvasPoint ver2 = CanvasPoint(rand()%256, rand()%256);
	CanvasTriangle allV(ver0, ver1, ver2); // calls CanvasTriangle struct on vertices

	int red = rand()%256; // randomizes colour values
	int green = rand()%256;
	int blue = rand()%256;

	Colour colour(red, green, blue);
	drawTriangle(allV, colour, window);
}

void keyPressF (DrawingWindow &window) {
	CanvasPoint ver0 = CanvasPoint(rand()%256, rand()%256); // making random value for each vertice point
	CanvasPoint ver1 = CanvasPoint(rand()%256, rand()%256);
	CanvasPoint ver2 = CanvasPoint(rand()%256, rand()%256);
	CanvasTriangle allV(ver0, ver1, ver2); // calls CanvasTriangle struct on vertices

	int red = rand()%256; // randomizes colour values
	int green = rand()%256;
	int blue = rand()%256;

	Colour colour(red, green, blue);
	fillMapper(allV, colour, window);
}



void loadObj (std::string filepath, float scale) {
	std::ifstream file(filepath, std::ifstream::in);
	std::string line;
	
	std::vector<glm::vec3> vertices;

	std::vector<TexturePoint> texturePoints;

	while (!file.eof()) {
		std::getline(file, line);
		std::vector<std::string> tokens = split(line, ' ');

		std::cout << line;
		if (line.substr(0,2) == "v ") {
			std::istringstream s(line.substr(2));
      glm::vec3 v; s >> v.x; s >> v.y; s >> v.z;

			vertices.push_back(v);
		}
		else if (line.substr(0,2) == "f ") {
			std::istringstream s(line.substr(2));
			glm::vec3 f; s >> f.x; s >> f.y; s >> f.z;
		}
	}
			
}


void draw(DrawingWindow &window) {
  glm::vec3 topLeft(255, 0, 0); // size of window
  glm::vec3 topRight(0, 0, 255);       
  glm::vec3 bottomRight(0, 255, 0);    
  glm::vec3 bottomLeft(255, 255, 0);   
	
	std::vector<glm::vec3> left = interpolateThreeElementValues(topLeft, bottomLeft, window.height);
	std::vector<glm::vec3> right = interpolateThreeElementValues(topRight, bottomRight, window.height);

	window.clearPixels();
	for (size_t y = 0; y < window.height; y++) {
		std::vector<glm::vec3> row = interpolateThreeElementValues(left[y], right[y], window.width);
		for (size_t x = 0; x < window.width; x++) {

			uint32_t colour = (255 << 24) + (int(row[x].r) << 16) + (int(row[x].g) << 8) + int(row[x].b);
			window.setPixelColour(x, y, colour);
		}
	}
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
		else if (event.key.keysym.sym == SDLK_u) keyPressU(window);
		else if (event.key.keysym.sym == SDLK_f) keyPressF(window);
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;

	/*
	std::vector<float> result;
  result = interpolateSingleFloats(2.2, 8.5, 7);
  for(size_t i=0; i<result.size(); i++) std::cout << result[i] << " ";
  std::cout << std::endl;
	*/

 /*
  std::vector<glm::vec3> result;
	glm::vec3 from(1.0, 4.0, 9.2);
	glm::vec3 to(4.0, 1.0, 9.8);

	for(size_t i=0; i<result.size(); i++) std::cout << result[i].x << " ";
  std::cout << std::endl;
	for(size_t i=0; i<result.size(); i++) std::cout << result[i].y << " ";
  std::cout << std::endl;
	for(size_t i=0; i<result.size(); i++) std::cout << result[i].x << " ";
  std::cout << std::endl;
*/
CanvasPoint v0;
v0.x = 160;
v0.y = 10;
v0.texturePoint.x = 195;
v0.texturePoint.y = 5;

CanvasPoint v1;
v1.x = 300;
v1.y = 230;
v1.texturePoint.x = 395;
v1.texturePoint.y = 380;

CanvasPoint v2;
v2.x = 10;
v2.y = 150;
v2.texturePoint.x = 65;
v2.texturePoint.y = 330;

CanvasTriangle triangle(v0,v1,v2);

Colour colour;
colour.red = 255;
colour.green = 255;
colour.blue = 255;
	
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		//draw(window);
		//fillMapper(triangle, colour, window);
		textureMapper(triangle, colour, window);
		//loadObj("cornell-box.obj", 0.35);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}	

}
