#pragma once
#include "Engine/Renderer/Texture.hpp"
#include<vector>

class FrameBuffer
{
public:
	FrameBuffer(); 
	~FrameBuffer();

	unsigned int GetHandle() const;
	unsigned int GetWidth() const;
	unsigned int GetHeight() const;
	unsigned int GetColorTargetCount() const;
	bool IsMultiSample() const;

	void SetColorTarget( Texture* colorTarget, unsigned int index = 0U );
	void RemoveColorTarget( unsigned int index );
	void RemoveColorTarget( Texture* colorTarget );
	void SetDepthStencilTarget( Texture* depthTarget );
	void BindColorTargets();
	void BindDepthTarget();
	void MarkDirty();

	bool Finalize();

public:
	unsigned int m_handle = 0;
	bool m_isDirty = false;

	std::vector< Texture* >m_colorTargets;
	Texture* m_depthStencilTarget = nullptr;

};
