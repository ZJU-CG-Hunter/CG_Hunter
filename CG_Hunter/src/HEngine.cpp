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
	shadow_depth_shader_tree = new HShader("./resources/shader/vs/shadow_empty_tree.vert", "./resources/shader/fs/shadow_empty_tree.frag");
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
	shadow_depth_shader_tree->use();
	shadow_depth_shader_tree->setMat4("lightSpaceMatrix", lightSpaceMatrix);

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	HShader* old_shaders;
	for (int i = 0; i < _models.size(); i++) {
		old_shaders = _models[i]->get_current_shader();
		if(_models[i]->get_model_type() != Model_Type::Bullet)
			_models[i]->BindShader(shadow_depth_shader);
		else 
			_models[i]->BindShader(shadow_depth_shader_tree);
		_models[i]->Draw();
		_models[i]->BindShader(old_shaders);
		//cout << "TT" << endl;
	}
	old_shaders = _hunter->get_current_shader();
	_hunter->BindShader(shadow_depth_shader);
	_hunter->Draw();
	_hunter->BindShader(old_shaders);

	old_shaders = _map->get_map_model()->get_current_shader();
	_map->get_map_model()->BindShader(shadow_depth_shader);
	_map->get_map_model()->Draw();
	_map->get_map_model()->BindShader(old_shaders);

	for (int i = 0; i < _map->get_landscape().size(); i++) {
		old_shaders = _map->get_landscape()[i]->get_current_shader();

		_map->get_landscape()[i]->BindShader(shadow_depth_shader_tree);
		_map->get_landscape()[i]->Draw();
		_map->get_landscape()[i]->BindShader(old_shaders);
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

void HEngine::create_map(string const& path, unsigned int map_seed) {
	_map = new HMap(path, map_seed);
}

HMap* HEngine::get_map() {
	return _map;
}

/* Create hunter */
void HEngine::create_hunter(string const& path, const glm::vec3 front, const glm::vec3 up, const glm::vec3 right, const glm::vec3 worldup, float yaw, float pitch) {
	_hunter = new HHunter(path, front, up, right, worldup, yaw, pitch);

	_map->insert_model(_hunter);
}

/* Return the map */
HHunter* HEngine::get_hunter() {
	return _hunter;
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


void HEngine::insert_model(string const& path, bool gamma, Model_Type model_type) {
	HModel* created_model;
	switch (model_type) {
	case Model_Type::Unknown:
		created_model = new HModel(path, gamma);
		_map->insert_model(created_model);
		created_model->AdjustStepOnGround(_map);
		break;
	case Model_Type::Pig:
		created_model = new HPig(path, gamma);
		_map->insert_model(created_model);
		created_model->AdjustStepOnGround(_map);
		break;
	case Model_Type::Bullet:
		created_model = new HBullet(path, gamma);
		break;
	}
	_models.emplace_back(created_model);
}

void HEngine::remove_model(int index) {
	_models.erase(_models.begin() + index);
}

HModel* HEngine::get_model(int model_index) {
	check_splited(model_index, _models);
	
	return _models[model_index];
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

	bool is_press = false;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		_hunter->move(Camera_Movement::FORWARD, _deltatime);
		is_press = true;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		_hunter->move(Camera_Movement::BACKWARD, _deltatime);
		is_press = true;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		_hunter->move(Camera_Movement::LEFT, _deltatime);
		is_press = true;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		_hunter->move(Camera_Movement::RIGHT, _deltatime);
		is_press = true;
	}

	if(!is_press)
		_hunter->idle();

	_map->update_model(_hunter);

}

void HEngine::collision_detection() {
	vector<Model_Data> nearby;
	_hunter->collision_detection(_map);
	
	for (int i = 0; i < _models.size(); i++) {
		_models[i]->collision_detection(_map);
	}
}

void HEngine::clear_buffer() {
	glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void HEngine::run() {
	ini_render_setting();

	while (!glfwWindowShouldClose(_windows[_current_window])) {
		processInput();
		glfwPollEvents();

		/* Event */
		collision_detection();

		/* Process time */
		adjust_time();

		/* action */
		_hunter->Action(_map, _deltatime);
		_hunter->update_camera();

		for (unsigned int i = 0; i < _models.size(); i++)
			_models[i]->Action(_map, _deltatime);

		/* generate shadow */
		draw_shadow();

		/* clear */
		clear_buffer();

		_map->Draw();

		_hunter->Draw();

		for (unsigned int i = 0; i < _models.size(); i++)
			_models[i]->Draw();

		/* render skybox */
		_skybox[_current_skybox]->Draw();

		_hunter->DrawMagnifier();


		/* swap buffers and poll IO events */
		glfwSwapBuffers(_windows[_current_window]);


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