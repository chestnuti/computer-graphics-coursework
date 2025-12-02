#pragma once
#include "Mesh.h"
#include "Camera.h"
#include <map>


/*class Buffer {
	Vec3* colorBuffer;
	float* depthBuffer;
	int width;
	int height;
public:
	Buffer(int pWidth, int pHeight) : width(pWidth), height(pHeight) {
		colorBuffer = new Vec3[width * height];
		depthBuffer = new float[width * height];
	}
	Vec3* getColorBuffer() { return colorBuffer; }
	float* getDepthBuffer() { return depthBuffer; }
	void clearColorBuffer(const Vec3& clearColor) {
		for (int i = 0; i < width * height; i++) {
			colorBuffer[i] = clearColor;
		}
	}
	void clearDepthBuffer() {
		for (int i = 0; i < width * height; i++) {
			depthBuffer[i] = 255;
		}
	}
	void clear(){
		clearColorBuffer(Vec3(0.0f, 0.0f, 0.0f));
		clearDepthBuffer();
	}
	~Buffer() {
		delete[] colorBuffer;
		delete[] depthBuffer;
	}

	void drawIntoBuffer(const Triangle& triangle, Camera& camera) {
		Triangle tri_transformed;
		tri_transformed.v0 = camera.getViewProjectionVector(triangle.v0);
		tri_transformed.v1 = camera.getViewProjectionVector(triangle.v1);
		tri_transformed.v2 = camera.getViewProjectionVector(triangle.v2);
		tri_transformed.color0 = triangle.color0;
		tri_transformed.color1 = triangle.color1;
		tri_transformed.color2 = triangle.color2;
		//Convert to screen coordinates
		tri_transformed.v0.v[0] = (tri_transformed.v0.v[0] + 1.0f) * 0.5f * ScreenWidth;
		tri_transformed.v0.v[1] = (1.0f - (tri_transformed.v0.v[1] + 1.0f) * 0.5f) * ScreenHeight;
		tri_transformed.v0.v[2] = 0;
		tri_transformed.v1.v[0] = (tri_transformed.v1.v[0] + 1.0f) * 0.5f * ScreenWidth;
		tri_transformed.v1.v[1] = (1.0f - (tri_transformed.v1.v[1] + 1.0f) * 0.5f) * ScreenHeight;
		tri_transformed.v1.v[2] = 0;
		tri_transformed.v2.v[0] = (tri_transformed.v2.v[0] + 1.0f) * 0.5f * ScreenWidth;
		tri_transformed.v2.v[1] = (1.0f - (tri_transformed.v2.v[1] + 1.0f) * 0.5f) * ScreenHeight;
		tri_transformed.v2.v[2] = 0;
		//Calculate distance
		float distance0 = remap((triangle.v0 - camera.position).getLength(), camera.clipNear, camera.clipFar, 0.0f, 255.0f)
			* (camera.outcoming.normalize().Dot((triangle.v0 - camera.position).normalize()) >= 0 ? 1 : -1);
		float distance1 = remap((triangle.v1 - camera.position).getLength(), camera.clipNear, camera.clipFar, 0.0f, 255.0f)
			* (camera.outcoming.normalize().Dot((triangle.v1 - camera.position).normalize()) >= 0 ? 1 : -1);
		float distance2 = remap((triangle.v2 - camera.position).getLength(), camera.clipNear, camera.clipFar, 0.0f, 255.0f)
			* (camera.outcoming.normalize().Dot((triangle.v2 - camera.position).normalize()) >= 0 ? 1 : -1);
		//Rasterization
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				Vec4 point(static_cast<float>(x), static_cast<float>(y), 0.0f, 1.0f);
				Vec4 checkInfo = tri_transformed.checkPointInside(point);
				if (checkInfo.v[3] > 0) { //inside triangle (alpha, beta, gamma, inside)
					//correct depth value
					float depth = perspectiveCorrectInterpolateAttribute(distance0, distance1, distance2,
						tri_transformed.v0.v[3], tri_transformed.v1.v[3], tri_transformed.v2.v[3],
						checkInfo.v[0], checkInfo.v[1], checkInfo.v[2]);
					//Interpolate color
					int index = y * width + x;
					if (depth < depthBuffer[index] && depth >= camera.clipNear) {
						depthBuffer[index] = depth;
						Vec3 colorDepth = perspectiveCorrectInterpolateAttribute(triangle.color0, triangle.color1, triangle.color2,
							tri_transformed.v0.v[3], tri_transformed.v1.v[3], tri_transformed.v2.v[3],
							checkInfo.v[0], checkInfo.v[1], checkInfo.v[2]);
						colorBuffer[index] = Vec3(colorDepth.v[0], colorDepth.v[1], colorDepth.v[2]);
					}
				}
			}
		}
	}
};*/


struct ConstantBufferVariable
{
	unsigned int offset;
	unsigned int size;
};

class ConstantBuffer {
public:
	std::string name;
	std::map<std::string, ConstantBufferVariable> constantBufferData;

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
};
