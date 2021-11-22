#ifndef SHADER_PROGRAM_R_H
#define SHADER_PROGRAM_R_H

#include "include.h"

namespace Backend {
	class Context;
	class ShaderProgram;
	class ShaderUniform;
	class ShaderSlot;

	enum ShaderSlotType { SHADER_VERTEX_SLOT, SHADER_FRAGMENT_SLOT, SHADER_GEOMETRY_SLOT };

	class ShaderSlot {
		public:
			~ShaderSlot();

			ShaderSlotType Type() { return mType; }
			bool Loaded() { return mIsLoaded; }

		private:
			ShaderSlot(ShaderSlotType type, const std::string& source);

			GLenum ConvertTypeToNative(ShaderSlotType type);
			bool CheckErrors(GLuint flag);

		private:
			GLuint mShaderHandle;

			ShaderSlotType mType;
			bool mIsLoaded;

			friend class ShaderProgram;

	};

	class ShaderUniform {
		protected:
			ShaderUniform() { mBindingHandle = 0; }

			std::string mBindingName;
			GLint mBindingHandle;

			friend class ShaderProgram;
	};

	class ShaderProgram {
		public:
			ShaderProgram();
			~ShaderProgram();

			void Compile();

			// Slots and attribs
			bool HasSlot(ShaderSlotType type);
			ShaderProgram* AddSlot(const std::string& source, ShaderSlotType type);
			ShaderProgram* ReloadSlot(const std::string& source, ShaderSlotType type);
			
			ShaderProgram* SetAttributes(const std::vector<std::string>& attribs);

			// Uniform setters
			ShaderProgram* SetInt(const std::string& uniformName, int value);
			ShaderProgram* SetFloat(const std::string& uniformName, float value);
			ShaderProgram* SetFloat2(const std::string& uniformName, float value1, float value2);
			ShaderProgram* SetFloat3(const std::string& uniformName, float value1, float value2, float value3);
			ShaderProgram* SetFloat4(const std::string& uniformName, float value1, float value2, float value3, float value4);
			ShaderProgram* SetMatrix4x4(const std::string& uniformName, float* matrix);

			// PLACEHOLDER
			void BindForRendering();

		private:
			ShaderUniform* GetUniform(const std::string& uniformName);
			bool CheckForErrors(std::ostream& stream, GLuint flag);

		private:
			GLuint mProgramHandle;
			bool mIsPrepared;

			std::map<std::string, ShaderUniform*> mUniforms;
			std::map<ShaderSlotType, ShaderSlot*> mSlots;
			std::vector<std::string> mAttributes;

		protected:
			Context* mContext;

			friend class Context;

	};

}

#endif
