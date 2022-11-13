#include <utility.h>

void Adjustoffset(unsigned int& buffer_offset, unsigned int base_offset) {
  unsigned int space = buffer_offset % base_offset;
  buffer_offset += (space == 0 ? 0 : base_offset - space);
}

void BindUniformData(unsigned int& buffer_offset, glm::mat4* mat) {
  Adjustoffset(buffer_offset, MATRIX_BUFFER_SIZE);
  glBufferSubData(GL_UNIFORM_BUFFER, buffer_offset, MATRIX_BUFFER_SIZE, mat);
  buffer_offset += MATRIX_BUFFER_SIZE;
}

// Used for debug
void show_mat4(glm::mat4 mat, string mat_name) {
  cout << endl << mat_name;
  for (int i = 0; i < 4; i++) {
    cout << endl;
    for (int j = 0; j < 4; j++)
      cout << mat[i][j] << ", ";
  }

}