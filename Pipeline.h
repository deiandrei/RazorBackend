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

		PipelineState() {
			Shader = nullptr;
		}

		void operator=(const PipelineState& other) {
			CullMode = other.CullMode;
			BlendMode = other.BlendMode;
			DepthMode = other.DepthMode;
			Shader = other.Shader;
		}
	};

	class Pipeline {
		public:
			Pipeline(int screenWidth, int screenHeight);
			~Pipeline() { }

			RenderBuffer* DefaultRenderBuffer;
			void SetRenderBuffer(RenderBuffer* rb);

			PipelineState State;

			void SaveState();
			void RestoreState();

			void RenderV(RenderMode mode, DataBuffer* buffer, int count, int startOffset = 0);
			void RenderI(RenderMode mode, DataBuffer* buffer, int count, int startOffset = 0);
			void RenderI(RenderMode mode, DataBuffer* buffer, int count, int indicesOffset, int verticesOffset);

		protected:
			GLenum ConvertRenderModeToNative(RenderMode mode);

			void CreateDefaultRB(int w, int h);
			void CheckStateChanges();

			void SetShaderNative();
			void SetCullModeNative();
			void SetBlendModeNative();
			void SetDepthModeNative();

		protected:
			PipelineState mLastState;
			RenderBuffer* mCurrentRB;

			std::vector<PipelineState> mSavedStates;

	};

}

#endif
