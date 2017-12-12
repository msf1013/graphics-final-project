#ifndef BOID_H
#define BOID_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/norm.hpp> 
#include <vector>
#include <random>
#include "obstacle.h"

class Boid {
public:
	// Constructor method that creates a new Boid whose center is given by the provided x, y, and z coordinates.
	// The Boid's vertices and faces are added to the scene.
	Boid(float x, float y, float z, std::vector<glm::vec4>& vertices, std::vector<glm::uvec3>& faces, int index) {
		this->index = index;
		center = glm::vec3(x, y, z);

		// Generate random velocity.
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

	// Method that updates the Boid's position and velocity according to the
	// rules of the flock.
	void update(std::vector<glm::vec4>& vertices, std::vector<Boid*> boids, std::vector<Obstacle*> obstacles) {
		// Calculate contributions of all rules.
		glm::vec3 v1 = cohesion(boids);
		glm::vec3 v2 = separation(boids);
		glm::vec3 v3 = alignment(boids);
		glm::vec3 v4 = avoid_obstacles(obstacles);
		glm::vec3 v5 = bound_position();

		// Update velocity with contributions.
		velocity = velocity + v1 + v2 + v3 + v4 + v5;
		velocity = limit_velocity(velocity); 

		// Get rotation transformation that will move our original velocity
		// to the new velocity obtained after applying the flock rules.
		glm::vec3 new_front = glm::normalize(velocity);
		glm::quat q = rotation_between_vectors(front, new_front);

		// Apply rotation to all vertices in Boid.
		for (int i = 0; i < 6; i ++) {
			vertices[vertex_base_index + i] = glm::vec4(center + glm::vec3(q * glm::vec4(glm::vec3(vertices[vertex_base_index + i]) - center, 1.0f)), 1.0f);
		}

		front = glm::normalize(glm::vec3(q * glm::vec4(front, 1.0f)));
		up = glm::normalize(glm::vec3(q * glm::vec4(up, 1.0f)));
		right = glm::normalize(glm::vec3(q * glm::vec4(right, 1.0f)));

		// Apply translation to all vertices in Boid.
		for (int i = 0; i < 6; i ++) {
			vertices[vertex_base_index + i] = glm::vec4(glm::vec3(vertices[vertex_base_index + i]) + velocity, 1.0f);
		}

		center = center + velocity;
	}

	// Forbid boid from going faster than the limit.
	glm::vec3 limit_velocity(glm::vec3 v) {
		if (glm::length(v) > velocity_limit) {
			return velocity_limit * glm::normalize(v);
		}
		return v;
	}

	// Cohesion rule: generate vector that moves Boid towards center of mass of
	// neighboring flockmates.
	glm::vec3 cohesion(std::vector<Boid*> boids) {
		glm::vec3 avg_position = glm::vec3(0.0f, 0.0f, 0.0f);
		float count = 0.0f;

		// Iterate over list of Boids.
		for (unsigned int i = 0; i < boids.size(); i ++) {
			// Detect nearby Boids.
			if (i != index && glm::length((boids[i]->center) - center) < 10.0f) {
				count = count + 1.0f;
				avg_position += boids[i]->center;
			}
		}

		// Get average position of nearby Boids, if any.
		if (count > 0.0f) {
			avg_position /= count;
			return (avg_position - center) / 100.0f;
		}

		return glm::vec3(0.0f, 0.0f, 0.0f);
	}

	// Separation rule: generate vector that moves Boid away from nearby
	// flockmates in order to prevent crowding.
	glm::vec3 separation(std::vector<Boid*> boids) {
		glm::vec3 displacement = glm::vec3(0.0f, 0.0f, 0.0f);

		// Iterate over list of Boids.
		for (unsigned int i = 0; i < boids.size(); i ++) {
			float d = glm::length((boids[i]->center) - center);

			// Detect nearby Boids.
			if (i != index && d < 2.0f) {
				glm::vec3 sample = center - (boids[i]->center);
				sample /= d;
				displacement += sample;
			}
		}

		return displacement;
	}

	// Alignment rule: generate vector that makes the Boid point
	// towards the average position where nearby flockmates point to.
	glm::vec3 alignment(std::vector<Boid*> boids) {
		glm::vec3 orientation = glm::vec3(0.0f, 0.0f, 0.0f);
		float count = 0.0f;

		// Iterate over list of Boids.
		for (unsigned int i = 0; i < boids.size(); i ++) {
			float d = glm::length((boids[i]->center) - center);

			// Detect nearby Boids and add their velocity.
			if (i != index && d < 10.0f) {
				glm::vec3 sample = boids[i]->velocity;
				sample /= d;
				orientation += sample;
			}
		}

		return orientation / 50.0f;
	}

	// Bound position rule: restrict Boids to remain within a distance of
	// 70 units from the origin.
	glm::vec3 bound_position() {
		if (glm::length(center) < 70.0f) {
			return glm::vec3(0.0f, 0.0f, 0.0f);
		}

		return -20.0f * glm::normalize(center);
	}

	// Obstacle avoidance rule: generate vector that makes the Boid move to
	// a perpendicular direction with respect to an Obstacle's position, in order
	// to prevent it from crashing into it.
	glm::vec3 avoid_obstacles(std::vector<Obstacle*> obstacles) {
		glm::vec3 displacement = glm::vec3(0.0f, 0.0f, 0.0f);

		// Iterate over list of obstacles.
		for (unsigned int i = 0; i < obstacles.size(); i ++) {
			float d = glm::length((obstacles[i]->center) - center);
			
			// Detect obstacles that are getting closer.
			if (d < obstacles[i]->radius * 3.0f) {
				glm::vec3 sample = center - (obstacles[i]->center);
				glm::vec3 perpendicular;

				// Generate arbitrary perpendicular vector.
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

	// Method that calculates the quaternion that describes the rotation between two vectors.
	//
	// Retrieved from: http://www.opengl-tutorial.org/es/intermediate-tutorials/tutorial-17-quaternions/
	glm::quat rotation_between_vectors(glm::vec3 start, glm::vec3 end){
		start = glm::normalize(start);
		end = glm::normalize(end);

		float cos_ = glm::dot(start, end);
		glm::vec3 rotation_axis;

		// Edge case: vectors point in opposite directions.
		if (cos_ < -1.001f){
			rotation_axis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), start);
			
			// Find another rotation axis.
			if (glm::length2(rotation_axis) < 0.01f ) {
				rotation_axis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), start);
			}

			rotation_axis = normalize(rotation_axis);
			return glm::angleAxis(glm::radians(180.0f), rotation_axis);
		}

		rotation_axis = glm::cross(start, end);

		float s = glm::sqrt((1.0f + cos_) * 2.0f);

		return glm::quat(s*0.5f, rotation_axis.x / s, rotation_axis.y / s, rotation_axis.z / s);
	}

	// Custom random function that returns a float between -1 and 1.
	float rand_d() {
		return 2.0f * (((double) rand() / (RAND_MAX)) + 1.0) - 1.0f;
	}

	~Boid();
	float velocity_limit = 0.6f;
	unsigned int index;
	int vertex_base_index;
	glm::vec3 center;
	glm::vec3 velocity;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
};

#endif
