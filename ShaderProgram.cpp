#include "ShaderProgram.h"
#include <ostream>

namespace Backend {

	ShaderProgram::ShaderProgram() {
		mProgramHandle = glCreateProgram();
		mIsPrepared = false;
	}

	ShaderProgram::~ShaderProgram() {
		//todo: delete slots

		glDeleteProgram(mProgramHandle);
	}

	void ShaderProgram::Compile() {
		if (!HasSlot(ShaderSlotType::SHADER_VERTEX_SLOT) || !HasSlot(ShaderSlotType::SHADER_FRAGMENT_SLOT)) {
			mIsPrepared = false;
			return;
		}

		glLinkProgram(mProgramHandle);
		
		if (CheckForErrors(std::cout, GL_LINK_STATUS)) {
			mIsPrepared = false;
		}
		else {
			glValidateProgram(mProgramHandle);
			mIsPrepared = true;
		}
	}

	bool ShaderProgram::HasSlot(ShaderSlotType type) {
		return mSlots.find(type) != mSlots.end();
	}

	ShaderProgram* ShaderProgram::AddSlot(const std::string& source, ShaderSlotType type) {
		if (mSlots.find(type) != mSlots.end() || source.empty()) return this;

		ShaderSlot* slot = new ShaderSlot(type, source);
		if (!slot->Loaded()) {
			delete slot;
		}
		else {
			mSlots.insert({ type, slot });

			glAttachShader(mProgramHandle, slot->mShaderHandle);
		}

		return this;
	}

	ShaderProgram* ShaderProgram::ReloadSlot(const std::string& source, ShaderSlotType type) {
		if (mSlots.find(type) == mSlots.end()) return this;

		glDetachShader(mProgramHandle, mSlots[type]->mShaderHandle);
		mSlots.erase(type);

		return AddSlot(source, type);
	}

	ShaderProgram* ShaderProgram::SetAttributes(const std::vector<std::string>& attribs) {
		int idx = 0;
		for (auto attrib : attribs) {
			glBindAttribLocation(mProgramHandle, idx, attrib.c_str());
			mAttributes.push_back(attrib);
			idx++;
		}

		return this;
	}

	ShaderProgram* ShaderProgram::SetInt(const std::string& uniformName, int value) {
		glUniform1i(GetUniform(uniformName)->mBindingHandle, value);

		return this;
	}

	ShaderProgram* ShaderProgram::SetFloat(const std::string& uniformName, float value) {
		glUniform1f(GetUniform(uniformName)->mBindingHandle, value);

		return this;
	}

	ShaderProgram* ShaderProgram::SetFloat2(const std::string& uniformName, float value1, float value2) {
		glUniform2f(GetUniform(uniformName)->mBindingHandle, value1, value2);
		
		return this;
	}

	ShaderProgram* ShaderProgram::SetFloat3(const std::string& uniformName, float value1, float value2, float value3) {
		glUniform3f(GetUniform(uniformName)->mBindingHandle, value1, value2, value3);
		
		return this;
	}

	ShaderProgram* ShaderProgram::SetFloat4(const std::string& uniformName, float value1, float value2, float value3, float value4) {
		glUniform4f(GetUniform(uniformName)->mBindingHandle, value1, value2, value3, value4);
		
		return this;
	}

	ShaderProgram* ShaderProgram::SetMatrix4x4(const std::string& uniformName, float* matrix) {
		glUniformMatrix4fv(GetUniform(uniformName)->mBindingHandle, 1, GL_FALSE, matrix);

		return this;
	}

	void ShaderProgram::BindForRendering() {
		if (!mIsPrepared) return;

		glUseProgram(mProgramHandle);
	}

	ShaderUniform* ShaderProgram::GetUniform(const std::string& uniformName) {
		if (mUniforms.find(uniformName) == mUniforms.end()) {
			ShaderUniform* uniform = new ShaderUniform;

			uniform->mBindingHandle = glGetUniformLocation(mProgramHandle, uniformName.c_str());
			uniform->mBindingName = uniformName;

			mUniforms.insert({ uniformName, uniform });
		}

		return mUniforms[uniformName];
	}

	bool ShaderProgram::CheckForErrors(std::ostream& stream, GLuint flag) {
		GLint success = 0;
		GLchar error[1024] = { 0 };

		glGetProgramiv(mProgramHandle, flag, &success);

		if (!success) {
			glGetProgramInfoLog(mProgramHandle, sizeof(error), NULL, error);

			stream << "[Error] Shader program: " << error << std::endl;
		}

		return !success;
	}

	ShaderSlot::ShaderSlot(ShaderSlotType type, const std::string& source) {
		mShaderHandle = glCreateShader(ConvertTypeToNative(type));

		if (source.empty()) {
			mIsLoaded = false;
		}
		else {
			const GLchar* shaderSourcePtr = source.c_str();
			GLint shaderSourceSize = (GLint)source.length();

			glShaderSource(mShaderHandle, 1, &shaderSourcePtr, &shaderSourceSize);
			glCompileShader(mShaderHandle);

			mIsLoaded = !CheckErrors(GL_COMPILE_STATUS);
		}
	}

	GLenum ShaderSlot::ConvertTypeToNative(ShaderSlotType type) {
		if (type == ShaderSlotType::SHADER_VERTEX_SLOT) {
			return GL_VERTEX_SHADER;
		}
		else if (type == ShaderSlotType::SHADER_FRAGMENT_SLOT) {
			return GL_FRAGMENT_SHADER;
		}
		else {
			return GL_GEOMETRY_SHADER;
		}

	}

	ShaderSlot::~ShaderSlot() {

	}

	bool ShaderSlot::CheckErrors(GLuint flag) {
		GLint success = 0;
		GLchar error[1024] = { 0 };

		glGetShaderiv(mShaderHandle, flag, &success);

		if (!success) {
			glGetShaderInfoLog(mShaderHandle, sizeof(error), NULL, error);

			std::cerr << "[Error] Shader part: " << error << std::endl;
		}

		return !success;
	}

}
