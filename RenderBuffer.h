#ifndef RENDER_BUFFER_R_H
#define RENDER_BUFFER_R_H

#include "include.h"
#include "TextureBuffer.h"

namespace Backend {

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
			RenderBuffer(const glm::vec2& size) : RenderBuffer(size.x, size.y) { }
			~RenderBuffer();

			// PLACEHOLDER
			GLuint GetNativeHandle() { return mBufferHandle; }

			// Attachments
			RenderBuffer* AddSlot(const std::string& name, AttachmentType type, TextureBuffer* tex);
			RenderBuffer* AddSlot(const std::string& name, AttachmentType type, TextureFormat textureFormat);

			RenderBufferSlot* GetSlot(const std::string& name);
			RenderBufferSlot* GetSlotByType(AttachmentType type);

			RenderBuffer* DeleteSlot(const std::string& name);
			RenderBuffer* DeleteSlotByType(AttachmentType type);

			RenderBuffer* SetSlotsUsedToDraw(const std::vector<std::string>& slots);
			RenderBuffer* UseAllSlotsToDraw();

			// Basic stuff
			void Bind(BindingType bindType = BindingType::RENDERBUFFER_READWRITE, bool saveLastRB = false);
			void Unbind();

			void Copy(RenderBuffer* destination, AttachmentType copyType);
			void CopyLegacy(GLuint destination, int w, int h, AttachmentType copyType);

			int GetWidth() { return mWidth; }
			int GetHeight() { return mHeight; }

			static void Clear(bool clearColor, bool clearDepth, bool clearStencil);
			static void SetClearColor(const glm::vec4 color);

		private:
			void AddSlotImpl(const std::string& name, AttachmentType type, TextureBuffer* tex, bool owned);
			static GLbitfield ConvertAttachmentToBitfield(AttachmentType type);

		private:
			GLuint mBufferHandle;
			int mWidth, mHeight;

			std::map<std::string, RenderBufferSlot*> mSlots;
			unsigned int mColorAttachmentsCount;

			// Save data, until I implement the state machine
			int mLastWidth, mLastHeight;
			GLint mLastRBHandle;
			bool mUsedSave;

	};

}

#endif
