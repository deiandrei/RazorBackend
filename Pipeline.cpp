#include "Pipeline.h"

#include "DataBuffer.h"
#include "RenderBuffer.h"
#include "ShaderProgram.h"

namespace Backend {
	Pipeline::Pipeline(int screenWidth, int screenHeight) {
		CreateDefaultRB(screenWidth, screenHeight);

		mLastState.BlendMode = BlendingMode::BLEND_NONE;
		mLastState.CullMode = CullingMode::CULL_NONE;
		mLastState.DepthMode = DepthTestMode::DEPTH_OFF;
		mLastState.Shader = nullptr;
		mLastState.Renderbuffer = DefaultRenderBuffer;

		State = mLastState;
		State.DepthMode = DepthTestMode::DEPTH_READ_WRITE;

		SetCullModeNative();
		SetBlendModeNative();
		SetDepthModeNative();
	}

	void Pipeline::SaveState() {
		mSavedStates.push_back(State);
	}

	void Pipeline::RestoreState() {
		if (mSavedStates.empty()) return;

		State = mSavedStates.back();
		mSavedStates.pop_back();
	}

	void Pipeline::RenderV(RenderMode mode, DataBuffer* buffer, int count, int startOffset) {
		CheckStateChanges();

		GLenum renderTypeNative = ConvertRenderModeToNative(mode);

		glBindVertexArray(buffer->mArrayBufferHandle);

		glDrawArrays(renderTypeNative, startOffset, count);
	}

	void Pipeline::RenderI(RenderMode mode, DataBuffer* buffer, int count, int startOffset) {
		CheckStateChanges();

		GLenum renderTypeNative = ConvertRenderModeToNative(mode);

		glBindVertexArray(buffer->mArrayBufferHandle);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->mIndicesSlotHandle);

		glDrawElements(renderTypeNative, count, GL_UNSIGNED_INT, (const void*) startOffset);
	}

	void Pipeline::RenderI(RenderMode mode, DataBuffer* buffer, int count, int indicesOffset, int verticesOffset) {
		CheckStateChanges();

		GLenum renderTypeNative = ConvertRenderModeToNative(mode);

		glBindVertexArray(buffer->mArrayBufferHandle);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->mIndicesSlotHandle);

		glDrawElementsBaseVertex(renderTypeNative, count, GL_UNSIGNED_INT, (void*)indicesOffset, verticesOffset);
	}

	GLenum Pipeline::ConvertRenderModeToNative(RenderMode mode) {
		if (mode == RenderMode::RENDER_TRIANGLES) {
			return GL_TRIANGLES;
		}
		else if (mode == RenderMode::RENDER_LINES) {
			return GL_LINES;
		}
		else {
			return GL_POINTS;
		}
	}

	void Pipeline::CreateDefaultRB(int w, int h) {
		DefaultRenderBuffer = new RenderBuffer(w, h);
		glDeleteFramebuffers(1, &DefaultRenderBuffer->mBufferHandle);
		DefaultRenderBuffer->mBufferHandle = 0;
	}

	void Pipeline::CheckStateChanges() {
		bool changed = false;

		if (State.Shader != mLastState.Shader) {
			SetShaderNative();
			changed = true;
		}

		if (State.Renderbuffer != mLastState.Renderbuffer) {
			SetRenderbufferNative();
			changed = true;
		}

		if (State.CullMode != mLastState.CullMode) {
			SetCullModeNative();
			changed = true;
		}

		if (State.BlendMode != mLastState.BlendMode) {
			SetBlendModeNative();
			changed = true;
		}

		if (State.DepthMode != mLastState.DepthMode) {
			SetDepthModeNative();
			changed = true;
		}

		if(changed) mLastState = State;
	}

	void Pipeline::SetShaderNative() {
		if (State.Shader) {
			glUseProgram(State.Shader->mProgramHandle);
		}
		else {
			glUseProgram(0);
		}
	}

	void Pipeline::SetRenderbufferNative() {
		if (!State.Renderbuffer) State.Renderbuffer = DefaultRenderBuffer;

		if (State.Renderbuffer != mLastState.Renderbuffer) {
			int lastWidth = mLastState.Renderbuffer->GetWidth();
			int lastHeight = mLastState.Renderbuffer->GetHeight();

			int width = State.Renderbuffer->GetWidth();
			int height = State.Renderbuffer->GetHeight();

			if (lastWidth != width || lastHeight != height) {
				glViewport(0, 0, width, height);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, State.Renderbuffer->mBufferHandle);
		}
	}

	void Pipeline::ClearBuffer(bool clearColor, bool clearDepth, bool clearStencil) {
		if (State.Renderbuffer != mLastState.Renderbuffer) {
			SetRenderbufferNative();
			mLastState.Renderbuffer = State.Renderbuffer;
		}

		GLbitfield clearMaskNative = 0;
		if (clearColor) clearMaskNative = clearMaskNative | GL_COLOR_BUFFER_BIT;
		if (clearDepth) clearMaskNative = clearMaskNative | GL_DEPTH_BUFFER_BIT;
		if (clearStencil) clearMaskNative = clearMaskNative | GL_STENCIL_BUFFER_BIT;

		glClear(clearMaskNative);
	}

	void Pipeline::SetClearColor(float r, float g, float b, float a) {
		glClearColor(r, g, b, a);
	}

	void Pipeline::SetCullModeNative() {
		if (State.CullMode == CullingMode::CULL_NONE) {
			glDisable(GL_CULL_FACE);
		}
		else {
			if (mLastState.CullMode == CullingMode::CULL_NONE) glEnable(GL_CULL_FACE);

			if (State.CullMode == CullingMode::CULL_BACK) glCullFace(GL_BACK);
			else if (State.CullMode == CullingMode::CULL_FRONT) glCullFace(GL_FRONT);
			else if (State.CullMode == CullingMode::CULL_FRONT_AND_BACK) glCullFace(GL_FRONT_AND_BACK);
			else return;
		}
	}

	void Pipeline::SetBlendModeNative() {
		if (State.BlendMode == BlendingMode::BLEND_NONE) {
			glDisable(GL_BLEND);
		}
		else {
			if (mLastState.BlendMode == BlendingMode::BLEND_NONE) glEnable(GL_BLEND);

			if (State.BlendMode == BlendingMode::BLEND_DEFAULT) glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			else return;
		}
	}

	void Pipeline::SetDepthModeNative() {
		if (State.DepthMode == DepthTestMode::DEPTH_OFF) {
			glDisable(GL_DEPTH_TEST);
		}
		else {
			if (mLastState.DepthMode == DepthTestMode::DEPTH_OFF) glEnable(GL_DEPTH_TEST);

			if (State.DepthMode == DepthTestMode::DEPTH_READ_ONLY) glDepthMask(GL_FALSE);
			else if (State.DepthMode == DepthTestMode::DEPTH_READ_WRITE) glDepthMask(GL_TRUE);
			else return;
		}
	}

}
