#include <HEngine.h>




HEngine::HEngine(): _deltatime(0.0f), _lasttime(0.0f) , _current_window(-1), _lastX(SCR_WIDTH / 2.0f), _lastY(SCR_HEIGHT / 2.0f), _firstMouse(true) {
	ini_window_setting();
	ini_stb_setting();
	ini_enging_setting();
}

GLFWwindow* HEngine::get_window_ptr(const int index) {
	check_splited(index, _windows);

	return _windows[index];
}

void HEngine::setup_magnifier() {
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);

	magnifier_shader = new HShader("./resources/shader/vs/magnifier.vert", "./resources/shader/fs/magnifier.frag");
}

void HEngine::setup_depthMap() {
	glGenFramebuffers(1, &depthMapFBO);
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shadow_depth_shader = new HShader("./resources/shader/vs/shadow_empty.vert", "./resources/shader/fs/shadow_empty.frag");
}

void HEngine::draw_shadow() {
	// 1. render depth of scene to texture (from light's perspective)
	glm::vec3 hunter_position = _hunter->get_position();

	glm::vec3 lightPos = hunter_position + glm::vec3(0.0f, 50.0f, 0.0f);


	glm::mat4 lightProjection, lightView;
	glm::mat4 lightSpaceMatrix;
	float near_plane = 0.1f, far_plane = 200.0f;
	lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);
  lightView = glm::lookAt(lightPos, hunter_position, glm::vec3(0.0, 0.0, -1.0));
	lightSpaceMatrix = lightProjection * lightView;


	// render scene from light's point of view
	shadow_depth_shader->use();
	shadow_depth_shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	HShader* old_shaders;
	for (int i = 0; i < _models.size(); i++) {
		old_shaders = _models[i]->get_current_shader();
		_models[i]->BindShader(shadow_depth_shader);
		_models[i]->Draw();
		_models[i]->BindShader(old_shaders);
		
	}
	old_shaders = _hunter->get_current_shader();
	_hunter->BindShader(shadow_depth_shader);
	_hunter->Draw();
	_hunter->BindShader(old_shaders);



	old_shaders = _map->get_map_model()->get_current_shader();
	_map->get_map_model()->BindShader(shadow_depth_shader);
	_map->get_map_model()->Draw();
	_map->get_map_model()->BindShader(old_shaders);

	for (int i = 0; i < _map->get_draw_model().size(); i++) {
		old_shaders = _map->get_draw_model()[i]._model->get_current_shader();

		_map->get_draw_model()[i]._model->BindShader(shadow_depth_shader);

		_map->get_draw_model()[i]._model->SetPosition(* _map->get_draw_model()[i]._adjust_pos);
		_map->get_draw_model()[i]._model->Draw();
		_map->get_draw_model()[i]._model->BindShader(old_shaders);

	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// reset viewport
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	// set light uniforms
	glm::vec3 lightDir(0.0f, 1.0f, 0.0f);
	for (int i = 0; i < _shaders.size(); i++) {
		_shaders[i]->use();
		_shaders[i]->setVec3("cameraPos", _cameras[_current_camera]->Position);
		_shaders[i]->setVec3("lightDir", lightDir);
		_shaders[i]->setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		_shaders[i]->setMat4("lightSpaceMatrix", lightSpaceMatrix);
		_shaders[i]->setInt("shadowMap", DEPTH_MAP_TEXTURE);

	}
	
	glActiveTexture(GL_TEXTURE0 + DEPTH_MAP_TEXTURE);	
	glBindTexture(GL_TEXTURE_2D, depthMap);
	
}


void HEngine::insert_window(const unsigned int screen_width, const unsigned int screen_height, const char* title, GLFWmonitor* monitor, GLFWwindow* share) {
	GLFWwindow* created_window = glfwCreateWindow(screen_width, screen_height, title, monitor, share);
	if (created_window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		assert(0);
	}
	_windows.emplace_back(created_window);
}

int HEngine::current_window_index() {
	return _current_window;
}

void HEngine::set_current_window(int current_window) {
	check_splited(current_window, _windows);
	_current_window = current_window;
	glfwMakeContextCurrent(_windows[_current_window]);
	ini_glad();
	return;
}

void HEngine::set_framebuffersize_callback(int window_index, GLFWframebuffersizefun funptr) {
	check_splited(window_index, _windows);
	glfwSetFramebufferSizeCallback(_windows[window_index], funptr);
	return;
}

void HEngine::set_cursorpos_callback(int window_index, GLFWcursorposfun funptr) {
	check_splited(window_index, _windows);
	glfwSetCursorPosCallback(_windows[window_index], funptr);
}

void HEngine::set_scroll_callback(int window_index, GLFWscrollfun funptr) {
	check_splited(window_index, _windows);
	glfwSetScrollCallback(_windows[window_index], funptr);
}

void HEngine::set_mouse_button_callback(int window_index, GLFWmousebuttonfun funptr) {
	check_splited(window_index, _windows);
	glfwSetMouseButtonCallback(_windows[window_index], funptr);
}

void HEngine::set_cursor_hide(int window_index) {
	set_window_inputmode(window_index, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

}

void HEngine::draw_magnifier() {
	if (_is_magnifier_draw) {
		magnifier_shader->use();
		glBindVertexArray(magnifier_VAO);
		glDrawArrays(GL_TRIANGLES, 0, magnifier_vertices.size());
		glBindVertexArray(0);
	}
}

void HEngine::is_draw_magnifier(bool flag) {
	_is_magnifier_draw = flag;
}

void HEngine::shoot() {
	_bullet->SetEventType();

	glm::vec3 direction = get_camera(0)->Front;

	_bullet->SetBulletDirection(direction);

}

void HEngine::set_window_inputmode(int window_index, int mode, int value) {
	check_splited(window_index, _windows);
	glfwSetInputMode(_windows[window_index], mode, value);
}

void HEngine::insert_shader(const char* vertex_path, const char* fragment_path, const char* geometry_path) {
	HShader* created_shader = new HShader(vertex_path, fragment_path, geometry_path);
	_shaders.emplace_back(created_shader);
	return;
}

HShader* HEngine::get_shader(int shader_index) {
	check_splited(shader_index, _shaders);

	return _shaders[shader_index];
}

void HEngine::create_map(string const& path) {
	_map = new HMap(path);
}

HMap* HEngine::get_map() {
	return _map;
}

/* Create hunter */
void HEngine::create_hunter(string const& path, const glm::vec3 front, const glm::vec3 up, const glm::vec3 right, const glm::vec3 worldup, float yaw, float pitch) {
	_hunter = new HHunter(path, front, up, right, worldup, yaw, pitch);

	_hunter->SetModelType(Model_Type::Hunter);

	_hunter->SetEventType();

	_map->insert_model(_hunter);
}

/* Return the map */
HHunter* HEngine::get_hunter() {
	return _hunter;
}

void HEngine::create_bullet(string const& path, bool gamma) {
	_bullet = new HModel(path, gamma);

	_bullet->SetModelType(Model_Type::Bullet);

}

HModel* HEngine::get_bullet() {
	return _bullet;
}

void HEngine::insert_camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) {
	HCamera* created_camera = new HCamera(position, up, yaw, pitch);
	_cameras.emplace_back(created_camera);
	return;
}

HCamera* HEngine::get_camera(int camera_index) {
	check_splited(camera_index, _cameras);
	
	return _cameras[camera_index];
}

int HEngine::current_camera_index() {
	return _current_camera;
}

void HEngine::set_camera(int camera_index) {
	check_splited(camera_index, _cameras);
	
	_current_camera = camera_index;

	return;
}

void HEngine::insert_skybox(vector<float>& skyboxvertices, vector<string>& skyboxfaces) {
	HSkybox* created_skybox = new HSkybox(skyboxvertices, skyboxfaces);

	_skybox.emplace_back(created_skybox);

	return;
}

void HEngine::set_skybox(int skybox_index, string skybox_name) {
	check_splited(skybox_index, _skybox);

	_current_skybox = skybox_index;

	/* Bind Skybox and Shader*/
	for (unsigned int i = 0; i < _shaders.size(); i++) {
		_shaders[i]->use();
		_shaders[i]->setInt("skybox", SKYBOX_ID);
	}

	return;
}

HSkybox* HEngine::get_skybox(int skybox_index) {
	check_splited(skybox_index, _skybox);

	return _skybox[skybox_index];
}

Model_Type HEngine::findModelType(string const& path) {
	Model_Type model_type = Model_Type::Unknown;

	if (path.find("Pig") != string::npos || path.find("Sheep") != string::npos)
		model_type = Model_Type::Animal;

	// may not be used
	else if (path.find("bullet") != string::npos)
		model_type = Model_Type::Bullet;

	return model_type;
}

void HEngine::insert_model(string const& path, bool gamma) {
	HModel* created_model = new HModel(path, gamma);

	Model_Type model_type = findModelType(path);

	created_model->SetModelType(model_type);

	created_model->SetEventType();

	_models.emplace_back(created_model);

	_map->insert_model(created_model);

	created_model->AdjustStepOnGround(_map);
}

HModel* HEngine::get_model(int model_index) {
	check_splited(model_index, _models);
	
	return _models[model_index];
}

int HEngine::get_model_size() { 
	return _models.size(); 
}


/* Get the current mouse info */
void HEngine::get_mouse_xy(float& lastX, float& lastY) {
	lastX = _lastX;
	lastY = _lastY;
}

/* Set the current mouse info */
void HEngine::set_mouse_xy(float lastX, float lastY) {
	_lastX = lastX;
	_lastY = lastY;
}

/* Return is mouse first appeared */
bool HEngine::is_firstmouse() {
	return _firstMouse;
}

/* Set firstmosue */
void HEngine::set_firstmouse(bool firstMouse) {
	_firstMouse = firstMouse;
}


void HEngine::processInput() {
	GLFWwindow* window = _windows[_current_window];

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	bool press = false;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		Events event(Event_Type::Walk);
		_hunter->move(Camera_Movement::FORWARD, _deltatime);
		_hunter->Event(event, _deltatime);

		_bullet->SetPosition(get_camera(0)->Position);
		press = true;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		Events event(Event_Type::Walk);
		_hunter->move(Camera_Movement::BACKWARD, _deltatime);
		_hunter->Event(event, _deltatime);

		_bullet->SetPosition(get_camera(0)->Position);
		press = true;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		Events event(Event_Type::Walk);
		_hunter->move(Camera_Movement::LEFT, _deltatime);
		_hunter->Event(event, _deltatime);
		
		_bullet->SetPosition(get_camera(0)->Position);
		press = true;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		Events event(Event_Type::Walk);
		_hunter->move(Camera_Movement::RIGHT, _deltatime);
		_hunter->Event(event, _deltatime);
		
		_bullet->SetPosition(get_camera(0)->Position);
		press = true;
	}

	if (!press) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE) {
			Events event(Event_Type::Stop);
			_hunter->Event(event, _deltatime);
			return;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE) {
			Events event(Event_Type::Stop);
			_hunter->Event(event, _deltatime);
			return;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE) {
			Events event(Event_Type::Stop);
			_hunter->Event(event, _deltatime);
			return;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE) {
			Events event(Event_Type::Stop);
			_hunter->Event(event, _deltatime);
			return;
		}
	}

	_map->update_model(_hunter);

}

void HEngine::collision_detection() {
	vector<Model_Data> nearby;
	// hunter
	if (_hunter->is_need_detect_collision()) {
		nearby.clear();
		nearby = _map->get_model_nearby(_hunter, 0.10f);

		for (int i = 0; i < nearby.size(); i++) {
			if(nearby[i]._adjust_pos)
				nearby[i]._model->SetPosition(*nearby[i]._adjust_pos);

			Collision collision_type = get_collide_type(_hunter, nearby[i]._model);
			_hunter->Event(collision_type, _deltatime);
			nearby[i]._model->Event(collision_type, _deltatime);
		}
	}

	// animal
	for (int index = 0; index < _models.size(); index++) {
		if (_models[index]->is_need_detect_collision()) {
			nearby.clear();
			nearby = _map->get_model_nearby(_models[index], 0.10f);

			for (int i = 0; i < nearby.size(); i++) {
				if (nearby[i]._adjust_pos)
					nearby[i]._model->SetPosition(*nearby[i]._adjust_pos);

				Collision collision_type = get_collide_type(_models[index], nearby[i]._model);
				_models[index]->Event(collision_type, _deltatime);
				nearby[i]._model->Event(collision_type, _deltatime);
			}
		}
	}

	// bullet
	if (_bullet->is_need_detect_collision()) {
		nearby.clear();
		nearby = _map->get_model_nearby(_bullet, 0.10f);

		for (int i = 0; i < nearby.size(); i++) {
			if (nearby[i]._adjust_pos)
				nearby[i]._model->SetPosition(*nearby[i]._adjust_pos);

			Collision collision_type = get_collide_type(_bullet, nearby[i]._model);
			_bullet->BulletEvent(get_camera(0)->Position);
			nearby[i]._model->Event(collision_type, _deltatime);
		}
	}

}



void HEngine::clear_buffer() {
	glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void HEngine::run() {
	ini_render_setting();

	while (!glfwWindowShouldClose(_windows[_current_window])) {
		/* Process time */
		adjust_time();

		/* action */
		_hunter->Action(_map, _deltatime);
		_hunter->update_camera();

		_bullet->Action(_map, _deltatime);

		for (unsigned int i = 0; i < _models.size(); i++)
			_models[i]->Action(_map, _deltatime);

		/* input */
		processInput();

		/* Event */
		collision_detection();

		glm::vec3 old_camera_position = _cameras[_current_camera]->Position;
		if (_is_magnifier_draw) {
			_cameras[_current_camera]->Position = _cameras[_current_camera]->Position + _cameras[_current_camera]->Front * glm::vec3(10.0f, 10.0f, 10.0f) - _cameras[_current_camera]->WorldUp * glm::vec3(5.0f, 5.0f, 5.0f);
		}

		/* generate shadow */
		draw_shadow();

		/* clear */
		clear_buffer();

		_map->Draw();

		_hunter->Draw();

		if(_bullet->GetEventType() == Event_Type::Shoot)
			_bullet->Draw();  //debug

		for (unsigned int i = 0; i < _models.size(); i++)
			_models[i]->Draw();

		draw_magnifier();

		_cameras[_current_camera]->Position = old_camera_position;

		/* render skybox */
		_skybox[_current_skybox]->Draw();

		/* swap buffers and poll IO events */
		glfwSwapBuffers(_windows[_current_window]);
		glfwPollEvents();
	}

	_windows.erase(_windows.begin()+_current_window);
}

void HEngine::terminate() {
	_windows.clear();
	_models.clear();
	_shaders.clear();
	_cameras.clear();
	_skybox.clear();

	glfwTerminate();
}

void HEngine::ini_window_setting() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

void HEngine::ini_stb_setting() {
	stbi_set_flip_vertically_on_load(true);
}

void HEngine::ini_render_setting() {
	glEnable(GL_DEPTH_TEST);
}

void HEngine::ini_glad() {
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		assert(0);
	}
}

void HEngine::ini_enging_setting() {
	;
}


void HEngine::adjust_time() {
	_currenttime = static_cast<float>(glfwGetTime());
	_deltatime = _currenttime - _lasttime;
	_lasttime = _currenttime;
}

template <class T>
void HEngine::check_splited(int index, vector<T> vec) {
	if (index < 0 || index >= vec.size()) {
		cout << "ERROR::HENGINE::SPLITED:: the index given: " << index << " is splited. Current size: " << vec.size() << endl;
		assert(0);
	}
}

Collision HEngine::get_collide_type(HModel* model1, HModel* model2)
{
	bool collide_bool = false;
	vector<int> model_1_meshes_index, model_2_meshes_index;

	HCollider* a = model1->get_collider();
	HCollider* b = model2->get_collider();

	vector<glm::vec3> Points1(8);
	vector<glm::vec3> Points2(8);

	glm::mat4 wvp1 = model1->GetPositionMat() * model1->GetRotationMat() * model1->GetScalingMat();
	glm::mat4 wvp2 = model2->GetPositionMat() * model2->GetRotationMat() * model2->GetScalingMat();


	for (int i = 0; i < 8; i++) {
		Points1[i] = glm_vec4_to_glm_vec3(wvp1 * glm::vec4(a->get_Points(i), 1.0f));
		Points2[i] = glm_vec4_to_glm_vec3(wvp2 * glm::vec4(b->get_Points(i), 1.0f));
	}

	//show_mat4(wvp1, "wvp1");
	//show_mat4(wvp2, "wvp2");
	/*for(int i = 0; i<2; i++)
		cout << "Points1: " << Points1[i].x << ", " << Points1[i].y << ", " << Points1[i].z << endl;
	for(int i = 0; i<2; i++)
		cout << "Points2: " << Points2[i].x << ", " << Points2[i].y << ", " << Points2[i].z << endl;*/
	
	collide_bool = if_collide(Points1, Points2);
	
	//if(collide_bool)
	 //cout << "Big Box detection" << endl;

	if (collide_bool)
	{
		collide_bool = false;
		for (int i = 0; i < model1->get_meshes().size(); i++)
		{
			a = model1->get_meshes()[i].ini_collider;
			wvp1 = model1->get_meshes()[i].mesh_transform_mat;
			for (int j = 0; j < model2->get_meshes().size(); j++)
			{
				b = model2->get_meshes()[j].ini_collider;
				wvp2 = model2->get_meshes()[j].mesh_transform_mat;

				for (int k = 0; k < 8; k++) {
					Points1[k] = glm_vec4_to_glm_vec3(wvp1 * glm::vec4(a->get_Points(i), 1.0f));
					Points2[k] = glm_vec4_to_glm_vec3(wvp2 * glm::vec4(b->get_Points(i), 1.0f));
				}

				if (if_collide(Points1, Points2)) {
					collide_bool = true;
					model_1_meshes_index.emplace_back(i);
					model_2_meshes_index.emplace_back(j);
				}
			}
		}
	}

	return Collision(collide_bool, model1, model2, model_1_meshes_index, model_2_meshes_index);
}

bool HEngine::if_collide(vector<glm::vec3> Points1, vector<glm::vec3> Points2)
{
	glm::vec3 fs[6];

	int plane[6][4] = { {0,1,3,2},{0,1,5,4},{0,2,6,4},{7,6,2,3},{5,7,3,1},{5,7,6,4} };

	fs[0] = glm::normalize(Points1[1] - Points1[0]);
	fs[1] = glm::normalize(Points1[2] - Points1[0]);
	fs[2] = glm::normalize(Points1[4] - Points1[0]);
	fs[3] = glm::normalize(Points2[1] - Points2[0]);
	fs[4] = glm::normalize(Points2[2] - Points2[0]);
	fs[5] = glm::normalize(Points2[4] - Points2[0]);

	int flagtime = 0;
	int flag = true;
	for (size_t i = 0; i < 6; i++)
	{
		for (size_t j = i + 1; j < 6; j++) {

			flagtime = 0;
			flag = true;
			for (size_t i1 = 0; i1 < 6; i1++)
			{
				for (size_t j1 = 0; j1 < 6; j1++) {
					glm::vec2 p1[4];
					glm::vec2 p2[4];
					for (size_t p = 0; p < 4; p++)
					{
						glm::vec2 v1 = glm::vec2(glm::dot(Points1[plane[i1][p]], fs[i]), glm::dot(Points1[plane[i1][p]], fs[j]));
						p1[p] = v1;
						glm::vec2 v2 = glm::vec2(glm::dot(Points2[plane[j1][p]], fs[i]), glm::dot(Points2[plane[j1][p]], fs[j]));
						p2[p] = v2;

					}
					if (!inspection_2D(p1, p2))
					{
						flag = false;
						break;
					}
					else flagtime++;
				}

				if (!flag) break;
			}
			if (flagtime == 36)
			{
				return false;
			}
		}
	}
	return true;

}

bool HEngine::inspection_2D(glm::vec2* p1, glm::vec2* p2)
{
	glm::vec2 fs[4];


	glm::vec2 temp1 = glm::vec2(p1[1][1] - p1[0][1], p1[0][0] - p1[1][0]);
	fs[0] = glm::normalize(temp1);

	glm::vec2 temp2 = glm::vec2(p1[3][1] - p1[0][1], p1[0][0] - p1[3][0]);
	fs[1] = glm::normalize(temp2);

	glm::vec2 temp3 = glm::vec2(p2[1][1] - p2[0][1], p2[0][0] - p2[1][0]);
	fs[2] = glm::normalize(temp3);

	glm::vec2 temp4 = glm::vec2(p2[3][1] - p2[0][1], p2[0][0] - p2[3][0]);
	fs[3] = glm::normalize(temp4);


	for (size_t i = 0; i < 4; i++)
	{
		double p1_min, p1_max;
		double p2_min, p2_max;
		for (size_t j = 0; j < 4; j++) {
			if (j == 0)
			{
				p1_min = glm::dot(p1[0], fs[i]);
				p1_max = glm::dot(p1[0], fs[i]);
			}
			else
			{
				if (glm::dot(p1[j], fs[i]) > p1_max)
				{
					p1_max = glm::dot(p1[j], fs[i]);
				}

				if (glm::dot(p1[j], fs[i]) < p1_min)
				{
					p1_min = glm::dot(p1[j], fs[i]);
				}
			}
		}
		for (size_t j = 0; j < 4; j++) {
			if (j == 0)
			{
				p2_min = glm::dot(p2[0], fs[i]);
				p2_max = glm::dot(p2[0], fs[i]);
			}
			else
			{
				if (glm::dot(p2[j], fs[i]) > p2_max)
				{
					p2_max = glm::dot(p2[j], fs[i]);
				}

				if (glm::dot(p2[j], fs[i]) < p2_min)
				{
					p2_min = glm::dot(p2[j], fs[i]);
				}
			}
		}

		if (p1_max < p2_min || p2_max < p1_min)
		{
			return true;
		}
	}
	return false;
}