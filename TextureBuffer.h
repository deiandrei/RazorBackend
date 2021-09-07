#ifndef TEXTURE_BUFFER_R_H
#define TEXTURE_BUFFER_R_H

#include "include.h"

namespace Backend {

	enum TextureFace { TEXTURE_FACE_POSITIVE_X, TEXTURE_FACE_NEGATIVE_X, TEXTURE_FACE_POSITIVE_Y, TEXTURE_FACE_NEGATIVE_Y, TEXTURE_FACE_POSITIVE_Z, TEXTURE_FACE_NEGATIVE_Z, TEXTURE_FACE_PLANE };
	enum TextureType { TEXTURE_STANDARD, TEXTURE_CUBE };
	enum TextureFormat { TEXTURE_R_16, TEXTURE_R, TEXTURE_RG_16, TEXTURE_RG, TEXTURE_RGB_16, TEXTURE_RGB, TEXTURE_RGBA_16, TEXTURE_RGBA, TEXTURE_SRGB, TEXTURE_SRGBA, TEXTURE_DEPTH_16, TEXTURE_DEPTH_24, TEXTURE_DEPTH_32, TEXTURE_STENCIL, NUM_FORMATS };
	enum TextureWrapType { WRAP_NONE, WRAP_REPEAT, WRAP_CLAMP };
	enum TextureFilter { FILTER_NEAREST, FILTER_LINEAR };
	enum MipmapFilter { MIPMAP_FILTER_NONE, MIPMAP_FILTER_NEAREST, MIPMAP_FILTER_LINEAR };

	////

	class TextureBuffer {
		public:
			TextureBuffer(TextureType type = TextureType::TEXTURE_STANDARD);
			~TextureBuffer();

			GLuint& GetNativeHandle() { return mTextureRef; }

			TextureType GetType() { return mType; }
			TextureFormat GetFormat() { return mFormat; }

			// Data
			TextureBuffer* CreateFromFormat(TextureFormat format, int width, int height);
			TextureBuffer* UploadSubData(const void* dataPtr, int width, int height, int xOffset, int yOffset, TextureFace face = TextureFace::TEXTURE_FACE_PLANE, int layer = 0);
			TextureBuffer* UploadData(const void* dataPtr, int width, int height, int numComponents, bool srgb = false, TextureFace face = TextureFace::TEXTURE_FACE_PLANE, int layer = 0);
			TextureBuffer* UploadData(const void* dataPtr, int width, int height, TextureFormat format, TextureFace face = TextureFace::TEXTURE_FACE_PLANE, int layer = 0);
			TextureBuffer* GenerateMipmap();

			// Wrap
			TextureBuffer* SetWrapV(TextureWrapType type);
			TextureBuffer* SetWrapH(TextureWrapType type);
			TextureBuffer* SetWrapVH(TextureWrapType vWrapType, TextureWrapType hWrapType);

			TextureBuffer* SetBorderColor(float r, float g, float b, float a);

			// Filter
			TextureBuffer* SetFilterMin(TextureFilter filter, MipmapFilter mipmapFilter = MipmapFilter::MIPMAP_FILTER_NONE);
			TextureBuffer* SetFilterMag(TextureFilter filter, MipmapFilter mipmapFilter = MipmapFilter::MIPMAP_FILTER_NONE);
			TextureBuffer* SetFilterMinMag(TextureFilter minFilter, TextureFilter magFilter, MipmapFilter minMipmapFilter = MipmapFilter::MIPMAP_FILTER_NONE, MipmapFilter magMipmapFilter = MipmapFilter::MIPMAP_FILTER_NONE);

		private:
			void Bind();
			void BindForRendering(int level = 0);

			void SetWrapImpl(GLenum wrap, TextureWrapType wrapType);
			void SetFilterImpl(GLenum filter, TextureFilter filterType, MipmapFilter mipmapFilterType);
			void SetBorderColorImpl(float r, float g, float b, float a);
			void UploadDataImpl(const void* dataPtr, int width, int height, TextureFormat format, TextureFace face, int layer);

		private:
			GLuint mTextureRef;

			TextureType mType;

			TextureFormat mFormat;
			TextureWrapType mVWrap, mHWrap;
			TextureFilter mMinFilter, mMagFilter;
			MipmapFilter mMinMipmapFilter, mMagMipmapFilter;

			static const GLenum TextureTypeConvertNative[2];
			static const GLenum InternalFormatConvertNative[TextureFormat::NUM_FORMATS];
			static const GLenum FormatConvertNative[TextureFormat::NUM_FORMATS];
			
			friend class Context;

	};


}

#endif
