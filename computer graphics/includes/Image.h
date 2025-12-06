#pragma once
#include <string>
#include <wincodec.h>
#include "Core.h"
#include <wrl/client.h>
#include <unordered_map>


class Image {
public:
	unsigned int width;
	unsigned int height;
	unsigned int channels;
	unsigned char* data;
	size_t pixelSize;

	ID3D12Resource* texture;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;

	void uploadImage(Core* core) {
		// Create a texture resource in GPU memory
		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		texture = nullptr;
		D3D12_HEAP_PROPERTIES defaultHeap = {};
		defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;
		core->device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texture));

		// get footprint size
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
		UINT numRows;
		UINT64 rowSizeInBytes;
		UINT64 totalBytes;
		core->device->GetCopyableFootprints(&texDesc,
			0,                  // First subresource
			1,                  // Num subresources
			0,                  // Base offset
			&footprint, &numRows, &rowSizeInBytes, &totalBytes);

		// copy data into upload buffer with proper row pitch
		std::vector<BYTE> uploadData(totalBytes);
		BYTE* dst = uploadData.data();
		BYTE* src = data;
		for (UINT row = 0; row < numRows; row++)
		{
			memcpy(dst, src, rowSizeInBytes);
			dst += footprint.Footprint.RowPitch;
			src += rowSizeInBytes;
		}
		core->uploadResource(texture, uploadData.data(), (unsigned int)uploadData.size(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &footprint);
	}

	void apply(Core* core) {
		core->getCommandList()->SetGraphicsRootDescriptorTable(2, gpuHandle);
	}

	bool load(const std::string& filename) {
		Microsoft::WRL::ComPtr<IWICImagingFactory> factory;
		HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
		if (FAILED(hr))
		{
			return false;
		}

		Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
		IWICStream* stream = NULL;
		factory->CreateStream(&stream);

		std::wstring wFilename = std::wstring(filename.begin(), filename.end());
		stream->InitializeFromFilename(wFilename.c_str(), GENERIC_READ);
		factory->CreateDecoderFromStream(stream, 0, WICDecodeMetadataCacheOnDemand, &decoder);

		Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
		decoder->GetFrame(0, &frame);

		frame->GetSize(&width, &height);
		WICPixelFormatGUID pixelFormat = { 0 };
		frame->GetPixelFormat(&pixelFormat);

		channels = 0;
		int isRGB = 0;
		pixelSize = width * height * 4;

		// Determine the number of channels based on the pixel format
		if (pixelFormat == GUID_WICPixelFormat24bppBGR)
		{
			channels = 3;
		}
		if (pixelFormat == GUID_WICPixelFormat32bppBGRA)
		{
			channels = 4;
		}
		if (pixelFormat == GUID_WICPixelFormat24bppRGB)
		{
			channels = 3;
			isRGB = 1;
		}
		if (pixelFormat == GUID_WICPixelFormat32bppRGBA)
		{
			channels = 4;
			isRGB = 1;
		}
		if (channels == 0)
		{
			return false;
		}

		data = new unsigned char[width * height * channels];
		unsigned int stride = (width * channels + 3) & ~3; // Align stride to 4 bytes

		if (stride == (width * channels))
		{
			// Copy pixels directly if stride matches
			frame->CopyPixels(0, stride, width * height * channels, data);
		}
		else
		{
			// Handle images with padded stride
			unsigned char* strideData = new unsigned char[stride * height];
			frame->CopyPixels(0, stride, width * height * channels, strideData);
			for (unsigned int i = 0; i < height; i++)
			{
				memcpy(&data[i * width * channels], &strideData[i * stride], width * channels * sizeof(unsigned char));
			}
			delete[] strideData;
		}

		if (isRGB == 0)
		{
			// Swap red and blue channels for BGR formats
			for (unsigned int i = 0; i < width * height; i++)
			{
				unsigned char p = data[i * channels];
				data[i * channels] = data[(i * channels) + 2];
				data[(i * channels) + 2] = p;
			}
		}
		return true;
	}

	// Returns a pointer to the pixel data at (x, y)
		// Note, the bounds are handled via clamping
	unsigned char* at(const unsigned int x, const unsigned int y) const
	{
		return &data[((min(y, height - 1) * width) + min(x, width - 1)) * channels];
	}

	// Returns the alpha value of the pixel at (x, y)
	// Note, the bounds are handled via clamping
	unsigned char alphaAt(const unsigned int x, const unsigned int y) const
	{
		if (channels == 4)
		{
			return data[((min(y, height - 1) * width) + min(x, width - 1)) * channels + 3];
		}
		return 255;
	}

	// Returns a the colour specified by index at (x, y)
	// Note, the image bounds are handled via clamping, but the index is not checked
	unsigned char at(const unsigned int x, const unsigned int y, const unsigned int index) const
	{
		return data[(((min(y, height - 1) * width) + min(x, width - 1)) * channels) + index];
	}

	// Returns a pointer to the pixel data at (x, y)
	// Note, no checks performed on x and y coordinates
	unsigned char* atUnchecked(const unsigned int x, const unsigned int y) const
	{
		return &data[((y * width) + x) * channels];
	}

	// Returns the alpha value of the pixel at (x, y)
	// Note, no checks performed on x and y coordinates
	unsigned char alphaAtUnchecked(const unsigned int x, const unsigned int y) const
	{
		if (channels == 4)
		{
			return data[(((y * width) + x) * channels) + 3];
		}
		return 255;
	}

	// Checks if the image has an alpha channel
	bool hasAlpha() const
	{
		return channels == 4;
	}
};



class Sampler {
public:
	ID3D12DescriptorHeap* samplerHeap;

	void init(Core* core){
		// create sampler descriptor heap
		D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
		samplerHeapDesc.NumDescriptors = 1;
		samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		core->device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&samplerHeap));

		D3D12_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

		// write sampler to descriptor heap (s0)
		core->device->CreateSampler(&samplerDesc, samplerHeap->GetCPUDescriptorHandleForHeapStart());
	}

	ID3D12DescriptorHeap* getHeap() {
		return samplerHeap;
	}
};



class ImageLoader {
public:
	std::unordered_map<std::string, Image> images;
	Core* core;
	// Sampler for images
	Sampler sampler;

	// Shader Resource View heap
	ID3D12DescriptorHeap* srvHeap;
	UINT srvDescriptorSize;
	UINT srvCount; // Number of SRVs allocated

	ImageLoader(Core* _core) : core(_core) {
		// Create shader resource view heap
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 100;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		core->device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&srvHeap));
		srvDescriptorSize = core->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		srvCount = 0;
		sampler.init(core);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE allocateSRV() {
		D3D12_CPU_DESCRIPTOR_HANDLE handle = srvHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += srvCount * srvDescriptorSize;
		srvCount++;
		return handle;
	}

	bool loadImage(std::string name, const std::string& filename) {
		Image image;
		if (!image.load(filename)) {
			return false;
		}
		images.insert({ name, image });
		return true;
	}

	void uploadImages(std::string name) {
		Image* image = &images[name];
		image->uploadImage(core);
		// allocate descriptor handles
		image->cpuHandle = allocateSRV();
		image->gpuHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();
		image->gpuHandle.ptr += (srvCount - 1) * srvDescriptorSize;
		// craete shader resource view
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		core->device->CreateShaderResourceView(image->texture, &srvDesc, image->cpuHandle);
	}

	void applyImage(std::string name) {
		// set descriptor heaps
		ID3D12DescriptorHeap* heaps[] = { srvHeap, sampler.getHeap()};
		core->getCommandList()->SetDescriptorHeaps(2, heaps);
		// bind sampler (s0)
		core->getCommandList()->SetGraphicsRootDescriptorTable(3, sampler.getHeap()->GetGPUDescriptorHandleForHeapStart());

		// apply image
		Image* image = &images[name];
		image->apply(core);
	}
};