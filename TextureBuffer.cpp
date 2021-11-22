#include "TextureBuffer.h"

namespace Backend {
	const GLenum TextureBuffer::TextureTypeConvertNative[2] = { GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP };
	const GLenum TextureBuffer::InternalFormatConvertNative[TextureFormat::NUM_FORMATS] = { GL_R16F, GL_RED, GL_RG16F, GL_RG, GL_RGB16F, GL_RGB, GL_RGBA16F, GL_RGBA, GL_SRGB, GL_SRGB_ALPHA, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32 };
	const GLenum TextureBuffer::FormatConvertNative[TextureFormat::NUM_FORMATS] = { GL_RED, GL_RED, GL_RG, GL_RG, GL_RGB, GL_RGB, GL_RGBA, GL_RGBA, GL_RGB, GL_RGBA, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT };

	TextureBuffer::TextureBuffer(Context* ctx, TextureType type) {
		mContext = ctx;

		glGenTextures(1, &mTextureRef);

		mType = type;

		SetWrapVH(TextureWrapType::WRAP_REPEAT, TextureWrapType::WRAP_REPEAT);
		SetFilterMinMag(TextureFilter::FILTER_NEAREST, TextureFilter::FILTER_NEAREST);
	}


	TextureBuffer::~TextureBuffer() {
		glDeleteTextures(1, &mTextureRef);
	}

	TextureBuffer* TextureBuffer::CreateFromFormat(TextureFormat format, int width, int height) {
		mFormat = format;

		Bind();

		if (mType == TextureType::TEXTURE_STANDARD) {
			glTexImage2D(TextureTypeConvertNative[mType], 0, InternalFormatConvertNative[format], width, height, 0, FormatConvertNative[format], GetDatatypeFromFormat(), NULL);
		}
		else if (mType == TextureType::TEXTURE_CUBE) {
			for (int i = 0; i < 6; ++i) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, InternalFormatConvertNative[format], width, height, 0, FormatConvertNative[format], GetDatatypeFromFormat(), NULL);
				
			}
		}

		return this;
	}

	TextureBuffer* TextureBuffer::UploadSubData(const void* dataPtr, int width, int height, int xOffset, int yOffset, TextureFace face, int layer) {
		Bind();
		
		if (mType == TextureType::TEXTURE_STANDARD) {
			glTexSubImage2D(TextureTypeConvertNative[mType], layer, xOffset, yOffset, width, height, FormatConvertNative[mFormat], GetDatatypeFromFormat(), dataPtr);
		}
		else if (mType == TextureType::TEXTURE_CUBE) {
			if (face == TextureFace::TEXTURE_FACE_PLANE) return this;

			glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, layer, xOffset, yOffset, width, height, FormatConvertNative[mFormat], GetDatatypeFromFormat(), dataPtr);
		}

		return this;
	}

	TextureBuffer* TextureBuffer::UploadData(const void* dataPtr, int width, int height, int numComponents, bool srgb, TextureFace face, int layer) {
		Bind();

		if (numComponents == 4) {
			if (srgb) mFormat = TextureFormat::TEXTURE_SRGBA;
			else mFormat = TextureFormat::TEXTURE_RGBA;
		}
		else if (numComponents == 3) {
			if (srgb) mFormat = TextureFormat::TEXTURE_SRGB;
			else mFormat = TextureFormat::TEXTURE_RGB;
		}
		else if (numComponents == 2) {
			mFormat = TextureFormat::TEXTURE_RG;
		}
		else {
			mFormat = TextureFormat::TEXTURE_R;
		}

		UploadDataImpl(dataPtr, width, height, mFormat, face, layer);

		return this;
	}

	TextureBuffer* TextureBuffer::UploadData(const void* dataPtr, int width, int height, TextureFormat format, TextureFace face, int layer) {
		Bind();

		UploadDataImpl(dataPtr, width, height, format, face, layer);

		return this;
	}

	TextureBuffer* TextureBuffer::GenerateMipmap() {
		glGenerateMipmap(TextureTypeConvertNative[mType]);

		return this;
	}

	void TextureBuffer::Bind() {
		glBindTexture(TextureTypeConvertNative[mType], mTextureRef);
	}

	void TextureBuffer::BindForRendering(int level) {
		glActiveTexture(GL_TEXTURE0 + level);
		Bind();
	}

	GLenum TextureBuffer::GetDatatypeFromFormat() {
		if (mFormat == TextureFormat::TEXTURE_R_16 || mFormat == TextureFormat::TEXTURE_RG_16 || mFormat == TextureFormat::TEXTURE_RGB_16 || mFormat == TextureFormat::TEXTURE_RGBA_16) {
			return GL_FLOAT;
		}

		return GL_UNSIGNED_BYTE;
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

		glTexParameteri(TextureTypeConvertNative[mType], wrap, wrapTypeNative);
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

		glTexParameteri(TextureTypeConvertNative[mType], filter, filterTypeNative);
	}

	void TextureBuffer::SetBorderColorImpl(float r, float g, float b, float a) {
		float color[4] = { r, g, b, a };
		glTexParameterfv(TextureTypeConvertNative[mType], GL_TEXTURE_BORDER_COLOR, &color[0]);
	}

	void TextureBuffer::UploadDataImpl(const void* dataPtr, int width, int height, TextureFormat format, TextureFace face, int layer) {
		mFormat = format;

		GLenum internalFormatNative = InternalFormatConvertNative[format];
		GLenum formatNative = FormatConvertNative[format];

		if (mType == TextureType::TEXTURE_STANDARD) {
			glTexImage2D(TextureTypeConvertNative[mType], layer, internalFormatNative, width, height, 0, formatNative, GetDatatypeFromFormat(), dataPtr);
		}
		else if (mType == TextureType::TEXTURE_CUBE) {
			if (face == TextureFace::TEXTURE_FACE_PLANE) return;

			//do the smart conversion
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, layer, internalFormatNative, width, height, 0, formatNative, GetDatatypeFromFormat(), dataPtr);
		}

		if (mMinMipmapFilter != MipmapFilter::MIPMAP_FILTER_NONE || mMagMipmapFilter != MipmapFilter::MIPMAP_FILTER_NONE) GenerateMipmap();
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
