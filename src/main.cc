#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

// OpenGL library includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <debuggl.h>
#include "camera.h"
#include "boid.h"
#include "obstacle.h"

int window_width = 800, window_height = 600;

// VBO and VAO descriptors.
enum { kVertexBuffer, kIndexBuffer, kNumVbos };

// These are our VAOs.
enum { kBoidsVao, kObstaclesVao, kNumVaos };

GLuint g_array_objects[kNumVaos];  // This will store the VAO descriptors.
GLuint g_buffer_objects[kNumVaos][kNumVbos];  // These will store VBO descriptors.

// C++ 11 String Literal
// See http://en.cppreference.com/w/cpp/language/string_literal
const char* vertex_shader =
R"zzz(#version 330 core
in vec4 vertex_position;
uniform mat4 view;
uniform vec4 light_position;
out vec4 vs_light_direction;
out vec3 vertex_world_position;
void main()
{
	gl_Position = view * vertex_position;
	vs_light_direction = -gl_Position + view * light_position;
	vertex_world_position = vec3(vertex_position.x, vertex_position.y, vertex_position.z);
}
)zzz";

const char* geometry_shader =
R"zzz(#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;
uniform mat4 view;
uniform mat4 projection;
in vec4 vs_light_direction[];
in vec3 vertex_world_position[];
flat out vec4 normal;
flat out vec4 color_normal;
out vec4 light_direction;
out vec3 world_position;
void main()
{
	int n = 0;
	vec3 co = normalize(cross(vertex_world_position[1] - vertex_world_position[0], vertex_world_position[2] - vertex_world_position[0]));
	color_normal = vec4(co, 0.0);
	normal = view * color_normal;
	for (n = 0; n < gl_in.length(); n++) {
		light_direction = vs_light_direction[n];
		gl_Position = projection * gl_in[n].gl_Position;
		world_position = vertex_world_position[n];
		EmitVertex();
	}
	EndPrimitive();
}
)zzz";

const char* boids_fragment_shader =
R"zzz(#version 330 core
flat in vec4 normal;
flat in vec4 color_normal;
in vec4 light_direction;
in vec3 world_position;
out vec4 fragment_color;
void main()
{
	vec4 color = vec4(0.0, 1.0, 0.0, 1.0);
	float dot_nl = dot(normalize(light_direction), normalize(normal));
	dot_nl = clamp(dot_nl, 0.1, 1.0);
	fragment_color = clamp(dot_nl * color, 0.0, 1.0);
}
)zzz";

const char* obstacles_fragment_shader =
R"zzz(#version 330 core
flat in vec4 color_normal;
in vec4 light_direction;
in vec3 world_position;
out vec4 fragment_color;
void main()
{
	fragment_color = vec4(1.0, 0.0, 0.0, 0.0);
	float dot_nl = dot(normalize(vec4(-1.0, -1.0, 0.0, 1.0)), normalize(color_normal));
	float dot_nl_2 = dot(normalize(vec4(1.0, 1.0, 0.0, 1.0)), normalize(color_normal));
	dot_nl = clamp(dot_nl, 0.1, 1.0);
	dot_nl_2 = clamp(dot_nl_2, 0.1, 1.0);
	fragment_color = clamp((dot_nl + dot_nl_2) * fragment_color, 0.0, 1.0);
}
)zzz";

void
ErrorCallback(int error, const char* description)
{
	std::cerr << "GLFW Error: " << description << "\n";
}

Camera g_camera(100.0f, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

// Interaction variables
bool fpsMode = false;
Camera::Action cAction = Camera::none;
bool w_pressed = false;
bool s_pressed = false;

bool a_pressed = false;
bool d_pressed = false;

bool q_pressed = false;
bool r_pressed = false;

bool down_pressed = false;
bool up_pressed = false;

bool right_pressed = false;
bool left_pressed = false;

// Method defined in Project 3 (Menger sponge) for tracking keyboard events.
void
KeyCallback(GLFWwindow* window,
            int key,
            int scancode,
            int action,
            int mods)
{
	// Note:
	// This is only a list of functions to implement.
	// you may want to re-organize this piece of code.
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	else if (key == GLFW_KEY_S && mods == GLFW_MOD_CONTROL && action == GLFW_RELEASE) {
		// FIXME: save geometry to OBJ
	} else if (key == GLFW_KEY_W && action != GLFW_RELEASE) {
		w_pressed = true;
	} else if (key == GLFW_KEY_S && action != GLFW_RELEASE) {
		s_pressed = true;
	} else if (key == GLFW_KEY_A && action != GLFW_RELEASE) {
		a_pressed = true;
	} else if (key == GLFW_KEY_D && action != GLFW_RELEASE) {
		d_pressed = true;
	} else if (key == GLFW_KEY_LEFT && action != GLFW_RELEASE) {
		left_pressed = true;
	} else if (key == GLFW_KEY_RIGHT && action != GLFW_RELEASE) {
		right_pressed = true;
	} else if (key == GLFW_KEY_DOWN && action != GLFW_RELEASE) {
		down_pressed = true;
	} else if (key == GLFW_KEY_UP && action != GLFW_RELEASE) {
		up_pressed = true;
	} else if (key == GLFW_KEY_C && action != GLFW_RELEASE) {
		fpsMode = !fpsMode;
	} else if (key == GLFW_KEY_Q && action != GLFW_RELEASE) {
		q_pressed = true;
	} else if (key == GLFW_KEY_R && action != GLFW_RELEASE) {
		r_pressed = true;
	}

	if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
		w_pressed = false;
	} else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
		s_pressed = false;
	} else if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
		a_pressed = false;
	} else if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
		d_pressed = false;
	} else if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) {
		down_pressed = false;
	} else if (key == GLFW_KEY_UP && action == GLFW_RELEASE) {
		up_pressed = false;
	} else if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) {
		left_pressed = false;
	} else if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) {
		right_pressed = false;
	} else if (key == GLFW_KEY_Q && action == GLFW_RELEASE) {
		q_pressed = false;
	} else if (key == GLFW_KEY_R && action == GLFW_RELEASE) {
		r_pressed = false;
	}
}

int g_current_button;
bool g_mouse_pressed;

// Interaction variables
glm::vec2 dragDirection(0.0f, 0.0f);

#define NOISE 9999999.9f

float mouse_x_past = NOISE;
float mouse_y_past = NOISE;

float mouse_x_current = NOISE;
float mouse_y_current = NOISE;

float mouse_x_captured;
float mouse_y_captured;

float mouse_x_captured_without_button_press;
float mouse_y_captured_without_button_press;

bool mouse_capture_left = false;
bool mouse_capture_right = false;

float magnitude = 0.0f;

// Method defined in Project 3 (Menger sponge) for tracking mouse events.
void
MousePosCallback(GLFWwindow* window, double mouse_x, double mouse_y)
{
	if (!g_mouse_pressed) {
		mouse_x_captured_without_button_press = mouse_x;
		mouse_y_captured_without_button_press = window_height - mouse_y;
		return;
	}
	if (g_current_button == GLFW_MOUSE_BUTTON_LEFT) {
		mouse_x_captured = mouse_x;
		mouse_y_captured = mouse_y;
		mouse_capture_left = true;
	} else if (g_current_button == GLFW_MOUSE_BUTTON_RIGHT) {
		mouse_x_captured = mouse_x;
		mouse_y_captured = mouse_y;
		mouse_capture_right = true;
	} else if (g_current_button == GLFW_MOUSE_BUTTON_MIDDLE) {
		
	}
}

// Method defined in Project 3 (Menger sponge) for tracking mouse events.
void
MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	g_mouse_pressed = (action == GLFW_PRESS);
	g_current_button = button;
}

// Method that checks the status of the interaction variables, updating
// the parameters that will be passed to the matrix transformation.
void
checkInput() {
	// Check interaction variables for every interaction type.
	if (mouse_capture_left) {
		// Previous state was stationary.
		if (mouse_x_current == NOISE) {
			mouse_x_current = mouse_x_captured;
			mouse_y_current = mouse_y_captured;

			mouse_x_past = NOISE;
			mouse_y_past = NOISE;

			dragDirection = glm::vec2(0.0f, 0.0f);
		} else {
			mouse_x_past = mouse_x_current;
			mouse_y_past = mouse_y_current;

			mouse_x_current = mouse_x_captured;
			mouse_y_current = mouse_y_captured;

			cAction = Camera::rotation;

			dragDirection = glm::vec2(mouse_x_current - mouse_x_past, mouse_y_current - mouse_y_past);
		}

		mouse_capture_left = false;
	} else if (mouse_capture_right) {
		// Previous state was stationary.
		if (mouse_x_current == NOISE) {
			mouse_x_current = mouse_x_captured;
			mouse_y_current = mouse_y_captured;

			mouse_x_past = NOISE;
			mouse_y_past = NOISE;

			dragDirection = glm::vec2(0.0f, 0.0f);
		} else {
			mouse_x_past = mouse_x_current;
			mouse_y_past = mouse_y_current;

			mouse_x_current = mouse_x_captured;
			mouse_y_current = mouse_y_captured;

			cAction = Camera::zoom_mouse;

			dragDirection = glm::vec2(mouse_x_current - mouse_x_past, mouse_y_current - mouse_y_past);
		}

		mouse_capture_right = false;
	} else if (w_pressed) {
		cAction = Camera::zoom_keys;
		magnitude = -1.0f;		
	} else if (s_pressed) {
		cAction = Camera::zoom_keys;
		magnitude = 1.0f;
	} else if (a_pressed) {
		cAction = Camera::strafe;
		magnitude = -1.0f;		
	} else if (d_pressed) {
		cAction = Camera::strafe;
		magnitude = 1.0f;
	} else if (down_pressed) {
		cAction = Camera::translation;
		magnitude = -1.0f;		
	} else if (up_pressed) {
		cAction = Camera::translation;
		magnitude = 1.0f;
	} else if (left_pressed) {
		cAction = Camera::roll;
		magnitude = -1.0f;		
	} else if (right_pressed) {
		cAction = Camera::roll;
		magnitude = 1.0f;
	} else {
		// Reset interaction variables.
		mouse_x_current = NOISE;
		mouse_y_current = NOISE;

		mouse_x_past = NOISE;
		mouse_y_past = NOISE;

		cAction = Camera::none;
		
		dragDirection = glm::vec2(0.0f, 0.0f);
	}
}

// Method that checks the status of the interaction variables related to object input,
// adding new objects to the scene in case that the user presses keys 'q' (for boids)
// or 'r' (for obstacles).
int
checkNewObjectsInput(glm::mat4 view_matrix, glm::mat4 projection_matrix,
					 std::vector<Boid*> &boids, std::vector<glm::vec4> &boids_vertices, std::vector<glm::uvec3> &boids_faces,
					 std::vector<Obstacle*> &obstacles, std::vector<glm::vec4> &obstacles_vertices, std::vector<glm::uvec3> &obstacles_faces) {
	
	glm::uvec4 viewport = glm::uvec4(0, 0, window_width, window_height);

	// We'll project a ray going from the near coordinate to the far coordinate,
	// and choose a random location inbetween to position the object. 
	glm::vec3 near_coordinate = glm::vec3(mouse_x_captured_without_button_press, mouse_y_captured_without_button_press, 0.0f);
	glm::vec3 far_coordinate = glm::vec3(mouse_x_captured_without_button_press, mouse_y_captured_without_button_press, 1.0f);

	glm::vec3 world_near_coordinate = glm::unProject(near_coordinate, glm::mat4(1.0f), projection_matrix * view_matrix, viewport);
	glm::vec3 world_far_coordinate = glm::unProject(far_coordinate, glm::mat4(1.0f), projection_matrix * view_matrix, viewport);

	glm::vec3 direction = glm::normalize(world_far_coordinate - world_near_coordinate);

	float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

	glm::vec3 position;

	// Insert new boid in scene.
	if (q_pressed) {
		position = world_near_coordinate + (world_far_coordinate - world_near_coordinate) * glm::max(r, 0.5f) * 0.05f;

		boids.push_back(new Boid(position.x, position.y, position.z, boids_vertices, boids_faces, boids.size()));
		q_pressed = false;
		return 1;

	// Insert new obstacle in scene.
	} else if (r_pressed) {
		position = world_near_coordinate + (world_far_coordinate - world_near_coordinate) * glm::max(r, 0.5f) * 0.2f;

		// For the case of obstacles, we'll have to check whether the position we got
		// is within the boundaries.
		
		// Attempt to locate the obstacle within the boundary.
		int no_of_attempts = 0;
		while (glm::length(position) > 50.0f && no_of_attempts < 20) {
			r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			position = world_near_coordinate + (world_far_coordinate - world_near_coordinate) * glm::max(r, 0.5f) * 0.2f;
			no_of_attempts ++;
		}

		// If this wasn't possible, we'll position the obstacle in the ray at a distance of 90 from the origin. 
		if (no_of_attempts == 20) {
			// We apply the quadratic formula to find the location over the ray
			// that is at a distance of 90 from the origin.
			float x, y, z;
			float a, b, c;
			glm::vec3 direction = glm::normalize(world_far_coordinate - world_near_coordinate);

			a = world_near_coordinate.x;
			b = world_near_coordinate.y;
			c = world_near_coordinate.z;

			x = direction.x;
			y = direction.y;
			z = direction.z;

			float a_q, b_q, c_q;

			a_q = x*x + y*y + z*z;
			b_q = 2.0f * (a*x + b*y + c*z);
			c_q = a*a + b*b + c*c - 80.0f*80.0f;

			float k;

			// Check if the equation has a solution.
			// If no solution exists, we'll choose an aribitrary position for the obstacle
			// (k = 200).
			if (glm::sqrt(b_q*b_q - 4.0f*a_q*c_q) >= 0.0f) {
				k = (-b_q + glm::sqrt(b_q*b_q - 4.0f*a_q*c_q)) / (2.0f * a_q);
			} else {
				k = 200.0f;
			}
			position = world_near_coordinate + direction * k;
		}

		obstacles.push_back(new Obstacle(position.x, position.y, position.z, obstacles_vertices, obstacles_faces));
		r_pressed = false;
		return 2;
	}
	return 0;
}

int main(int argc, char* argv[])
{
	std::string window_title = "Boids";
	if (!glfwInit()) exit(EXIT_FAILURE);
	glfwSetErrorCallback(ErrorCallback);

	// Ask an OpenGL 3.3 core profile context 
	// It is required on OSX and non-NVIDIA Linux
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(window_width, window_height,
			&window_title[0], nullptr, nullptr);
	CHECK_SUCCESS(window != nullptr);
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;

	CHECK_SUCCESS(glewInit() == GLEW_OK);
	glGetError();  // clear GLEW's error for it
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MousePosCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSwapInterval(1);
	const GLubyte* renderer = glGetString(GL_RENDERER);  // get renderer string
	const GLubyte* version = glGetString(GL_VERSION);    // version as a string
	std::cout << "Renderer: " << renderer << "\n";
	std::cout << "OpenGL version supported:" << version << "\n";

	// Common setup for boids and obstacles.
	glm::vec4 light_position = glm::vec4(10.0f, 10.0f, 10.0f, 1.0f);
	float aspect = 0.0f;
	float theta = 0.0f;

	// Setup vertex shader.
	GLuint vertex_shader_id = 0;
	const char* vertex_source_pointer = vertex_shader;
	CHECK_GL_ERROR(vertex_shader_id = glCreateShader(GL_VERTEX_SHADER));
	CHECK_GL_ERROR(glShaderSource(vertex_shader_id, 1, &vertex_source_pointer, nullptr));
	glCompileShader(vertex_shader_id);
	CHECK_GL_SHADER_ERROR(vertex_shader_id);

	// Setup geometry shader.
	GLuint geometry_shader_id = 0;
	const char* geometry_source_pointer = geometry_shader;
	CHECK_GL_ERROR(geometry_shader_id = glCreateShader(GL_GEOMETRY_SHADER));
	CHECK_GL_ERROR(glShaderSource(geometry_shader_id, 1, &geometry_source_pointer, nullptr));
	glCompileShader(geometry_shader_id);
	CHECK_GL_SHADER_ERROR(geometry_shader_id);

	//////////////////////
	/////   BOIDS   //////
	//////////////////////

	// Create data structures for the boids.
	std::vector<Boid*> boids;
	std::vector<glm::vec4> boids_vertices;
	std::vector<glm::uvec3> boids_faces;

	// Add boids to scene.
	// Maximum separation from the origin in each axis.
	int tam = 40;
	for (int i = 0; i < 500; i ++) {
		float rand_x = rand() % (2*tam) - tam;
		float rand_y = rand() % (2*tam) - tam;
		float rand_z = rand() % (2*tam) - tam;
		boids.push_back(new Boid(rand_x, rand_y, rand_z, boids_vertices, boids_faces, i));
	}

	// Setup our VAO array.
	CHECK_GL_ERROR(glGenVertexArrays(kNumVaos, &g_array_objects[0]));

	// Switch to the VAO for boids.
	CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kBoidsVao]));

	// Generate buffer objects
	CHECK_GL_ERROR(glGenBuffers(kNumVbos, &g_buffer_objects[kBoidsVao][0]));

	// Setup vertex data in a VBO.
	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, g_buffer_objects[kBoidsVao][kVertexBuffer]));
	// NOTE: We do not send anything right now, we just describe it to OpenGL.
	CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
				sizeof(float) * boids_vertices.size() * 4, nullptr,
				GL_STATIC_DRAW));
	CHECK_GL_ERROR(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));
	CHECK_GL_ERROR(glEnableVertexAttribArray(0));

	// Setup element array buffer.
	CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_buffer_objects[kBoidsVao][kIndexBuffer]));
	CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				sizeof(uint32_t) * boids_faces.size() * 3,
				&boids_faces[0], GL_STATIC_DRAW));

	// Setup fragment shader for the boids objects
	GLuint boids_fragment_shader_id = 0;
	const char* boids_fragment_source_pointer = boids_fragment_shader;
	CHECK_GL_ERROR(boids_fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER));
	CHECK_GL_ERROR(glShaderSource(boids_fragment_shader_id, 1,
				&boids_fragment_source_pointer, nullptr));
	glCompileShader(boids_fragment_shader_id);
	CHECK_GL_SHADER_ERROR(boids_fragment_shader_id);


	// Let's create our boids program.
	GLuint boids_program_id = 0;
	CHECK_GL_ERROR(boids_program_id = glCreateProgram());
	CHECK_GL_ERROR(glAttachShader(boids_program_id, vertex_shader_id));
	CHECK_GL_ERROR(glAttachShader(boids_program_id, boids_fragment_shader_id));
	CHECK_GL_ERROR(glAttachShader(boids_program_id, geometry_shader_id));

	// Bind attributes.
	CHECK_GL_ERROR(glBindAttribLocation(boids_program_id, 0, "vertex_position"));
	CHECK_GL_ERROR(glBindFragDataLocation(boids_program_id, 0, "fragment_color"));
	glLinkProgram(boids_program_id);
	CHECK_GL_PROGRAM_ERROR(boids_program_id);

	// Get the uniform locations.
	GLint boids_projection_matrix_location = 0;
	CHECK_GL_ERROR(boids_projection_matrix_location =
			glGetUniformLocation(boids_program_id, "projection"));
	GLint boids_view_matrix_location = 0;
	CHECK_GL_ERROR(boids_view_matrix_location =
			glGetUniformLocation(boids_program_id, "view"));
	GLint boids_light_position_location = 0;
	CHECK_GL_ERROR(boids_light_position_location =
			glGetUniformLocation(boids_program_id, "light_position"));

	///////////////////////////
	//////   OBSTACLES   //////
	///////////////////////////

	// Create data structures for the obstacles.
	std::vector<Obstacle*> obstacles;
	std::vector<glm::vec4> obstacles_vertices;
	std::vector<glm::uvec3> obstacles_faces;

	// Add boids to scene.
	for (int i = 0; i < 80; i ++) {
		float rand_x = rand() % (2*tam) - tam;
		float rand_y = rand() % (2*tam) - tam;
		float rand_z = rand() % (2*tam) - tam;
		obstacles.push_back(new Obstacle(rand_x, rand_y, rand_z, obstacles_vertices, obstacles_faces));
	}

	// Switch to the VAO for obstacles.
	CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kObstaclesVao]));

	// Generate buffer objects
	CHECK_GL_ERROR(glGenBuffers(kNumVbos, &g_buffer_objects[kObstaclesVao][0]));

	// Setup vertex data in a VBO.
	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, g_buffer_objects[kObstaclesVao][kVertexBuffer]));
	// NOTE: We do not send anything right now, we just describe it to OpenGL.
	CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
				sizeof(float) * obstacles_vertices.size() * 4, nullptr,
				GL_STATIC_DRAW));
	CHECK_GL_ERROR(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));
	CHECK_GL_ERROR(glEnableVertexAttribArray(0));

	// Setup element array buffer.
	CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_buffer_objects[kObstaclesVao][kIndexBuffer]));
	CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				sizeof(uint32_t) * obstacles_faces.size() * 3,
				&obstacles_faces[0], GL_STATIC_DRAW));

	// Setup fragment shader for the obstacles.
	GLuint obstacles_fragment_shader_id = 0;
	const char* obstacles_fragment_source_pointer = obstacles_fragment_shader;
	CHECK_GL_ERROR(obstacles_fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER));
	CHECK_GL_ERROR(glShaderSource(obstacles_fragment_shader_id, 1,
				&obstacles_fragment_source_pointer, nullptr));
	glCompileShader(obstacles_fragment_shader_id);
	CHECK_GL_SHADER_ERROR(obstacles_fragment_shader_id);


	// Let's create our obstacles program.
	GLuint obstacles_program_id = 0;
	CHECK_GL_ERROR(obstacles_program_id = glCreateProgram());
	CHECK_GL_ERROR(glAttachShader(obstacles_program_id, vertex_shader_id));
	CHECK_GL_ERROR(glAttachShader(obstacles_program_id, obstacles_fragment_shader_id));
	CHECK_GL_ERROR(glAttachShader(obstacles_program_id, geometry_shader_id));

	// Bind attributes.
	CHECK_GL_ERROR(glBindAttribLocation(obstacles_program_id, 0, "vertex_position"));
	CHECK_GL_ERROR(glBindFragDataLocation(obstacles_program_id, 0, "fragment_color"));
	glLinkProgram(obstacles_program_id);
	CHECK_GL_PROGRAM_ERROR(obstacles_program_id);

	// Get the uniform locations.
	GLint obstacles_projection_matrix_location = 0;
	CHECK_GL_ERROR(obstacles_projection_matrix_location =
			glGetUniformLocation(obstacles_program_id, "projection"));
	GLint obstacles_view_matrix_location = 0;
	CHECK_GL_ERROR(obstacles_view_matrix_location =
			glGetUniformLocation(obstacles_program_id, "view"));
	GLint obstacles_light_position_location = 0;
	CHECK_GL_ERROR(obstacles_light_position_location =
			glGetUniformLocation(obstacles_program_id, "light_position"));

	while (!glfwWindowShouldClose(window)) {
		// Setup some basic window stuff.
		glfwGetFramebufferSize(window, &window_width, &window_height);
		glViewport(0, 0, window_width, window_height);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDepthFunc(GL_LESS);

		// Compute the projection matrix.
		aspect = static_cast<float>(window_width) / window_height;
		glm::mat4 projection_matrix =
			glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.0f);

		// This function will capture important data from mouse/keyboard events,
		// which might translate to control actions that can affect
		// the camera's view matrix.
		checkInput();
		
		// Compute the view matrix.
		glm::mat4 view_matrix = g_camera.get_view_matrix(cAction, fpsMode, dragDirection, magnitude);

		// This function will potentially add new objects to the scene.
		int objects_changed = checkNewObjectsInput(view_matrix, projection_matrix,
			            						   boids, boids_vertices, boids_faces,
			            						   obstacles, obstacles_vertices, obstacles_faces); 

		if (objects_changed == 1) {
			CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kBoidsVao]));
			CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_buffer_objects[kBoidsVao][kIndexBuffer]));
			CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
						sizeof(uint32_t) * boids_faces.size() * 3,
						&boids_faces[0], GL_STATIC_DRAW));
		} else if (objects_changed == 2) {
			CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kObstaclesVao]));
			CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_buffer_objects[kObstaclesVao][kIndexBuffer]));
			CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
						sizeof(uint32_t) * obstacles_faces.size() * 3,
						&obstacles_faces[0], GL_STATIC_DRAW));
		}

		// Update boids positions.
		for (int i = 0; i < boids.size(); i ++) {
			boids[i]->update(boids_vertices, boids, obstacles);
		}

		/**************
		 *            *
		 * Draw boids *
		 *
		 **************/

		// Switch to the boids VAO.
		CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kBoidsVao]));
		// Send vertices to the GPU.
		CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER,
		                            g_buffer_objects[kBoidsVao][kVertexBuffer]));
		CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
		                            sizeof(float) * boids_vertices.size() * 4,
		                            &boids_vertices[0], GL_STATIC_DRAW));

		// Use boids program.
		CHECK_GL_ERROR(glUseProgram(boids_program_id));

		// Pass uniforms in.
		CHECK_GL_ERROR(glUniformMatrix4fv(boids_projection_matrix_location, 1, GL_FALSE,
					&projection_matrix[0][0]));
		CHECK_GL_ERROR(glUniformMatrix4fv(boids_view_matrix_location, 1, GL_FALSE,
					&view_matrix[0][0]));
		CHECK_GL_ERROR(glUniform4fv(boids_light_position_location, 1, &light_position[0]));

		// Draw triangles.
		CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, boids_faces.size() * 3, GL_UNSIGNED_INT, 0));

		/******************
		 *                *
		 * Draw obstacles *
		 *                *
		 ******************/

		// Switch to the obstacles VAO.
		CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kObstaclesVao]));
		// Send vertices to the GPU.
		CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER,
		                            g_buffer_objects[kObstaclesVao][kVertexBuffer]));
		CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
		                            sizeof(float) * obstacles_vertices.size() * 4,
		                            &obstacles_vertices[0], GL_STATIC_DRAW));

		// Use obstacles program.
		CHECK_GL_ERROR(glUseProgram(obstacles_program_id));

		// Pass uniforms in.
		CHECK_GL_ERROR(glUniformMatrix4fv(obstacles_projection_matrix_location, 1, GL_FALSE,
					&projection_matrix[0][0]));
		CHECK_GL_ERROR(glUniformMatrix4fv(obstacles_view_matrix_location, 1, GL_FALSE,
					&view_matrix[0][0]));
		CHECK_GL_ERROR(glUniform4fv(obstacles_light_position_location, 1, &light_position[0]));

		// Draw triangles.
		CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, obstacles_faces.size() * 3, GL_UNSIGNED_INT, 0));

		// Poll and swap.
		glfwPollEvents();
		glfwSwapBuffers(window);
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
