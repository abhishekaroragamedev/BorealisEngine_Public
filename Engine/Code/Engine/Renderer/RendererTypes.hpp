#pragma once

#include <string>

enum DrawPrimitiveType
{
	PRIMITIVE_TYPE_INVALID = -1,
	LINES,
	LINE_LOOP,
	TRIANGLES,
	QUADS,
	POLYGON,
	NUM_TYPES
};

enum RPolygonType
{
	INVALID = -1,
	REGULAR = 0,
	BROKEN_LINES = 1
};

enum DepthTestCompare
{
	COMPARE_NEVER,       // GL_NEVER
	COMPARE_LESS,        // GL_LESS
	COMPARE_LEQUAL,      // GL_LEQUAL
	COMPARE_GREATER,     // GL_GREATER
	COMPARE_GEQUAL,      // GL_GEQUAL
	COMPARE_EQUAL,       // GL_EQUAL
	COMPARE_NOT_EQUAL,   // GL_NOTEQUAL
	COMPARE_ALWAYS,      // GL_ALWAYS
};

enum TextDrawMode
{
	TEXT_DRAW_MODE_INVALID = -1,
	TEXT_DRAW_SHRINK_TO_FIT,
	TEXT_DRAW_WORD_WRAP,
	TEXT_DRAW_OVERRUN,
	NUM_MODES
};

enum RendererDataType
{
	RENDER_TYPE_FLOAT,
	RENDER_TYPE_UNSIGNED_INT
};

enum RendererCullMode
{
	CULL_MODE_FRONT,
	CULL_MODE_BACK,
	CULL_MODE_FRONT_AND_BACK,
	CULL_MODE_NONE
};

enum RendererWindOrder 
{
	WIND_CLOCKWISE,
	WIND_COUNTER_CLOCKWISE,
};

enum RendererBlendOperation
{
	OPERATION_ADD,
	OPERATION_SUBTRACT,
	OPERATION_REVERSE_SUBRACT,
	OPERATION_MIN,
	OPERATION_MAX
};

enum RendererBlendFactor
{
	FACTOR_ZERO,
	FACTOR_ONE,
	FACTOR_SRC_COLOR,
	FACTOR_ONE_MINUS_SRC_COLOR,
	FACTOR_DST_COLOR,
	FACTOR_ONE_MINUS_DST_COLOR,
	FACTOR_SRC_ALPHA,
	FACTOR_ONE_MINUS_SRC_ALPHA,
	FACTOR_DST_ALPHA,
	FACTOR_ONE_MINUS_DST_ALPHA
};

enum RendererPolygonMode
{
	POLYGON_MODE_LINE,
	POLYGON_MODE_FILL
};

enum RendererBlendMode
{
	ALPHA,
	ADDITIVE,
	NUM_BLEND_MODES
};

enum TextureType
{
	TEXTURE_TYPE_INVALID = -1,
	TEXTURE_TYPE_2D,
	TEXTURE_TYPE_2D_MULTISAMPLE,
	TEXTURE_TYPE_CUBEMAP
};

enum TextureAccessType
{
	TEXTURE_ACCESS_INVALID = -1,
	TEXTURE_ACCESS_READ_ONLY,
	TEXTURE_ACCESS_WRITE_ONLY,
	TEXTURE_ACCESS_READ_WRITE
};

enum TextureCubeSide
{
	TEXCUBE_INVALID = -1,
	TEXCUBE_RIGHT,
	TEXCUBE_LEFT,
	TEXCUBE_TOP,
	TEXCUBE_BOTTOM,
	TEXCUBE_FRONT,
	TEXCUBE_BACK
};

RendererBlendMode GetBlendModeFromName( const std::string& name );
RendererBlendOperation GetBlendOperationFromName( const std::string& name );
RendererBlendFactor GetBlendFactorFromName( const std::string& name );
RendererCullMode GetCullModeFromName( const std::string& name );
RendererWindOrder GetWindOrderFromName( const std::string& name );
RendererPolygonMode GetFillModeFromName( const std::string& name );
DepthTestCompare GetDepthCompareFromName( const std::string& name );
