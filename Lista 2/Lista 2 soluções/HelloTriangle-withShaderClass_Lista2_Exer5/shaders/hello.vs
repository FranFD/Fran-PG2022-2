#version 450

layout (location = 0) in vec3 position;

uniform mat4 projection; //matriz de projeção

void main()
{

	//...pode ter mais linhas de código aqui!
	gl_Position = projection * vec4(position.x, position.y, position.z, 1.0);

}