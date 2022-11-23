#include <HEngine.h>

HEngine* engine;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

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

	/* Create new models */
	engine->insert_model("./resources/model/nanosuit2.fbx", false);
	engine->get_model(0)->SetScaling(glm::vec3(0.01, 0.01, 0.01)); // Set model scaling

	/* Create new skybox */
	engine->insert_skybox(default_skyboxvertices, default_skyboxfaces);
	engine->set_skybox(0);

	/* Set binding */
	engine->get_model(0)->BindShader(engine->get_shader(0));
	engine->get_model(0)->BindCamera(engine->get_camera(0));
	engine->set_model_matrix_bindpoint(0, 1);
	engine->get_skybox(0)->BindShader(engine->get_shader(1));
	engine->get_skybox(0)->BindCamera(engine->get_camera(0));
	
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

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	engine->get_camera(engine->current_camera_index())->ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	engine->get_camera(engine->current_camera_index())->ProcessMouseScroll(static_cast<float>(yoffset));
}
