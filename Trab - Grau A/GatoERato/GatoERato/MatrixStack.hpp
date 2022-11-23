#pragma once

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

class MatrixStack {
public:

	MatrixStack() {
		stack[0] = glm::identity<glm::mat3>();
		top = 0;
	}

	void Push(const glm::mat3& mat) {
		if (top < 9) {
			glm::mat3& topM = stack[top];
			stack[++top] = topM * mat;
		}
	}

	void Pop() {
		if (top > 0) {
			top--;
		}
	}

	const glm::mat3& Top() const {
		return stack[top];
	}

	void Clear() {
		top = 0;
	}

private:
	glm::mat3 stack[10];
	int top = 0;
};