#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/TextureCube.hpp"

Camera::Camera( Texture* colorTarget /* = nullptr */, Texture* depthTarget /* = nullptr */, bool shouldClearColor /* = true */, bool shouldClearDepth /* = true */, int sortOrder /* = 0 */ )
	:	m_shouldClearColor( shouldClearColor ),
		m_shouldClearDepth( shouldClearDepth ),
		m_sortOrder( sortOrder )
{
	m_frameBuffer = new FrameBuffer();
	m_frameBuffer->SetColorTarget( colorTarget );
	m_frameBuffer->SetDepthStencilTarget( depthTarget );
	m_viewport = AABB2( 0.0f, 0.0f, 1.0f, 1.0f );
}

Camera::~Camera()
{
	if ( m_frameBuffer != nullptr )
	{
		delete m_frameBuffer;
	}
	m_frameBuffer = nullptr;

	if ( HasSkybox() )
	{
		RemoveSkybox();
	}
}

bool Camera::ShouldClearColor() const
{
	return m_shouldClearColor;
}

bool Camera::ShouldClearDepth() const
{
	return m_shouldClearDepth;
}

bool Camera::IsBloomEnabled() const
{
	return m_bloomEnabled;
}

bool Camera::IsMSAAEnabled() const
{
	return m_msaaEnabled;
}

unsigned int Camera::GetNumMSAASamples() const
{
	return m_numMSAASamples;
}

bool Camera::SeesShadows() const
{
	return ( m_maxShadowDepth > 0.0f );
}

float Camera::GetMaxShadowDepth() const
{
	return m_maxShadowDepth;
}

int Camera::GetSortOrder() const
{
	return m_sortOrder;
}

Matrix44 Camera::GetModelMatrix()
{
	return m_transform.GetAsMatrixWorld();
}

Matrix44 Camera::GetViewMatrix()
{
	Matrix44 viewMatrix = GetModelMatrix();
	viewMatrix.tX = DotProduct( m_transform.GetWorldPosition(), GetRight() );
	viewMatrix.tY = DotProduct( m_transform.GetWorldPosition(), GetUp() );
	viewMatrix.tZ = DotProduct( m_transform.GetWorldPosition(), GetForward() );
	
	viewMatrix.FastInvert();
	return viewMatrix;
}

Matrix44 Camera::GetProjectionMatrix() const
{
	return m_projectionMatrix;
}

FrameBuffer* Camera::GetFrameBuffer() const
{
	return m_frameBuffer;
}

unsigned int Camera::GetFrameBufferHandle() const
{
	return m_frameBuffer->GetHandle();
}

AABB2 Camera::GetViewport() const
{
	return m_viewport;
}

void Camera::SetColorTarget( Texture* colorTarget, unsigned int index /* = 0U */ )
{
	m_frameBuffer->SetColorTarget( colorTarget, index );
}

void Camera::RemoveColorTarget( Texture* colorTarget )
{
	m_frameBuffer->RemoveColorTarget( colorTarget );
}

void Camera::RemoveColorTarget( unsigned int index )
{
	m_frameBuffer->RemoveColorTarget( index );
}

void Camera::SetDepthStencilTarget( Texture* depthTarget )
{
	m_frameBuffer->SetDepthStencilTarget( depthTarget );
}

void Camera::LookAt( const Vector3& position, const Vector3& target, const Vector3& up /* = Vector3::UP */ )
{
	m_transform.m_position = position;

	Matrix44 lookAt = Matrix44::LookAt( position, target, up );
	m_transform.m_scale = Vector3( lookAt.GetIBasis().GetLength(), lookAt.GetJBasis().GetLength(), lookAt.GetKBasis().GetLength() );
	lookAt.NormalizeBases();

	m_transform.m_eulerAngles = lookAt.GetEulerAngles();
	m_transform.MarkDirty();
}

Transform* Camera::GetTransform()
{
	return &m_transform;
}

Vector3 Camera::GetForward()
{
	Matrix44 modelMatrix = GetModelMatrix();
	return modelMatrix.GetKBasis();
}

Vector3 Camera::GetRight()
{
	Matrix44 modelMatrix = GetModelMatrix();
	return modelMatrix.GetIBasis();
}

Vector3 Camera::GetUp()
{
	Matrix44 modelMatrix = GetModelMatrix();
	return modelMatrix.GetJBasis();
}

Vector3 Camera::GetPosition()
{
	return m_transform.GetWorldPosition();
}

Vector3 Camera::GetEulerAngles()
{
	return m_transform.GetWorldPosition();
}

Vector3 Camera::GetScale()
{
	return m_transform.GetWorldPosition();
}

void Camera::GetOrthoCameraPropertiesForShadows( const Vector3& lightDirection, Vector3& out_lightPosition, float& out_orthoSize, float& out_farZ )
{
	Vector4 pointAtForwardEdge = Vector4( 0.0f, 0.0f, m_maxShadowDepth, 1.0f );	// View space
	pointAtForwardEdge = m_projectionMatrix.Transform( pointAtForwardEdge );	// Clip space
	pointAtForwardEdge /= pointAtForwardEdge.w;									// NDC

	Matrix44 inverseViewProjection = m_projectionMatrix;
	inverseViewProjection.Append( GetViewMatrix() );
	inverseViewProjection.Invert();

	Vector4 pointsToConvert[ 8 ] = {	// Frustum points initially in NDC, to convert to world space
		Vector4( -1.0f, -1.0f, -1.0f ),	// Bottom left back (from camera's point of view)
		Vector4( -1.0f, 1.0f, -1.0f ),	// Top left back
		Vector4( 1.0f, -1.0f, -1.0f ),	// Bottom right back
		Vector4( 1.0f, 1.0f, -1.0f ),	// Top right back
		Vector4( -1.0f, -1.0f, pointAtForwardEdge.z ),	// Bottom left front
		Vector4( -1.0f, 1.0f, pointAtForwardEdge.z ),	// Top left front
		Vector4( 1.0f, -1.0f, pointAtForwardEdge.z ),	// Bottom right front
		Vector4( 1.0f, 1.0f, pointAtForwardEdge.z )		// Top right front
	};

	for ( int index = 0; index < 8; index++ )
	{
		pointsToConvert[ index ] = inverseViewProjection.Transform( pointsToConvert[ index ] );
		pointsToConvert[ index ] /= pointsToConvert[ index ].w;	// Convert to world space
	}

	Vector4 lightDirAsVec4 = Vector4( lightDirection );	// Project the bottom-back corners to the light's plane
	pointsToConvert[ 0 ] = pointsToConvert[ 0 ] - ( lightDirAsVec4 * DotProduct( ( pointsToConvert[ 0 ] - pointsToConvert[ 5 ] ), lightDirAsVec4 ) );
	pointsToConvert[ 2 ] = pointsToConvert[ 2 ] - ( lightDirAsVec4 * DotProduct( ( pointsToConvert[ 2 ] - pointsToConvert[ 7 ] ), lightDirAsVec4 ) );

	Vector4 centerOfAABB3Entry = (
		pointsToConvert[ 1 ] +
		pointsToConvert[ 3 ] +
		pointsToConvert[ 5 ] +
		pointsToConvert[ 7 ]
	) / 4.0f;

	out_lightPosition = centerOfAABB3Entry.GetVector3();
	out_orthoSize = ( pointsToConvert[ 7 ] - pointsToConvert[ 5 ] ).GetVector3().GetLength();
	out_farZ = 1000.0f; //DotProduct( ( pointsToConvert[ 6 ] - pointsToConvert[ 7 ] ).GetVector3(), lightDirection );	// TODO: Compute from Frustum
}

Vector3 Camera::ScreenToWorld( const Vector2& screenSpaceXY, float viewSpaceZ /* = 0.0f */ )
{
	Vector2 clipSpaceXY = Vector2( screenSpaceXY );
	clipSpaceXY.x = RangeMapFloat( clipSpaceXY.x, 0.0f, Window::GetInstance()->GetWidthF(), -1.0f, 1.0f );
	clipSpaceXY.y = RangeMapFloat( clipSpaceXY.y, Window::GetInstance()->GetHeightF(), 0.0f, -1.0f, 1.0f );

	Vector3 viewSpaceXYZ;
	float xyMultiplier = ( m_projectionMatrix.kW > 0.0f )? viewSpaceZ /* Perspective */	:	1.0f /* Ortho */;
	viewSpaceXYZ.x = ( clipSpaceXY.x / m_projectionMatrix.iX ) * xyMultiplier;
	viewSpaceXYZ.y = ( clipSpaceXY.y / m_projectionMatrix.jY ) * xyMultiplier;
	viewSpaceXYZ.z = ( m_projectionMatrix.kW > 0.0f )? viewSpaceZ /* Perspective */	:	0.0f /* Ortho */;;

	Matrix44 viewToWorldMatrix = GetModelMatrix();

	Vector3 worldPoint = viewToWorldMatrix.TransformPosition( viewSpaceXYZ );
	return worldPoint;
}

CameraUBO Camera::MakeUniformBufferStruct()
{
	return CameraUBO( GetViewMatrix(), m_projectionMatrix, GetPosition() );
}

void Camera::Translate( const Vector3& translation )
{
	m_transform.Translate( translation );
}

void Camera::Rotate( const Vector3& rotationEuler )
{
	m_transform.Rotate( rotationEuler );
	ClampEulerAngles();
}

void Camera::ClampEulerAngles()
{
	m_transform.m_eulerAngles.x = ClampFloat( m_transform.m_eulerAngles.x, -90.0f, 90.0f );
	
	if ( IsFloatGreaterThanOrEqualTo( m_transform.m_eulerAngles.y, 360.0f ) )
	{
		m_transform.m_eulerAngles.y -= 360.0f;
	}
	if ( m_transform.m_eulerAngles.y < 0.0f )
	{
		m_transform.m_eulerAngles.y += 360.0f;
	}

	m_transform.m_eulerAngles.z = 0.0f;	// Only 5 degrees of freedom!
}

void Camera::Scale( const Vector3& scale )
{
	m_transform.Scale( scale );
}

void Camera::SetPosition( const Vector3& position )
{
	m_transform.m_position = position;
}

void Camera::SetEuler( const Vector3& eulerAngles )
{
	m_transform.m_eulerAngles = eulerAngles;
}

void Camera::SetScale( const Vector3& scale )
{
	m_transform.m_scale = scale;
}

void Camera::SetProjection( const Matrix44& projection )
{
	m_projectionMatrix = Matrix44( projection );
}

void Camera::SetProjectionOrtho( float size, float aspect, float nearZ, float farZ )
{
	float width = size * aspect;
	float height = size;

	float orthoMatrix[ 16 ] = {		// This assumes we are already in view space, and the NDC cube just needs to be scaled down
		( 2.0f / width ), 0.0f, 0.0f, 0.0f,
		0.0f, ( 2.0f / height ), 0.0f, 0.0f,
		0.0f, 0.0f, ( 2.0f / ( farZ - nearZ ) ), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	m_projectionAspect = aspect;
	m_nearZ = nearZ;
	m_farZ = farZ;

	m_projectionMatrix = Matrix44( orthoMatrix );
}

void Camera::SetProjectionPerspective( float fovDegrees, float aspect, float nearZ, float farZ )
{
	m_fovDegrees = fovDegrees;
	m_projectionAspect = aspect;
	m_nearZ = nearZ;
	m_farZ = farZ;

	m_projectionMatrix = Matrix44::MakePerspective( fovDegrees, aspect, nearZ, farZ );
}

void Camera::SetViewport( const AABB2& viewport )
{
	m_viewport = viewport;
	m_viewport.mins.x = ClampFloat( m_viewport.mins.x, 0.0f, 1.0f );
	m_viewport.mins.y = ClampFloat( m_viewport.mins.y, 0.0f, 1.0f );
	m_viewport.maxs.x = ClampFloat( m_viewport.maxs.x, 0.0f, 1.0f );
	m_viewport.maxs.y = ClampFloat( m_viewport.maxs.y, 0.0f, 1.0f );
}

void Camera::SetShouldClearColor( bool shouldClear )
{
	m_shouldClearColor = shouldClear;
}

void Camera::SetShouldClearDepth( bool shouldClear )
{
	m_shouldClearDepth = shouldClear;
}

void Camera::SetMaxShadowDepth( float maxShadowDepth )
{
	m_maxShadowDepth = maxShadowDepth;
}

void Camera::DisableShadowViewing()
{
	SetMaxShadowDepth( 0.0f );
}

void Camera::SetBloomEnabled( bool bloomEnabled )
{
	m_bloomEnabled = bloomEnabled;
}

void Camera::EnableMSAA( unsigned int numSamples )
{
	m_msaaEnabled = true;
	m_numMSAASamples = numSamples;

	if ( m_numMSAASamples == 0U )
	{
		ERROR_AND_DIE( "FATAL: Camera::EnableMSAA(): MSAA samples set to 0 while enabled. Aborting..." )
	}
}

void Camera::DisableMSAA()
{
	m_msaaEnabled = false;
	m_numMSAASamples = 0U;
}

void Camera::SetSortOrder( int sortOrder )
{
	m_sortOrder = sortOrder;
}

bool Camera::Finalize()
{
	return m_frameBuffer->Finalize();
}

void Camera::AddSkybox( TextureCube* textureCube )
{
	m_skybox = textureCube;
}

void Camera::AddSkybox( const char* textureCubeFileName )
{
	m_skybox = new TextureCube();
	m_skybox->MakeFromImage( textureCubeFileName );
}

void Camera::RemoveSkybox()
{
	m_skybox = nullptr;
}

void Camera::RenderSkybox()
{
	Renderer::GetInstance()->SetShaderPass( Renderer::GetInstance()->CreateOrGetShader( "Skybox" )->GetPass( 0 ) );
	Renderer::GetInstance()->SetCamera( this );
	Renderer::GetInstance()->DrawTextureCube( *m_skybox, Vector3::ONE );
}

bool Camera::HasSkybox() const
{
	return ( m_skybox != nullptr );
}
