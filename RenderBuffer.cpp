#include "RenderBuffer.h"

namespace Backend {

	unsigned int RenderBuffer::MAX_COLOR_ATTACHMENTS = 8;

	RenderBuffer::RenderBuffer(int w, int h) {
		glGenFramebuffers(1, &mBufferHandle);

		mWidth = w;
		mHeight = h;

		mColorAttachmentsCount = 0;
	}

	RenderBuffer::~RenderBuffer() {
		//todo
	}

	void RenderBuffer::Resize(int w, int h) {
		mWidth = w;
		mHeight = h;

		for (auto slot : mSlots) {
			if (slot.second->mOwnedByRenderbuffer) {
				TextureFormat tempFormat = slot.second->mTexture->GetFormat();
				slot.second->mTexture->CreateFromFormat(tempFormat, w, h);
			}
		}
	}

	void RenderBuffer::Copy(RenderBuffer* destination, AttachmentType copyType) {
		if (!destination) return;

		// save the last state
		int tempWidth, tempHeight;
		GLint vp[4], tempHandleNative;
		glGetIntegerv(GL_VIEWPORT, vp); tempWidth = vp[2]; tempHeight = vp[3];
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &tempHandleNative);

		int destWidth = destination->GetWidth();
		int destHeight = destination->GetHeight();
		
		if (destWidth != tempWidth || destHeight != tempHeight) {
			glViewport(0, 0, destWidth, destHeight);
		}
		
		glBindFramebuffer(GL_READ_FRAMEBUFFER, mBufferHandle);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destination->mBufferHandle);

		glBlitFramebuffer(0, 0, mWidth, mHeight, 0, 0, destination->mWidth, destination->mHeight, ConvertAttachmentToBitfield(copyType), GL_NEAREST);

		// restore the last state
		if (destWidth != tempWidth || destHeight != tempHeight) {
			glViewport(0, 0, tempWidth, tempWidth);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, tempHandleNative);
	}

	void RenderBuffer::AddSlotImpl(const std::string& name, AttachmentType type, TextureBuffer* tex, TextureFace face, int level, bool owned) {
		// check if there is already an attachment with this type, we can only have 1 depth/stencil attachment
		if (type != AttachmentType::ATTACHMENT_COLOR) {
			for (auto& slot : mSlots) {
				if (slot.second->Type() == type) return;
			}
		}
		// also check if there are some color attachments still available
		else {
			if (mColorAttachmentsCount >= MAX_COLOR_ATTACHMENTS) return;
		}

		// Add the slot to the list
		RenderBufferSlot* slot = new RenderBufferSlot();
		slot->mType = type;
		slot->mLevel = level;
		slot->mTexture = tex;
		slot->mOwnedByRenderbuffer = owned;

		mSlots.insert({ name, slot });

		GLenum attachmentTypeNative;
		if (type == AttachmentType::ATTACHMENT_DEPTH) {
			attachmentTypeNative = GL_DEPTH_ATTACHMENT;
		}
		else if (type == AttachmentType::ATTACHMENT_STENCIL) {
			attachmentTypeNative = GL_STENCIL_ATTACHMENT;
		}
		else if (type == AttachmentType::ATTACHMENT_COLOR) {
			slot->mColorAttID = mColorAttachmentsCount++;
			attachmentTypeNative = GL_COLOR_ATTACHMENT0 + slot->mColorAttID;
		}
		else return;

		// Setup the slot
		Bind();

		if (tex->GetType() == TextureType::TEXTURE_STANDARD) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentTypeNative, GL_TEXTURE_2D, tex->GetNativeHandle(), level);
		}
		else if(tex->GetType() == TextureType::TEXTURE_CUBE) {
			if (face == TextureFace::TEXTURE_FACE_PLANE) face = TextureFace::TEXTURE_FACE_POSITIVE_X;

			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentTypeNative, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, tex->GetNativeHandle(), level);
		}

		slot->mFace = face;
	}

	GLbitfield RenderBuffer::ConvertAttachmentToBitfield(AttachmentType type) {
		if (type == AttachmentType::ATTACHMENT_DEPTH) {
			return GL_DEPTH_BUFFER_BIT;
		}
		else if (type == AttachmentType::ATTACHMENT_STENCIL) {
			return GL_STENCIL_BUFFER_BIT;
		}
		else {
			return GL_COLOR_BUFFER_BIT;
		}
	}

	void RenderBuffer::Bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, mBufferHandle);
	}

	RenderBuffer* RenderBuffer::AddSlot(const std::string& name, AttachmentType type, TextureBuffer* tex, TextureFace face, int level) {
		AddSlotImpl(name, type, tex, face, level, false);

		return this;
	}

	RenderBuffer* RenderBuffer::AddSlot(const std::string& name, AttachmentType type, TextureFormat textureFormat) {
		TextureBuffer* tex = new TextureBuffer;
		tex->CreateFromFormat(textureFormat, mWidth, mHeight)->SetFilterMinMag(TextureFilter::FILTER_LINEAR, TextureFilter::FILTER_LINEAR);

		AddSlotImpl(name, type, tex, TextureFace::TEXTURE_FACE_PLANE, 0, true);

		return this;
	}

	RenderBufferSlot* RenderBuffer::GetSlot(const std::string& name) {
		if (mSlots.find(name) != mSlots.end()) return mSlots[name];

		return nullptr;
	}

	RenderBufferSlot* RenderBuffer::GetSlotByType(AttachmentType type) {
		for (auto slot : mSlots) {
			if (slot.second->Type() == type) return slot.second;
		}

		return nullptr;
	}

	RenderBuffer* RenderBuffer::DeleteSlot(const std::string& name) {
		auto itr = mSlots.find(name);
		if (itr != mSlots.end()) {
			RenderBufferSlot* slot = mSlots[name];

			if (slot->mOwnedByRenderbuffer) {
				delete slot->mTexture;
			}

			mSlots.erase(itr);
		}

		return this;
	}

	RenderBuffer* RenderBuffer::DeleteSlotByType(AttachmentType type) {
		std::vector<std::string> nameList;

		for (auto slot : mSlots) {
			if (slot.second->Type() == type) {
				nameList.push_back(slot.first);
			}
		}

		for (auto name : nameList) {
			DeleteSlot(name);
		}

		return this;
	}

	RenderBuffer* RenderBuffer::ReplaceSlotTexture(const std::string& name, TextureBuffer* tex, TextureFace face, int level) {
		if (mSlots.find(name) == mSlots.end() || !tex) return this;

		auto slot = mSlots[name];

		if (slot->mOwnedByRenderbuffer) {
			delete slot->mTexture;
		}

		slot->mOwnedByRenderbuffer = false;
		slot->mTexture = tex;
		slot->mLevel = level;

		GLenum attachmentTypeNative;
		if (slot->mType == AttachmentType::ATTACHMENT_DEPTH) {
			attachmentTypeNative = GL_DEPTH_ATTACHMENT;
		}
		else if (slot->mType == AttachmentType::ATTACHMENT_STENCIL) {
			attachmentTypeNative = GL_STENCIL_ATTACHMENT;
		}
		else if (slot->mType == AttachmentType::ATTACHMENT_COLOR) {
			attachmentTypeNative = GL_COLOR_ATTACHMENT0 + slot->mColorAttID;
		}
		else return this;

		// Setup the slot
		Bind();

		if (tex->GetType() == TextureType::TEXTURE_STANDARD) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentTypeNative, GL_TEXTURE_2D, tex->GetNativeHandle(), level);
		}
		else if (tex->GetType() == TextureType::TEXTURE_CUBE) {
			if (face == TextureFace::TEXTURE_FACE_PLANE) face = TextureFace::TEXTURE_FACE_POSITIVE_X;

			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentTypeNative, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, tex->GetNativeHandle(), level);
		}

		slot->mFace = face;

		return this;
	}

	RenderBuffer* RenderBuffer::SetSlotsUsedToDraw(const std::vector<std::string>& slots) {
		std::vector<GLuint> drawBuffersNative;
		
		for (auto& key : slots) {
			if (mSlots.find(key) != mSlots.end()) {
				if (mSlots[key]->Type() != AttachmentType::ATTACHMENT_COLOR) continue;

				drawBuffersNative.push_back(GL_COLOR_ATTACHMENT0 + mSlots[key]->mColorAttID);
			}
		}

		if (drawBuffersNative.empty()) {
			glDrawBuffer(GL_NONE);
		}
		else {
			glDrawBuffers((int)drawBuffersNative.size(), &drawBuffersNative[0]);
		}


		return this;
	}

	RenderBuffer* RenderBuffer::UseAllSlotsToDraw() {
		std::vector<GLuint> drawBuffersNative;

		for (auto slot : mSlots) {
			if (slot.second->Type() != AttachmentType::ATTACHMENT_COLOR) continue;

			drawBuffersNative.push_back(GL_COLOR_ATTACHMENT0 + slot.second->mColorAttID);
		}

		glDrawBuffers((int)drawBuffersNative.size(), &drawBuffersNative[0]);

		return this;
	}

}
