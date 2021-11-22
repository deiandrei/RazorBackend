#include "InternalStateManager.h"

void InternalStateManager::BindShaderProgram(GLuint handle) {
	if (handle != mShaderProgram) {
		glUseProgram(handle);
		mShaderProgram = handle;
	}
}

void InternalStateManager::BindFramebuffer(GLuint handle) {
	if (handle != mFramebuffer) {
		//glBind(handle);
		mShaderProgram = handle;
	}
}
