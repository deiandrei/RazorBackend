#ifndef TEXTURE_BUFFER_R_H
#define TEXTURE_BUFFER_R_H

#include "include.h"

namespace Backend {

	enum TextureFormat { TEXTURE_R_16, TEXTURE_R, TEXTURE_RG_16, TEXTURE_RG, TEXTURE_RGB_16, TEXTURE_RGB, TEXTURE_RGBA_16, TEXTURE_RGBA, TEXTURE_DEPTH_16, TEXTURE_DEPTH_24, TEXTURE_DEPTH_32, TEXTURE_STENCIL };
	enum TextureWrapType { WRAP_NONE, WRAP_REPEAT, WRAP_CLAMP };
	enum TextureFilter { FILTER_NEAREST, FILTER_LINEAR };
	enum MipmapFilter { MIPMAP_FILTER_NONE, MIPMAP_FILTER_NEAREST, MIPMAP_FILTER_LINEAR };

	class TextureBuffer {
		public:
			TextureBuffer();
			~TextureBuffer();

			GLuint& GetNativeHandle() { return mTextureRef; }

			TextureFormat GetFormat() { return mFormat; }

			// PLACEHOLDER
			void BindForRendering(int level = 0);

			// Data
			TextureBuffer* CreateFromFormat(TextureFormat format, int width, int height);
			TextureBuffer* UploadData(const void* dataPtr, int width, int height, int numComponents, bool srgb = false);
			TextureBuffer* UploadData(const void* dataPtr, int width, int height, TextureFormat format, bool srgb = false);
			TextureBuffer* GenerateMipmap();

			// Wrap
			TextureBuffer* SetWrapV(TextureWrapType type);
			TextureBuffer* SetWrapH(TextureWrapType type);
			TextureBuffer* SetWrapVH(TextureWrapType vWrapType, TextureWrapType hWrapType);

			// Filter
			TextureBuffer* SetFilterMin(TextureFilter filter, MipmapFilter mipmapFilter = MipmapFilter::MIPMAP_FILTER_NONE);
			TextureBuffer* SetFilterMag(TextureFilter filter, MipmapFilter mipmapFilter = MipmapFilter::MIPMAP_FILTER_NONE);
			TextureBuffer* SetFilterMinMag(TextureFilter minFilter, TextureFilter magFilter, MipmapFilter minMipmapFilter = MipmapFilter::MIPMAP_FILTER_NONE, MipmapFilter magMipmapFilter = MipmapFilter::MIPMAP_FILTER_NONE);

		private:
			void Bind();

			void SetWrapImpl(GLenum wrap, TextureWrapType wrapType);
			void SetFilterImpl(GLenum filter, TextureFilter filterType, MipmapFilter mipmapFilterType);

			std::pair<GLenum, GLenum> ConvertFormatToNative(TextureFormat format);

		private:
			GLuint mTextureRef;

			TextureFormat mFormat;
			TextureWrapType mVWrap, mHWrap;
			TextureFilter mMinFilter, mMagFilter;
			MipmapFilter mMinMipmapFilter, mMagMipmapFilter;

	};


}

#endif
