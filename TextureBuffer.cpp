#include "TextureBuffer.h"

namespace Backend {
	TextureBuffer::TextureBuffer() {
		glGenTextures(1, &mTextureRef);

		SetWrapVH(TextureWrapType::WRAP_REPEAT, TextureWrapType::WRAP_REPEAT);
		SetFilterMinMag(TextureFilter::FILTER_NEAREST, TextureFilter::FILTER_NEAREST);
	}


	TextureBuffer::~TextureBuffer() {
		glDeleteTextures(1, &mTextureRef);
	}

	TextureBuffer* TextureBuffer::CreateFromFormat(TextureFormat format, int width, int height) {
		auto formatConvert = ConvertFormatToNative(format);
		mFormat = format;

		Bind();

		glTexImage2D(GL_TEXTURE_2D, 0, formatConvert.first, width, height, 0, formatConvert.second, GL_UNSIGNED_BYTE, NULL);

		return this;
	}

	TextureBuffer* TextureBuffer::UploadData(const void* dataPtr, int width, int height, int numComponents, bool srgb) {
		if (numComponents == 4) {
			mFormat = TextureFormat::TEXTURE_RGBA;
		}
		else if (numComponents == 3) {
			mFormat = TextureFormat::TEXTURE_RGB;
		}
		else if (numComponents == 2) {
			mFormat = TextureFormat::TEXTURE_RG;
		}
		else {
			mFormat = TextureFormat::TEXTURE_R;
		}

		return UploadData(dataPtr, width, height, mFormat, srgb);
	}

	TextureBuffer* TextureBuffer::UploadData(const void* dataPtr, int width, int height, TextureFormat format, bool srgb) {
		Bind();

		mFormat = format;
		auto formatConvert = ConvertFormatToNative(mFormat);
		if (srgb) {
			if (formatConvert.first == TextureFormat::TEXTURE_RGB) formatConvert.first = GL_SRGB;
			else if (formatConvert.first == TextureFormat::TEXTURE_RGBA) formatConvert.first = GL_SRGB_ALPHA;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, formatConvert.first, width, height, 0, formatConvert.second, GL_UNSIGNED_BYTE, dataPtr);
		if (mMinMipmapFilter != MipmapFilter::MIPMAP_FILTER_NONE || mMagMipmapFilter != MipmapFilter::MIPMAP_FILTER_NONE) GenerateMipmap();

		return this;
	}

	TextureBuffer* TextureBuffer::GenerateMipmap() {
		glGenerateMipmap(GL_TEXTURE_2D);

		return this;
	}

	void TextureBuffer::Bind() {
		glBindTexture(GL_TEXTURE_2D, mTextureRef);
	}

	void TextureBuffer::BindForRendering(int level) {
		glActiveTexture(GL_TEXTURE0 + level);
		Bind();
	}

	void TextureBuffer::SetWrapImpl(GLenum wrap, TextureWrapType wrapType) {
		GLint wrapTypeNative = 0;
		if (wrapType == TextureWrapType::WRAP_NONE) {
			wrapTypeNative = GL_CLAMP_TO_BORDER;

			SetBorderColorImpl(0.0f, 0.0f, 0.0f, 1.0f);
		}
		else if (wrapType == TextureWrapType::WRAP_REPEAT) {
			wrapTypeNative = GL_REPEAT;
		}
		else {
			wrapTypeNative = GL_CLAMP_TO_EDGE;
		}

		glTexParameteri(GL_TEXTURE_2D, wrap, wrapTypeNative);
	}

	void TextureBuffer::SetFilterImpl(GLenum filter, TextureFilter filterType, MipmapFilter mipmapType) {
		GLuint filterTypeNative = 0;
		if (filterType == TextureFilter::FILTER_LINEAR) {
			if (mipmapType == MipmapFilter::MIPMAP_FILTER_LINEAR) {
				filterTypeNative = GL_LINEAR_MIPMAP_LINEAR;
			}
			else if (mipmapType == MipmapFilter::MIPMAP_FILTER_NEAREST) {
				filterTypeNative = GL_LINEAR_MIPMAP_NEAREST;
			}
			else if (mipmapType == MipmapFilter::MIPMAP_FILTER_NONE) {
				filterTypeNative = GL_LINEAR;
			}
			else return;
		}
		else if (filterType == TextureFilter::FILTER_NEAREST) {
			if (mipmapType == MipmapFilter::MIPMAP_FILTER_LINEAR) {
				filterTypeNative = GL_NEAREST_MIPMAP_LINEAR;
			}
			else if (mipmapType == MipmapFilter::MIPMAP_FILTER_NEAREST) {
				filterTypeNative = GL_NEAREST_MIPMAP_NEAREST;
			}
			else if (mipmapType == MipmapFilter::MIPMAP_FILTER_NONE) {
				filterTypeNative = GL_NEAREST;
			}
			else return;
		}
		else return;

		glTexParameteri(GL_TEXTURE_2D, filter, filterTypeNative);
	}

	void TextureBuffer::SetBorderColorImpl(float r, float g, float b, float a) {
		float color[4] = { r, g, b, a };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &color[0]);
	}

	std::pair<GLenum, GLenum> TextureBuffer::ConvertFormatToNative(TextureFormat format)
	{
		// Return = internalFormat, Format

		if (format == TextureFormat::TEXTURE_R) {
			return { GL_R, GL_R };
		}
		else if (format == TextureFormat::TEXTURE_R_16) {
			return { GL_R16F, GL_RG };
		}
		else if (format == TextureFormat::TEXTURE_RG) {
			return { GL_RG, GL_RG };
		}
		else if (format == TextureFormat::TEXTURE_RG_16) {
			return { GL_RG16F, GL_RG };
		}
		else if (format == TextureFormat::TEXTURE_RGB) {
			return { GL_RGB, GL_RGB };
		}
		else if (format == TextureFormat::TEXTURE_RGB_16) {
			return { GL_RGB16F, GL_RGB };
		}
		else if (format == TextureFormat::TEXTURE_RGBA) {
			return { GL_RGBA, GL_RGBA };
		}
		else if (format == TextureFormat::TEXTURE_RGBA_16) {
			return { GL_RGBA16F, GL_RGBA };
		}
		else if (format == TextureFormat::TEXTURE_DEPTH_16) {
			return { GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT };
		}
		else if (format == TextureFormat::TEXTURE_DEPTH_24) {
			return { GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT };
		}
		else if (format == TextureFormat::TEXTURE_DEPTH_32) {
			return { GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT };
		}
		/*else if (format == TextureFormat::TEXTURE_STENCIL) {
			formatNative = GL_STENCIL_COMPONENTS;
			internalformatNative = GL_DEPTH_COMPONENT16;
		}*/

		return { 0, 0 };
	}

	TextureBuffer* TextureBuffer::SetWrapV(TextureWrapType type) {
		Bind();

		mVWrap = type;
		SetWrapImpl(GL_TEXTURE_WRAP_S, type);

		return this;
	}

	TextureBuffer* TextureBuffer::SetWrapH(TextureWrapType type) {
		Bind();

		mHWrap = type;
		SetWrapImpl(GL_TEXTURE_WRAP_T, type);

		return this;
	}

	TextureBuffer* TextureBuffer::SetWrapVH(TextureWrapType vWrapType, TextureWrapType hWrapType) {
		Bind();

		mVWrap = vWrapType;
		mHWrap = hWrapType;
		SetWrapImpl(GL_TEXTURE_WRAP_S, vWrapType);
		SetWrapImpl(GL_TEXTURE_WRAP_T, hWrapType);

		return this;
	}

	TextureBuffer* TextureBuffer::SetBorderColor(float r, float g, float b, float a) {
		Bind();

		SetBorderColorImpl(r, g, b, a);
		
		return this;
	}

	TextureBuffer* TextureBuffer::SetFilterMin(TextureFilter filter, MipmapFilter mipmapFilter) {
		Bind();

		mMinFilter = filter;
		mMinMipmapFilter = mipmapFilter;
		SetFilterImpl(GL_TEXTURE_MIN_FILTER, filter, mipmapFilter);

		return this;
	}

	TextureBuffer* TextureBuffer::SetFilterMag(TextureFilter filter, MipmapFilter mipmapFilter) {
		Bind();

		mMagFilter = filter;
		mMagMipmapFilter = mipmapFilter;
		SetFilterImpl(GL_TEXTURE_MAG_FILTER, filter, mipmapFilter);

		return this;
	}

	TextureBuffer* TextureBuffer::SetFilterMinMag(TextureFilter minFilter, TextureFilter magFilter, MipmapFilter minMipmapFilter, MipmapFilter magMipmapFilter) {
		Bind();

		mMinFilter = minFilter;
		mMinMipmapFilter = minMipmapFilter;
		mMagFilter = magFilter;
		mMagMipmapFilter = magMipmapFilter;
		SetFilterImpl(GL_TEXTURE_MIN_FILTER, minFilter, minMipmapFilter);
		SetFilterImpl(GL_TEXTURE_MAG_FILTER, magFilter, magMipmapFilter);

		return this;
	}

}
