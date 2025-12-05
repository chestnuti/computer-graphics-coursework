#pragma once
#include "Core.h"
#include <unordered_map>
#include <fstream>
#include <sstream>
#include "Buffer.h"


enum ShaderType {
	VERTEX_SHADER,
	PIXEL_SHADER
};


class Shader {
public:
	// Shaders
	ID3DBlob* vertexShader;
	ID3DBlob* pixelShader;

	std::vector<ConstantBuffer> vsConstantBuffers;
	std::vector<ConstantBuffer> psConstantBuffers;

	Shader() : vertexShader(nullptr), pixelShader(nullptr) {}

	void init(const std::string& vertexShaderCode, const std::string& pixelShaderCode) {
		// Compile shaders
		ID3DBlob* status;
		std::string vertexShaderStr = readShaderFile(vertexShaderCode);
		HRESULT hr1 = D3DCompile(vertexShaderStr.c_str(), vertexShaderStr.size(), NULL,
			NULL, NULL, "VS", "vs_5_0", 0, 0, &vertexShader, &status);
		//check hr
		if (FAILED(hr1))
		{
			throw std::runtime_error("Vertex shader compilation failed");
			(char*)status->GetBufferPointer();
		}
		std::string pixelShaderStr = readShaderFile(pixelShaderCode);
		HRESULT hr2 = D3DCompile(pixelShaderStr.c_str(), pixelShaderStr.size(), NULL,
			NULL, NULL, "PS", "ps_5_0", 0, 0, &pixelShader, &status);
		//check hr
		if (FAILED(hr2))
		{
			throw std::runtime_error("Pixel shader compilation failed");
			(char*)status->GetBufferPointer();
		}
	}

	std::string readShaderFile(const std::string& filename)
	{
		std::ifstream file(filename);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	void reflect(Core* core, ID3DBlob* shader, std::vector<ConstantBuffer> &toBuffer) {
		// Reflect shader to get constant buffer info
		ID3D12ShaderReflection* reflection;
		D3DReflect(shader->GetBufferPointer(), shader->GetBufferSize(), IID_PPV_ARGS(&reflection));
		D3D12_SHADER_DESC desc;
		reflection->GetDesc(&desc);

		// Iterate over constant buffers
		for (int i = 0; i < desc.ConstantBuffers; i++)
		{
			ID3D12ShaderReflectionConstantBuffer* constantBuffer = reflection->GetConstantBufferByIndex(i);
			D3D12_SHADER_BUFFER_DESC cbDesc;
			constantBuffer->GetDesc(&cbDesc);
			ConstantBuffer buffer;
			buffer.init(core, cbDesc.Size, 1);
			buffer.name = cbDesc.Name;
			unsigned int totalSize = 0;
			// Iterate over variables in constant buffer
			for (int j = 0; j < cbDesc.Variables; j++)
			{
				ID3D12ShaderReflectionVariable* var = constantBuffer->GetVariableByIndex(j);
				D3D12_SHADER_VARIABLE_DESC vDesc;
				var->GetDesc(&vDesc);
				ConstantBufferVariable bufferVariable;
				bufferVariable.offset = vDesc.StartOffset;
				bufferVariable.size = vDesc.Size;
				buffer.constantBufferData.insert({ vDesc.Name, bufferVariable });
				totalSize += bufferVariable.size;
			}
			toBuffer.push_back(buffer);
		}
		reflection->Release();
	}

	void reflect(Core* core) {
		reflect(core, vertexShader, vsConstantBuffers);
		reflect(core, pixelShader, psConstantBuffers);
	}

	void apply(Core* core) {
		// bind VS constant buffers
		for (int i = 0; i < vsConstantBuffers.size(); i++) 
		{
			core->getCommandList()->SetGraphicsRootConstantBufferView(i, vsConstantBuffers[i].getGPUAddress());
			vsConstantBuffers[i].next();
		}

		// bind PS constant buffers, starting after VS CBVs
		int psStartIndex = vsConstantBuffers.size();
		for (int i = 0; i < psConstantBuffers.size(); i++) 
		{
			core->getCommandList()->SetGraphicsRootConstantBufferView(psStartIndex + i, psConstantBuffers[i].getGPUAddress());
			psConstantBuffers[i].next();
		}
	}

	void updateConstantBuffer(const std::string& cbName, std::string varName, void* data, enum ShaderType type) {
		std::vector<ConstantBuffer>& buffers = (type == VERTEX_SHADER) ? vsConstantBuffers : psConstantBuffers;
		for (auto& cb : buffers) {
			if (cb.name == cbName) {
				cb.update(varName, data);
				return;
			}
		}
	}

};


// Pipeline State Object Manager
class PSOManager {
public:
	std::unordered_map<std::string, ID3D12PipelineState*> psos;
	std::unordered_map<std::string, ID3D12RootSignature*> rootSignatures;

	void createPSO(Core* core, std::string name, ID3DBlob* vs, ID3DBlob* ps, D3D12_INPUT_LAYOUT_DESC layout)
	{
		if (psos.find(name) != psos.end())
		{
			return;
		}
		
		// Create a unique root signature for this PSO
		ID3D12RootSignature* rootSig = createRootSignature(core, psos.size());

		// Create graphics pipeline state object (PSO)
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		desc.InputLayout = layout;
		desc.pRootSignature = rootSig;
		desc.VS = { vs->GetBufferPointer(), vs->GetBufferSize() };
		desc.PS = { ps->GetBufferPointer(), ps->GetBufferSize() };
		// Rasterizer State
		D3D12_RASTERIZER_DESC rasterDesc = {};
		rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
		rasterDesc.CullMode = D3D12_CULL_MODE_NONE;
		rasterDesc.FrontCounterClockwise = FALSE;
		rasterDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		rasterDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		rasterDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		rasterDesc.DepthClipEnable = TRUE;
		rasterDesc.MultisampleEnable = FALSE;
		rasterDesc.AntialiasedLineEnable = FALSE;
		rasterDesc.ForcedSampleCount = 0;
		rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		desc.RasterizerState = rasterDesc;
		// Depth stencil state
		D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		depthStencilDesc.StencilEnable = FALSE;
		desc.DepthStencilState = depthStencilDesc;
		// Blend state
		D3D12_BLEND_DESC blendDesc = {};
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = FALSE;
		const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlend = {
		FALSE, FALSE,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL
		};
		for (int i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		{
			blendDesc.RenderTarget[i] = defaultRenderTargetBlend;
		}
		desc.BlendState = blendDesc;
		// Render target state and topology
		desc.SampleMask = UINT_MAX;
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.NumRenderTargets = 1;
		desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		desc.SampleDesc.Count = 1;
		// create PSO
		ID3D12PipelineState* pso;
		HRESULT hr = core->device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pso));
		if (FAILED(hr)) {
			// handle error
			throw std::runtime_error("Failed to create pipeline state object, pso name: " + name);
		}
		// insert into maps
		psos.insert({ name, pso });
		rootSignatures.insert({ name, rootSig });
	}

	void createPSO(Core* core, std::string name, Shader* shader, D3D12_INPUT_LAYOUT_DESC layout) {
		createPSO(core, name, shader->vertexShader, shader->pixelShader, layout);
	}

	void bind(Core* core, std::string name) {
		// Set both root signature and PSO
		core->getCommandList()->SetGraphicsRootSignature(rootSignatures[name]);
		core->getCommandList()->SetPipelineState(psos[name]);
	}

	ID3D12RootSignature* createRootSignature(Core* core, int registerSlot) {
		// Upload Root Signature
		std::vector<D3D12_ROOT_PARAMETER> parameters;
		// Register space for VS CBV
		D3D12_ROOT_PARAMETER rootParameterCBVS;
		rootParameterCBVS.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParameterCBVS.Descriptor.ShaderRegister = 0; // Register(b0)
		rootParameterCBVS.Descriptor.RegisterSpace = 0;
		rootParameterCBVS.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		parameters.push_back(rootParameterCBVS);
		// Register space for PS CBV
		D3D12_ROOT_PARAMETER rootParameterCBPS;
		rootParameterCBPS.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParameterCBPS.Descriptor.ShaderRegister = 0; // Register(b0)
		rootParameterCBPS.Descriptor.RegisterSpace = 0;
		rootParameterCBPS.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		parameters.push_back(rootParameterCBPS);
		// Register space for texture SRV
		D3D12_DESCRIPTOR_RANGE srvRange = {};
		srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		srvRange.NumDescriptors = 1;        // only t0
		srvRange.BaseShaderRegister = 0;    // register(t0)
		srvRange.RegisterSpace = 0;
		srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		D3D12_ROOT_PARAMETER rootParameterSRV = {};
		rootParameterSRV.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameterSRV.DescriptorTable.NumDescriptorRanges = 1;
		rootParameterSRV.DescriptorTable.pDescriptorRanges = &srvRange;
		rootParameterSRV.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // typical for textures
		parameters.push_back(rootParameterSRV);
		// Register space for sampler
		D3D12_DESCRIPTOR_RANGE samplerRange = {};
		samplerRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
		samplerRange.NumDescriptors = 1;       // only s0
		samplerRange.BaseShaderRegister = 0;   // register(s0)
		samplerRange.RegisterSpace = 0;
		samplerRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		D3D12_ROOT_PARAMETER rootParameterSampler = {};
		rootParameterSampler.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameterSampler.DescriptorTable.NumDescriptorRanges = 1;
		rootParameterSampler.DescriptorTable.pDescriptorRanges = &samplerRange;
		rootParameterSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // sampler used in PS
		parameters.push_back(rootParameterSampler);
		// Update Root Signature Description
		D3D12_ROOT_SIGNATURE_DESC desc = {};
		desc.NumParameters = parameters.size();
		desc.pParameters = &parameters[0];
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		// allow drawing
		ID3DBlob* serialized;
		ID3DBlob* error;
		D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &error);
		
		ID3D12RootSignature* rootSig;
		core->device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(&rootSig));
		core->device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(&core->rootSignature));
		serialized->Release();
		
		return rootSig;
	}
};


class ShaderManager {
public:
	std::unordered_map<std::string, Shader*> shaders;

	Shader* createShader(Core* core, std::string name, const std::string& vertexShaderFile, const std::string& pixelShaderFile)
	{
		if (shaders.find(name) != shaders.end())
		{
			return shaders[name];
		}
		Shader* shader = new Shader();
		shader->init(vertexShaderFile, pixelShaderFile);
		shader->reflect(core);
		shaders.insert({ name, shader });
		return shader;
	}

	void applyShader(Core* core, std::string name) {
		shaders[name]->apply(core);
	}

	void updateConstantBuffers(std::string shaderName, const std::string& cbName, void* data) {
		Shader* shader = shaders[shaderName];
		// Update VS constant buffers
		for (auto& cb : shader->vsConstantBuffers) {
			if (cb.name == cbName) {
				cb.update(cbName, data);
			}
		}
		// Update PS constant buffers
		for (auto& cb : shader->psConstantBuffers) {
			if (cb.name == cbName) {
				cb.update(cbName, data);
				return;
			}
		}
	}

	void updateConstantBuffer(const std::string& shaderName, const std::string& cbName, std::string varName, void* data, enum ShaderType type) {
		Shader* shader = shaders[shaderName];
		shader->updateConstantBuffer(cbName, varName, data, type);
	}
};