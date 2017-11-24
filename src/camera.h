#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

class Camera {
public:
	enum Action { none = 0, rotation = 1, zoom_mouse = 2, zoom_keys = 3, strafe = 4, translation = 5, roll = 6 };
	Camera(float camera_distance, glm::vec3 look, glm::vec3 up) {
		camera_distance_ = camera_distance;
		eye_ = glm::vec3(0.0f, 0.0f, camera_distance_);
		look_ = look;
		up_ = up;
		center_ = eye_ + camera_distance_ * look_;
	}
	glm::mat4 get_view_matrix(Action action, bool fpsMode, glm::vec2 dir, float magnitude);
	glm::mat4 LookAt(glm::vec3 eye, glm::vec3 target, glm::vec3 up) const;
private:
	float camera_distance_ = 1.0;
	glm::vec3 look_ = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up_ = glm::vec3(0.0f, 1.0, 0.0f);
	glm::vec3 eye_ = glm::vec3(0.0f, 0.0f, camera_distance_);
	glm::vec3 center_ = eye_ + camera_distance_ * look_;
};

#endif
