#pragma once
#include "Mesh.h"
#include "Camera.h"
#include <map>



struct ConstantBufferVariable
{
	unsigned int offset;
	unsigned int size;
};

class ConstantBuffer {
public:
	std::string name;
	std::map<std::string, ConstantBufferVariable> constantBufferData;
	std::map<std::string, void*> dataPointers;

	ID3D12Resource* constantBuffer;
	unsigned char* buffer;
	unsigned int cbSizeInBytes;
	unsigned int maxDrawCalls;
	unsigned int offsetIndex;
	unsigned int numInstances;

	void init(Core* core, unsigned int sizeInBytes, unsigned int _maxDrawCalls = 1024)
	{
		cbSizeInBytes = (sizeInBytes + 255) & ~255;
		maxDrawCalls = _maxDrawCalls;
		unsigned int cbSizeInBytesAligned = cbSizeInBytes * maxDrawCalls;
		numInstances = _maxDrawCalls;
		offsetIndex = 0;
		HRESULT hr;
		D3D12_HEAP_PROPERTIES heapprops = {};
		heapprops.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapprops.CreationNodeMask = 1;
		heapprops.VisibleNodeMask = 1;
		D3D12_RESOURCE_DESC cbDesc = {};
		cbDesc.Width = cbSizeInBytesAligned;
		cbDesc.Height = 1;
		cbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		cbDesc.DepthOrArraySize = 1;
		cbDesc.MipLevels = 1;
		cbDesc.SampleDesc.Count = 1;
		cbDesc.SampleDesc.Quality = 0;
		cbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		core->device->CreateCommittedResource(&heapprops, D3D12_HEAP_FLAG_NONE, &cbDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
			IID_PPV_ARGS(&constantBuffer));
		constantBuffer->Map(0, NULL, (void**)&buffer);	//0 measns no subresource, NULL means entire resource, save pointer to data
	}

	// update via memcpy
	void update(std::string name, void* data)
	{
		ConstantBufferVariable cbVariable = constantBufferData[name];
		unsigned int offset = offsetIndex * cbSizeInBytes;
		memcpy(&buffer[offset + cbVariable.offset], data, cbVariable.size);
	}

	void updateAll()
	{
		for (const auto& pair : dataPointers)
		{
			std::string name = pair.first;
			void* pointer = pair.second;
			update(name, pointer);
		}
	}

	// get GPU virtual address
	D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress() const
	{
		return (constantBuffer->GetGPUVirtualAddress() + (offsetIndex * cbSizeInBytes));
	}

	// move to next offset
	void next()
	{
		offsetIndex++;
		if (offsetIndex >= maxDrawCalls)
		{
			offsetIndex = 0;
		}
	}

	void setValuePointer(std::string name, void* pointer)
	{
		dataPointers[name] = pointer;
	}
};
