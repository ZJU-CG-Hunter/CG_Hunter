#include <HEngine.h>


HEngine::HEngine(): _deltatime(0.0f), _lasttime(0.0f) , _current_window(-1) {
	ini_window_setting();
	ini_stb_setting();
	ini_enging_setting();
}

GLFWwindow* HEngine::get_window_ptr(const int index) {
	check_splited(index, _windows);

	return _windows[index];
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


void HEngine::insert_model(string const& path, bool gamma) {
	HModel* created_model = new HModel(path, gamma);

	_models.emplace_back(created_model);

	_map->insert_model(created_model);
}

HModel* HEngine::get_model(int model_index) {
	check_splited(model_index, _models);
	
	return _models[model_index];
}


void HEngine::set_model_matrix_bindpoint(int model_index, int binding_point) {
	check_splited(model_index, _models);
	_models[model_index]->BindShaderUniformBuffer(binding_point);

	return;
}


void HEngine::processInput() {
	GLFWwindow* window = _windows[_current_window];
	HCamera* camera = _cameras[_current_camera];

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera->ProcessKeyboard(FORWARD, _deltatime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera->ProcessKeyboard(BACKWARD, _deltatime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera->ProcessKeyboard(LEFT, _deltatime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera->ProcessKeyboard(RIGHT, _deltatime);
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

		/* input */
		processInput();

		/* action */
		for (unsigned int i = 0; i < _models.size(); i++)
			_models[i]->Action(_map, _deltatime);

		/* clear */
		clear_buffer();

		/* render model */
		for (unsigned int i = 0; i < _models.size(); i++) 
			_models[i]->Draw();
	
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
	_map = new HMap();
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