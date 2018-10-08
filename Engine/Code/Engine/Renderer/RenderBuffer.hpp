#pragma once

#include "Engine/Renderer/External/gl/glcorearb.h"
#include "Engine/Renderer/External/gl/glext.h"

class RenderBuffer
{
public:
	RenderBuffer();   // initialize data
	~RenderBuffer();  // cleanup OpenGL resource 

	// copies data to the GPU
	virtual bool CopyToGPU( const size_t byteCount, const void* data );
	unsigned int GetHandle() const;

public:
	unsigned int m_handle = 0U;       // OpenGL handle to the GPU buffer, default = NULL; 
	unsigned int m_bufferSize = 0U;  // how many bytes are in this buffer, default = 0

};

class VertexBuffer : public RenderBuffer
{

public:
	VertexBuffer();
	~VertexBuffer();

public:
	bool CopyToGPU( const size_t byteCount, const void* data ) override;
	void Finalize() const;

public:
	unsigned int m_vertexCount = 0U;
	unsigned int m_vertexStride = 0U;

};

class IndexBuffer : public RenderBuffer
{

public:
	IndexBuffer();
	~IndexBuffer();

public:
	bool CopyToGPU( const size_t byteCount, const void* data ) override;
	void Finalize() const;

public:
	unsigned int m_indexCount = 0U;
	unsigned int m_indexStride = 0U;

};
