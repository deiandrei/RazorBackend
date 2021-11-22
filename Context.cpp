#include "Context.h"

#include "DataBuffer.h"
#include "RenderBuffer.h"
#include "ShaderProgram.h"

namespace Backend {
	Context::Context(int screenWidth, int screenHeight, int defaultFBO) {
		CreateDefaultRB(screenWidth, screenHeight, defaultFBO);

		memset(mBoundTextures, 0, sizeof(mBoundTextures));

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

	RenderBuffer* Context::CreateRenderBuffer(int w, int h) {
		return new RenderBuffer(this, w, h);
	}

	ShaderProgram* Context::CreateShaderProgram() {
		return new ShaderProgram(this);
	}

	DataBuffer* Context::CreateDataBuffer() {
		return new DataBuffer(this);
	}

	TextureBuffer* Context::CreateTextureBuffer(TextureType type) {
		return new TextureBuffer(this, type);
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

	void Context::FrameBegin() {
		UnbindAllTextures();

		SetRenderbuffer(DefaultRenderBuffer, true);
		SetDatabuffer(nullptr);
		SetShader(nullptr);
	}

	void Context::FrameEnd() {

	}

	void Context::RenderV(RenderMode mode, int count, int startOffset) {
		GLenum renderTypeNative = ConvertRenderModeToNative(mode);

		glDrawArrays(renderTypeNative, startOffset, count);
	}

	void Context::RenderI(RenderMode mode, int count, int startOffset) {
		GLenum renderTypeNative = ConvertRenderModeToNative(mode);

		glDrawElements(renderTypeNative, count, GL_UNSIGNED_INT, (const void*) startOffset);
	}

	void Context::RenderI(RenderMode mode, int count, int indicesOffset, int verticesOffset) {
		GLenum renderTypeNative = ConvertRenderModeToNative(mode);

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

	void Context::CreateDefaultRB(int w, int h, int defaultFBO) {
		DefaultRenderBuffer = new RenderBuffer(this, w, h);
		glDeleteFramebuffers(1, &DefaultRenderBuffer->mBufferHandle);
		DefaultRenderBuffer->mBufferHandle = defaultFBO;
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

	void Context::BindTextures(const std::vector<std::pair<int, TextureBuffer*>>& textures) {
		for (auto tex : textures) {
			tex.second->BindForRendering(tex.first);

			mBoundTextures[tex.first][tex.second->GetType()] = true;
		}
	}

	void Context::UnbindAllTextures() {
		for (int i = 0; i < 32;++i) {
			for (int j = 0; j < 2; ++j) {
				if (mBoundTextures[i][j]) {
					glActiveTexture(GL_TEXTURE0 + i);
					glBindTexture(TextureBuffer::TextureTypeConvertNative[j], 0);

					mBoundTextures[i][j] = false;
				}

			}
		}
	}

	void Context::UnbindTexturesByType(TextureType type) {
		for (int i = 0; i < 32; ++i) {
			if (mBoundTextures[i][type]) {
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(TextureBuffer::TextureTypeConvertNative[type], 0);

				mBoundTextures[i][type] = false;
			}
		}
	}

	void Context::SetDefaultFramebufferInternalHandle(int handle) {
		DefaultRenderBuffer->mBufferHandle = handle;
	}

	void Context::SetDatabuffer(DataBuffer* buffer, bool forceSet) {
		if (buffer != mCurrentState.Databuffer || forceSet) {
			if (buffer == nullptr) {
				glBindVertexArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			}
			else {
				glBindVertexArray(buffer->mArrayBufferHandle);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->mIndicesSlotHandle);

				mCurrentState.Databuffer = buffer;
			}
		}
	}

	void Context::BindTextures(const std::vector<TextureBindKey>& textures) {
		for (auto& key : textures) {
			mCurrentState.Shader->SetInt(key.UniformName, key.Slot);
			key.Texture->BindForRendering(key.Slot);

			mBoundTextures[key.Slot][key.Texture->GetType()] = true;
		}
	}
	
	void Context::SetRenderbuffer(RenderBuffer* rb, bool setAnyway) {
		if (!rb) rb = DefaultRenderBuffer;

		if (setAnyway || rb != mCurrentState.Renderbuffer) {
			int lastWidth = mCurrentState.Renderbuffer->GetWidth();
			int lastHeight = mCurrentState.Renderbuffer->GetHeight();

			int width = rb->GetWidth();
			int height = rb->GetHeight();

			if (lastWidth != width || lastHeight != height || setAnyway) {
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
