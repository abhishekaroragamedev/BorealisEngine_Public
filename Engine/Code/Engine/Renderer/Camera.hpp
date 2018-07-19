#pragma once

#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/Framebuffer.hpp"
#include "Engine/Renderer/Texture.hpp"

class TextureCube;

struct CameraUBO	/* std140 */
{

public:
	CameraUBO( const Matrix44& view, const Matrix44& projection, const Vector3& eyePosition )
		:	m_view( view ),
			m_projection( projection ),
			m_eyePosition( eyePosition )
	{

	}

public:
	Matrix44 m_view = Matrix44::IDENTITY;		// Byte 0-63
	Matrix44 m_projection = Matrix44::IDENTITY;	// Byte 64-127
	Vector3 m_eyePosition = Vector3::ZERO;	// Byte 128-139
	float m_padding = 0.0f;					// Byte 140-143

};

class Camera
{

public:
	Camera( Texture* colorTarget = nullptr, Texture* depthTarget = nullptr, bool shouldClearColor = true, bool shouldClearDepth = true, int sortOrder = 0 );
	~Camera();

	bool ShouldClearColor() const;
	bool ShouldClearDepth() const;
	bool IsBloomEnabled() const;
	bool IsMSAAEnabled() const;
	int GetSortOrder() const;
	Matrix44 GetModelMatrix();
	Matrix44 GetViewMatrix();
	Matrix44 GetProjectionMatrix() const;
	FrameBuffer* GetFrameBuffer() const;
	unsigned int GetFrameBufferHandle() const;
	unsigned int GetNumMSAASamples() const;
	bool SeesShadows() const;
	float GetMaxShadowDepth() const;
	AABB2 GetViewport() const;
	Transform* GetTransform();
	Vector3 GetForward();
	Vector3 GetRight();
	Vector3 GetUp();
	Vector3 GetPosition();
	Vector3 GetEulerAngles();
	Vector3 GetScale();
	void GetOrthoCameraPropertiesForShadows( const Vector3& lightDirection, Vector3& out_lightPosition, float& out_orthoSize, float& out_farZ ); // w is used to store Ortho size

	CameraUBO MakeUniformBufferStruct();
	Vector3 ScreenToWorld( const Vector2& screenSpaceXY, float viewSpaceZ = 0.0f );

	void SetColorTarget( Texture* colorTarget, unsigned int index = 0U );
	void RemoveColorTarget( Texture* colorTarget );
	void RemoveColorTarget( unsigned int index );
	void SetDepthStencilTarget( Texture* depthTarget );

	void LookAt( const Vector3& position, const Vector3& target, const Vector3& up = Vector3::UP );

	void Translate( const Vector3& translation );
	void Rotate( const Vector3& rotationEuler );
	void Scale( const Vector3& scale );

	void SetPosition( const Vector3& position );
	void SetEuler( const Vector3& eulerAngles );
	void SetScale( const Vector3& scale );
	void SetProjection( const Matrix44& projection );
	void SetProjectionPerspective( float fovDegrees, float aspect, float nearZ, float farZ );
	void SetProjectionOrtho( float size, float aspect, float nearZ, float farZ );
	void SetViewport( const AABB2& viewport );	// Viewport should be ( 0.0f, 0.0f, 1.0f, 1.0f )

	void SetShouldClearColor( bool shouldClear );
	void SetShouldClearDepth( bool shouldClear );
	void SetMaxShadowDepth( float maxShadowDepth );
	void DisableShadowViewing();
	void SetBloomEnabled( bool bloomEnabled );
	void EnableMSAA( unsigned int numSamples );
	void DisableMSAA();
	void SetSortOrder( int sortOrder );

	void AddSkybox( TextureCube* skyboxTexture );	// The Camera will own the TextureCube after this
	void AddSkybox( const char* textureCubeFileName );
	void RemoveSkybox();
	void RenderSkybox();
	bool HasSkybox() const;

	bool Finalize();

protected:
	void ClampEulerAngles();

protected:
	Transform m_transform;	// Model matrix representation; can be used to generate model and view matrices on demand

	// Projection properties
	Matrix44 m_projectionMatrix;
	float m_projectionAspect = 1.0f;
	float m_nearZ = 0.1f;
	float m_farZ = 1000.0f;
	float m_fovDegrees = 0.0f;
	
	AABB2 m_viewport = AABB2::ZERO_TO_ONE;

	FrameBuffer* m_frameBuffer = nullptr;

	TextureCube* m_skybox = nullptr;

	// Forward Rendering settings
	bool m_shouldClearColor = true;
	bool m_shouldClearDepth = true;
	int m_sortOrder = 0;

	// Shadows
	float m_maxShadowDepth = 0.0f;

	// Bloom
	bool m_bloomEnabled = false;

	// MSAA
	bool m_msaaEnabled = false;
	unsigned int m_numMSAASamples = 0U;

};
