#include <HEngine.h>

HEngine* engine;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);


int main() {

  engine = new HEngine();


	/* Create new window */
	engine->insert_window(SCR_WIDTH, SCR_HEIGHT, "CG_Hunter");
  engine->set_current_window(0);

	/* Set callback*/
	engine->set_framebuffersize_callback(0, framebuffer_size_callback);
	engine->set_cursorpos_callback(0, mouse_callback);
	engine->set_scroll_callback(0, scroll_callback);
	engine->set_mouse_button_callback(0, mouse_button_callback);
  engine->set_cursor_hide(0);

	/* Create new camera*/
	engine->insert_camera();
  engine->set_camera(0);

	/* Create new shaders */
	engine->insert_shader("./resources/shader/vs/model.vert", "./resources/shader/fs/model.frag");
	engine->insert_shader("./resources/shader/vs/skybox.vert", "./resources/shader/fs/skybox.frag");
	engine->insert_shader("./resources/shader/vs/map.vert", "./resources/shader/fs/map.frag");

	/* Create new map */
	engine->create_map("./resources/map/map10fbx.fbx");

	/* Create new hunter */
	engine->create_hunter("./resources/model/cube.fbx", glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, 0.0f);
	engine->get_hunter()->SetScaling(glm::vec3(0.6f, 0.6f, 0.6f)); // Set model scaling
	engine->get_hunter()->SetPosition(glm::vec3(0.0, 20.0, 0.0));

	/* Create new skybox */
	engine->insert_skybox(default_skyboxvertices, default_skyboxfaces);
	engine->set_skybox(0);


	//engine->insert_model("./resources/model/Pig4.fbx", false);
	//engine->get_model(0)->SetPosition(glm::vec3(0.0, 0.0, 0.0));
	//engine->get_model(0)->SetScaling(glm::vec3(0.1f, 0.1f, 0.1f));
	engine->insert_model("./resources/model/Sheep.fbx", false);
	engine->get_model(0)->SetPosition(glm::vec3(0.0, 0.0, 0.0));
	engine->get_model(0)->SetScaling(glm::vec3(0.1f, 0.1f, 0.1f));


	/* Set binding */
	engine->get_hunter()->BindShader(engine->get_shader(0));
	engine->get_hunter()->BindCamera(engine->get_camera(0));
	engine->get_hunter()->BindShaderUniformBuffer(BINDING_POINT_MODEL_BASE);

	engine->get_skybox(0)->BindShader(engine->get_shader(1));
	engine->get_skybox(0)->BindCamera(engine->get_camera(0));
	engine->get_skybox(0)->BindShaderUniformBuffer(BINDING_POINT_SKYBOX);

	//engine->get_model(0)->BindShader(engine->get_shader(0));
	//engine->get_model(0)->BindCamera(engine->get_camera(0));
	//engine->get_model(0)->BindShaderUniformBuffer(BINDING_POINT_MODEL_BASE + 10);
	engine->get_model(0)->BindShader(engine->get_shader(0));
	engine->get_model(0)->BindCamera(engine->get_camera(0));
	engine->get_model(0)->BindShaderUniformBuffer(BINDING_POINT_MODEL_BASE + 10);

	engine->get_map()->get_map_model()->BindShader(engine->get_shader(2));
	engine->get_map()->get_map_model()->BindCamera(engine->get_camera(0));
	engine->get_map()->get_map_model()->BindShaderUniformBuffer(BINDING_POINT_MAP);

	for (int i = 0; i < engine->get_map()->get_landscape().size(); i++) {
		engine->get_map()->get_landscape()[i]->BindShader(engine->get_shader(0));
		engine->get_map()->get_landscape()[i]->BindCamera(engine->get_camera(0));
		engine->get_map()->get_landscape()[i]->BindShaderUniformBuffer(BINDING_POINT_MODEL_BASE + i + 1);
	}
	
	/* Set collision */
	engine->get_hunter()->set_need_detect_collision(true);

	engine->setup_magnifier();

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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	switch (button){
	case GLFW_MOUSE_BUTTON_RIGHT:
		if (action == GLFW_PRESS) {
			engine->is_draw_magnifier(true);
			cout << "Press" << endl;
		}
		else if (action == GLFW_RELEASE)
			engine->is_draw_magnifier(false);
		break;
	}

	return;
}