#ifndef BOID_H
#define BOID_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/norm.hpp> 
#include <vector>

class Boid {
public:
	Boid(float x, float y, float z, std::vector<glm::vec4>& vertices, std::vector<glm::uvec3>& faces, int index) {
		this->index = index;
		center = glm::vec3(x, y, z);
		velocity = glm::vec3(0.0f, 0.0f, -1.0f);
		front = glm::vec3(0.0f, 0.0f, -1.0f);
		up = glm::vec3(0.0f, 1.0f, 0.0f);
		right = glm::vec3(1.0f, 0.0f, 0.0f);

		vertex_base_index = vertices.size();

		// Add all vertices of boid
		vertices.push_back(glm::vec4(center - 0.5f * right, 1.0f));
		vertices.push_back(glm::vec4(center + 0.5f * right, 1.0f));

		vertices.push_back(glm::vec4(center - 0.25f * front, 1.0f));
		vertices.push_back(glm::vec4(center + 0.75f * front, 1.0f));

		vertices.push_back(glm::vec4(center - 0.25f * up, 1.0f));
		vertices.push_back(glm::vec4(center + 0.25f * up, 1.0f));

		// Add all faces of boid
		faces.push_back(glm::uvec3(vertex_base_index,     vertex_base_index + 3, vertex_base_index + 5));
		faces.push_back(glm::uvec3(vertex_base_index + 3, vertex_base_index + 1, vertex_base_index + 5));
		faces.push_back(glm::uvec3(vertex_base_index,     vertex_base_index + 5, vertex_base_index + 2));
		faces.push_back(glm::uvec3(vertex_base_index + 2, vertex_base_index + 5, vertex_base_index + 1));

		faces.push_back(glm::uvec3(vertex_base_index,     vertex_base_index + 4, vertex_base_index + 3));
		faces.push_back(glm::uvec3(vertex_base_index + 4, vertex_base_index + 1, vertex_base_index + 3));
		faces.push_back(glm::uvec3(vertex_base_index,     vertex_base_index + 2, vertex_base_index + 4));
		faces.push_back(glm::uvec3(vertex_base_index + 2, vertex_base_index + 1, vertex_base_index + 4));
	}

	void update(std::vector<glm::vec4>& vertices, std::vector<Boid*> boids) {
		glm::vec3 v1 = cohesion(boids);


		velocity = velocity + v1;
		velocity = limit_velocity(velocity); 

		glm::vec3 new_front = glm::normalize(velocity);
		//new_front = front + 0.01f * right + 0.01f * front + 0.01f * up;
		glm::quat q = RotationBetweenVectors(front, new_front);

		for (int i = 0; i < 6; i ++) {
			vertices[vertex_base_index + i] = glm::vec4(center + glm::vec3(q * glm::vec4(glm::vec3(vertices[vertex_base_index + i]) - center, 1.0f)), 1.0f);
		}

		front = glm::normalize(glm::vec3(q * glm::vec4(front, 1.0f)));
		up = glm::normalize(glm::vec3(q * glm::vec4(up, 1.0f)));
		right = glm::normalize(glm::vec3(q * glm::vec4(right, 1.0f)));

		for (int i = 0; i < 6; i ++) {
			vertices[vertex_base_index + i] = glm::vec4(glm::vec3(vertices[vertex_base_index + i]) + velocity, 1.0f);
		}

		center = center + velocity;
	}

	glm::vec3 limit_velocity(glm::vec3 v) {
		if (glm::length(v) > velocity_limit) {
			return velocity_limit * glm::normalize(v);
		}
		return v;
	}

	glm::vec3 cohesion(std::vector<Boid*> boids) {
		glm::vec3 avg_position = glm::vec3(0.0f, 0.0f, 0.0f);

		for (int i = 0; i < boids.size(); i ++) {
			if (i != index) {
				avg_position += boids[i]->center;
			}
		}

		avg_position /= ((float) (boids.size() - 1));

		return (avg_position - center) / 100.0f;
	}

	glm::quat RotationBetweenVectors(glm::vec3 start, glm::vec3 dest){
		start = glm::normalize(start);
		dest = glm::normalize(dest);

		float cosTheta = glm::dot(start, dest);
		glm::vec3 rotationAxis;

		if (cosTheta < -1 + 0.001f){
			// special case when vectors in opposite directions:
			// there is no "ideal" rotation axis
			// So guess one; any will do as long as it's perpendicular to start
			rotationAxis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), start);
			if (glm::length2(rotationAxis) < 0.01 ) // bad luck, they were parallel, try again!
				rotationAxis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), start);

			rotationAxis = normalize(rotationAxis);
			return glm::angleAxis(glm::radians(180.0f), rotationAxis);
		}

		rotationAxis = glm::cross(start, dest);

		float s = glm::sqrt( (1+cosTheta)*2 );
		float invs = 1 / s;

		return glm::quat(
			s * 0.5f, 
			rotationAxis.x * invs,
			rotationAxis.y * invs,
			rotationAxis.z * invs
		);

	}

	~Boid();
	float velocity_limit = 0.01f;
	int index;
	int vertex_base_index;
	glm::vec3 center;
	glm::vec3 velocity;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;

	//std::vector<glm::vec4> vertices;
	//std::vector<glm::uvec3> faces;
};

#endif
