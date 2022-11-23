#include <HSkybox.h>

HSkybox::HSkybox(vector<float>& skyboxvertices, vector<string>& skyboxfaces) : _skyboxvertices(skyboxvertices), _skyboxfaces(skyboxfaces) {
	bindbuffer();
	loadskybox();
}	

void HSkybox::Draw() {
	glDepthFunc(GL_LEQUAL); // change depth function so depth test passes when values are equal to depth buffer's content
	_shader->use();
	glBindBuffer(GL_UNIFORM_BUFFER, _skybox_buffer_id);
	unsigned int buffer_offset = 0;

	// Bind projection and view
	glm::mat4 perspective = glm::perspective(glm::radians(_camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	glm::mat4 view = glm::mat4(glm::mat3(_camera->GetViewMatrix()));
	BindUniformData(buffer_offset, &perspective);
	BindUniformData(buffer_offset, &view);

	glBindVertexArray(_skyboxVAO);
	glActiveTexture(GL_TEXTURE0 + SKYBOX_ID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _skyboxtexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glBindVertexArray(0);
	glDepthFunc(GL_LESS); // set depth function back to default
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void HSkybox::BindShader(HShader* shader){
	_shader = shader;
}

void HSkybox::BindCamera(HCamera* camera) {
	_camera = camera;
}

void HSkybox::bindbuffer() {
	// skybox VAO
	glGenVertexArrays(1, &_skyboxVAO);
	glGenBuffers(1, &_skyboxVBO);
	glBindVertexArray(_skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, _skyboxVBO);

	float temp_skyboxvertices[MAX_SKYBOX_VERTEXS];
	for (int i = 0; i < _skyboxvertices.size(); i++)
		temp_skyboxvertices[i] = _skyboxvertices.at(i);

	glBufferData(GL_ARRAY_BUFFER, _skyboxvertices.size()*sizeof(float), &temp_skyboxvertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// skybox uniform buffer
	glGenBuffers(1, &_skybox_buffer_id);
	glBindBuffer(GL_UNIFORM_BUFFER, _skybox_buffer_id);
	glBufferData(GL_UNIFORM_BUFFER, 2*MATRIX_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, _skybox_buffer_id);
}

void HSkybox::loadskybox() {
	glGenTextures(1, &_skyboxtexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _skyboxtexture);

	int width, height, nrChannels;

	for (unsigned int i = 0; i < _skyboxfaces.size(); i++)
	{
		unsigned char* data = stbi_load(_skyboxfaces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << _skyboxfaces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return ;
}


vector<float> default_skyboxvertices = {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};

vector<string> default_skyboxfaces = {
			"resources/texture/skybox/right.jpg",
			"resources/texture/skybox/left.jpg",
			"resources/texture/skybox/bottom.jpg",
			"resources/texture/skybox/top.jpg",
			"resources/texture/skybox/front.jpg",
			"resources/texture/skybox/back.jpg"
};