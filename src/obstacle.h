#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/norm.hpp> 
#include <vector>
#include <random>

class Obstacle {
public:
	float rand_d() {
		return 2.0f * (((double) rand() / (RAND_MAX)) + 1.0) - 1.0f;
	}

	Obstacle(float x, float y, float z, std::vector<glm::vec4>& vertices, std::vector<glm::uvec3>& faces) {
		center = glm::vec3(x, y, z);
		side = (float)(rand() % 5 + 4);
		radius = glm::sqrt(2.0f) * side / 2.0f; 

		// Randomize front
		front = glm::normalize(glm::vec3(rand_d(), rand_d(), rand_d()));

		// Calculate up
		glm::vec3 v = front;
		if (v.x < v.y && v.x < v.z) {
			v.x = 1.0f;
			v.y = 0.0f;
			v.z = 0.0f;
		} else if (v.y < v.z && v.y < v.z) {
			v.x = 0.0f;
			v.y = 1.0f;
			v.z = 0.0f;
		} else {
			v.x = 0.0f;
			v.y = 0.0f;
			v.z = 1.0f;
		}
		up = glm::cross(front, v) / glm::length(glm::cross(front, v));

		// Calculate right
		right = glm::cross(front, up);

		int vertex_base_index = vertices.size();

		glm::vec3 right_delta = right * side / 2.0f;
		glm::vec3 up_delta    = up * side / 2.0f;
		glm::vec3 front_delta = front * side / 2.0f;

		// Add all vertices of obstacle
		vertices.push_back(glm::vec4(center + front_delta + up_delta - right_delta, 1.0f));
		vertices.push_back(glm::vec4(center + front_delta + up_delta + right_delta, 1.0f));
		vertices.push_back(glm::vec4(center + front_delta - up_delta - right_delta, 1.0f));
		vertices.push_back(glm::vec4(center + front_delta - up_delta + right_delta, 1.0f));

		vertices.push_back(glm::vec4(center - front_delta + up_delta - right_delta, 1.0f));
		vertices.push_back(glm::vec4(center - front_delta + up_delta + right_delta, 1.0f));
		vertices.push_back(glm::vec4(center - front_delta - up_delta - right_delta, 1.0f));
		vertices.push_back(glm::vec4(center - front_delta - up_delta + right_delta, 1.0f));

		// Add all faces of obstacle
		faces.push_back(glm::uvec3(vertex_base_index,     vertex_base_index + 2, vertex_base_index + 1));
		faces.push_back(glm::uvec3(vertex_base_index + 2, vertex_base_index + 3, vertex_base_index + 1));

		faces.push_back(glm::uvec3(vertex_base_index + 3, vertex_base_index + 5, vertex_base_index + 1));
		faces.push_back(glm::uvec3(vertex_base_index + 3, vertex_base_index + 7, vertex_base_index + 5));

		faces.push_back(glm::uvec3(vertex_base_index + 2, vertex_base_index + 7, vertex_base_index + 3));
		faces.push_back(glm::uvec3(vertex_base_index + 2, vertex_base_index + 6, vertex_base_index + 7));

		faces.push_back(glm::uvec3(vertex_base_index,     vertex_base_index + 4, vertex_base_index + 6));
		faces.push_back(glm::uvec3(vertex_base_index,     vertex_base_index + 6, vertex_base_index + 2));

		faces.push_back(glm::uvec3(vertex_base_index,     vertex_base_index + 1, vertex_base_index + 5));
		faces.push_back(glm::uvec3(vertex_base_index,     vertex_base_index + 5, vertex_base_index + 4));

		faces.push_back(glm::uvec3(vertex_base_index + 4, vertex_base_index + 5, vertex_base_index + 6));
		faces.push_back(glm::uvec3(vertex_base_index + 6, vertex_base_index + 5, vertex_base_index + 7));
	}

	

	~Obstacle();
	glm::vec3 center;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	float radius;
	float side; 

	//std::vector<glm::vec4> vertices;
	//std::vector<glm::uvec3> faces;
};

#endif
