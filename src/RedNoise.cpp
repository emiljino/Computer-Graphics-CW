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
uint32_t colourLine (int red, int green, int blue) {
	uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
	return colour;
}


void drawLine(CanvasPoint from, CanvasPoint to, Colour colour, DrawingWindow &window) {
  float fromX = from.x; // starting x value
	float fromY = from.y; // starting y value
	float toX = to.x;
	float toY = to.y;
	float xDiff = toX - fromX; // distance from start to end
	float yDiff = toY - fromY;
	
	float numberOfSteps = std::max(abs(xDiff), abs(yDiff)); // max of distance
	float xStepSize = xDiff/numberOfSteps; 
	float yStepSize = yDiff/numberOfSteps;

	for (float i = 0.0; i<numberOfSteps; i++) {
		float x = fromX + (xStepSize*i);
		float y = fromY + (yStepSize*i);
		window.setPixelColour(round(x), round(y), colourLine(colour.red, colour.green, colour.blue));
	}
}

void strokedTriangle (CanvasTriangle vertices, Colour colour, DrawingWindow &window) {
	drawLine(vertices.v0(), vertices.v1(), colour, window); // drawing line from vertice 0 to vertice 1
	drawLine(vertices.v1(), vertices.v2(), colour, window);
	drawLine(vertices.v2(), vertices.v0(), colour, window);

}

void fillTriangle (CanvasPoint v0, CanvasPoint xNewPoint, CanvasPoint v1, float height, Colour colour, DrawingWindow &window) {
	// interpolate between v0 and v1
	std::vector<float> v0midX = interpolateSingleFloats(v0.x, xNewPoint.x, height + 1);
	std::vector<float> v0midY = interpolateSingleFloats(v0.y, xNewPoint.y, height + 1);
	std::vector<float> v0v1x = interpolateSingleFloats(v0.x, v1.x, height + 1);
	std::vector<float> v0v1y = interpolateSingleFloats(v0.y, v1.y, height + 1);

  for (float i = 0; i < height; i++) {
		CanvasPoint from(v0midX[i], v0midY[i]);
		CanvasPoint to(v0v1x[i], v0v1y[i]);
		drawLine(from, to, colour, window);
	}
}


void fill (CanvasTriangle verticePoint, Colour colour, DrawingWindow &window) {
	// sorting vertices in order of height
	if (verticePoint.v0().y > verticePoint.v1().y) std::swap(verticePoint.v0().y, verticePoint.v1().y);
	if (verticePoint.v0().y > verticePoint.v2().y) std::swap(verticePoint.v0().y, verticePoint.v2().y);
	if (verticePoint.v1().y > verticePoint.v2().y) std::swap(verticePoint.v1().y, verticePoint.v2().y);
  // finding gradient of line from v0 to v2
	float gradient = ((verticePoint.v2().y - verticePoint.v0().y) / (verticePoint.v2().x - verticePoint.v0().x));
	// finding value of x on line v0 to v2
	float xNew = ((verticePoint.v1().y - verticePoint.v0().y) / gradient) + verticePoint.v0().x;
	// co-ordinates of x on line v0 to v2
	CanvasPoint xNewPoint(xNew, verticePoint.v1().y);
  // splitting triangle into two by using new x point
	CanvasTriangle topTriangle(verticePoint.v0(), xNewPoint, verticePoint.v1());
	CanvasTriangle bottomTriangle(xNewPoint, verticePoint.v1(), verticePoint.v2());
	
	// fill in top triangle area
	strokedTriangle(topTriangle, colour, window);
	// height of triangle
	float topHeight = xNewPoint.y - verticePoint.v0().y;
  // calling func to fill top triangle
  fillTriangle(verticePoint.v0(), xNewPoint, verticePoint.v1(), topHeight, colour, window);
	// fill in bottom triangle area
	strokedTriangle(bottomTriangle, colour, window);
	// height of triangle
	float bottomHeight = verticePoint.v2().y - xNewPoint.y;
  fillTriangle(verticePoint.v2(), xNewPoint, verticePoint.v1(), bottomHeight, colour, window);
  // white border around triangles
	int red = 255;
	int green = 255;
	int blue = 255;

	Colour whiteBorder(red, green, blue);
	strokedTriangle(verticePoint, whiteBorder, window);
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
	strokedTriangle(allV, colour, window);
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
	fill(allV, colour, window);
}

void textureMapper(DrawingWindow &window, CanvasTriangle vertices, std::string filename) {
TextureMap texture(filename)	;

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
v0.x = 0;
v0.y = 0;

CanvasPoint v1;
v1.x = 50;
v1.y = 100;

CanvasPoint v2;
v2.x = 150;
v2.y = 200;

CanvasTriangle verticePoint(v0,v1,v2);

Colour colour;
colour.red = 255;
colour.green = 255;
colour.blue = 0;

	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		//draw(window);
		fill(verticePoint, colour, window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}	

}
