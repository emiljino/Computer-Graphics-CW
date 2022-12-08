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
#include "../libs/sdw/TexturePoint.h"
#include <sstream>
#include <unordered_map>
#include <string>
#include "../libs/sdw//RayTriangleIntersection.h"


#define WIDTH 320
#define HEIGHT 240
#define pi 3.14159265358979323846
enum RenderMode{ WIREFRAME, RASTERIZED, RAYTRACE};
RenderMode renderMode = RASTERIZED;
//#define LIGHT 10

std::vector<std::vector<float> > depthBuffer;
glm::vec3 camPosition(0.0, 0.0, 4.0);
glm::vec3 lightSource(0.0, 0.5, 0.5);
glm::mat3 camRotation = glm::mat3(
				1.0, 0.0, 0.0, // first column (not row!)
				0.0, 1.0, 0.0, // second column
				0.0, 0.0, 1.0 // third column
			);
			
glm::mat3 camOrientation = glm::mat3(
				1.0, 0.0, 0.0, // first column (not row!)
				0.0, 1.0, 0.0, // second column
				0.0, 0.0, 1.0 // third column
			);
float focalLength = 2.0;
bool orbitMode = false;

int rasterizeoutput =  000071;



std::vector<float> interpolateSingleFloats(float from, float to, float numberOfValues) {
	std::vector<float> points; //variable single of type float
  float step = (to-from) / float(numberOfValues-1); //step is dividing distance of two pixels by number of values
  for(int i=0; i<numberOfValues; i++) { //iterating through number of values 
		points.push_back(from + i*step);  // push_back val to end of vector
    }
  return points;
}

std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, float numberOfValues) {
	std::vector<glm::vec3> points; // declaring variable
	glm::vec3 step = (to-from) / float(numberOfValues-1); // step is dividing distance of two pixels by number of values
	for(float i=0; i<numberOfValues; i++) {
		points.push_back(from + i*step);
	}
	return points;
}

//takes in r,g,b and makes unsigned int of colours and returns that colour
uint32_t lineColour (int red, int green, int blue) {
	uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
	return colour;
}



void drawLine(CanvasPoint from, CanvasPoint to, Colour colour, DrawingWindow &window) {
	float xDiff = to.x - from.x; // distance from start to end
	float yDiff = to.y - from.y;
  float depthDiff = to.depth - from.depth; // depth difference

	float numberOfSteps = std::max(abs(xDiff), abs(yDiff)); // max of distance

	float xStepSize = xDiff/numberOfSteps; 
	float yStepSize = yDiff/numberOfSteps;
	float depthStepSize = depthDiff/numberOfSteps; // number of steps

	//uint32_t c = (255 << 24) + (int(colour.red) << 16) + (int(colour.green) << 8) + int(colour.blue);


	for (float i = 0.0; i < numberOfSteps+1; i++) {
		float x = from.x + (xStepSize * i);
		float y = from.y + (yStepSize * i);
		float zDepth = from.depth + (depthStepSize * i);
    //std::cout << zDepth;
		if (x <= 0 || x > WIDTH-1 || y <= 0 || y > HEIGHT-1) {
			continue;
		}
    // if inverse of depth is bigger than buffer, add it to buffer
		if ((-1 / zDepth) >= depthBuffer[x][y]) {

			//std::cout << zDepth << std::endl;
      window.setPixelColour(int(x), int(y), lineColour(colour.red, colour.green, colour.blue));
			depthBuffer[int(x)][int(y)] = (-1 / zDepth);
		}
	//	window.setPixelColour(int(x), int(y), c);
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
		float x = round(from.x + (xStepSize*i));
		float y = round(from.y + (yStepSize*i));

		float xTex = round(texFrom.x + (xTexStepSize*i));
		float yTex = round(texFrom.y + (yTexStepSize*i));
		
		window.setPixelColour(int(x), int(y), texture.pixels[xTex + yTex * texture.width]);
	}
}

void drawTriangle (CanvasTriangle triangle, Colour colour, DrawingWindow &window) {
	drawLine(triangle.vertices[0], triangle.vertices[1], colour, window); // drawing line from vertice 0 to vertice 1
	drawLine(triangle.vertices[1], triangle.vertices[2], colour, window);
	drawLine(triangle.vertices[2], triangle.vertices[0], colour, window); 
}

/*
CanvasPoint find_mid(CanvasPoint top, CanvasPoint mid, CanvasPoint bot) {
	float midX = top.x + ((mid.y - top.y) / (bot.y - top.y)) * (bot.x - top.x);
	return CanvasPoint(round(midX), mid.y);
}
*/

void fillHalfTriangle (CanvasPoint top, CanvasPoint bot, CanvasPoint mid, float height, Colour colour, DrawingWindow &window) {
	// interpolate between v0 and v1
	std::vector<float> topToMidX = interpolateSingleFloats(top.x, mid.x, height + 1);
	std::vector<float> topToMidY = interpolateSingleFloats(top.y, mid.y, height + 1);
	std::vector<float> topToMidDepth = interpolateSingleFloats(top.depth, mid.depth, height + 1); // interpolating between depth of top and middle points
	std::vector<float> topToBotX = interpolateSingleFloats(top.x, bot.x, height + 1);
	std::vector<float> topToBotY = interpolateSingleFloats(top.y, bot.y, height + 1);
	std::vector<float> topToBotDepth = interpolateSingleFloats(top.depth, bot.depth, height + 1);
	
  for (float i = 0; i < height; i++) {
		CanvasPoint from(topToMidX[i], topToMidY[i], topToMidDepth[i]);
		CanvasPoint to(topToBotX[i], topToBotY[i], topToBotDepth[i]);
	  drawLine(from, to, colour, window);
	}
}

void textureHalfTriangle (CanvasPoint top, CanvasPoint bot, CanvasPoint mid, float height, float textureH, TextureMap texture, DrawingWindow &window) {
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
	if (triangle.vertices[0].y > triangle.vertices[1].y) std::swap(triangle.vertices[0], triangle.vertices[1]);
	if (triangle.vertices[0].y > triangle.vertices[2].y) std::swap(triangle.vertices[0], triangle.vertices[2]);
	if (triangle.vertices[1].y > triangle.vertices[2].y) std::swap(triangle.vertices[1], triangle.vertices[2]);

	//if (triangle.vertices[1].x > triangle.vertices[2].x) std::swap(triangle.vertices[1], triangle.vertices[2]);

  // finding gradient of line from v0 to v2
	float gradient = ((triangle.vertices[2].y - triangle.vertices[0].y) / (triangle.vertices[2].x - triangle.vertices[0].x));
	//float zDepthDiff = triangle.vertices[0].depth - triangle.vertices[2].depth;
  // finding x value of extra point
	float midX = ((triangle.vertices[1].y - triangle.v0().y) / gradient) + triangle.vertices[0].x;
	// creating canvaspoint midpoint 
	CanvasPoint midPoint(midX, triangle.vertices[1].y);

	// depth of midPoint
	// finding ratio 
	float ratio = (triangle.vertices[0].y - midPoint.y) / (triangle.vertices[0].y - triangle.vertices[2].y);
  // finding depth of mid point
	midPoint.depth = triangle.vertices[0].depth - (ratio * (triangle.vertices[0].depth - triangle.vertices[2].depth));
	//std::cout << midPoint.depth;
	//std::cout << triangle.vertices[0].depth;
	
	//CanvasPoint midPoint = find_mid(triangle.vertices[0], triangle.vertices[1], triangle.vertices[2]);

  // splitting triangle into two by using extra point
	CanvasTriangle topTriangle(triangle.vertices[0], triangle.vertices[1], midPoint);
	CanvasTriangle bottomTriangle(midPoint, triangle.vertices[1], triangle.vertices[2]);
	

	// fill in top triangle area
	// height of triangle
	// calling func to fill top triangle
	drawTriangle(topTriangle, colour, window);
	float topHeight = midPoint.y - triangle.vertices[0].y;
  fillHalfTriangle(triangle.vertices[0], triangle.vertices[1], midPoint, topHeight, colour, window);

  // drawing line to fill missing point
	drawLine(midPoint, triangle.vertices[1], colour, window);

	// fill in bottom triangle area
	// height of triangle
	// calling func to fill bottom triangle
	drawTriangle(bottomTriangle, colour, window);
	float bottomHeight = triangle.vertices[2].y - midPoint.y;
  fillHalfTriangle(triangle.vertices[2], triangle.vertices[1], midPoint, bottomHeight, colour, window);
  /* // white border around triangles
	int red = 255;
	int green = 255;
	int blue = 255;

	Colour whiteBorder(red, green, blue);
	drawTriangle(triangle, whiteBorder, window);
	*/
}

void textureMapper(CanvasTriangle triangle, Colour colour, DrawingWindow &window) {
  
	TextureMap texture = TextureMap("texture.ppm");

	// sorting vertices in order of height
	if (triangle.vertices[0].y > triangle.vertices[1].y) std::swap(triangle.vertices[0], triangle.vertices[1]);
	if (triangle.vertices[0].y > triangle.vertices[2].y) std::swap(triangle.vertices[0], triangle.vertices[2]);
	if (triangle.vertices[1].y > triangle.vertices[2].y) std::swap(triangle.vertices[1], triangle.vertices[2]);

	//if (triangle.vertices[1].x > triangle.vertices[2].x) std::swap(triangle.vertices[1], triangle.vertices[2]);
  // finding gradient of line from v0 to v2
	float gradient = ((triangle.v2().y - triangle.v0().y) / (triangle.v2().x - triangle.v0().x));
  // finding value of x on line v0 to v2
	float midX = ((triangle.v1().y - triangle.v0().y) / gradient) + triangle.v0().x;
	// co-ordinates of new x point on line v0 to v2
	CanvasPoint midPoint(midX, triangle.v1().y);

  //CanvasPoint midPoint = find_mid(triangle.vertices[0], triangle.vertices[1], triangle.vertices[2]);
  
	drawTriangle(triangle, colour, window);

	// finding midpoint of texture using ratio of triangles
	// ratio from top to midpoint
	float topToMidX = triangle.vertices[0].x - midPoint.x;
	float topToMidY = triangle.vertices[0].y - midPoint.y;

	// ratio from top to bottom
	float topToBotX = triangle.vertices[0].x - triangle.vertices[2].x;
	float topToBotY = triangle.vertices[0].y - triangle.vertices[2].y;

	float topToMidRatioX = topToMidX/topToBotX;
	float topToMidRatioY = topToMidY/topToBotY;

	// ratio from top to bottom of x and y co-ordinates for texture
	float texTopToBotX = triangle.vertices[0].texturePoint.x - triangle.vertices[2].texturePoint.x;
	float texTopToBotY = triangle.vertices[0].texturePoint.y - triangle.vertices[2].texturePoint.y;

	float texTopToMidX = topToMidRatioX * texTopToBotX;
	float texTopToMidY = topToMidRatioY * texTopToBotY;
  
  // co-ordinates of texture middle point
	midPoint.texturePoint.x = triangle.vertices[0].texturePoint.x - texTopToMidX;
	midPoint.texturePoint.y = triangle.vertices[0].texturePoint.y - texTopToMidY;

	// filling top triangle
	float topHeight = midPoint.y - triangle.vertices[0].y;
	float texTopHeight = midPoint.texturePoint.y - triangle.vertices[0].texturePoint.y;
	textureHalfTriangle(triangle.vertices[0], triangle.vertices[1], midPoint, topHeight, texTopHeight, texture, window);

	drawTexLine(midPoint, triangle.vertices[1], midPoint.texturePoint, triangle.vertices[1].texturePoint, texture, window);

	// filling bottom triangle
	float botHeight = triangle.vertices[2].y - midPoint.y ;
	float texBotHeight = triangle.vertices[2].texturePoint.y - midPoint.texturePoint.y ;
	textureHalfTriangle(triangle.vertices[2], triangle.vertices[1], midPoint, botHeight, texBotHeight, texture, window);

}

/* std::unordered_map<std::string, Colour> loadMtl(std::string mtlFilepath) {
  // object file patch
	std::ifstream mtlFile(mtlFilepath);
	std::string mtlLine;
  // declaring hashmap of colours
	std::unordered_map<std::string, Colour> colours;
	std::string colour_name;


  // loop to parse mtl file
	while (getline(mtlFile, mtlLine)) {
	
		std::vector<std::string> token = split(mtlLine, ' ');

	  // checking for newmtl 
		if (token[0] == "newmtl") {
			colour_name = token[1];
		}
		if (token[0] == "Kd") {
			int r = 255 * stof(token[1]);
			int g = 255 * stof(token[2]);
			int b = 255 * stof(token[3]);

			colours[colour_name] = Colour(colour_name, r, g, b);
		}
  }
	mtlFile.close();
	return colours;
}
*/

std::unordered_map<std::string, Colour> loadMtl (std::string mtlFilepath) {
  // material file path
	std::ifstream mtlFile(mtlFilepath);
	std::string mtlLine;
	std::unordered_map<std::string, Colour> materials;
	std::string colour_name;
	std::string texturePath;


	while (getline(mtlFile, mtlLine)) {
	
		std::vector<std::string> token = split(mtlLine, ' ');

	  // checking for newmtl 
		if (token[0] == "newmtl") {
			colour_name = token[1];
		}
		if (token[0] == "Kd") {
			Colour colourVal(std::stof(token[1])*255, std::stof(token[2]) * 255, std::stof(token[3]) * 255);
			materials[colour_name] = colourVal;
    } 
		if (token[0] == "map_Kd") {
			Colour colourMap = materials[colour_name];
			colourMap.name = token[1];
			materials[colour_name] = colourMap;
		}
  }
	mtlFile.close();
	return materials;
}


std::vector<ModelTriangle> loadObj (std::string objFilepath, float scale) {
	std::unordered_map<std::string, Colour> materials = loadMtl("textured-cornell-box.mtl.obj");
  // object file path
	std::ifstream objFile(objFilepath);
	std::string objLine;
  // declaring variables for v and f
	std::vector<glm::vec3> vertices;
	std::vector<ModelTriangle> triangles;
	std::vector<TexturePoint> texture_points;
	std::string colour_name;
	
  // loop to parse obj file
	while (getline(objFile, objLine)) {

		std::vector<std::string> token = split(objLine, ' ');
		// checking for colour value of obj file
		if (token[0] == "usemtl") {
			colour_name = token[1];
		}
		// checking for v and pushing values into vertices
		if (token[0] == "v") {
			vertices.push_back(glm::vec3((stof(token[1]))*scale, (stof(token[2]))*scale, (stof(token[3]))*scale));
		}
		if (token[0] == "vt") {
			texture_points.push_back(TexturePoint(stof(token[1]), stof(token[2])));
		}
		// checking for line starting with f and pushing values triangles 
		else if (token[0] == "f") {
			std::vector<std::string> l1 = split(token[1], '/');
			std::vector<std::string> l2 = split(token[2], '/');
			std::vector<std::string> l3 = split(token[3], '/');

			ModelTriangle triangle(vertices[stoi(l1[0]) - 1], vertices[stoi(l2[0]) - 1], vertices[stoi(l3[0]) - 1], materials[colour_name]);
			triangle.normal = glm::cross(glm::vec3(vertices[stoi(l2[0]) - 1] - vertices[stoi(l1[0]) - 1]), glm::vec3(vertices[stoi(l3[0]) - 1] - vertices[stoi(l1[0]) - 1]));

      if(!texture_points.empty() && l1[1] != "") {
				triangle.texturePoints[0] = texture_points[stoi(l1[1]) - 1];
			  triangle.texturePoints[1] = texture_points[stoi(l2[1]) - 1];
			  triangle.texturePoints[2] = texture_points[stoi(l3[1]) - 1];
			}
			// std::cout << triangle.texturePoints[0];
			// std::cout << triangle.texturePoints[1];
			// std::cout << triangle.texturePoints[2];

			triangles.push_back(triangle);
    }
	}
	objFile.close();
	return triangles;
}


std::vector<ModelTriangle> loadSphereObj (std::string objFilepath, float scale) {
	//std::unordered_map<std::string, Colour> materials = loadMtl("cornell-box.mtl.obj");
  // object file path
	std::ifstream objFile(objFilepath);
	std::string objLine;
  // declaring variables for v and f
	std::vector<glm::vec3> vertices;
	std::vector<ModelTriangle> triangles;
	//std::vector<TexturePoint> texture_points;
	//std::string colour_name;
	
  // loop to parse obj file
	while (getline(objFile, objLine)) {

		std::vector<std::string> token = split(objLine, ' ');
		// checking for colour value of obj file
		/* if (token[0] == "usemtl") {
			colour_name = token[1];
		} */
		// checking for v and pushing values into vertices
		if (token[0] == "v") {
			vertices.push_back(glm::vec3((stof(token[1]))*scale, (stof(token[2]))*scale, (stof(token[3]))*scale));
		}
		/* if (token[0] == "vt") {
			texture_points.push_back(TexturePoint(stof(token[1]), stof(token[2])));
		} */
		// checking for line starting with f and pushing values triangles 
		else if (token[0] == "f") {
			std::vector<std::string> l1 = split(token[1], '/');
			std::vector<std::string> l2 = split(token[2], '/');
			std::vector<std::string> l3 = split(token[3], '/');

			Colour colour(255, 0, 0);
			ModelTriangle triangle(vertices[stoi(l1[0]) - 1], vertices[stoi(l2[0]) - 1], vertices[stoi(l3[0]) - 1], colour);
			triangle.normal = glm::cross(glm::vec3(vertices[stoi(l2[0]) - 1] - vertices[stoi(l1[0]) - 1]), glm::vec3(vertices[stoi(l3[0]) - 1] - vertices[stoi(l1[0]) - 1]));

			triangles.push_back(triangle);
    }
	}
	objFile.close();
	return triangles;
}




/*
std::vector<ModelTriangle> loadObjAndMtl (std::string mtlFilepath, std::string objFilepath, float scale) {
  // material file path
	std::ifstream mtlFile(mtlFilepath);
	std::string mtlLine;
  // object file path
	std::ifstream objFile(objFilepath);
	std::string objLine;
  // declaring variables for v and f
	std::vector<glm::vec3> vertices;
	std::vector<ModelTriangle> triangles;
	std::vector<TexturePoint> texturePoints;
	
	std::unordered_map<std::string, Colour> colours;
	std::string colour_name;
	std::string texture_path;
	
	
	while (getline(mtlFile, mtlLine)) {
	
		std::vector<std::string> token = split(mtlLine, ' ');

	  // checking for newmtl 
		if (token[0] == "newmtl") {
			colour_name = token[1];
		}
		if (token[0] == "Kd") {
			Colour colourVal(std::stof(token[1])*255, std::stof(token[2]) * 255, std::stof(token[3]) * 255);
			colours[colour_name] = colourVal;
		}
  }
	mtlFile.close();
	
	
  // loop to parse obj file
	while (getline(objFile, objLine)) {

		std::vector<std::string> token = split(objLine, ' ');
	  
		// checking for colour value of obj file
		if (token[0] == "usemtl") {
			colour_name = token[1];
		}
		// checking for v and pushing values into vertices
		else if (token[0] == "v") {
			vertices.push_back(glm::vec3((stof(token[1]))*scale, (stof(token[2]))*scale, (stof(token[3]))*scale));
		}
		// checking for line starting with f and pushing values triangles 
		else if (token[0] == "f") {
			triangles.push_back(ModelTriangle(vertices[stoi(token[1]) - 1], vertices[stoi(token[2]) - 1], vertices[stoi(token[3]) - 1], colours[colour_name]));
    }
	
	}

	objFile.close();
	return triangles;
}
*/



CanvasPoint getCanvasIntersectionPoint (glm::vec3 camPosition, glm::vec3 vertexPosition, float focalLength) {

glm::vec3 camToVertex = (vertexPosition - camPosition);
glm::vec3 adjustedVertex = camToVertex * camOrientation;

float u = (-180 * (focalLength * (adjustedVertex.x)/(adjustedVertex.z))) + (WIDTH/2);
float v = (180 * (focalLength * (adjustedVertex.y)/(adjustedVertex.z))) + (HEIGHT/2);

return CanvasPoint (u , v, adjustedVertex.z);
}


void pointCloudRender (std::vector<ModelTriangle> triangle, glm::vec3 camPosition, float focalLength, DrawingWindow &window ) {

int r = 255;
int g = 255;
int b = 255;

uint32_t colour = (255 << 24) + (int(r) << 16) + (int(g) << 8) + int(b);

for (float i = 0; i < triangle.size(); i++) {

	CanvasPoint v0 = getCanvasIntersectionPoint(camPosition, triangle[i].vertices[0], focalLength);
	CanvasPoint v1 = getCanvasIntersectionPoint(camPosition, triangle[i].vertices[1], focalLength);
	CanvasPoint v2 = getCanvasIntersectionPoint(camPosition, triangle[i].vertices[2], focalLength);

	if ((v0.x >= 0 && v0.x < WIDTH) && (v0.y >= 0 && v0.y < HEIGHT)) {
		window.setPixelColour(v0.x, v0.y, colour);
	}

	if ((v1.x >= 0 && v1.x < WIDTH) && (v1.y >= 0 && v1.y < HEIGHT)) {
		window.setPixelColour(v1.x, v1.y, colour);
	}

	if ((v2.x >= 0 && v2.x < WIDTH) && (v2.y >= 0 && v2.y < HEIGHT)) {
		window.setPixelColour(v2.x, v2.y, colour);
	}
}
}

void wireFrameRender (std::vector<ModelTriangle> triangle, glm::vec3 camPosition, float focalLength, DrawingWindow &window) {
	window.clearPixels();
	depthBuffer.resize(WIDTH);
  for(int x = 0; x < WIDTH; x++) {
	  depthBuffer[x].resize(HEIGHT);
	  for(int y = 0; y < HEIGHT; y++) {
		  depthBuffer[x][y] = 0.0;
	  }
  }
  int r = 255;
  int g = 255;
  int b = 255;

  Colour triangleColour(r,g,b);

	for (float i = 0; i < triangle.size(); i++) {

	CanvasPoint v0 = getCanvasIntersectionPoint(camPosition, triangle[i].vertices[0], focalLength);
	CanvasPoint v1 = getCanvasIntersectionPoint(camPosition, triangle[i].vertices[1], focalLength);
	CanvasPoint v2 = getCanvasIntersectionPoint(camPosition, triangle[i].vertices[2], focalLength);

	CanvasTriangle t(v0, v1, v2);

	drawTriangle(t, triangleColour, window);
	}
}


void rasterizeRender (std::vector<ModelTriangle> triangle, glm::vec3 camPosition, float focalLength, DrawingWindow &window) {

	window.clearPixels();
	depthBuffer.resize(WIDTH);
  for(int x = 0; x < WIDTH; x++) {
	  depthBuffer[x].resize(HEIGHT);
	  for(int y = 0; y < HEIGHT; y++) {
		  depthBuffer[x][y] = 0.0;
	  }
  }

	for (float i = 0; i < triangle.size(); i++) {
		CanvasPoint v0 = getCanvasIntersectionPoint(camPosition, triangle[i].vertices[0], focalLength);
	  CanvasPoint v1 = getCanvasIntersectionPoint(camPosition, triangle[i].vertices[1], focalLength);
	  CanvasPoint v2 = getCanvasIntersectionPoint(camPosition, triangle[i].vertices[2], focalLength);

	  CanvasTriangle t(v0, v1, v2);

	  int r = triangle[i].colour.red;
    int g = triangle[i].colour.green;
    int b = triangle[i].colour.blue;

	  Colour colour(r,g,b);

	  fillMapper(t, colour, window);
	}
}

RayTriangleIntersection getClosestIntersection (glm::vec3 camPosition, glm::vec3 rayDirection, std::vector<ModelTriangle> triangle) {

	RayTriangleIntersection result;
	result.distanceFromCamera = INFINITY;
	// finding ray triangle intersection 
	for (int i = 0; i < triangle.size(); i++) {
		glm::vec3 e0 = triangle[i].vertices[1] - triangle[i].vertices[0];
    glm::vec3 e1 = triangle[i].vertices[2] - triangle[i].vertices[0];
    glm::vec3 SPVector = camPosition - triangle[i].vertices[0];
    glm::mat3 DEMatrix(-rayDirection, e0, e1);
    glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
		// t is absolute distance along ray from cam to intersection pt
		// u is proportional distance along the triangle's first edge that the intersection pt occurs
		// v is proportional distance along the triangle's second edge that the intersection pt occurs
		float t = possibleSolution[0]; 
		float u = possibleSolution[1]; 
		float v = possibleSolution[2];
    // validating the coordinates of any potential intersections before accepting it as a solution
		// t checks distance from cam to intersection pt is positive so we dont end up rendering triangles behind camera
		if ((t < result.distanceFromCamera) && (t > 0) && (u >= 0.0) && (u <= 1.0) && (v >= 0.0) && (v <= 1.0) && (u + v <= 1.0)) {
			result.distanceFromCamera = t;
			result.intersectedTriangle = triangle[i];
			result.triangleIndex = i;
			result.intersectionPoint = triangle[i].vertices[0] + u * e0 + v * e1;
		}
	}
	return result;
}


RayTriangleIntersection getClosestShadowIntersection (glm::vec3 camPosition, glm::vec3 rayDirection, std::vector<ModelTriangle> triangle, int triangleIndex) {

	RayTriangleIntersection result;
	result.distanceFromCamera = INFINITY;
	//float distance = glm::length(rayDirection);
	// finding ray triangle intersection 
	for (int i = 0; i < triangle.size(); i++) {
		glm::vec3 e0 = triangle[i].vertices[1] - triangle[i].vertices[0];
    glm::vec3 e1 = triangle[i].vertices[2] - triangle[i].vertices[0];
    glm::vec3 SPVector = camPosition - triangle[i].vertices[0];
    glm::mat3 DEMatrix(-rayDirection, e0, e1);
    glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
		// t is absolute distance along ray from cam to intersection pt
		// u is proportional distance along the triangle's first edge that the intersection pt occurs
		// v is proportional distance along the triangle's second edge that the intersection pt occurs
		float t = possibleSolution[0]; 
		float u = possibleSolution[1]; 
		float v = possibleSolution[2];
    // validating the coordinates of any potential intersections before accepting it as a solution
		// t checks distance from cam to intersection pt is positive so we dont end up rendering triangles behind camera
		if ((t < result.distanceFromCamera) && (t > 0) && (u >= 0.0) && (u <= 1.0) && (v >= 0.0) && (v <= 1.0) && (u + v <= 1.0) && (i != triangleIndex)) {
			result.distanceFromCamera = t;
			result.intersectedTriangle = triangle[i];
			result.triangleIndex = i;
			result.intersectionPoint = triangle[i].vertices[0] + u * e0 + v * e1;
		}
	}
	return result;
}


void lookAt(glm::vec3 to){
	glm::vec3 up(0, 1, 0);
	glm::vec3 forward = glm::normalize(to - camPosition );
	glm::vec3 right = glm::normalize(glm::cross(up, -forward));
	glm::vec3 newUp = glm::normalize(glm::cross(-forward, right));
	camOrientation = glm::mat3(right, newUp, -forward);
}

void orbit() {
	glm::vec3 origin(0.0, 0.0, 0.0);
	glm::mat3 camRotation = glm::mat3(
				cos(0.005), 0.0, sin(0.005), // first column (not row!)
				0.0, 1.0, 0.0, // second columns
				-sin(0.005), 0.0, cos(0.005) // third coloumn
			);
			// speed changed by changing from 0.0174533 to 0.005
			camPosition = camRotation * camPosition;
			lookAt(origin);
}



void drawRayTrace(DrawingWindow &window, std::vector<ModelTriangle> triangle) {
	// clearning window everytime drawRayTrace is called
	window.clearPixels();

  // resetting buffer evertime drawRayTrace is called
	depthBuffer.resize(WIDTH);
  for(int x = 0; x < WIDTH; x++) {
	  depthBuffer[x].resize(HEIGHT);
	  for(int y = 0; y < HEIGHT; y++) {
		  depthBuffer[x][y] = 0.0;
	  }
  }

  // iterating through top to bottom and left to right of window
	for (float i = 0; i < window.width; i++) {
		for (float j = 0; j < window.height; j++) {

      // 2D canvas position
			CanvasPoint twoDPoint = {i,j};

      //float u = (-180 * (focalLength * (adjustedVertex.x)/(adjustedVertex.z))) + (WIDTH/2);
      
			// reversing getCanvasIntersectionPoint operation to get 3D direction
      float u = (((i - WIDTH/2) / 180) / focalLength);
			float v = (-((j - HEIGHT/2) / 180) / focalLength);
			float z = -1;

			glm::vec3 imagePoint(u, v, z);
      glm::vec3 rayDirection = glm::normalize(imagePoint);
			RayTriangleIntersection result = getClosestIntersection(camPosition, rayDirection, triangle);
      
			// finding where shadows are based of lightsource hitting boxes
      glm::vec3 shadowRay = lightSource - result.intersectionPoint;
			// normalized so has direction but magnitude is 1
			glm::vec3 shadowRayDir = glm::normalize(shadowRay);
			RayTriangleIntersection shadowResult = getClosestShadowIntersection(result.intersectionPoint, shadowRayDir, triangle, result.triangleIndex);
			glm::vec3 camDir = glm::normalize(camPosition - result.intersectionPoint);
			glm::vec3 normal = result.intersectedTriangle.normal;
			// ambient lighting
			// proximity lighting
			float proximity = (5 / (2 * pi * pow(glm::length(shadowRay), 2)));

			// angle of incidences
			float AOI = pow(glm::clamp(glm::dot(shadowRayDir, normal), 0.0f, 1.0f), 0.1);
		  //glm::vec3 normalAOI(2 * normal.x * AOI, 2 * normal.y * AOI, 2 * normal.z * AOI);
			// vector of reflection
			glm::vec3 Rr = shadowRayDir - (2.0f * (normal) * glm::dot(shadowRayDir, normal));
			float spec = (glm::dot(glm::normalize(Rr), camDir));
		  float specular = float(glm::pow(spec, 256));
			// std::cout << specular;
			// std::cout << AOI;

      // colours of intersecting points
			float red = glm::clamp((result.intersectedTriangle.colour.red * (glm::clamp(float((proximity * AOI) + specular), 0.1f, 1.0f))), 0.0f, 255.0f);
			float green = glm::clamp((result.intersectedTriangle.colour.green * (glm::clamp(float((proximity * AOI) + specular), 0.1f, 1.0f))), 0.0f, 255.0f);
			float blue = glm::clamp((result.intersectedTriangle.colour.blue * (glm::clamp(float((proximity * AOI) + specular), 0.1f, 1.0f))), 0.0f, 255.0f);

			float shadowR = result.intersectedTriangle.colour.red;
			float shadowG = result.intersectedTriangle.colour.green;
			float shadowB = result.intersectedTriangle.colour.blue;

			if (shadowResult.distanceFromCamera < glm::length(shadowRay)) {
				/*if (red > 255) {
						red = 255;
					}
					if (green > 255) {
						green = 255;
					}
					if (blue > 255) {
						blue = 255;
					}
					*/
				window.setPixelColour(i,j,lineColour(shadowR * 0.2f, shadowG * 0.2f, shadowB * 0.2f));
			}
			else {
				/* if (red > 255) {
						red = 255;
					}
					if (green > 255) {
						green = 255;
					}
					if (blue > 255) {
						blue = 255;
					}*/
				window.setPixelColour(i,j,lineColour(red, green, blue));
			}
		}
	}
}



void drawRasterised(DrawingWindow &window) {
	// clearing frame everytime drawRasterised is called
  window.clearPixels();
 /* glm::vec3 topLeft(255, 0, 0); // size of window
  glm::vec3 topRight(0, 0, 255);       
  glm::vec3 bottomRight(0, 255, 0);    
  glm::vec3 bottomLeft(255, 255, 0);   
	
	std::vector<glm::vec3> left = interpolateThreeElementValues(topLeft, bottomLeft, window.height);
	std::vector<glm::vec3> right = interpolateThreeElementValues(topRight, bottomRight, window.height);
*/

// resetting depth buffer everytime drawRasterised is called

  depthBuffer.resize(WIDTH);
  for(int x = 0; x < WIDTH; x++) {
	  depthBuffer[x].resize(HEIGHT);
	  for(int y = 0; y < HEIGHT; y++) {
		  depthBuffer[x][y] = 0.0;
	  }
  }

  // when y is pressed orbitMode set to true and orbit func called
	if (orbitMode) {
		orbit();
	}
  // load in obj and mtl files and calls rasterizeRender to draw it
  std::vector<ModelTriangle> loadCornell = loadObj("cornell-box.obj", 0.35);
	  std::vector<ModelTriangle> loadSphere = loadSphereObj("sphere.obj", 0.35);

	rasterizeRender(loadCornell, camPosition, focalLength, window);

	//change output thiny
	


	/*for (size_t y = 0; y < window.height; y++) {
		//std::vector<glm::vec3> row = interpolateThreeElementValues(left[y], right[y], window.width);
		for (size_t x = 0; x < window.width; x++) {
			float red = rand()%256;
			float green = 0;
			float blue = 0;
    
			uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
			window.setPixelColour(x, y, colour);
		}
	}
	*/
}


glm::vec3 getBaryCoord(glm::vec3 point, ModelTriangle triangle, ModelTriangle vecnorms) {
	glm::vec3 bary;
	float areaABC = glm::dot(vecnorms.vertices[0], glm::cross((triangle.vertices[1] - triangle.vertices[0]), (triangle.vertices[2] - triangle.vertices[0])));
	float areaPBC = glm::dot(vecnorms.vertices[0], glm::cross((triangle.vertices[1] - point), (triangle.vertices[2] - point)));
	float areaPCA = glm::dot(vecnorms.vertices[1], glm::cross((triangle.vertices[2] - point), (triangle.vertices[0] - point)));

	bary.x = areaPBC / areaABC; // alpha
	bary.y = areaPCA / areaABC; // beta
	bary.z = 1.0f - bary.x - bary.y; // gamma

	return bary;
}



void draw(DrawingWindow &window, std::vector<ModelTriangle> triangle) {
	window.clearPixels();

	depthBuffer.resize(WIDTH);
  for(int x = 0; x < WIDTH; x++) {
	  depthBuffer[x].resize(HEIGHT);
	  for(int y = 0; y < HEIGHT; y++) {
		  depthBuffer[x][y] = 0.0;
	  }
  }


	switch (renderMode) 
	{
	case WIREFRAME:

		wireFrameRender(triangle, camPosition, focalLength, window);
		break;

	case RASTERIZED:
		drawRasterised(window);
		break;

	case RAYTRACE:
		drawRayTrace(window, triangle);
		break;
	}
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

void keyPressO (DrawingWindow &window) {
	std::vector<ModelTriangle> loadCornell = loadObj("cornell-box.obj", 0.35);
	for (int i = 0; i < loadCornell.size(); i++) {
		std::cout << loadCornell[i];
	}
}


void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) {
			camPosition.x = camPosition.x + 0.1;
		}
		else if (event.key.keysym.sym == SDLK_RIGHT) {
			camPosition.x = camPosition.x - 0.1;
		}
		else if (event.key.keysym.sym == SDLK_UP) {
			camPosition.y = camPosition.y - 0.1;
		}
		else if (event.key.keysym.sym == SDLK_DOWN) {
			camPosition.y = camPosition.y + 0.1;
		}
		else if (event.key.keysym.sym == SDLK_z) {
			camPosition.z = camPosition.z + 0.1;
		}
		else if (event.key.keysym.sym == SDLK_x) {
			camPosition.z = camPosition.z - 0.1;
		}
		else if (event.key.keysym.sym == SDLK_w) {
			glm::mat3 camRotation = glm::mat3(
				1.0, 0.0, 0.0, // first column (not row!)
				0.0, cos(-0.0174533), -sin(-0.0174533), // second column
				0.0, sin(-0.0174533), cos(-0.0174533) // third coloumn
			);
			camPosition = camRotation * camPosition;
			camOrientation = camRotation * camOrientation;
		}
		else if (event.key.keysym.sym == SDLK_s) {
			glm::mat3 camRotation = glm::mat3(
				1.0, 0.0, 0.0, // first column (not row!)
				0.0, cos(0.0174533), -sin(0.0174533), // second column
				0.0, sin(0.0174533), cos(0.0174533) // third coloumn
			);
			camPosition = camRotation * camPosition;
			camOrientation = camRotation * camOrientation;
		}
		else if (event.key.keysym.sym == SDLK_a) {
			glm::mat3 camRotation = glm::mat3(
				cos(-0.001), 0.0, sin(-0.001), // first column (not row!)
				0.0, 1.0, 0.0, // second column
				-sin(-0.001), 0.0, cos(-0.001) // third coloumn
			);
			camPosition = camRotation * camPosition;
			camOrientation = camRotation * camOrientation;
		}
		else if (event.key.keysym.sym == SDLK_d) {
			glm::mat3 camRotation = glm::mat3(
				cos(0.0174533), 0.0, sin(0.0174533), // first column (not row!)
				0.0, 1.0, 0.0, // second column
				-sin(0.0174533), 0.0, cos(0.0174533) // third coloumn
			);
			camPosition = camRotation * camPosition;
			camOrientation = camRotation * camOrientation;
		}
		else if (event.key.keysym.sym == SDLK_q) {
			orbitMode = !orbitMode;
		}
		else if (event.key.keysym.sym == SDLK_u) keyPressU(window);
		else if (event.key.keysym.sym == SDLK_f) keyPressF(window);
		else if (event.key.keysym.sym == SDLK_o) keyPressO(window);
		else if (event.key.keysym.sym == SDLK_r) {
			/* window.clearPixels();
			depthBuffer.resize(WIDTH);
      for(float x = 0.0; x < WIDTH; x++) {
	      depthBuffer[x].resize(HEIGHT);
	      for(float y = 0.0; y < HEIGHT; y++) {
		      depthBuffer[x][y] = 0.0;
	      }
      }  
			drawRasterised(window); */
			renderMode = RASTERIZED;
		}
		else if (event.key.keysym.sym == SDLK_t) {
			/* window.clearPixels();
			depthBuffer.resize(WIDTH);
      for(float x = 0.0; x < WIDTH; x++) {
	      depthBuffer[x].resize(HEIGHT);
	      for(float y = 0.0; y < HEIGHT; y++) {
		      depthBuffer[x][y] = 0.0;
	      }
      }   
			std::vector<ModelTriangle> load = loadObj("cornell-box.obj", 0.35);
			drawRayTrace(window, load); */
			renderMode = RAYTRACE;
		}
		else if (event.key.keysym.sym == SDLK_y) {
			/* window.clearPixels();
			depthBuffer.resize(WIDTH);
      for(float x = 0.0; x < WIDTH; x++) {
	      depthBuffer[x].resize(HEIGHT);
	      for(float y = 0.0; y < HEIGHT; y++) {
		      depthBuffer[x][y] = 0.0;
	      }
      }   
			std::vector<ModelTriangle> load = loadObj("cornell-box.obj", 0.35);
			wireFrameRender(load, camPosition, focalLength, window);
			*/
		  renderMode = WIREFRAME;
		}
		else if (event.key.keysym.sym == SDLK_g) {
			lightSource.x = lightSource.x - 0.1;
		}
		else if (event.key.keysym.sym == SDLK_h) {
			lightSource.x = lightSource.x + 0.1;
		}
		else if (event.key.keysym.sym == SDLK_j) {
			lightSource.z = lightSource.z + 0.1;
		}
		else if (event.key.keysym.sym == SDLK_k) {
			lightSource.z = lightSource.z - 0.1;
		}
		else if (event.key.keysym.sym == SDLK_n) {
			lightSource.y = lightSource.y - 0.1;
		}
		else if (event.key.keysym.sym == SDLK_m) {
			lightSource.y = lightSource.y + 0.1;
		}
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		rasterizeoutput=rasterizeoutput+00001;
	std::string ext = ".ppm";

	std::string filename= std::to_string(rasterizeoutput) + ext;
	window.savePPM(filename);

	//	window.savePPM("outputrastorbi10.ppm");
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
// depthBuffer;

//std::vector<std::vector<float> > depthBuffer = {{0}};

/*
depthBuffer.resize(WIDTH);
for(int x = 0; x < WIDTH; x++) {
	depthBuffer[x].resize(HEIGHT);
	for(int y = 0; y < HEIGHT; y++) {
		depthBuffer[x][y] = 0.0;
	}
}
window.clearPixels();
*/

//depthBuffer.resize(WIDTH, float(HEIGHT, 0));
/*
float default_value = 0;
std::vector<std::vector<float>> depthBuffer(WIDTH, std::vector<float>(HEIGHT, default_value));
*/

depthBuffer.resize(WIDTH);
for(float x = 0.0; x < WIDTH; x++) {
	depthBuffer[x].resize(HEIGHT);
	for(float y = 0.0; y < HEIGHT; y++) {
		depthBuffer[x][y] = 0.0;
	}
}   

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

std::vector<ModelTriangle> loadCornell = loadObj("cornell-box.obj", 0.35);
std::vector<ModelTriangle> loadSphere = loadSphereObj("sphere.obj", 0.35);


Colour colour;
colour.red = 255;
colour.green = 255;
colour.blue = 255;


	
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		//drawRasterised(window);
		drawRayTrace(window, loadSphere);
		//draw(window, loadCornell);
		//fillMapper(triangle, colour, window);
	  //textureMapper(triangle, colour, window);
		//loadObj("cornell-box.obj", 0.35);
    //wireFrameRender(load, camPosition, focalLength, window);
		//rasterizeRender(load, camPosition, focalLength, window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}	

}
