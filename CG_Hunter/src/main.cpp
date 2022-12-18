#include <HEngine.h>

HEngine* engine;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

int main() {

  engine = new HEngine();


	/* Create new window */
	engine->insert_window(SCR_WIDTH, SCR_HEIGHT, "CG_Hunter");
  engine->set_current_window(0);

	/* Set callback*/
	engine->set_framebuffersize_callback(0, framebuffer_size_callback);
	engine->set_cursorpos_callback(0, mouse_callback);
	engine->set_scroll_callback(0, scroll_callback);
  engine->set_cursor_hide(0);

	/* Create new camera*/
	engine->insert_camera();
  engine->set_camera(0);

	/* Create new shaders */
	engine->insert_shader("./resources/shader/vs/model.vert", "./resources/shader/fs/model.frag");
	engine->insert_shader("./resources/shader/vs/skybox.vert", "./resources/shader/fs/skybox.frag");
	engine->insert_shader("./resources/shader/vs/map.vert", "./resources/shader/fs/map.frag");


	/* Create new hunter */
	engine->create_hunter("./resources/model/cube.fbx", glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, 0.0f);
	engine->get_hunter()->SetScaling(glm::vec3(0.6f, 0.6f, 0.6f)); // Set model scaling
	
	/* Create new map */
	engine->create_map("./resources/map/map.fbx");
	engine->get_map()->get_map_model()->SetScaling(glm::vec3(Scale_X, Scale_Y, Scale_Z));
	engine->get_map()->get_map_model()->SetRotation(glm::quat(glm::highp_vec3(glm::radians(-90.0f), 0.0f, 0.0f)));
	engine->get_map()->get_map_model()->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));


	/* Create new skybox */
	engine->insert_skybox(default_skyboxvertices, default_skyboxfaces);
	engine->set_skybox(0);


	/* Set binding */
	engine->get_hunter()->BindShader(engine->get_shader(0));
	engine->get_hunter()->BindCamera(engine->get_camera(0));
	engine->set_model_matrix_bindpoint(-2, 1);
	engine->get_skybox(0)->BindShader(engine->get_shader(1));
	engine->get_skybox(0)->BindCamera(engine->get_camera(0));
	engine->get_map()->get_map_model()->BindShader(engine->get_shader(2));
	engine->get_map()->get_map_model()->BindCamera(engine->get_camera(0));
	engine->set_model_matrix_bindpoint(-1, 2);
	
	engine->run();

	engine->terminate();

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (engine->is_firstmouse())
	{
		engine->set_mouse_xy(xpos, ypos);
		engine->set_firstmouse(false);
	}
	
	float lastX = 0, lastY = 0;
	engine->get_mouse_xy(lastX, lastY);

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	engine->set_mouse_xy(xpos, ypos);
	
	engine->get_hunter()->turn(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	engine->get_camera(engine->current_camera_index())->ProcessMouseScroll(static_cast<float>(yoffset));
}
