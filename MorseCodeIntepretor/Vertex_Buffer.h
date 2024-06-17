#ifndef VBO_H
#define VBO_H

#include <glad/glad.h>
#include "Vertex.h"
#include <vector>

class VBO
{

	public:
		GLuint ID;
		VBO(std::vector<Vertex>& vertices);
		VBO(){ };


		void genBuffer()
		{

			glGenBuffers(1, &this->ID);

		};
		void Bind();
		void unBind();
		void Delete();
};

#endif
