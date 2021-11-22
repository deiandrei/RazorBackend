#ifndef CONTEXT_R_H
#define CONTEXT_R_H

#include "include.h"
#include "TextureBuffer.h"

namespace Backend {

	class Context;

	class DataBuffer;
	class ShaderProgram;
	class RenderBuffer;
	class TextureBuffer;

	enum TextureType;

	enum CullingMode { CULL_NONE, CULL_FRONT, CULL_BACK, CULL_FRONT_AND_BACK };
	enum BlendingMode { BLEND_NONE, BLEND_DEFAULT }; // todo: implement all blend modes
	enum DepthTestMode { DEPTH_OFF, DEPTH_READ_ONLY, DEPTH_READ_WRITE };
	enum RenderMode { RENDER_LINES, RENDER_LINES_STRIP, RENDER_TRIANGLES };

	struct TextureBindKey {
		TextureBuffer* Texture;
		std::string UniformName;
		int Slot;

		TextureBindKey(int slot, TextureBuffer* texture, const std::string& uniformName) {
			Slot = slot;
			Texture = texture;
			UniformName = uniformName;
		}
	};

	using TextureBindVector = std::vector<TextureBindKey>;

	class Context {
		protected:
			struct ContextState {
				CullingMode CullMode;
				BlendingMode BlendMode;
				DepthTestMode DepthMode;
				ShaderProgram* Shader;
				RenderBuffer* Renderbuffer;
				DataBuffer* Databuffer;

				ContextState() {
					Shader = nullptr;
					Databuffer = nullptr;
				}

				void operator=(const ContextState& other) {
					CullMode = other.CullMode;
					BlendMode = other.BlendMode;
					DepthMode = other.DepthMode;
					Shader = other.Shader;
					Renderbuffer = other.Renderbuffer;
					Databuffer = other.Databuffer;
				}
			};

		public:
			Context(int screenWidth, int screenHeight, int defaultFBO = 0);
			~Context() { }

			RenderBuffer* DefaultRenderBuffer;

			// Factory
			RenderBuffer* CreateRenderBuffer(int w, int h);
			ShaderProgram* CreateShaderProgram();
			DataBuffer* CreateDataBuffer();
			TextureBuffer* CreateTextureBuffer(TextureType type = TextureType::TEXTURE_STANDARD);

			// State setup and history
			void SaveState();
			void RestoreState();

			void FrameBegin();
			void FrameEnd();

			// Mode stuff
			void SetCullMode(CullingMode mode);
			void SetBlendMode(BlendingMode mode);
			void SetDepthMode(DepthTestMode mode);

			CullingMode CullMode() { return mCurrentState.CullMode; }
			BlendingMode BlendMode() { return mCurrentState.BlendMode; }
			DepthTestMode DepthMode() { return mCurrentState.DepthMode; }

			// Rendering stuff
			void SetDatabuffer(DataBuffer* buffer, bool forceSet = false);

			void RenderV(RenderMode mode, int count, int startOffset = 0);
			void RenderI(RenderMode mode, int count, int startOffset = 0);
			void RenderI(RenderMode mode, int count, int indicesOffset, int verticesOffset);

			void BindTextures(const std::vector<std::pair<int, TextureBuffer*>>& textures);
			void BindTextures(const TextureBindVector& textures);

			void UnbindAllTextures();
			void UnbindTexturesByType(TextureType type);

			void SetDefaultFramebufferInternalHandle(int handle);

			// Render buffer stuff
			void SetRenderbuffer(RenderBuffer* rb, bool setAnyway = false);
			void SetClearColor(float r, float g, float b, float a);
			void ClearBuffer(bool clearColor = true, bool clearDepth = true, bool clearStencil = false);

			RenderBuffer* Renderbuffer() { return mCurrentState.Renderbuffer; }

			// Shader stuff
			void SetShader(ShaderProgram* shader);

			ShaderProgram* Shader() { return mCurrentState.Shader; }
			

		protected:
			GLenum ConvertRenderModeToNative(RenderMode mode);

			void CreateDefaultRB(int w, int h, int defaultFBO);
			//void CheckStateChanges();


		protected:
			ContextState mCurrentState;

			std::vector<ContextState> mSavedStates;
			bool mBoundTextures[32][2];

	};

}

#endif
