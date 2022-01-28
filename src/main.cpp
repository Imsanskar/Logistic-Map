#include <iostream>
#include <GLFW/glfw3.h>
#include "Math/matrix.h"
#include "Math/vec.h"
#include <sys/types.h>
#include <immintrin.h>

#define no_values 40000
#define THREAD_NO_BIFURC_CALC 15
#define MULTI_THREAD
MathLib::vec2 start, end;

struct Point {
	double x, y;
};

#define EPSILON 0.001

double bifurcation_value(double r, double xn) {
	double xnext = r * xn * (1 - xn);
	xn = xnext;
	for (int i = 0; i < 100; i++) {
		xnext = r * xn * (1 - xn);
		xn = xnext;
	}

	return xn;
}

__m256 bifurcation_value_simd(__m256 r, __m256 xn){
	__m256 sub_term = _mm256_sub_ps(_mm256_set1_ps(1.0f), xn);
	__m256 xnext = _mm256_mul_ps(r, _mm256_mul_ps(xn, sub_term));
	xn = xnext;
	for (int i = 0; i < 200; i++) {
		sub_term = _mm256_sub_ps(_mm256_set1_ps(1.0f), xn);
		xnext = _mm256_mul_ps(r, _mm256_mul_ps(xn, sub_term));
		xn = xnext;
	}

	return xn;
}

struct BifurcateData{
	__m256* y_values;
	int start, end;
};

#if defined(__gnu_linux__) || defined(__linux__) || defined(linux) || defined(__linux)
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
// Declaration of thread condition variable
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
#endif


#ifdef _MSC_VER
#include <Windows.h>
DWORD bifurcate_multithreading(void* param){
	BifurcateData* thread_info = (BifurcateData*)param;
	for (int i = thread_info->start; i < thread_info->end; i += 8) {
		float x_val[8];
		// float b_value[8];
		for (int j = 0; j < 8; j++) {
			x_val[j] = start.x + ((float)(i + j) / no_values) * (end.x - start.x);
		}

		__m256 x_val_simd = _mm256_set_ps(x_val[0], x_val[1], x_val[2], x_val[3], x_val[4], x_val[5], x_val[6], x_val[7]);
		__m256 b_val_simd;

		b_val_simd = bifurcation_value_simd(x_val_simd, _mm256_set1_ps(0.4));

		thread_info->y_values[i / 8] = b_val_simd;
	}

	return NULL;
}

void bifurcation(__m256* y_value, int width, int height) {
#ifdef MULTI_THREAD
	HANDLE threads[THREAD_NO_BIFURC_CALC];
	BifurcateData data[THREAD_NO_BIFURC_CALC];


	int start_index = 0;
	const int partition_size = no_values / THREAD_NO_BIFURC_CALC;
	int end_index = start_index + partition_size;

	for (int i = 0; i < THREAD_NO_BIFURC_CALC && start_index < no_values && end_index <= no_values; i++) {
		data[i] = BifurcateData{
			y_value,
			start_index,
			end_index
		};
		start_index = end_index;
		end_index += partition_size;
		end_index = end_index < no_values ? end_index : no_values;
		threads[i] = CreateThread(0, 0, bifurcate_multithreading, &data[i], 0, 0);
	}

	WaitForMultipleObjects(THREAD_NO_BIFURC_CALC, threads, 1, INFINITE);

#else
	for (int x = 0; x < no_values; x += 8) {
		float x_val[8];
		// float b_value[8];
		for (int j = 0; j < 8; j++) {
			x_val[j] = start.x + ((float)(x + j) / no_values) * (end.x - start.x);
		}

		__m256 x_val_simd = _mm256_set_ps(x_val[0], x_val[1], x_val[2], x_val[3], x_val[4], x_val[5], x_val[6], x_val[7]);
		__m256 b_val_simd;

		b_val_simd = bifurcation_value_simd(x_val_simd, _mm256_set1_ps(0.4));

		y_value[(int)(x / 8)] = b_val_simd;
	}
#endif
}


#endif // _MSC_VER





/*
--------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------
------------------------------------------------LINUX---------------------------------------------------
--------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------
*/
#if defined(__gnu_linux__) || defined(__linux__) || defined(linux) || defined(__linux)
#include <pthread.h>
void* bifurcate_multithreading(void* param){
	BifurcateData *thread_info = (BifurcateData *)param;
	for(int i = thread_info->start; i < thread_info->end; i += 7){
		float x_val[8]; 
		// float b_value[8];
		for(int j = 0; j < 8; j++){
			x_val[j] = start.x + ((float)(i + j) / no_values) * (end.x - start.x);
		}

		__m256 x_val_simd = _mm256_set_ps(x_val[0], x_val[1], x_val[2], x_val[3], x_val[4], x_val[5], x_val[6], x_val[7]);
		__m256 b_val_simd;
		
		b_val_simd = bifurcation_value_simd(x_val_simd, _mm256_set1_ps(0.4));

		

#if defined(MULTI_THREAD)
		// lock mutex for data assignment
		pthread_mutex_lock(&lock);
		thread_info->y_values[i / 8] = b_val_simd;
		pthread_mutex_unlock(&lock);
#else
		// thread_info->y_values[i] = _mm256_set_ps(x_val[0], x_val[1], x_val[2], x_val[3], x_val[4], x_val[5], x_val[6], x_val[7]);
		thread_info->y_values[i] = b_val_simd;
#endif

	}

	return NULL;
}

void bifurcation(__m256 *y_value, int width, int height) {
#ifdef MULTI_THREAD
	pthread_t threads[THREAD_NO];
	BifurcateData data[THREAD_NO];


	int start_index = 0;
	const uint partition_size = no_values / THREAD_NO;
	int end_index = start_index + partition_size;
	for(int i = 0; i < THREAD_NO && start_index < no_values && end_index <= no_values; i++){
		data[i] = BifurcateData{
			y_value,
			start_index,
			end_index
		};
		start_index = end_index;
		end_index += partition_size;
		end_index = end_index < no_values?end_index:no_values; 
		pthread_create(&threads[i], NULL, bifurcate_multithreading, &data[i]);
	}

	for(int i = 0; i < THREAD_NO; i++){
		pthread_join(threads[i], 0);
	}

#else
	for (int x = 0; x < no_values; x += 8) {
		float x_val[8]; 
		// float b_value[8];
		for(int j = 0; j < 8; j++){
			x_val[j] = start.x + ((float)(x + j) / no_values) * (end.x - start.x);
		}

		__m256 x_val_simd = _mm256_set_ps(x_val[0], x_val[1], x_val[2], x_val[3], x_val[4], x_val[5], x_val[6], x_val[7]);
		__m256 b_val_simd;
		
		b_val_simd = bifurcation_value_simd(x_val_simd, _mm256_set1_ps(0.4));

		y_value[x / 8] = b_val_simd;
	}
#endif
}
#endif





inline float MapRange(float from_x1, float from_x2, float to_x1, float to_x2, float x) {
	return (to_x2 - to_x1) / (from_x2 - from_x1) * (x - from_x1) + to_x1;
}

void render_logistic_map(__m256* y_values, int width, int height) {
	glLoadIdentity();
	glOrtho(0, width, 0, height, -1, 1);

	glBegin(GL_POINTS);
	float x_val[8];
	float b_value[8] = { 0 };
	for (int x = 0; x < no_values; x += 8) {
		glColor3f(0.3, 0.5, 0.5);

		__m256 y_value_simd = y_values[x / 8];
		// get the b_value from the simd variable
		_mm256_storeu_ps(b_value, y_value_simd);

		// initial rendering
		for(int j = 0; j < 8; j++){
			x_val[j] = start.x + ((float)(x + j) / no_values) * (end.x - start.x);
			glVertex2f(MapRange(start.x, end.x, 0, (float)width, x_val[j]), MapRange(start.y, end.y, 0, height, b_value[j]));
		}

		// simd variable for x so that latex operation can be done
		__m256 x_value_simd = _mm256_set_ps(x_val[0], x_val[1], x_val[2], x_val[3], x_val[4], x_val[5], x_val[6], x_val[7]);
		for(int i = 0; i < 20; i++){
			y_value_simd = _mm256_mul_ps(x_value_simd, _mm256_mul_ps(y_value_simd, _mm256_sub_ps(_mm256_set1_ps(1.0), y_value_simd))); 
			
			// store into array for rendering
			_mm256_storeu_ps(b_value, y_value_simd);
			
			for(int j = 0; j < 8; j++){
				glVertex2f(MapRange(start.x, end.x, 0, (float)width, x_val[j]), MapRange(start.y, end.y, 0, height, b_value[j]));
			}
		}
	}
	glEnd();
}
//screen parameters
typedef struct Screen {
	__m256* y_value;
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


__m256 *Align(__m256 *location, size_t alignment) {
    return reinterpret_cast<__m256 *>(reinterpret_cast<size_t>(location + (alignment - 1)) & ~(alignment - 1));
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

	start = MathLib::vec2(0,0);
	end = MathLib::vec2(8, 1);

	glfwMakeContextCurrent(window);
	// glViewport(0, 0, width, height);

	__m256* y_values = (__m256 *)malloc(sizeof(__m256) * no_values / 8);
	//y_values = Align(y_values, 32);
	screen.y_value = y_values;
	bifurcation(y_values, width, height);
	glfwSetWindowUserPointer(window, &screen);

	double previousTime = glfwGetTime();
	int frameCount = 0;


	while (!glfwWindowShouldClose(window)){
		glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		render_logistic_map(y_values, screen.width, screen.height);

		glfwSwapBuffers(window);

		// Measure speed
		double currentTime = glfwGetTime();
		frameCount++;
		if (currentTime - previousTime > 0.5){
			// If a second has passed.
			// Display the frame count here any way you want
			printf("Frame Count:%d\n", frameCount * 2);

			frameCount = 0;
			previousTime = currentTime;
		}


		glfwPollEvents();
		processKeyboardInput(window);
	}
}
