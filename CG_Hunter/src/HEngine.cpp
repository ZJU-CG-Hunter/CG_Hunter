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

void HEngine::create_map(string const& path) {
	_map = new HMap(path);
}

HMap* HEngine::get_map() {
	return _map;
}

/* Create hunter */
void HEngine::create_hunter(string const& path, const glm::vec3 front, const glm::vec3 up, const glm::vec3 right, const glm::vec3 worldup, float yaw, float pitch) {
	_hunter = new HHunter(path, front, up, right, worldup, yaw, pitch);
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


void HEngine::insert_model(string const& path, bool gamma) {
	HModel* created_model = new HModel(path, gamma);

	_models.emplace_back(created_model);

	//_map->insert_model(created_model);
}

HModel* HEngine::get_model(int model_index) {
	check_splited(model_index, _models);
	
	return _models[model_index];
}


void HEngine::set_model_matrix_bindpoint(int model_index, int binding_point) {
	if (model_index == -2)
		_hunter->BindShaderUniformBuffer(binding_point);
	else if (model_index == -1)
		_map->get_map_model()->BindShaderUniformBuffer(binding_point);
	else {
		check_splited(model_index, _models);
		_models[model_index]->BindShaderUniformBuffer(binding_point);
	}

	return;
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

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) 
		_hunter->move(Camera_Movement::FORWARD, _deltatime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		_hunter->move(Camera_Movement::BACKWARD, _deltatime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		_hunter->move(Camera_Movement::LEFT, _deltatime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		_hunter->move(Camera_Movement::RIGHT, _deltatime);

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

		/* auto action */
		_hunter->Action(_map, _deltatime);

		for (unsigned int i = 0; i < _models.size(); i++)
			_models[i]->Action(_map, _deltatime);

		/* event: collision */
		for (unsigned int i = 0; i < _models.size(); i++)
		{
			HModel* model1 = _models[i];
			set<HModel*> set_near = _map->get_model_nearby(model1);
			for (set<HModel*>::iterator j = set_near.begin(); j != set_near.end(); j++)
			{
				HModel* model2 = *j;
				collides type = get_collide_type(model1, model2);
				if (type.if_collide)
				{
					/*model_action(model1, type.model1_event);
					model_action(model2, type.model2_event);*/
				}
			}
		}

		/* clear */
		clear_buffer();

		/* render the hunter */
		_hunter->Draw();

		/* render model */
		for (unsigned int i = 0; i < _models.size(); i++) 
			_models[i]->Draw();

		/* render map */
		_map->Draw();
	
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

collides HEngine::get_collide_type(HModel* model1, HModel* model2)
{

	collides res;

	HCollider* a = model1->get_collider();
	HCollider* b = model2->get_collidber();

	glm::vec3* Points1 = Tran_Points(a);
	glm::vec3* Points2 = Tran_Points();

	bool collide_bool = if_collide(Points1, Points2);
	bool Flag = collide_bool;

	if (collide_bool)
	{
		collide_bool = false;
		for (int i = 0; i < model1->get_collider_size() - 1; i++)
		{
			a = model1->get_colliders(i);
			for (int j = 0; j < model2->get_collider_size() - 1; j++)
			{
				b = model2->get_colliders(j);
				Points1 = Tran_Points(a);
				Points2 = Tran_Points(b);
				collide_bool = if_collide(Points1, Points2);
				if (collide_bool)
				{
					res.if_collide = collide_bool;
					if (collide_bool)
					{
						res.model1_event = Events::type1;
						res.model2_event = Events::type2;
					}
					return res;
				}
			}
		}
	}

	res.if_collide = collide_bool;
	return res;


	/*while (Flag)
	{
		collide_bool = false;
		for (int i = 0; i < model1->get_collider_size() - 1; i++)
		{
			a = model1->get_colliders(i);
			for (int j = 0; j < model2->get_collider_size() - 1; j++)
			{
				b = model2->get_colliders(j);
				Points1 = Tran_Points(a);
				Points2 = Tran_Points(b);
				collide_bool = if_collide(Points1, Points2);
				if (i == model1->get_collider_size() - 2 && j == model2->get_collider_size() - 2)
				{
					Flag = false;
				}
				if (collide_bool)
				{
					Flag = false;
					break;
				}
			}
			if (collide_bool) break;
		}
	}



	if (collide_bool)
	{
		res.model1_event = Events::type1;
		res.model2_event = Events::type2;
	}
	return res;*/
}
bool HEngine::if_collide(glm::vec3* Points1, glm::vec3* Points2)
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