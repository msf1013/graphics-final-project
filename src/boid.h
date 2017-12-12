#ifndef BOID_H
#define BOID_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/norm.hpp> 
#include <vector>
#include <random>
#include "obstacle.h";

float lim = 40.0f;

class Boid {
public:
	float rand_d() {
		return 2.0f * (((double) rand() / (RAND_MAX)) + 1.0) - 1.0f;
	}

	Boid(float x, float y, float z, std::vector<glm::vec4>& vertices, std::vector<glm::uvec3>& faces, int index) {
		this->index = index;
		center = glm::vec3(x, y, z);

		// Randomize velocity
		velocity = glm::vec3(rand_d(), rand_d(), rand_d());
		velocity = glm::normalize(velocity);
		front = velocity;
		velocity *= 2.0f;

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

	void update(std::vector<glm::vec4>& vertices, std::vector<Boid*> boids, std::vector<Obstacle*> obstacles) {
		glm::vec3 v1 = cohesion(boids);
		glm::vec3 v2 = separation(boids);
		glm::vec3 v3 = alignment(boids);
		glm::vec3 v4 = avoid_obstacles(obstacles);
		glm::vec3 v5 = bound_position();

		velocity = velocity + v1 + v2 + v3 + v4 + v5;
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
		float count = 0.0f;

		for (int i = 0; i < boids.size(); i ++) {
			if (i != index && glm::length((boids[i]->center) - center) < 10.0f) {
				count = count + 1.0f;
				avg_position += boids[i]->center;
			}
		}

		if (count > 0.0f) {
			avg_position /= count;

			return (avg_position - center) / 100.0f;
		}

		return glm::vec3(0.0f, 0.0f, 0.0f);
	}

	glm::vec3 separation(std::vector<Boid*> boids) {
		glm::vec3 displacement = glm::vec3(0.0f, 0.0f, 0.0f);

		for (int i = 0; i < boids.size(); i ++) {
			float d = glm::length((boids[i]->center) - center);
			if (i != index && d < 2.0f) {
				glm::vec3 sample = center - (boids[i]->center);
				sample /= d;

				displacement += sample;
			}
		}

		return displacement;
	}

	glm::vec3 alignment(std::vector<Boid*> boids) {
		glm::vec3 orientation = glm::vec3(0.0f, 0.0f, 0.0f);
		float count = 0.0f;

		for (int i = 0; i < boids.size(); i ++) {
			float d = glm::length((boids[i]->center) - center);
			if (i != index && d < 10.0f) {
				glm::vec3 sample = boids[i]->velocity;
				sample /= d;
				count = count + 1.0f;
				orientation += sample;
			}
		}

		return orientation / 50.0f;
	}

	glm::vec3 bound_position() {
		if (glm::length(center) < 70.0f) {
			return glm::vec3(0.0f, 0.0f, 0.0f);
		}

		return -20.0f * glm::normalize(center); // * (((float) rand() / (RAND_MAX)) + 1.0f);


		/*glm::vec3 orientation = glm::vec3(0.0f, 0.0f, 0.0f);


		if (center.x < -lim) {
			orientation.x = (((double) rand() / (RAND_MAX)) + 1.0);
		} else if (center.x > lim) {
			orientation.x = -(((double) rand() / (RAND_MAX)) + 1.0);
		}

		if (center.y < -lim) {
			orientation.y = (((double) rand() / (RAND_MAX)) + 1.0);
		} else if (center.y > lim) {
			orientation.y = -(((double) rand() / (RAND_MAX)) + 1.0);
		}

		if (center.z < -lim) {
			orientation.z = (((double) rand() / (RAND_MAX)) + 1.0);
		} else if (center.z > lim) {
			orientation.z = -(((double) rand() / (RAND_MAX)) + 1.0);
		}
		
		return orientation;*/
	}

	glm::vec3 avoid_obstacles(std::vector<Obstacle*> obstacles) {
		glm::vec3 displacement = glm::vec3(0.0f, 0.0f, 0.0f);

		//return displacement;

		for (int i = 0; i < obstacles.size(); i ++) {
			float d = glm::length((obstacles[i]->center) - center);
			if (d < obstacles[i]->radius * 3.0f) {
				glm::vec3 sample = center - (obstacles[i]->center);
				glm::vec3 perpendicular;

				if (sample.y != 0.0f && sample.z != 0.0f) {
					perpendicular = glm::cross(sample, glm::vec3(1.0f, 0.0f, 0.0f));
				} else {
					perpendicular = glm::cross(sample, glm::vec3(0.0f, 1.0f, 0.0f));
				}

				perpendicular = glm::normalize(perpendicular) / d;

				displacement += perpendicular;
			}
		}

		return displacement * 60.0f;
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
	float velocity_limit = 0.6f;
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
