#ifndef PIPELINE_R_H
#define PIPELINE_R_H

#include "include.h"

namespace Backend {

	class Pipeline;

	class DataBuffer;
	class ShaderProgram;

	enum CullingMode { CULL_NONE, CULL_FRONT, CULL_BACK, CULL_FRONT_AND_BACK };
	enum BlendingMode { BLEND_NONE, BLEND_DEFAULT }; // todo: implement all blend modes
	enum DepthTestMode { DEPTH_OFF, DEPTH_READ_ONLY, DEPTH_READ_WRITE };
	enum RenderMode { RENDER_LINES, RENDER_TRIANGLES };

	struct PipelineState {
		CullingMode CullMode;
		BlendingMode BlendMode;
		DepthTestMode DepthMode;

		ShaderProgram* Shader;

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

			PipelineState State;

			void RenderV(RenderMode mode, DataBuffer* buffer, int count, int startOffset = 0);
			void RenderI(RenderMode mode, DataBuffer* buffer, int count, int startOffset = 0);
			void RenderI(RenderMode mode, DataBuffer* buffer, int count, int indicesOffset, int verticesOffset);

			

		protected:
			GLenum ConvertRenderModeToNative(RenderMode mode);

			void CheckStateChanges();

			void SetShaderNative();
			void SetCullModeNative();
			void SetBlendModeNative();
			void SetDepthModeNative();

		protected:
			PipelineState mLastState;

	};

}

#endif
