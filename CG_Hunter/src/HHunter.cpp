#include <HHunter.h>
#include <HMap.h>


HHunter::HHunter(string const& path, const glm::vec3 front, const glm::vec3 up, const glm::vec3 right, const glm::vec3 worldup, float yaw, float pitch) : HModel(path, false, Hunter_Idle) {

	cout << "Num: " << scene->mNumAnimations << endl;

	Front = front;
	Up = up;
	Right = right;
	WorldUp = worldup;
	Yaw = yaw;
	Pitch = pitch;

	double pi = 3.1415926;
	double c = pi / 180;
	glm::vec3 a[721];
	int index = 0;
	for (size_t i = 0; i < 360; i++)
	{
		glm::vec3 temp1(0.8 * cos(c * i), 0.8 * sin(c * i), 0);
		a[index++] = temp1;

		if (i >= 315 || i < 45)
		{
			glm::vec3 temp2(1, tan(c * i), 0);
			a[index++] = temp2;
		}
		else if (i >= 45 && i < 135)
		{
			glm::vec3 temp2(1 / tan(c * i), 1, 0);
			a[index++] = temp2;
		}
		else if (i >= 135 && i < 225)
		{
			glm::vec3 temp2(-1, -tan(c * i), 0);
			a[index++] = temp2;
		}
		else if (i >= 225 && i < 315)
		{
			glm::vec3 temp2(-1 / tan(c * i), -1, 0);
			a[index++] = temp2;
		}
	}

	vector<float> cross_mag{
		-1.0, 0.95, 0.0,
		0.0, 0.0, 0.0,
		-0.95, 1.0, 0.0,

		1.0, 0.95, 0.0,
		0.0, 0.0, 0.0,
		0.95, 1.0, 0.0,

		-1.0, -0.95, 0.0,
		0.0, 0.0, 0.0,
		-0.95, -1.0, 0.0,

		1.0, -0.95, 0.0,
		0.0, 0.0, 0.0,
		0.95, -1.0, 0.0,
	};

	for (int i = 0; i < 719; i++)
		for (int j = 0; j < 3; j++)
			for (int k = 0; k < 3; k++)
				magnifier_vertices.emplace_back(a[i + j][k]);
	for (int i = 0; i < cross_mag.size(); i++)
		magnifier_vertices.emplace_back(cross_mag[i]);

	glGenVertexArrays(1, &magnifier_VAO);
	glGenBuffers(1, &magnifier_VBO);
	glBindVertexArray(magnifier_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, magnifier_VBO);
	glBufferData(GL_ARRAY_BUFFER, magnifier_vertices.size() * sizeof(float), &magnifier_vertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

}

void HHunter::Action(HMap* map, float duration_time) {


	switch (animation_index) {
	case INVALID_ANIMATION_INDEX:
		cout << "Invalid_animation_index" << endl;
		assert(0);
	case Hunter_Run:
		CalCurrentTicks(duration_time);
		// Iterative do this
		animation_ticks = fmod(animation_ticks, scene->mAnimations[animation_index]->mDuration);
		break;
	case Hunter_Idle:
		CalCurrentTicks(duration_time);
		// Iterative do this
		animation_ticks = fmod(animation_ticks, scene->mAnimations[animation_index]->mDuration);
		break;
	}
	UpdateBoneTransform();
	//UpdateColliderTransform();
	AdjustStepOnGround(map);
	map->update_model(this);
	update_camera();
}

void HHunter::Event(Events* event) {
	switch (event->_event_type) {
	case Event_Type::Collision:
		Collision* collision_event = reinterpret_cast<Collision*>(event);
		HModel* another_model = collision_event->_model_1 == this ? collision_event->_model_2 : collision_event->_model_1;
		if (collision_event->_is_collide && another_model->get_model_type() != Model_Type::Bullet){
			//cout << "Collision happens!" << endl;
			position = last_position;
		}
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
	animation_index = Hunter_Run;

	update_camera();
}

void HHunter::idle() {
	animation_index = Hunter_Idle;
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

	rotation.y -= xoffset;

	camera->Front = Front;
	camera->Up = Up;
	camera->Right = Right;

	update_camera();

}

void HHunter::update_camera(){
	if (!is_aim) {
		glm::vec3 back = glm::cross(Right, WorldUp);
		back = glm::normalize(back);
		camera->Position = position + back * glm::vec3(8.0f, 8.0f, 8.0f) + WorldUp * glm::vec3(8.0f, 8.0f, 8.0f);
	}
	else {
		camera->Position = position + Front * glm::vec3(5.0f, 5.0f, 5.0f);
	}
}

void HHunter::collision_detection(HMap* _map) {
	vector<Model_Data> nearby;

	if (engine_detect_collision) {
		nearby = _map->get_model_nearby(this, 5.0f);

		for (int i = 0; i < nearby.size(); i++) {
			if (nearby[i]._adjust_pos)
				nearby[i]._model->SetPosition(*nearby[i]._adjust_pos);

			Collision* collision_type = get_collide_type(this, nearby[i]._model);
			this->Event(collision_type);
			nearby[i]._model->Event(collision_type);
			delete collision_type;
		}
	}
}

void HHunter::BindGun(HModel* gun) {
	this->gun = reinterpret_cast<HBullet*>(gun);
}

void HHunter::aim(bool is_aim) {
	this->is_aim = is_aim;
	if (is_aim)
		camera->Zoom = ZOOM - 15.0f;
	else
		camera->Zoom = ZOOM;
}

void HHunter::shoot() {
	glm::vec3 gum_position = position + Front * glm::vec3(5.0f, 5.0f, 5.0f);

	glm::vec3 radians_roation1(0.0f, glm::radians(-Yaw), 0.0f);
	glm::vec3 radians_roation2(0.0f, 0.0f, glm::radians(Pitch));

	//gun->insert_bullet(bullet(gum_position, camera->Front, glm::mat4_cast(glm::quat(radians_roation)), 1.0f));
	gun->insert_bullet(bullet(gum_position, camera->Front, glm::mat4_cast(glm::quat(radians_roation1)) * glm::mat4_cast(glm::quat(radians_roation2)), 5.0f));
}

void HHunter::BindMagnifierShader(HShader* magnifier_shader) {
	this->magnifier_shader = magnifier_shader;
}

void HHunter::DrawMagnifier() {
	if (is_aim) {
		magnifier_shader->use();
		glBindVertexArray(magnifier_VAO);
		glDrawArrays(GL_TRIANGLES, 0, magnifier_vertices.size());
		glBindVertexArray(0);
	}
}

void HHunter::shoot_ready(bool up_down) {
	if (up_down) {
		animation_index = Hunter_Shoot_Up;
	}
	else {

	}
}

