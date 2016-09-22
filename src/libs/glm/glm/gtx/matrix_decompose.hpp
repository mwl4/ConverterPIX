/// @ref gtx_matrix_decompose
/// @file glm/gtx/matrix_decompose.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_matrix_decompose GLM_GTX_matrix_decompose
/// @ingroup gtx
///
/// @brief Decomposes a model matrix to translations, rotation and scale components
///
/// <glm/gtx/matrix_decompose.hpp> need to be included to use these functionalities.

#pragma once

// Dependencies
#include "../mat4x4.hpp"
#include "../vec3.hpp"
#include "../vec4.hpp"
#include "../geometric.hpp"
#include "../gtc/quaternion.hpp"
#include "../gtc/matrix_transform.hpp"

#if GLM_MESSAGES == GLM_MESSAGES_ENABLED && !defined(GLM_EXT_INCLUDED)
#	pragma message("GLM: GLM_GTX_matrix_decompose extension included")
#endif

namespace glm
{
	/// @addtogroup gtx_matrix_decompose
	/// @{

	/// Decomposes a model matrix to translations, rotation and scale components 
	/// @see gtx_matrix_decompose
	template <typename T, precision P>
	GLM_FUNC_DECL bool decompose(
		detail::tmat4x4<T, P> const & modelMatrix,
		detail::tvec3<T, P> & scale, detail::tquat<T, P> & orientation, detail::tvec3<T, P> & translation, detail::tvec3<T, P> & skew, detail::tvec4<T, P> & perspective);

	/// @}
}//namespace glm

#include "matrix_decompose.inl"
