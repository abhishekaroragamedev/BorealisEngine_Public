#include "Engine/Renderer/RendererTypes.hpp"
#include "Engine/Tools/DevConsole.hpp"

RendererBlendMode GetBlendModeFromName( const std::string& name )
{
	if ( name == "Alpha" || name == "alpha" )
	{
		return RendererBlendMode::ALPHA;
	}
	if ( name == "Additive" || name == "additive" )
	{
		return RendererBlendMode::ADDITIVE;
	}

	return RendererBlendMode::ALPHA;
}

RendererBlendOperation GetBlendOperationFromName( const std::string& name )
{
	if ( name == "Add" || name == "add" )
	{
		return RendererBlendOperation::OPERATION_ADD;
	}
	if ( name == "Subtract" || name == "subtract" )
	{
		return RendererBlendOperation::OPERATION_SUBTRACT;
	}
	if ( name == "ReverseSubtract" || name == "reverseSubtract" || name == "reversesubtract" )
	{
		return RendererBlendOperation::OPERATION_REVERSE_SUBRACT;
	}
	if ( name == "Max" || name == "max" )
	{
		return RendererBlendOperation::OPERATION_MAX;
	}
	if ( name == "Min" || name == "min" )
	{
		return RendererBlendOperation::OPERATION_MIN;
	}

	return RendererBlendOperation::OPERATION_ADD;
}

RendererBlendFactor GetBlendFactorFromName( const std::string& name )
{
	if ( name == "Zero" || name == "zero" )
	{
		return RendererBlendFactor::FACTOR_ZERO;
	}
	if ( name == "One" || name == "one" )
	{
		return RendererBlendFactor::FACTOR_ONE;
	}
	if ( name == "srcColor" )
	{
		return RendererBlendFactor::FACTOR_SRC_COLOR;
	}
	if ( name == "oneMinusSrcColor" )
	{
		return RendererBlendFactor::FACTOR_ONE_MINUS_SRC_COLOR;
	}
	if ( name == "dstColor" )
	{
		return RendererBlendFactor::FACTOR_DST_COLOR;
	}
	if ( name == "oneMinusDstColor" )
	{
		return RendererBlendFactor::FACTOR_ONE_MINUS_DST_COLOR;
	}
	if ( name == "srcAlpha" )
	{
		return RendererBlendFactor::FACTOR_SRC_ALPHA;
	}
	if ( name == "oneMinusSrcAlpha" )
	{
		return RendererBlendFactor::FACTOR_ONE_MINUS_SRC_ALPHA;
	}
	if ( name == "dstAlpha" )
	{
		return RendererBlendFactor::FACTOR_DST_ALPHA;
	}
	if ( name == "oneMinusDstAlpha" )
	{
		return RendererBlendFactor::FACTOR_ONE_MINUS_DST_ALPHA;
	}

	return RendererBlendFactor::FACTOR_ZERO;
}

RendererCullMode GetCullModeFromName( const std::string& name )
{
	if ( name == "Back" || name == "back" )
	{
		return RendererCullMode::CULL_MODE_BACK;
	}
	if ( name == "Front" || name == "front" )
	{
		return RendererCullMode::CULL_MODE_FRONT;
	}
	if ( name == "None" || name == "none" )
	{
		return RendererCullMode::CULL_MODE_NONE;
	}

	return RendererCullMode::CULL_MODE_BACK;
}

RendererWindOrder GetWindOrderFromName( const std::string& name )
{
	if ( name == "CW" || name == "cw" || name == "Clockwise" || name == "clockwise" )
	{
		return RendererWindOrder::WIND_CLOCKWISE;
	}
	if ( name == "CCW" || name == "ccw" || name == "CounterClockwise"|| name == "counterClockwise" || name == "counterclockwise" )
	{
		return RendererWindOrder::WIND_COUNTER_CLOCKWISE;
	}

	return RendererWindOrder::WIND_COUNTER_CLOCKWISE;
}

RendererPolygonMode GetFillModeFromName( const std::string& name )
{
	if ( name == "Fill" || name == "fill" || name == "Solid" || name == "solid" )
	{
		return RendererPolygonMode::POLYGON_MODE_FILL;
	}
	if ( name == "Wire" || name == "wire" || name == "Line" || name == "line" )
	{
		return RendererPolygonMode::POLYGON_MODE_LINE;
	}

	return RendererPolygonMode::POLYGON_MODE_FILL;
}

DepthTestCompare GetDepthCompareFromName( const std::string& name )
{
	if ( name == "Always" || name == "always" )
	{
		return DepthTestCompare::COMPARE_ALWAYS;
	}
	if ( name == "Never" || name == "never" )
	{
		return DepthTestCompare::COMPARE_NEVER;
	}
	if ( name == "Equal" || name == "equal" )
	{
		return DepthTestCompare::COMPARE_EQUAL;
	}
	if ( name == "NEqual" || name == "nequal" )
	{
		return DepthTestCompare::COMPARE_NOT_EQUAL;
	}
	if ( name == "Less" || name == "less" )
	{
		return DepthTestCompare::COMPARE_LESS;
	}
	if ( name == "LEqual" || name == "lequal" )
	{
		return DepthTestCompare::COMPARE_LEQUAL;
	}
	if ( name == "Greater" || name == "greater" )
	{
		return DepthTestCompare::COMPARE_GREATER;
	}
	if ( name == "GEqual" || name == "gequal" )
	{
		return DepthTestCompare::COMPARE_GEQUAL;
	}

	return DepthTestCompare::COMPARE_LESS;
}
