#include "Context.h"

#include "DataBuffer.h"
#include "RenderBuffer.h"
#include "ShaderProgram.h"

namespace Backend {
	Context::Context(int screenWidth, int screenHeight) {
		CreateDefaultRB(screenWidth, screenHeight);

		// Set these values so the compare logic will work
		mCurrentState.BlendMode = BlendingMode::BLEND_NONE;
		mCurrentState.CullMode = CullingMode::CULL_NONE;
		mCurrentState.DepthMode = DepthTestMode::DEPTH_OFF;
		mCurrentState.Shader = nullptr;
		mCurrentState.Renderbuffer = DefaultRenderBuffer;

		// Default state
		SetCullMode(mCurrentState.CullMode);
		SetDepthMode(DepthTestMode::DEPTH_READ_WRITE);
		SetBlendMode(mCurrentState.BlendMode);
		
	}

	void Context::SaveState() {
		mSavedStates.push_back(mCurrentState);
	}

	void Context::RestoreState() {
		if (mSavedStates.empty()) return;

		ContextState state = mSavedStates.back();
		mSavedStates.pop_back();

		SetCullMode(state.CullMode);
		SetBlendMode(state.BlendMode);
		SetDepthMode(state.DepthMode);
		SetShader(state.Shader);
		SetRenderbuffer(state.Renderbuffer);
	}

	void Context::RenderV(RenderMode mode, DataBuffer* buffer, int count, int startOffset) {
		GLenum renderTypeNative = ConvertRenderModeToNative(mode);

		if (buffer != mCurrentState.Databuffer) {
			glBindVertexArray(buffer->mArrayBufferHandle);

			mCurrentState.Databuffer = buffer;
		}

		glDrawArrays(renderTypeNative, startOffset, count);
	}

	void Context::RenderI(RenderMode mode, DataBuffer* buffer, int count, int startOffset) {
		GLenum renderTypeNative = ConvertRenderModeToNative(mode);

		if (buffer != mCurrentState.Databuffer) {
			glBindVertexArray(buffer->mArrayBufferHandle);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->mIndicesSlotHandle);

			mCurrentState.Databuffer = buffer;
		}

		glDrawElements(renderTypeNative, count, GL_UNSIGNED_INT, (const void*) startOffset);
	}

	void Context::RenderI(RenderMode mode, DataBuffer* buffer, int count, int indicesOffset, int verticesOffset) {
		GLenum renderTypeNative = ConvertRenderModeToNative(mode);

		if (buffer != mCurrentState.Databuffer) {
			glBindVertexArray(buffer->mArrayBufferHandle);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->mIndicesSlotHandle);

			mCurrentState.Databuffer = buffer;
		}

		glDrawElementsBaseVertex(renderTypeNative, count, GL_UNSIGNED_INT, (void*)indicesOffset, verticesOffset);
	}

	GLenum Context::ConvertRenderModeToNative(RenderMode mode) {
		if (mode == RenderMode::RENDER_TRIANGLES) {
			return GL_TRIANGLES;
		}
		else if (mode == RenderMode::RENDER_LINES) {
			return GL_LINES;
		}
		else if (mode == RenderMode::RENDER_LINES_STRIP) {
			return GL_LINE_STRIP;
		}
		else {
			return GL_POINTS;
		}
	}

	void Context::CreateDefaultRB(int w, int h) {
		DefaultRenderBuffer = new RenderBuffer(w, h);
		glDeleteFramebuffers(1, &DefaultRenderBuffer->mBufferHandle);
		DefaultRenderBuffer->mBufferHandle = 0;
	}

	void Context::SetShader(ShaderProgram* shader) {
		if (shader != mCurrentState.Shader) {
			if (shader) {
				glUseProgram(shader->mProgramHandle);
			}
			else {
				glUseProgram(0);
			}

			mCurrentState.Shader = shader;
		}
	}

	void Context::SetTextures(const std::vector<std::pair<int, TextureBuffer*>>& textures) {
		for (auto tex : textures) {
			tex.second->BindForRendering(tex.first);
		}
	}

	void Context::BindTextures(const std::vector<TextureBindKey>& textures) {
		for (auto& key : textures) {
			mCurrentState.Shader->SetInt(key.UniformName, key.Slot);
			key.Texture->BindForRendering(key.Slot);
		}
	}
	
	void Context::SetRenderbuffer(RenderBuffer* rb) {
		if (!rb) rb = DefaultRenderBuffer;

		if (rb != mCurrentState.Renderbuffer) {
			int lastWidth = mCurrentState.Renderbuffer->GetWidth();
			int lastHeight = mCurrentState.Renderbuffer->GetHeight();

			int width = rb->GetWidth();
			int height = rb->GetHeight();

			if (lastWidth != width || lastHeight != height) {
				glViewport(0, 0, width, height);
			}

			rb->Bind();

			mCurrentState.Renderbuffer = rb;
		}
	}

	void Context::ClearBuffer(bool clearColor, bool clearDepth, bool clearStencil) {
		GLbitfield clearMaskNative = 0;
		if (clearColor) clearMaskNative = clearMaskNative | GL_COLOR_BUFFER_BIT;
		if (clearDepth) clearMaskNative = clearMaskNative | GL_DEPTH_BUFFER_BIT;
		if (clearStencil) clearMaskNative = clearMaskNative | GL_STENCIL_BUFFER_BIT;

		glClear(clearMaskNative);
	}

	void Context::SetClearColor(float r, float g, float b, float a) {
		glClearColor(r, g, b, a);
	}

	void Context::SetCullMode(CullingMode mode) {
		if (mode != mCurrentState.CullMode) {
			if (mode == CullingMode::CULL_NONE) {
				glDisable(GL_CULL_FACE);
			}
			else {
				if (mCurrentState.CullMode == CullingMode::CULL_NONE) glEnable(GL_CULL_FACE);

				if (mode == CullingMode::CULL_BACK) glCullFace(GL_BACK);
				else if (mode == CullingMode::CULL_FRONT) glCullFace(GL_FRONT);
				else if (mode == CullingMode::CULL_FRONT_AND_BACK) glCullFace(GL_FRONT_AND_BACK);
				else return;
			}

			mCurrentState.CullMode = mode;
		}
	}

	void Context::SetBlendMode(BlendingMode mode) {
		if (mode != mCurrentState.BlendMode) {
			if (mode == BlendingMode::BLEND_NONE) {
				glDisable(GL_BLEND);
			}
			else {
				if (mCurrentState.BlendMode == BlendingMode::BLEND_NONE) glEnable(GL_BLEND);

				if (mode == BlendingMode::BLEND_DEFAULT) glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				else return;
			}

			mCurrentState.BlendMode = mode;
		}
	}

	void Context::SetDepthMode(DepthTestMode mode) {
		if (mode != mCurrentState.DepthMode) {
			if (mode == DepthTestMode::DEPTH_OFF) {
				glDisable(GL_DEPTH_TEST);
			}
			else {
				if (mCurrentState.DepthMode == DepthTestMode::DEPTH_OFF) glEnable(GL_DEPTH_TEST);

				if (mode == DepthTestMode::DEPTH_READ_ONLY) glDepthMask(GL_FALSE);
				else if (mode == DepthTestMode::DEPTH_READ_WRITE) glDepthMask(GL_TRUE);
				else return;
			}

			mCurrentState.DepthMode = mode;
		}
	}

}