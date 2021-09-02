#ifndef CONTEXT_R_H
#define CONTEXT_R_H

#include "include.h"

namespace Backend {

	class Context;

	class DataBuffer;
	class ShaderProgram;
	class RenderBuffer;
	class TextureBuffer;

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
			Context(int screenWidth, int screenHeight);
			~Context() { }

			RenderBuffer* DefaultRenderBuffer;

			// State setup and history
			void SaveState();
			void RestoreState();

			// Mode stuff
			void SetCullMode(CullingMode mode);
			void SetBlendMode(BlendingMode mode);
			void SetDepthMode(DepthTestMode mode);

			CullingMode CullMode() { return mCurrentState.CullMode; }
			BlendingMode BlendMode() { return mCurrentState.BlendMode; }
			DepthTestMode DepthMode() { return mCurrentState.DepthMode; }

			// Rendering stuff
			void RenderV(RenderMode mode, DataBuffer* buffer, int count, int startOffset = 0);
			void RenderI(RenderMode mode, DataBuffer* buffer, int count, int startOffset = 0);
			void RenderI(RenderMode mode, DataBuffer* buffer, int count, int indicesOffset, int verticesOffset);

			void SetTextures(const std::vector<std::pair<int, TextureBuffer*>>& textures);
			void BindTextures(const TextureBindVector& textures);

			// Render buffer stuff
			void SetRenderbuffer(RenderBuffer* rb);
			void SetClearColor(float r, float g, float b, float a);
			void ClearBuffer(bool clearColor = true, bool clearDepth = true, bool clearStencil = false);

			RenderBuffer* Renderbuffer() { return mCurrentState.Renderbuffer; }

			// Shader stuff
			void SetShader(ShaderProgram* shader);

			ShaderProgram* Shader() { return mCurrentState.Shader; }
			

		protected:
			GLenum ConvertRenderModeToNative(RenderMode mode);

			void CreateDefaultRB(int w, int h);
			//void CheckStateChanges();

			void SetShaderNative();
			void SetRenderbufferNative();
			void SetCullModeNative();
			void SetBlendModeNative();
			void SetDepthModeNative();

		protected:
			ContextState mCurrentState;

			std::vector<ContextState> mSavedStates;

	};

}

#endif
