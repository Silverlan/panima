/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PANIMA_EXPRESSION_HPP__
#define __PANIMA_EXPRESSION_HPP__

#include <sharedutils/util.h>
#include <sharedutils/magic_enum.hpp>
#include <udm_types.hpp>
#include <udm_trivial_types.hpp>

namespace panima
{
	template<typename T>
		concept is_supported_expression_type_v = (udm::is_numeric_type(udm::type_to_enum<T>()) && !std::is_same_v<T,udm::Half>) || udm::is_vector_type<T> ||
			udm::is_matrix_type<T> || std::is_same_v<T,Quat> || std::is_same_v<T,EulerAngles>;
};

#endif
