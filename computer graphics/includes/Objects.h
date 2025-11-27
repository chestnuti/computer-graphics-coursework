#include "Core.h"
#include <unordered_map>
#include <fstream>
#include <sstream>
#include "Buffer.h"


class Objects {
public:
	// Shaders
	ID3DBlob* vertexShader;
	ID3DBlob* pixelShader;

	Objects() : vertexShader(nullptr), pixelShader(nullptr) {}

	void init(const std::string& vertexShaderCode, const std::string& pixelShaderCode) {
		// Compile shaders
		ID3DBlob* status;
		std::string vertexShaderStr = readShaderFile(vertexShaderCode);
		HRESULT hr1 = D3DCompile(vertexShaderStr.c_str(), strlen(vertexShaderStr.c_str()), NULL,
			NULL, NULL, "VS", "vs_5_0", 0, 0, &vertexShader, &status);
		//check hr
		if (FAILED(hr1))
		{
			(char*)status->GetBufferPointer();
		}
		std::string pixelShaderStr = readShaderFile(pixelShaderCode);
		HRESULT hr2 = D3DCompile(pixelShaderStr.c_str(), strlen(pixelShaderStr.c_str()), NULL,
			NULL, NULL, "PS", "ps_5_0", 0, 0, &pixelShader, &status);
		//check hr
		if (FAILED(hr2))
		{
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
};


// Pipeline State Object Manager
class PSOManager {
public:
	std::unordered_map<std::string, ID3D12PipelineState*> psos;

	void createPSO(Core* core, std::string name, ID3DBlob* vs, ID3DBlob* ps, D3D12_INPUT_LAYOUT_DESC layout)
	{
		if (psos.find(name) != psos.end())
		{
			return;
		}
		// Create graphics pipeline state object (PSO)
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		desc.InputLayout = layout;
		desc.pRootSignature = core->rootSignature;
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
		// insert into map
		psos.insert({ name, pso });
	}

	void bind(Core* core, std::string name) {
		core->getCommandList()->SetPipelineState(psos[name]);
	}

};