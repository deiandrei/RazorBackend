#ifndef RENDER_BUFFER_R_H
#define RENDER_BUFFER_R_H

#include "include.h"
#include "TextureBuffer.h"

namespace Backend {
	class Pipeline;
	class RenderBuffer;
	class RenderBufferSlot;

	enum AttachmentType { ATTACHMENT_DEPTH, ATTACHMENT_STENCIL, ATTACHMENT_COLOR };
	enum BindingType { RENDERBUFFER_READ, RENDERBUFFER_DRAW, RENDERBUFFER_READWRITE };

	class RenderBufferSlot {
		public:
			AttachmentType Type() { return mType; }
			TextureBuffer* Texture() { return mTexture; }

		protected:
			RenderBufferSlot() { mColorAttID = -1; mTexture = nullptr; }

			AttachmentType mType;
			TextureBuffer* mTexture;
			int mColorAttID;
			bool mOwnedByRenderbuffer;

			friend class RenderBuffer;

	};

	class RenderBuffer {
		public:
			static unsigned int MAX_COLOR_ATTACHMENTS;

		public:
			RenderBuffer(int w, int h);
			~RenderBuffer();

			void Resize(int w, int h);

			// Attachments
			RenderBuffer* AddSlot(const std::string& name, AttachmentType type, TextureBuffer* tex);
			RenderBuffer* AddSlot(const std::string& name, AttachmentType type, TextureFormat textureFormat);

			RenderBufferSlot* GetSlot(const std::string& name);
			RenderBufferSlot* GetSlotByType(AttachmentType type);

			RenderBuffer* DeleteSlot(const std::string& name);
			RenderBuffer* DeleteSlotByType(AttachmentType type);

			RenderBuffer* ReplaceSlotTexture(const std::string& name, TextureBuffer* tex, int level = 0);

			RenderBuffer* SetSlotsUsedToDraw(const std::vector<std::string>& slots);
			RenderBuffer* UseAllSlotsToDraw();

			// Basic stuff
			void Copy(RenderBuffer* destination, AttachmentType copyType);

			int GetWidth() { return mWidth; }
			int GetHeight() { return mHeight; }

		private:
			void AddSlotImpl(const std::string& name, AttachmentType type, TextureBuffer* tex, bool owned);
			static GLbitfield ConvertAttachmentToBitfield(AttachmentType type);

			void Bind();

		private:
			GLuint mBufferHandle;
			int mWidth, mHeight;

			std::map<std::string, RenderBufferSlot*> mSlots;
			unsigned int mColorAttachmentsCount;

			friend class Context;

	};

}

#endif
