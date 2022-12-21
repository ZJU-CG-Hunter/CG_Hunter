#include <HHunter.h>


HHunter::HHunter(string const& path, const glm::vec3 front, const glm::vec3 up, const glm::vec3 right, const glm::vec3 worldup, float yaw, float pitch) : HModel(path, false) {
	Front = front;
	Up = up;
	Right = right;
	WorldUp = worldup;
	Yaw = yaw;
	Pitch = pitch;
}

void HHunter::Event(Collision event) {
	if (event._is_collide) {
		cout << "Collision happens!" << endl;
		position = last_position;
	}

}


void HHunter::move(Camera_Movement dirention, float deltaTime) {
	float velocity = MovementSpeed * deltaTime;
	last_position = position;

	switch (dirention) {
	case Camera_Movement::FORWARD:
		position += Front * velocity;
		break;
	case Camera_Movement::BACKWARD:
		position -= Front * velocity;
		break;
	case Camera_Movement::LEFT:
		position -= Right * velocity;
		break;
	case Camera_Movement::RIGHT:
		position += Right * velocity;
		break;
	}

	//cout << "current_pos: " << position.x << " " << position.y << " " << position.z << endl;

	update_camera();
}



void HHunter::turn(float xoffset, float yoffset) {
	xoffset *= MouseSensitivity;
	yoffset *= MouseSensitivity;

	Yaw += xoffset;
	Pitch += yoffset;

	if (Pitch > 89.0f)
		Pitch = 89.0f;
	if (Pitch < -89.0f)
		Pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));

	Front = glm::normalize(front);
	Right = glm::normalize(glm::cross(Front, WorldUp));
	Up = glm::normalize(glm::cross(Right, Front));

	camera->Front = Front;
	camera->Up = Up;
	camera->Right = Right;

	update_camera();

}

void HHunter::update_camera(){
	glm::vec3 back = glm::cross(Right, WorldUp);
	back = glm::normalize(back);
	camera->Position = position + back * glm::vec3(10.0f, 10.0f, 10.0f) + WorldUp * glm::vec3(5.0f, 5.0f, 5.0f);
}


