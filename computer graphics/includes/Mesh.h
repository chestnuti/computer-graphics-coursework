#pragma once

#include "Vector.h"
#include "Operators.h"
#include "Core.h"
#include "GEMLoader.h"
#include "Shader.h"
#include "Animation.h"


class Triangle {
public:
	Vec4 v0;
	Vec4 v1;
	Vec4 v2;
	Vec3 color0;
	Vec3 color1;
	Vec3 color2;
	Vec4 normal;
	Vec4 tangent;

	Triangle() : v0(), v1(), v2() {
		Vec4 v0v1 = v1 - v0;
		Vec4 v0v2 = v2 - v0;
		normal = v0v1.cross(v0v2).normalize();
		tangent = normal.cross(v0v1).normalize();
		color0 = Vec3(255, 255, 255);
		color1 = Vec3(255, 255, 255);
		color2 = Vec3(255, 255, 255);
	}
	Triangle(const Vec4& pV0, const Vec4 pV1, const Vec4& pV2) : v0(pV0), v1(pV1), v2(pV2) {
		Vec4 v0v1 = v1 - v0;
		Vec4 v0v2 = v2 - v0;
		normal = v0v1.cross(v0v2).normalize();
		tangent = normal.cross(v0v1).normalize();
		color0 = Vec3(255, 255, 255);
		color1 = Vec3(255, 255, 255);
		color2 = Vec3(255, 255, 255);
	}
	Triangle(const Vec4& pV0, const Vec4 pV1, const Vec4& pV2, const Vec3& pColor0, const Vec3& pColor1, const Vec3& pColor2)
		: v0(pV0), v1(pV1), v2(pV2), color0(pColor0), color1(pColor1), color2(pColor2) {
		Vec4 v0v1 = v1 - v0;
		Vec4 v0v2 = v2 - v0;
		normal = v0v1.cross(v0v2).normalize();
		tangent = normal.cross(v0v1).normalize();
	}
	Triangle(const Vec4& pV0, const Vec4 pV1, const Vec4& pV2, const Vec3& pColor0, const Vec3& pColor1, const Vec3& pColor2, const Vec4& pNormal, const Vec4& pTangent)
		: v0(pV0), v1(pV1), v2(pV2), color0(pColor0), color1(pColor1), color2(pColor2), normal(pNormal), tangent(pTangent) {
	}

	Vec4 checkPointInside(const Vec4& point)
	{
		//Calculate area of the triangle
		Vec4 v0v1 = v1 - v0;
		Vec4 v0v2 = v2 - v0;
		//v0v1.v[2] = 0.0f;
		//v0v2.v[2] = 0.0f;
		Vec4 n = v0v1.cross(v0v2);
		float area = sqrtf(n.v[0] * n.v[0] + n.v[1] * n.v[1] + n.v[2] * n.v[2]) / 2.0f;
		//Calculate area of sub-triangles
		Vec4 v0p = point - v0;
		Vec4 v1p = point - v1;
		Vec4 v2p = point - v2;
		//v0p.v[2] = 0.0f;
		//v1p.v[2] = 0.0f;
		//v2p.v[2] = 0.0f;
		Vec4 n0 = v0p.cross(v1p);
		Vec4 n1 = v0p.cross(v2p);
		Vec4 n2 = v1p.cross(v2p);
		float area0 = sqrtf(n0.v[0] * n0.v[0] + n0.v[1] * n0.v[1] + n0.v[2] * n0.v[2]) / 2.0f;
		float area1 = sqrtf(n1.v[0] * n1.v[0] + n1.v[1] * n1.v[1] + n1.v[2] * n1.v[2]) / 2.0f;
		float area2 = sqrtf(n2.v[0] * n2.v[0] + n2.v[1] * n2.v[1] + n2.v[2] * n2.v[2]) / 2.0f;
		float areaSum = area0 + area1 + area2;
		//Barycentric coordinates for color interpolation
		float alpha = area1 / area;
		float beta = area2 / area;
		float gamma = area0 / area;

		if (fabs(area - areaSum) <= 0.1f)
			return Vec4(alpha, beta, gamma, 1.0f); // inside
		else
			return Vec4(alpha, beta, gamma, 0.0f); // outside
	}

};



class Colour
{
public:
	float r;
	float g;
	float b;
	Colour() : r(0), g(0), b(0) {}
	Colour(float red, float green, float blue) : r(red), g(green), b(blue) {}
	Colour operator*(const Colour& col) const
	{
		return Colour(r * col.r, g * col.g, b * col.b);
	}
	Colour operator*(const float val) const
	{
		return Colour(r * val, g * val, b * val);
	}
	Colour operator/(const float val) const
	{
		return Colour(r / val, g / val, b / val);
	}
};

struct PRIM_VERTEX
{
	Vec3 position;
	Colour colour;
};

struct STATIC_VERTEX
{
	Vec3 pos;
	Vec3 normal;
	Vec3 tangent;
	float uv[2];
};

struct ANIMATED_VERTEX
{
	Vec3 pos;
	Vec3 normal;
	Vec3 tangent;
	float uv[2];
	unsigned int bonesIDs[4];
	float boneWeights[4];
};

STATIC_VERTEX addVertex(Vec3 p, Vec3 n, float tu, float tv)
{
	STATIC_VERTEX v;
	v.pos = p;
	v.normal = n;
	v.tangent = Vec3(0, 0, 0); // For now
	v.uv[0] = tu;
	v.uv[1] = tv;
	return v;
}




class LayoutCache
{
public:
	static const D3D12_INPUT_LAYOUT_DESC& getAnimatedLayout() {
		static const D3D12_INPUT_ELEMENT_DESC inputLayoutAnimated[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BONEIDS", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BONEWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		static const D3D12_INPUT_LAYOUT_DESC desc = { inputLayoutAnimated, 6 };
		return desc;
	}

	static const D3D12_INPUT_LAYOUT_DESC& getStaticLayout() {
		static const D3D12_INPUT_ELEMENT_DESC inputLayoutStatic[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
		static const D3D12_INPUT_LAYOUT_DESC desc = { inputLayoutStatic, 4 };
		return desc;
	}
};

class Mesh {
public:
	// Vertex buffer
	ID3D12Resource* vertexBuffer;
	ID3D12Resource* indexBuffer;
	// Vertex buffer view
	D3D12_VERTEX_BUFFER_VIEW vbView;
	D3D12_INDEX_BUFFER_VIEW ibView;
	// Layout of the vertex buffer
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;

	unsigned int numMeshIndices;

	Mat4 worldMatrix;
	std::string psoNames;

	void init(Core* core, void* vertices, int vertexSizeInBytes, int numVertices, unsigned int* indices, int numIndices)
	{
		worldMatrix = Mat4()._Identity();
		// Create an upload heap to upload the vertex buffer data
		D3D12_HEAP_PROPERTIES heapprops = {};
		heapprops.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapprops.CreationNodeMask = 1;
		heapprops.VisibleNodeMask = 1;
		// Create the vertex buffer on heap
		D3D12_RESOURCE_DESC vbDesc = {};
		vbDesc.Width = numVertices * vertexSizeInBytes;
		vbDesc.Height = 1;
		vbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		vbDesc.DepthOrArraySize = 1;
		vbDesc.MipLevels = 1;
		vbDesc.SampleDesc.Count = 1;
		vbDesc.SampleDesc.Quality = 0;
		vbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		// Allocate upload heap
		core->device->CreateCommittedResource(&heapprops, D3D12_HEAP_FLAG_NONE, &vbDesc,
			D3D12_RESOURCE_STATE_COMMON, NULL, IID_PPV_ARGS(&vertexBuffer));
		// Copy vertex data to the vertex buffer
		core->uploadResource(vertexBuffer, vertices, numVertices * vertexSizeInBytes,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		// Create vertex buffer view
		vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vbView.StrideInBytes = vertexSizeInBytes;
		vbView.SizeInBytes = numVertices * vertexSizeInBytes;
		
		// Create index buffer
		D3D12_RESOURCE_DESC ibDesc;
		memset(&ibDesc, 0, sizeof(D3D12_RESOURCE_DESC));
		ibDesc.Width = numIndices * sizeof(unsigned int);
		ibDesc.Height = 1;
		ibDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		ibDesc.DepthOrArraySize = 1;
		ibDesc.MipLevels = 1;
		ibDesc.SampleDesc.Count = 1;
		ibDesc.SampleDesc.Quality = 0;
		ibDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		HRESULT hr;
		hr = core->device->CreateCommittedResource(&heapprops, D3D12_HEAP_FLAG_NONE, &ibDesc,
			D3D12_RESOURCE_STATE_COMMON, NULL, IID_PPV_ARGS(&indexBuffer));
		core->uploadResource(indexBuffer, indices, numIndices * sizeof(unsigned int),
			D3D12_RESOURCE_STATE_INDEX_BUFFER);
		// Create index buffer view
		ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
		ibView.Format = DXGI_FORMAT_R32_UINT;
		ibView.SizeInBytes = numIndices * sizeof(unsigned int);
		numMeshIndices = numIndices;

	}

	void init(Core* core, std::vector<STATIC_VERTEX> vertices, std::vector<unsigned int> indices)
	{
		init(core, &vertices[0], sizeof(STATIC_VERTEX), vertices.size(), &indices[0], indices.size());
		inputLayoutDesc = LayoutCache::getStaticLayout();
	}

	void init(Core* core, std::vector<ANIMATED_VERTEX> vertices, std::vector<unsigned int> indices)
	{
		init(core, &vertices[0], sizeof(ANIMATED_VERTEX), vertices.size(), &indices[0], indices.size());
		inputLayoutDesc = LayoutCache::getAnimatedLayout();
	}

	void draw(Core* core)
	{
		core->getCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		core->getCommandList()->IASetVertexBuffers(0, 1, &vbView);
		core->getCommandList()->IASetIndexBuffer(&ibView);
		core->getCommandList()->DrawIndexedInstanced(numMeshIndices, 1, 0, 0, 0);
	}
};



class ScreenSpaceTriangle {
public:
	// Define triangle vertices
	PRIM_VERTEX vertices[3];
	Mesh mesh;

	ScreenSpaceTriangle() {
		vertices[0].position = Vec3(0, 1.0f, 0);
		vertices[0].colour = Colour(0, 1.0f, 0);
		vertices[1].position = Vec3(-1.0f, -1.0f, 0);
		vertices[1].colour = Colour(1.0f, 0, 0);
		vertices[2].position = Vec3(1.0f, -1.0f, 0);
		vertices[2].colour = Colour(0, 0, 1.0f);
	}
	
	void init(Core* core) {
		std::vector<STATIC_VERTEX> verts;
		for (int i = 0; i < 3; i++)
		{
			STATIC_VERTEX v;
			v.pos = vertices[i].position;
			v.normal = Vec3(0, 0, -1);
			v.tangent = Vec3(1, 0, 0);
			v.uv[0] = 0.0f;
			v.uv[1] = 0.0f;
			verts.push_back(v);
		}
		std::vector<unsigned int> indices = { 0, 1, 2 };
		mesh.init(core, verts, indices);
	}
};

class Plane {
public:
	std::vector<STATIC_VERTEX> vertices;
	std::vector<unsigned int> indices;
	Mesh mesh;
	
	void init(Core* core) {
		vertices.push_back(addVertex(Vec3(-15, 0, -15), Vec3(0, 1, 0), 0, 0));
		vertices.push_back(addVertex(Vec3(15, 0, -15), Vec3(0, 1, 0), 1, 0));
		vertices.push_back(addVertex(Vec3(-15, 0, 15), Vec3(0, 1, 0), 0, 1));
		vertices.push_back(addVertex(Vec3(15, 0, 15), Vec3(0, 1, 0), 1, 1));
		
		indices.push_back(2); indices.push_back(1); indices.push_back(0);
		indices.push_back(1); indices.push_back(2); indices.push_back(3);
		mesh.init(core, vertices, indices);
	}
};

class Cube {
public:
	std::vector<STATIC_VERTEX> vertices;
	std::vector<unsigned int> indices;
	Mesh mesh;

	Vec3 p0 = Vec3(-1.0f, -1.0f, -1.0f);
	Vec3 p1 = Vec3(1.0f, -1.0f, -1.0f);
	Vec3 p2 = Vec3(1.0f, 1.0f, -1.0f);
	Vec3 p3 = Vec3(-1.0f, 1.0f, -1.0f);
	Vec3 p4 = Vec3(-1.0f, -1.0f, 1.0f);
	Vec3 p5 = Vec3(1.0f, -1.0f, 1.0f);
	Vec3 p6 = Vec3(1.0f, 1.0f, 1.0f);
	Vec3 p7 = Vec3(-1.0f, 1.0f, 1.0f);

	void init(Core* core){
		// Front face
		vertices.push_back(addVertex(p0, Vec3(0.0f, 0.0f, -1.0f), 0.0f, 1.0f));
		vertices.push_back(addVertex(p1, Vec3(0.0f, 0.0f, -1.0f), 1.0f, 1.0f));
		vertices.push_back(addVertex(p2, Vec3(0.0f, 0.0f, -1.0f), 1.0f, 0.0f));
		vertices.push_back(addVertex(p3, Vec3(0.0f, 0.0f, -1.0f), 0.0f, 0.0f));
		// Back face
		vertices.push_back(addVertex(p5, Vec3(0.0f, 0.0f, 1.0f), 0.0f, 1.0f));
		vertices.push_back(addVertex(p4, Vec3(0.0f, 0.0f, 1.0f), 1.0f, 1.0f));
		vertices.push_back(addVertex(p7, Vec3(0.0f, 0.0f, 1.0f), 1.0f, 0.0f));
		vertices.push_back(addVertex(p6, Vec3(0.0f, 0.0f, 1.0f), 0.0f, 0.0f));
		// Left face
		vertices.push_back(addVertex(p4, Vec3(-1.0f, 0.0f, 0.0f), 0.0f, 1.0f));
		vertices.push_back(addVertex(p0, Vec3(-1.0f, 0.0f, 0.0f), 1.0f, 1.0f));
		vertices.push_back(addVertex(p3, Vec3(-1.0f, 0.0f, 0.0f), 1.0f, 0.0f));
		vertices.push_back(addVertex(p7, Vec3(-1.0f, 0.0f, 0.0f), 0.0f, 0.0f));
		// Right face
		vertices.push_back(addVertex(p1, Vec3(1.0f, 0.0f, 0.0f), 0.0f, 1.0f));
		vertices.push_back(addVertex(p5, Vec3(1.0f, 0.0f, 0.0f), 1.0f, 1.0f));
		vertices.push_back(addVertex(p6, Vec3(1.0f, 0.0f, 0.0f), 1.0f, 0.0f));
		vertices.push_back(addVertex(p2, Vec3(1.0f, 0.0f, 0.0f), 0.0f, 0.0f));
		// Top face
		vertices.push_back(addVertex(p3, Vec3(0.0f, 1.0f, 0.0f), 0.0f, 1.0f));
		vertices.push_back(addVertex(p2, Vec3(0.0f, 1.0f, 0.0f), 1.0f, 1.0f));
		vertices.push_back(addVertex(p6, Vec3(0.0f, 1.0f, 0.0f), 1.0f, 0.0f));
		vertices.push_back(addVertex(p7, Vec3(0.0f, 1.0f, 0.0f), 0.0f, 0.0f));
		// Bottom face
		vertices.push_back(addVertex(p4, Vec3(0.0f, -1.0f, 0.0f), 0.0f, 1.0f));
		vertices.push_back(addVertex(p5, Vec3(0.0f, -1.0f, 0.0f), 1.0f, 1.0f));
		vertices.push_back(addVertex(p1, Vec3(0.0f, -1.0f, 0.0f), 1.0f, 0.0f));
		vertices.push_back(addVertex(p0, Vec3(0.0f, -1.0f, 0.0f), 0.0f, 0.0f));

		// Add Indices
		indices.push_back(0); indices.push_back(1); indices.push_back(2);
		indices.push_back(0); indices.push_back(2); indices.push_back(3);
		indices.push_back(4); indices.push_back(5); indices.push_back(6);
		indices.push_back(4); indices.push_back(6); indices.push_back(7);
		indices.push_back(8); indices.push_back(9); indices.push_back(10);
		indices.push_back(8); indices.push_back(10); indices.push_back(11);
		indices.push_back(12); indices.push_back(13); indices.push_back(14);
		indices.push_back(12); indices.push_back(14); indices.push_back(15);
		indices.push_back(16); indices.push_back(17); indices.push_back(18);
		indices.push_back(16); indices.push_back(18); indices.push_back(19);
		indices.push_back(20); indices.push_back(21); indices.push_back(22);
		indices.push_back(20); indices.push_back(22); indices.push_back(23);

		mesh.init(core, vertices, indices);
	}
};



class MeshLoader {
public:
	std::vector<Mesh> meshes;
	Animation animation;

	PSOManager* psoManager;

	MeshLoader(PSOManager* psoMgr) : psoManager(psoMgr) {}

	void loadGEM(Core* core, const char* filename, std::vector<std::string> psonames) {
		// Load GEM file
		int numPSOs = psonames.size();
		GEMLoader::GEMModelLoader loader;
		std::vector<GEMLoader::GEMMesh> gemmeshes;
		GEMLoader::GEMAnimation gemanimation;
		loader.load(filename, gemmeshes, gemanimation);
		// check if have animation
		if (gemanimation.bones.size() > 0) {
			// Load Meshes
			for (int i = 0; i < gemmeshes.size(); i++) {
				Mesh mesh;
				std::vector<ANIMATED_VERTEX> vertices;
				for (int j = 0; j < gemmeshes[i].verticesAnimated.size(); j++) {
					ANIMATED_VERTEX v;
					memcpy(&v, &gemmeshes[i].verticesAnimated[j], sizeof(ANIMATED_VERTEX));
					vertices.push_back(v);
				}
				mesh.init(core, vertices, gemmeshes[i].indices);
				// Assign PSO name based on mesh index
				if (i < numPSOs)
					mesh.psoNames = psonames[i];
				else
					mesh.psoNames = psonames[numPSOs - 1];
				meshes.push_back(mesh);
			}
			// Load Bones for Animation
			for (int i = 0; i < gemanimation.bones.size(); i++)
			{
				Bone bone;
				bone.name = gemanimation.bones[i].name;
				memcpy(&bone.offset, &gemanimation.bones[i].offset, 16 * sizeof(float));
				bone.parentIndex = gemanimation.bones[i].parentIndex;
				animation.skeleton.bones.push_back(bone);
			}
			// Load Animations and copy data
			for (int i = 0; i < gemanimation.animations.size(); i++)
			{
				std::string name = gemanimation.animations[i].name;
				AnimationSequence aseq;
				aseq.ticksPerSecond = gemanimation.animations[i].ticksPerSecond;
				for (int n = 0; n < gemanimation.animations[i].frames.size(); n++)
				{
					AnimationFrame frame;
					for (int index = 0; index < gemanimation.animations[i].frames[n].positions.size(); index++)
					{
						Vec3 p;
						Vec4 q;
						Vec3 s;
						memcpy(&p, &gemanimation.animations[i].frames[n].positions[index], sizeof(Vec3));
						frame.positions.push_back(p);
						memcpy(&q, &gemanimation.animations[i].frames[n].rotations[index], sizeof(Vec4));
						frame.rotations.push_back(q);
						memcpy(&s, &gemanimation.animations[i].frames[n].scales[index], sizeof(Vec3));
						frame.scales.push_back(s);
					}
					aseq.frames.push_back(frame);
				}
				animation.animations.insert({ name, aseq });
			}
		}
		// No animation
		else {
			// Load Meshes
			for (int i = 0; i < gemmeshes.size(); i++) {
				Mesh mesh;
				std::vector<STATIC_VERTEX> vertices;
				for (int j = 0; j < gemmeshes[i].verticesStatic.size(); j++) {
					STATIC_VERTEX v;
					memcpy(&v, &gemmeshes[i].verticesStatic[j], sizeof(STATIC_VERTEX));
					vertices.push_back(v);
				}
				mesh.init(core, vertices, gemmeshes[i].indices);
				// Assign PSO name based on mesh index
				if (i < numPSOs)
					mesh.psoNames = psonames[i];
				else
					mesh.psoNames = psonames[numPSOs - 1];
				meshes.push_back(mesh);
			}
		}
	}

	void loadGEM(Core* core, const char* filename, std::string psoname) {
		std::vector<std::string> names;
		names.push_back(psoname);
		loadGEM(core, filename, names);
	}

	void draw(Core* core) {
		for (int i = 0; i < meshes.size(); i++) {
			psoManager->bind(core, meshes[i].psoNames);
			meshes[i].draw(core);
		}
	}
};