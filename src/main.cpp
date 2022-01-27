#include <iostream>
#include <GLFW/glfw3.h>
#include "Math/matrix.h"
#include "Math/vec.h"

#define no_values 20000

MathLib::vec2 start, end;

struct Point {
	double x, y;
};

#define EPSILON 0.001

double bifurcation_value(double r, double xn) {
	double xnext = r * xn * (1 - xn);
	xn = xnext;
	for (int i = 0; i < 50; i++) {
		xnext = r * xn * (1 - xn);
		xn = xnext;
	}

	return xn;
}


void bifurcation(double *y_value, int width, int height) {
	for (int x = 0; x < no_values; x++) {
		float x_val = start.x + ((float)x / no_values) * (end.x - start.x);

		y_value[x] = bifurcation_value(x_val, 0.4);
	}

}

inline float MapRange(float from_x1, float from_x2, float to_x1, float to_x2, float x) {
	return (to_x2 - to_x1) / (from_x2 - from_x1) * (x - from_x1) + to_x1;
}

void render_logistic_map(double* y_values, int width, int height) {
	glLoadIdentity();
	glOrtho(0, width, 0, height, -1, 1);
	glBegin(GL_POINTS);

	for (int x = 0; x < no_values; x++) {
		const float x_val = start.x + ((float)x / no_values) * (end.x - start.x);
		glColor3f(0.3, 0.5, 0.5);

		double y_value = y_values[x];
		glVertex2f(MapRange(start.x, end.x, 0, (float)width, x_val), MapRange(start.y, end.y, 0, height, y_value));
		for(int i = 0; i < 50; i++){
			y_value = x_val * y_value * (1 - y_value); 
			glVertex2f(MapRange(start.x, end.x, 0, (float)width, x_val), MapRange(start.y, end.y, 0, height, y_value));
		}
	}
	glEnd();
}
//screen parameters
typedef struct Screen {
	double* y_value;
	int width, height;
}Screen;


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	Screen* screen = (Screen*)glfwGetWindowUserPointer(window);
	glfwGetFramebufferSize(window, &screen->width, &screen->height);
	bifurcation(screen->y_value, width, height);
	glfwSetWindowUserPointer(window, screen);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	double xCoord, yCoord;
	glfwGetCursorPos(window, &xCoord, &yCoord);
	float zoomSize = 2.0f;
	Screen* screen = (Screen*)glfwGetWindowUserPointer(window);

	float cx = MapRange(0, (float)screen->width, start.x, end.x, (float)xCoord);
	float cy = MapRange(0, (float)screen->height, start.y, end.y, screen->width - (float)yCoord);

	start.x -= cx;
	end.x -= cx;
	start.y -= cy;
	end.y -= cy;

	float factor = yoffset > 0 ? 0.9f : 1.1f;

	start.x *= factor;
	end.x *= factor;
	start.y *= factor;
	end.y *= factor;

	start.x += cx;
	end.x += cx;
	start.y += cy;
	end.y += cy;

	bifurcation(screen->y_value, screen->width, screen->height);
}

void processKeyboardInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, 1);
	}
	Screen* context = (Screen *)glfwGetWindowUserPointer(window);


	float divideFactor = 50.0f;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		float moveFactor = (end.y - start.y) / divideFactor;
		start.y += moveFactor;
		end.y += moveFactor;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		float moveFactor = (end.x - start.x) / divideFactor;
		start.x -= moveFactor;
		end.x -= moveFactor;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		float moveFactor = (end.y - start.y) / divideFactor;
		start.y -= moveFactor;
		end.y -= moveFactor;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		float moveFactor = (end.x - start.x) / divideFactor;
		start.x += moveFactor;
		end.x += moveFactor;
	}
	bifurcation(context->y_value, context->width, context->height);
	glfwSetWindowUserPointer(window, context);
}


int main() {
	Screen screen;

	glfwInit();

	const int width = 1920, height = 1400;
	screen.width = width;
	screen.height = height;
	float aspectRatio = float(width) / float(height);
	GLFWwindow* window = glfwCreateWindow(width, height, "Bifurcation", NULL, NULL);

	glfwSetWindowUserPointer(window, &screen);
	glfwSetScrollCallback(window, scroll_callback);

	//shader set width height
	MathLib::vec2 rectMin(2.4, 0);
	MathLib::vec2 rectMax(5, 1.0);

	start = rectMin;
	end = rectMax;

	glfwMakeContextCurrent(window);
	glViewport(0, 0, width, height);

	double* y_values = (double *)malloc(sizeof(double) * no_values);
	screen.y_value = y_values;
	bifurcation(y_values, width, height);
	glfwSetWindowUserPointer(window, &screen);


	while (!glfwWindowShouldClose(window)){
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		render_logistic_map(y_values, screen.width, screen.height);

		processKeyboardInput(window);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}
