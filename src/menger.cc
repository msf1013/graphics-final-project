#include "menger.h"
#include <iostream>

namespace {
	const int kMinLevel = 0;
	const int kMaxLevel = 4;
};

Menger::Menger()
{
	// Add additional initialization if you like
}

Menger::~Menger()
{
}

void
Menger::set_nesting_level(int level)
{
	nesting_level_ = level;
	dirty_ = true;
}

bool
Menger::is_dirty() const
{
	return dirty_;
}

void
Menger::set_clean()
{
	dirty_ = false;
}

void
Menger::generate_geometry(std::vector<glm::vec4>& obj_vertices,
                          std::vector<glm::uvec3>& obj_faces) const
{
	CreateMenger(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, 0.5f, 0.5f), obj_vertices, obj_faces, 0);
}

// Create Menger sponge geometry (vertices and faces) given boundaries and recursion level. 
void
Menger::CreateMenger(glm::vec3 min,
				     glm::vec3 max,
					 std::vector<glm::vec4>& obj_vertices,
                     std::vector<glm::uvec3>& obj_faces,
                     int level) const
{
	float delta = max.x - min.x;

	glm::vec3 uX(1.0f, 0.0f, 0.0f);
	glm::vec3 uY(0.0f, 1.0f, 0.0f);
	glm::vec3 uZ(0.0f, 0.0f, 1.0f);


	// Termination condition: append cube's vertices and faces right away and stop recursion.
	if (level == nesting_level_) {
		// Add vertices
		obj_vertices.push_back(glm::vec4(min, 1.0f));
		obj_vertices.push_back(glm::vec4(min + glm::vec3(delta, 0.0f, 0.0f), 1.0f));
		obj_vertices.push_back(glm::vec4(min + glm::vec3(delta, 0.0f, 0.0f) + glm::vec3(0.0f, 0.0f, delta), 1.0f));
		obj_vertices.push_back(glm::vec4(min + glm::vec3(0.0f, 0.0f, delta), 1.0f));

		obj_vertices.push_back(glm::vec4(max, 1.0f));
		obj_vertices.push_back(glm::vec4(max - glm::vec3(delta, 0.0f, 0.0f), 1.0f));
		obj_vertices.push_back(glm::vec4(max - glm::vec3(delta, 0.0f, 0.0f) - glm::vec3(0.0f, 0.0f, delta), 1.0f));
		obj_vertices.push_back(glm::vec4(max - glm::vec3(0.0f, 0.0f, delta), 1.0f));

		// Add faces
		int baseIndex = obj_vertices.size() - 8;

		obj_faces.push_back(glm::uvec3(baseIndex + 1, baseIndex + 7, baseIndex + 4));
		obj_faces.push_back(glm::uvec3(baseIndex + 1, baseIndex + 4, baseIndex + 2));

		obj_faces.push_back(glm::uvec3(baseIndex + 5, baseIndex + 6, baseIndex));
		obj_faces.push_back(glm::uvec3(baseIndex + 5, baseIndex, baseIndex + 3));

		obj_faces.push_back(glm::uvec3(baseIndex + 2, baseIndex + 4, baseIndex + 5));
		obj_faces.push_back(glm::uvec3(baseIndex + 2, baseIndex + 5, baseIndex + 3));

		obj_faces.push_back(glm::uvec3(baseIndex, baseIndex + 6, baseIndex + 7));
		obj_faces.push_back(glm::uvec3(baseIndex, baseIndex + 7, baseIndex + 1));

		obj_faces.push_back(glm::uvec3(baseIndex + 4, baseIndex + 7, baseIndex + 6));
		obj_faces.push_back(glm::uvec3(baseIndex + 4, baseIndex + 6, baseIndex + 5));

		obj_faces.push_back(glm::uvec3(baseIndex, baseIndex + 1, baseIndex + 2));
		obj_faces.push_back(glm::uvec3(baseIndex, baseIndex + 2, baseIndex + 3));

		return;
	}

	float deltaP = delta / 3.0f;

	// Check sub-cubes and recursively create geometry for each of them, in case the given sub-cube is a valid one.
	for (int i = 0; i < 3; i ++) {
		for (int j = 0; j < 3; j ++) {
			for (int k = 0; k < 3; k ++) {
				if (!ignoreSubCube(i, j, k)) {
					// New min/max boundaries for sub-cube
					glm::vec3 newMin = min + ((float)i * uX * deltaP) + ((float)j * uY * deltaP) + ((float)k * uZ * deltaP);
					glm::vec3 newMax = newMin + (uX * deltaP) + (uY * deltaP) + (uZ * deltaP);
					CreateMenger(newMin, newMax, obj_vertices, obj_faces, level + 1);
				}
			}
		}
	}
}

// Check if sub-cube represented by given indeces should be ignored.
bool
Menger::ignoreSubCube(int i, int j, int k) const {
	// List of invalid faces.
	if ((i == 0 && j == 1 && k == 1) ||
		(i == 1 && j == 0 && k == 1) ||
		(i == 1 && j == 1 && k == 0) ||
		(i == 1 && j == 1 && k == 1) ||
		(i == 1 && j == 1 && k == 2) ||
		(i == 1 && j == 2 && k == 1) ||
		(i == 2 && j == 1 && k == 1)) {
		return true;
	}
	return false;
}
