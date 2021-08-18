#ifndef PIPELINE_R_H
#define PIPELINE_R_H

#include "include.h"

namespace Backend {

	class Pipeline;

	class DataBuffer;
	class ShaderProgram;
	class RenderBuffer;

	enum CullingMode { CULL_NONE, CULL_FRONT, CULL_BACK, CULL_FRONT_AND_BACK };
	enum BlendingMode { BLEND_NONE, BLEND_DEFAULT }; // todo: implement all blend modes
	enum DepthTestMode { DEPTH_OFF, DEPTH_READ_ONLY, DEPTH_READ_WRITE };
	enum RenderMode { RENDER_LINES, RENDER_TRIANGLES };

	struct PipelineState {
		CullingMode CullMode;
		BlendingMode BlendMode;
		DepthTestMode DepthMode;

		ShaderProgram* Shader;
		RenderBuffer* Renderbuffer;

		PipelineState() {
			Shader = nullptr;
		}

		void operator=(const PipelineState& other) {
			CullMode = other.CullMode;
			BlendMode = other.BlendMode;
			DepthMode = other.DepthMode;
			Shader = other.Shader;
			Renderbuffer = other.Renderbuffer;
		}
	};

	class Pipeline {
		public:
			Pipeline(int screenWidth, int screenHeight);
			~Pipeline() { }

			RenderBuffer* DefaultRenderBuffer;

			// State setup and history
			PipelineState State;

			void SaveState();
			void RestoreState();

			// Rendering stuff
			void RenderV(RenderMode mode, DataBuffer* buffer, int count, int startOffset = 0);
			void RenderI(RenderMode mode, DataBuffer* buffer, int count, int startOffset = 0);
			void RenderI(RenderMode mode, DataBuffer* buffer, int count, int indicesOffset, int verticesOffset);

			// Render buffer stuff
			void SetClearColor(float r, float g, float b, float a);
			void ClearBuffer(bool clearColor = true, bool clearDepth = true, bool clearStencil = false);

		protected:
			GLenum ConvertRenderModeToNative(RenderMode mode);

			void CreateDefaultRB(int w, int h);
			void CheckStateChanges();

			void SetShaderNative();
			void SetRenderbufferNative();
			void SetCullModeNative();
			void SetBlendModeNative();
			void SetDepthModeNative();

		protected:
			PipelineState mLastState;

			std::vector<PipelineState> mSavedStates;

	};

}

#endif
