#include "Vertex_Array.h"

VAO::VAO()
{

	glGenVertexArrays(1, &this->ID);

}

void VAO::LinkAttrib(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset)
{

	VBO.Bind();
	glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
	glEnableVertexAttribArray(layout);

}

void VAO::Bind()
{

	glBindVertexArray(this->ID);

}

void VAO::unBind()
{

	glBindVertexArray(0);

}

void VAO::Delete()
{

	glDeleteVertexArrays(1, &this->ID);

}
