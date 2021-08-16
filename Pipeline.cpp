#include "Pipeline.h"

#include "DataBuffer.h"
#include "ShaderProgram.h"

namespace Backend {
	Pipeline::Pipeline(int screenWidth, int screenHeight) {
		State.BlendMode = BlendingMode::BLEND_NONE;
		State.CullMode = CullingMode::CULL_NONE;
		State.DepthMode = DepthTestMode::DEPTH_READ_WRITE;
		State.Shader = nullptr;

		mLastState = State;

		SetCullModeNative();
		SetBlendModeNative();
		SetDepthModeNative();
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

	void Pipeline::CheckStateChanges() {
		bool changed = false;

		if (State.Shader != mLastState.Shader) {
			SetShaderNative();
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
