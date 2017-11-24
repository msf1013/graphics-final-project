#include "camera.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace {
	float pan_speed = 0.1f;
	float roll_speed = 2.0f;
	float rotation_speed = 5.0f;
	float zoom_speed = 0.05f;
};


// Method definition was updated to account for user controls: this method is provided with the action and
// the potential parameters required for the given action, and returns the view matrix that results.
// after applying the given action.
glm::mat4 Camera::get_view_matrix(Action action, bool fpsMode, glm::vec2 dir, float magnitude)
{
	// Depending on the user-control action, the matrix will be calculated differently.
	switch (action) {
		case rotation: {
			// Avoid rotation in case that there is no valid drag direction.
			if (!(dir.x == 0.0f && dir.y == 0.0f)) {
				// Calculate rotation axis based on drag direction.
				glm::vec3 right = - glm::cross(glm::normalize(look_), glm::normalize(up_));
				glm::vec3 drag  = dir.x * right + dir.y * glm::normalize(up_);
				glm::vec3 rotationAxis = glm::cross(-drag, glm::normalize(look_));

				// Update look and up directions
				look_ = glm::rotate(look_, glm::radians(rotation_speed), rotationAxis);
				up_ = glm::rotate(up_, glm::radians(rotation_speed), rotationAxis);

				if (fpsMode) {
					// For FPS camera, the center will be recalculated.
					center_ = eye_ + camera_distance_ * look_;
				} else {
					// For orbital camera, the eye will be recalculated.
					eye_ = center_ - camera_distance_ * look_;
				}
			}
			break;
		}
		case none:
			// Do nothing: view matrix parameters won't be changed.
			break;
		case zoom_mouse:
			// Decrease/increase camera distance.
			camera_distance_ += zoom_speed * dir.y;
			// Prevent camera_distance from going negative
			if (camera_distance_ < 0.01) {
				camera_distance_ = 0.01;
			}
			eye_  = center_ - camera_distance_ * look_;
			break;
		case zoom_keys:
			if (fpsMode) {
				// Translate eye and center in the look direction in FPS mode.
				eye_ = eye_ - look_ * magnitude;
				center_ = center_ - look_ * magnitude;
			} else {
				// Decrease/increase camera distance in orbital mode.
				camera_distance_ += zoom_speed * magnitude;
				// Prevent camera_distance from going negative
				if (camera_distance_ < 0.01) {
					camera_distance_ = 0.01;
				}
				eye_  = center_ - camera_distance_ * look_;
			}
			break;
		case strafe: {
			// Translate both eye and center in the right direction.
			glm::vec3 right = - glm::cross(glm::normalize(look_), glm::normalize(up_));
			if (fpsMode) {
				eye_ = eye_ - right * magnitude * pan_speed;
				center_ = eye_ + camera_distance_ * look_;
			} else {
				center_ = center_ - right * magnitude * pan_speed;
				eye_ = center_ - camera_distance_ * look_;	
			}
			break;
		}
		case translation:
			// Translate both eye and center in the up direction.
			if (fpsMode) {
				eye_ = eye_ + up_ * magnitude * pan_speed;
				center_ = eye_ + camera_distance_ * look_;
			} else {
				center_ = center_ + up_ * magnitude * pan_speed;
				eye_ = center_ - camera_distance_ * look_;
			}
			break;
		case roll:
			// Roll around up direction.
			up_ = glm::rotate(up_, glm::radians(magnitude * roll_speed), look_);
			break;
	}

	// Calculate look-at matrix with updated parameters.
	return LookAt(eye_, center_, up_);
}

// Inspired by https://www.3dgep.com/understanding-the-view-matrix/#Look_At_Camera.
// The trick consists of combining orientation and translation transforms in a single matrix.
glm::mat4 Camera::LookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up) const {
	glm::vec3 zAxis = glm::normalize(eye - center); // Look.
	glm::vec3 xAxis = glm::normalize(glm::cross(up, zAxis)); // Right.
	glm::vec3 yAxis = glm::cross(zAxis, xAxis); // Up.

	glm::mat4 view = {
		glm::vec4(xAxis.x, yAxis.x, zAxis.x, 0),
		glm::vec4(xAxis.y, yAxis.y, zAxis.y, 0),
		glm::vec4(xAxis.z, yAxis.z, zAxis.z, 0),
		glm::vec4(-glm::dot(xAxis, eye), -glm::dot(yAxis, eye), -glm::dot(zAxis, eye), 1)
	};

	return view;
}
