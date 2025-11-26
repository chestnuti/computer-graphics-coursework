#pragma once

#include "Vector.h"
#include "Operators.h"
#include "Core.h"



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


class Mesh {
public:
	// Vertex buffer
	ID3D12Resource* vertexBuffer;
	// Vertex buffer view
	D3D12_VERTEX_BUFFER_VIEW vbView;


	Mesh() : vertexBuffer(nullptr) {}

	void init(Core* core, void* vertices, int vertexSizeInBytes, int numVertices)
	{
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
	}

	void draw(Core* core)
	{
		core->getCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		core->getCommandList()->IASetVertexBuffers(0, 1, &vbView);
		core->getCommandList()->DrawInstanced(3, 1, 0, 0);
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
	
	void init(Core& core) {
		mesh.init(&core, vertices, sizeof(PRIM_VERTEX), 3);
	}

};