#ifndef BOID_H
#define BOID_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/norm.hpp> 
#include <vector>

class Boid {
public:
	Boid() {
		center = glm::vec3(5.0f, 0.0f, 0.0f);
		velocity = glm::vec3(0.0f, 0.0f, -1.0f);
		front = glm::vec3(0.0f, 0.0f, -1.0f);
		up = glm::vec3(0.0f, 1.0f, 0.0f);
		right = glm::vec3(1.0f, 0.0f, 0.0f);

		// Add all vertices of boid
		vertices.push_back(glm::vec4(center - 0.5f * right, 1.0f));
		vertices.push_back(glm::vec4(center + 0.5f * right, 1.0f));

		vertices.push_back(glm::vec4(center - 0.25f * front, 1.0f));
		vertices.push_back(glm::vec4(center + 0.75f * front, 1.0f));

		vertices.push_back(glm::vec4(center - 0.25f * up, 1.0f));
		vertices.push_back(glm::vec4(center + 0.25f * up, 1.0f));

		// Add all faces of boid
		faces.push_back(glm::uvec3(0, 3, 5));
		faces.push_back(glm::uvec3(3, 1, 5));
		faces.push_back(glm::uvec3(0, 5, 2));
		faces.push_back(glm::uvec3(2, 5, 1));

		faces.push_back(glm::uvec3(0, 4, 3));
		faces.push_back(glm::uvec3(4, 1, 3));
		faces.push_back(glm::uvec3(0, 2, 4));
		faces.push_back(glm::uvec3(2, 1, 4));
	}

	void update() {
		glm::vec3 new_front = front;
		new_front = front + 0.01f * right + 0.01f * front + 0.01f * up;
		glm::quat q = RotationBetweenVectors(front, new_front);

		for (int i = 0; i < vertices.size(); i ++) {
			vertices[i] = glm::vec4(center + glm::vec3(q * glm::vec4(glm::vec3(vertices[i]) - center, 1.0f)), 1.0f);
		}

		front = glm::normalize(glm::vec3(q * glm::vec4(front, 1.0f)));
		up = glm::normalize(glm::vec3(q * glm::vec4(up, 1.0f)));
		right = glm::normalize(glm::vec3(q * glm::vec4(right, 1.0f)));

		for (int i = 0; i < vertices.size(); i ++) {
			vertices[i] = glm::vec4(glm::vec3(vertices[i]) + 0.001f * front, 1.0f);
		}
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
	glm::vec3 center;
	glm::vec3 velocity;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;

	std::vector<glm::vec4> vertices;
	std::vector<glm::uvec3> faces;
};

#endif
