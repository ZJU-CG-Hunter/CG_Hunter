#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <HShader.h>
#include <HCamera.h>
#include <HModel.h>
#include <HSkybox.h>
#include <HMap.h>
#include <HHunter.h>

#include <ft2build.h>
#include FT_FREETYPE_H  

#include <iostream>

using namespace std;

class HEngine {
protected:
	// The glfw windows of the engine
	vector<GLFWwindow*> _windows;
	int _current_window;

	// The shaders used to render the model
	vector<HShader*> _shaders;

	// The hunter
	HHunter* _hunter;

	// The models in the engine
	vector<HModel*> _models;

	// The map in the engine
	HMap* _map;

	// The cameras in the engine
	vector<HCamera*> _cameras;
	int _current_camera;

	// The skybox in the engine
	vector<HSkybox*> _skybox;
	int _current_skybox;

	// The timing of the engine
	float _deltatime;
	float _lasttime;
	float _currenttime;

	// THe mouse info of the engine
	float _lastX;
	float _lastY;
	bool _firstMouse;

public:
	/* Constructor of the Engine */
	HEngine();

	/* Return the window pointer according to the given index */
	GLFWwindow* get_window_ptr(const int index);

	/* Insert a new window */
	void insert_window(const unsigned int screen_width, const unsigned int screen_height, const char* title, GLFWmonitor* monitor = nullptr, GLFWwindow* share = nullptr);

	int current_window_index();

	/* Methods for setting the window callback */
	void set_current_window(int current_window);

	void set_framebuffersize_callback(int window_index, GLFWframebuffersizefun funptr);

	void set_cursorpos_callback(int window_index, GLFWcursorposfun funptr);

	void set_scroll_callback(int window_index, GLFWscrollfun funptr);

	void set_window_inputmode(int window_index, int mode, int value);

	void set_cursor_hide(int window_index);

	/* Insert a new shader */
	void insert_shader(const char* vertex_path, const char* fragment_path, const char* geometry_path = nullptr);

	/* Return the reference of the shader according to the index */
	HShader* get_shader(int shader_index);

	/* Create a map */
	void create_map(string const& path);

	/* Return the map */
	HMap* get_map();

	/* Create hunter */
	void create_hunter(string const& path, const glm::vec3 front, const glm::vec3 up, const glm::vec3 right, const glm::vec3 worldup, float yaw, float pitch);

	/* Return the map */
	HHunter* get_hunter();

	void insert_model(string const& path, bool gamma);

	HModel* get_model(int model_index);

	/* Insert a new camera */
	void insert_camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

	/* Return the reference of the camera */
	HCamera* get_camera(int camera_index);

	int current_camera_index();

	/* Set the current camera */
	void set_camera(int camera_index);

	/* Insert a skybox */
	void insert_skybox(vector<float>& skyboxvertices, vector<string>& skyboxfaces);

	/* Set the current skybox */
	void set_skybox(int skybox_index, string skybox_name = "skybox");

	/* Get the skybox */
	HSkybox* get_skybox(int skybox_index);

	/* Get the current mouse info */
	void get_mouse_xy(float& lastX, float& lastY);

	/* Set the current mouse info */
	void set_mouse_xy(float lastX, float lastY);

	/* Return is mouse first appeared */
	bool is_firstmouse();

	/* Set firstmosue */
	void set_firstmouse(bool firstMouse);

	/* process all input */
	virtual void processInput();

	/* Collide */
	Collision get_collide_type(HModel* model1, HModel* model2);

	bool if_collide(vector<glm::vec3> Points1, vector<glm::vec3> Points2);

	bool inspection_2D(glm::vec2* p1, glm::vec2* p2);

	void collision_detection();

	/* Clear buffer before rendering */
	virtual void clear_buffer();

	/* Run the enigne. Normally, there will be a loop until user break it */
	virtual void run();

	/*  terminate, clearing all previously allocated GLFW resources. */
	virtual void terminate();


protected:
	/* Initialize Window */
	virtual void ini_window_setting();

	/* Initialize STB */
	virtual void ini_stb_setting();

	/* Initialize Opengl */
	virtual void ini_render_setting();

	/* Initialize Engine */
	virtual void ini_enging_setting();

	void ini_glad();

	/* Adjust current time */
	void adjust_time();

	/* Check splited error */
	template <class T>
	void check_splited(int index, vector<T>);

};
