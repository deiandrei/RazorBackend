#include "RenderBuffer.h"

namespace Backend {

	unsigned int RenderBuffer::MAX_COLOR_ATTACHMENTS = 8;

	RenderBuffer::RenderBuffer(int w, int h) {
		glGenFramebuffers(1, &mBufferHandle);

		mWidth = w;
		mHeight = h;

		mColorAttachmentsCount = 0;

		// temp
		mUsedSave = false;
	}

	RenderBuffer::~RenderBuffer() {
		//todo
	}

	void RenderBuffer::Resize(int w, int h) {
		mWidth = w;
		mHeight = h;

		for (auto slot : mSlots) {
			if (slot.second->mOwnedByRenderbuffer) {
				slot.second->mTexture->CreateFromFormat(slot.second->mTexture->GetFormat(), w, h);
			}
		}
	}

	void RenderBuffer::Bind(BindingType bindType, bool saveLastRB) {
		if (saveLastRB) {
			GLint vp[4]; glGetIntegerv(GL_VIEWPORT, vp);
			mLastWidth = vp[2]; mLastHeight = vp[3];
			glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &mLastRBHandle);
			mUsedSave = true;
		}

		GLenum targetBindNative;
		if (bindType == BindingType::RENDERBUFFER_DRAW) {
			targetBindNative = GL_DRAW_FRAMEBUFFER;
		}
		else if (bindType == BindingType::RENDERBUFFER_READ) {
			targetBindNative = GL_READ_FRAMEBUFFER;
		}
		else {
			targetBindNative = GL_FRAMEBUFFER;
		}

		glBindFramebuffer(targetBindNative, mBufferHandle);

		if (saveLastRB) {
			if (mLastWidth != mWidth || mLastHeight != mHeight) glViewport(0, 0, mWidth, mHeight);
		}
	}

	void RenderBuffer::Unbind() {
		if (mUsedSave) {
			mUsedSave = false;

			glBindFramebuffer(GL_FRAMEBUFFER, mLastRBHandle);

			if (mLastWidth != mWidth || mLastHeight != mHeight) glViewport(0, 0, mLastWidth, mLastHeight);
		}
		else {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

	void RenderBuffer::Copy(RenderBuffer* destination, AttachmentType copyType) {
		if (!destination) return;

		this->Bind(BindingType::RENDERBUFFER_READ, true);
		destination->Bind(BindingType::RENDERBUFFER_DRAW, true);

		glBlitFramebuffer(0, 0, mWidth, mHeight, 0, 0, destination->mWidth, destination->mHeight, ConvertAttachmentToBitfield(copyType), GL_NEAREST);

		destination->Unbind();
		this->Unbind();
	}

	void RenderBuffer::CopyLegacy(GLuint destination, int w, int h, AttachmentType copyType) {
		this->Bind(BindingType::RENDERBUFFER_READ, true);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destination);

		glBlitFramebuffer(0, 0, mWidth, mHeight, 0, 0, w, h, ConvertAttachmentToBitfield(copyType), GL_NEAREST);

		this->Unbind();
	}

	void RenderBuffer::AddSlotImpl(const std::string& name, AttachmentType type, TextureBuffer* tex, bool owned) {
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
		RenderBufferSlot* slot = new RenderBufferSlot;
		slot->mType = type;
		slot->mTexture = tex;
		slot->mColorAttID = mColorAttachmentsCount++;
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
			attachmentTypeNative = GL_COLOR_ATTACHMENT0 + slot->mColorAttID;
		}
		else return;

		// Setup the slot
		Bind();

		glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentTypeNative, GL_TEXTURE_2D, tex->GetNativeHandle(), 0);
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

	RenderBuffer* RenderBuffer::AddSlot(const std::string& name, AttachmentType type, TextureBuffer* tex) {
		AddSlotImpl(name, type, tex, false);

		return this;
	}

	RenderBuffer* RenderBuffer::AddSlot(const std::string& name, AttachmentType type, TextureFormat textureFormat) {
		TextureBuffer* tex = new TextureBuffer;
		tex->CreateFromFormat(textureFormat, mWidth, mHeight)->SetFilterMinMag(TextureFilter::FILTER_LINEAR, TextureFilter::FILTER_LINEAR);

		AddSlotImpl(name, type, tex, true);

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

	RenderBuffer* RenderBuffer::SetSlotsUsedToDraw(const std::vector<std::string>& slots) {
		std::vector<GLuint> drawBuffersNative;
		
		for (auto& key : slots) {
			if (mSlots.find(key) != mSlots.end()) {
				if (mSlots[key]->Type() != AttachmentType::ATTACHMENT_COLOR) continue;

				drawBuffersNative.push_back(GL_COLOR_ATTACHMENT0 + mSlots[key]->mColorAttID);
			}
		}

		glDrawBuffers((int)drawBuffersNative.size(), &drawBuffersNative[0]);

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
