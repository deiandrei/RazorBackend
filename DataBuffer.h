#ifndef BUFFER_OBJECT_R_H
#define BUFFER_OBJECT_R_H

#include "include.h"

namespace Backend {
	class Context;
	class DataBuffer;
	class BufferSlot;
	class BufferSlotDescriptor;

	enum BufferDataType { DATA_INT, DATA_FLOAT };

	class BufferSlotDescriptor {
		public:
			int ID() { return mID; }
			int ComponentsCount() { return mComponentsCount; }
			int BlockSize() { return mBlockSize; }
			const void* Offset() { return mOffset; }
			int InstanceDivisor() { return mInstanceDivisor; }
			BufferDataType DataType() { return mDataType; }

		protected:
			BufferSlotDescriptor() { }

			int mID, mComponentsCount, mBlockSize, mInstanceDivisor;
			BufferDataType mDataType;
			const void* mOffset;

			friend class BufferSlot;

	};

	class BufferSlot {
		public:
			~BufferSlot();

			BufferSlot* UploadData(const void* dataPtr, unsigned int dataSize, int dataOffset = 0);
			BufferSlot* AddDescriptor(int componentsCount, BufferDataType dataType = BufferDataType::DATA_FLOAT, int blockSize = 0, const void* startingOffset = (const void*)0, int instanceDivisor = 0);
			
			BufferSlot* ReserveSpace(unsigned int size);

			GLuint GetNativeHandle() { return mBufferHandle; }

			unsigned int GetDescriptorsCount() { return (unsigned int)mDescriptors.size(); }
			BufferSlotDescriptor& GetDescriptor(unsigned int id) { return mDescriptors[id]; }

			// Utility functions
			template<typename T>
			BufferSlot* UploadData(const std::vector<T> arr, int dataOffset = 0) { if (arr.empty()) return this; return UploadData(&arr[0], sizeof(T) * arr.size(), dataOffset); }

		protected:
			BufferSlot(DataBuffer* parent, bool dynamicSlot = false);

			GLuint mBufferHandle;
			DataBuffer* mParentObject;

			bool mIsDynamicSlot;

			std::vector<BufferSlotDescriptor> mDescriptors;

		protected:
			friend class DataBuffer;

	};

	class DataBuffer {
		public:
			DataBuffer();
			~DataBuffer();

			void ReserveIndices(unsigned int size);
			void UploadIndices(const void* indicesPtr, unsigned int dataSize, unsigned int dataOffset = 0);
			BufferSlot* AddBufferSlot(const std::string& name, bool dynamicSlot = false);
			BufferSlot* GetBufferSlot(const std::string& name);

			// Utility functions
			void UploadIndices(const std::vector<unsigned int>& indices) { if(indices.size()) UploadIndices(&indices[0], (unsigned int)(sizeof(indices[0]) * indices.size())); }
			
		protected:
			void Bind() { glBindVertexArray(mArrayBufferHandle); }

		protected:
			GLuint mArrayBufferHandle;
			
			GLuint mIndicesSlotHandle;
			std::map<std::string, BufferSlot*> mSlots;

			int mAttributeCount;
			bool mDynamicIndices;

			friend class BufferSlot;

		protected:
			Context* mContext;

			friend class Context;

	};

}

#endif
