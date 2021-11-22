#ifndef INTERNALSTATEMAN_R_H
#define INTERNALSTATEMAN_R_H

#include "include.h"

class InternalStateManager {
	public:
		InternalStateManager();

		// binders
		void BindShaderProgram(GLuint handle);
		void BindFramebuffer(GLuint handle);

		// getters
		GLuint GetShaderProgram() { return mShaderProgram; }
		GLuint GetFramebuffer() { return mFramebuffer; }

	protected:
		GLuint mShaderProgram;
		GLuint mFramebuffer;

};

#endif
