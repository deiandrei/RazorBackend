#include "DataBuffer.h"
#include "Context.h"

namespace Backend {

	DataBuffer::DataBuffer() {
		glGenVertexArrays(1, &mArrayBufferHandle);

		mIndicesSlotHandle = 0;
		mDynamicIndices = false;

		mAttributeCount = 0;
	}

	DataBuffer::~DataBuffer() {
		glDeleteVertexArrays(1, &mArrayBufferHandle);

		for (auto key : mSlots) {
			delete key.second;
		}
	}

	void DataBuffer::ReserveIndices(unsigned int size) {
		mDynamicIndices = true;

		if (!mIndicesSlotHandle) glGenBuffers(1, &mIndicesSlotHandle);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndicesSlotHandle);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, 0, GL_DYNAMIC_DRAW);
	}

	void DataBuffer::UploadIndices(const void* indicesPtr, unsigned int dataSize, unsigned int dataOffset) {
		if (dataSize == 0) return;

		Bind();

		if (!mIndicesSlotHandle) glGenBuffers(1, &mIndicesSlotHandle);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndicesSlotHandle);

		if (!mDynamicIndices) {
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, dataSize, indicesPtr, GL_STATIC_DRAW);
		}
		else {
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, dataOffset, dataSize, indicesPtr);
		}

	}

	BufferSlot* DataBuffer::AddBufferSlot(const std::string& name, bool dynamicSlot) {
		if (name.empty()) return nullptr;

		Bind();
		
		BufferSlot* bufPtr = new BufferSlot(this, dynamicSlot);
		mSlots.insert({ name, bufPtr });

		return bufPtr;
	}

	BufferSlot* DataBuffer::GetBufferSlot(const std::string& name) {
		if (mSlots.find(name) == mSlots.end()) return nullptr;

		return mSlots[name];
	}

	BufferSlot::~BufferSlot() {
		glDeleteBuffers(1, &mBufferHandle);
	}

	BufferSlot* BufferSlot::UploadData(const void* dataPtr, unsigned int dataSize, int dataOffset) {
		glBindBuffer(GL_ARRAY_BUFFER, mBufferHandle);

		if (mIsDynamicSlot) {
			glBufferSubData(GL_ARRAY_BUFFER, dataOffset, dataSize, dataPtr);
		}
		else {
			glBufferData(GL_ARRAY_BUFFER, dataSize, dataPtr, GL_STATIC_DRAW);
		}

		return this;
	}

	BufferSlot* BufferSlot::AddDescriptor(int componentsCount, BufferDataType dataType, int blockSize, const void* startingOffset, int instanceDivisor) {
		if (mParentObject->mAttributeCount > 15) return this; // failsafe

		BufferSlotDescriptor descriptor;
		descriptor.mID = mParentObject->mAttributeCount++;
		descriptor.mComponentsCount = componentsCount;
		descriptor.mBlockSize = blockSize;
		descriptor.mOffset = startingOffset;
		descriptor.mInstanceDivisor = instanceDivisor;
		descriptor.mDataType = dataType;

		GLenum dataTypeNative;
		if (dataType == BufferDataType::DATA_INT) dataTypeNative = GL_INT;
		else dataTypeNative = GL_FLOAT;

		glEnableVertexArrayAttrib(mParentObject->mArrayBufferHandle, descriptor.mID);

		glBindBuffer(GL_ARRAY_BUFFER, mBufferHandle);

		// For integer values, a different attrib setting method is used
		if (dataType == BufferDataType::DATA_INT) {
			glVertexAttribIPointer(descriptor.mID, descriptor.mComponentsCount, dataTypeNative, descriptor.mBlockSize, descriptor.mOffset);
		}
		// For float values, we use the basic method
		else {
			glVertexAttribPointer(descriptor.mID, descriptor.mComponentsCount, dataTypeNative, GL_FALSE, descriptor.mBlockSize, descriptor.mOffset);
		}

		if (descriptor.mInstanceDivisor) glVertexAttribDivisor(descriptor.mID, descriptor.mInstanceDivisor);

		mDescriptors.push_back(descriptor);

		return this;
	}

	BufferSlot* BufferSlot::ReserveSpace(unsigned int size) {
		if (!size || !mIsDynamicSlot) return this;

		glBindBuffer(GL_ARRAY_BUFFER, mBufferHandle);
		glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_DRAW);

		return this;
	}

	BufferSlot::BufferSlot(DataBuffer* parent, bool dynamicSlot) {
		mParentObject = parent;

		mIsDynamicSlot = dynamicSlot;

		glGenBuffers(1, &mBufferHandle);
	}

}
