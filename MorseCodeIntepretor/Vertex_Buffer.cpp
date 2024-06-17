#include "Vertex_Buffer.h"

VBO::VBO(std::vector<Vertex>& vertices)
{

	glGenBuffers(1, &ID);
	glBindBuffer(GL_ARRAY_BUFFER, ID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);

}

void VBO::Bind()
{

	glBindBuffer(GL_ARRAY_BUFFER, this->ID);

}
void VBO::unBind()
{

	glBindBuffer(GL_ARRAY_BUFFER, 0);

}
void VBO::Delete()
{

	glDeleteBuffers(1, &this->ID);

}