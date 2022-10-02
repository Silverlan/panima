/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "value_expression.hpp"

namespace panima::expression
{
	constexpr auto ENABLE_SAFETY_CHECKS = false;
	static ExprScalar q_from_axis_angle(exprtk::igeneric_function<ExprScalar>::parameter_list_t parameters)
	{
		using generic_type = exprtk::igeneric_function<ExprScalar>::generic_type;
		if constexpr(ENABLE_SAFETY_CHECKS)
		{
			if(parameters.size() < 3 ||
				parameters[0].type != generic_type::e_vector ||
				parameters[1].type != generic_type::e_scalar ||
				parameters[2].type != generic_type::e_vector
			)
				return {};
		}
		typename generic_type::vector_view axis {parameters[0]};
		typename generic_type::scalar_view angle {parameters[1]};
		typename generic_type::vector_view out {parameters[2]};
		if constexpr(ENABLE_SAFETY_CHECKS)
		{
			if(axis.size() < 3 || out.size() < 4)
				return {};
		}
		*reinterpret_cast<Quat*>(&out[0]) = uquat::create(*reinterpret_cast<::Vector3*>(&axis[0]),angle());
		return ExprScalar{};
	}

	static ExprScalar q_forward(exprtk::igeneric_function<ExprScalar>::parameter_list_t parameters)
	{
		using generic_type = exprtk::igeneric_function<ExprScalar>::generic_type;
		if constexpr(ENABLE_SAFETY_CHECKS)
		{
			if(parameters.size() < 2 ||
				parameters[0].type != generic_type::e_vector ||
				parameters[1].type != generic_type::e_vector
			)
				return {};
		}
		typename generic_type::vector_view vv {parameters[0]};
		typename generic_type::vector_view out {parameters[1]};
		if constexpr(ENABLE_SAFETY_CHECKS)
		{
			if(vv.size() < 4 || out.size() < 3)
				return {};
		}

		*reinterpret_cast<::Vector3*>(&out[0]) = uquat::forward(*reinterpret_cast<Quat*>(&vv[0]));
		return ExprScalar{};
	}

	static ExprScalar q_right(exprtk::igeneric_function<ExprScalar>::parameter_list_t parameters)
	{
		using generic_type = exprtk::igeneric_function<ExprScalar>::generic_type;
		if constexpr(ENABLE_SAFETY_CHECKS)
		{
			if(parameters.size() < 2 ||
				parameters[0].type != generic_type::e_vector ||
				parameters[1].type != generic_type::e_vector
			)
				return {};
		}
		typename generic_type::vector_view vv {parameters[0]};
		typename generic_type::vector_view out {parameters[1]};
		if constexpr(ENABLE_SAFETY_CHECKS)
		{
			if(vv.size() < 4 || out.size() < 3)
				return {};
		}

		*reinterpret_cast<::Vector3*>(&out[0]) = uquat::right(*reinterpret_cast<Quat*>(&vv[0]));
		return ExprScalar{};
	}

	static ExprScalar q_up(exprtk::igeneric_function<ExprScalar>::parameter_list_t parameters)
	{
		using generic_type = exprtk::igeneric_function<ExprScalar>::generic_type;
		if constexpr(ENABLE_SAFETY_CHECKS)
		{
			if(parameters.size() < 2 ||
				parameters[0].type != generic_type::e_vector ||
				parameters[1].type != generic_type::e_vector
			)
				return {};
		}
		typename generic_type::vector_view vv {parameters[0]};
		typename generic_type::vector_view out {parameters[1]};
		if constexpr(ENABLE_SAFETY_CHECKS)
		{
			if(vv.size() < 4 || out.size() < 3)
				return {};
		}

		*reinterpret_cast<::Vector3*>(&out[0]) = uquat::up(*reinterpret_cast<Quat*>(&vv[0]));
		return ExprScalar{};
	}

	static ExprScalar q_slerp(exprtk::igeneric_function<ExprScalar>::parameter_list_t parameters)
	{
		using generic_type = exprtk::igeneric_function<ExprScalar>::generic_type;
		if constexpr(ENABLE_SAFETY_CHECKS)
		{
			if(parameters.size() < 4 ||
				parameters[0].type != generic_type::e_vector ||
				parameters[1].type != generic_type::e_vector ||
				parameters[2].type != generic_type::e_scalar ||
				parameters[3].type != generic_type::e_vector
			)
				return {};
		}
		typename generic_type::vector_view q0 {parameters[0]};
		typename generic_type::vector_view q1 {parameters[1]};
		typename generic_type::scalar_view factor {parameters[2]};
		typename generic_type::vector_view out {parameters[3]};
		if constexpr(ENABLE_SAFETY_CHECKS)
		{
			if(q0.size() < 4 || q1.size() < 4 || out.size() < 4)
				return {};
		}

		*reinterpret_cast<::Quat*>(&out[0]) = uquat::slerp(*reinterpret_cast<Quat*>(&q0[0]),*reinterpret_cast<Quat*>(&q1[0]),factor());
		return ExprScalar{};
	}

	static ExprScalar q_lerp(exprtk::igeneric_function<ExprScalar>::parameter_list_t parameters)
	{
		using generic_type = exprtk::igeneric_function<ExprScalar>::generic_type;
		if constexpr(ENABLE_SAFETY_CHECKS)
		{
			if(parameters.size() < 4 ||
				parameters[0].type != generic_type::e_vector ||
				parameters[1].type != generic_type::e_vector ||
				parameters[2].type != generic_type::e_scalar ||
				parameters[3].type != generic_type::e_vector
			)
				return {};
		}
		typename generic_type::vector_view q0 {parameters[0]};
		typename generic_type::vector_view q1 {parameters[1]};
		typename generic_type::scalar_view factor {parameters[2]};
		typename generic_type::vector_view out {parameters[3]};
		if constexpr(ENABLE_SAFETY_CHECKS)
		{
			if(q0.size() < 4 || q1.size() < 4 || out.size() < 4)
				return {};
		}

		*reinterpret_cast<::Quat*>(&out[0]) = uquat::lerp(*reinterpret_cast<Quat*>(&q0[0]),*reinterpret_cast<Quat*>(&q1[0]),factor());
		return ExprScalar{};
	}

	static ExprScalar q_dot(exprtk::igeneric_function<ExprScalar>::parameter_list_t parameters)
	{
		using generic_type = exprtk::igeneric_function<ExprScalar>::generic_type;
		if constexpr(ENABLE_SAFETY_CHECKS)
		{
			if(parameters.size() < 2 ||
				parameters[0].type != generic_type::e_vector ||
				parameters[1].type != generic_type::e_vector
			)
				return {};
		}
		typename generic_type::vector_view q0 {parameters[0]};
		typename generic_type::vector_view q1 {parameters[1]};
		if constexpr(ENABLE_SAFETY_CHECKS)
		{
			if(q0.size() < 4 || q1.size() < 4)
				return {};
		}

		return uquat::dot_product(*reinterpret_cast<Quat*>(&q0[0]),*reinterpret_cast<Quat*>(&q1[0]));
	}

	static ExprScalar q_mul(exprtk::igeneric_function<ExprScalar>::parameter_list_t parameters)
	{
		using generic_type = exprtk::igeneric_function<ExprScalar>::generic_type;
		if constexpr(ENABLE_SAFETY_CHECKS)
		{
			if(parameters.size() < 3 ||
				parameters[0].type != generic_type::e_vector ||
				parameters[1].type != generic_type::e_vector ||
				parameters[2].type != generic_type::e_vector
			)
				return {};
		}
		typename generic_type::vector_view q0 {parameters[0]};
		typename generic_type::vector_view q1 {parameters[1]};
		typename generic_type::vector_view out {parameters[3]};
		if constexpr(ENABLE_SAFETY_CHECKS)
		{
			if(q0.size() < 4 || q1.size() < 4 || out.size() < 4)
				return {};
		}

		*reinterpret_cast<::Quat*>(&out[0]) = (*reinterpret_cast<Quat*>(&q0[0])) *(*reinterpret_cast<Quat*>(&q1[0]));
		return ExprScalar{};
	}

	static ExprScalar q_inverse(exprtk::igeneric_function<ExprScalar>::parameter_list_t parameters)
	{
		using generic_type = exprtk::igeneric_function<ExprScalar>::generic_type;
		if constexpr(ENABLE_SAFETY_CHECKS)
		{
			if(parameters.size() < 1 ||
				parameters[0].type != generic_type::e_vector
			)
				return {};
		}
		typename generic_type::vector_view q0 {parameters[0]};
		if constexpr(ENABLE_SAFETY_CHECKS)
		{
			if(q0.size() < 4)
				return {};
		}

		uquat::inverse(*reinterpret_cast<Quat*>(&q0[0]));
		return ExprScalar{};
	}

	static ExprScalar q_length(exprtk::igeneric_function<ExprScalar>::parameter_list_t parameters)
	{
		using generic_type = exprtk::igeneric_function<ExprScalar>::generic_type;
		if constexpr(ENABLE_SAFETY_CHECKS)
		{
			if(parameters.size() < 1 ||
				parameters[0].type != generic_type::e_vector
			)
				return {};
		}
		typename generic_type::vector_view vv {parameters[0]};
		if constexpr(ENABLE_SAFETY_CHECKS)
		{
			if(vv.size() < 4)
				return {};
		}
		auto &q = *reinterpret_cast<Quat*>(&vv[0]);
		return uquat::length(q);
	}

	exprtk::symbol_table<ExprScalar> &get_quaternion_symbol_table()
	{
		static exprtk::symbol_table<panima::expression::ExprScalar> symTable;
		static auto initialized = false;
		if(initialized)
			return symTable;
		initialized = true;

		static_assert(std::is_same_v<ExprScalar,Quat::value_type> && std::is_same_v<ExprScalar,::Vector3::value_type>);
		static ExprFuncGeneric<ExprScalar,q_from_axis_angle> f_q_from_axis_angle {};
		static ExprFuncGeneric<ExprScalar,q_forward> f_q_forward {};
		static ExprFuncGeneric<ExprScalar,q_right> f_q_right {};
		static ExprFuncGeneric<ExprScalar,q_up> f_q_up {};
		static ExprFuncGeneric<ExprScalar,q_slerp> f_q_slerp {};
		static ExprFuncGeneric<ExprScalar,q_lerp> f_q_lerp {};
		static ExprFuncGeneric<ExprScalar,q_dot> f_q_dot {};
		static ExprFuncGeneric<ExprScalar,q_mul> f_q_mul {};
		static ExprFuncGeneric<ExprScalar,q_inverse> f_q_inverse {};
		static ExprFuncGeneric<ExprScalar,q_length> f_q_length {};
		
		symTable.add_function("q_from_axis_angle",f_q_from_axis_angle);
		symTable.add_function("q_forward",f_q_forward);
		symTable.add_function("q_right",f_q_right);
		symTable.add_function("q_up",f_q_up);
		symTable.add_function("q_slerp",f_q_slerp);
		symTable.add_function("q_lerp",f_q_lerp);
		symTable.add_function("q_dot",f_q_dot);
		symTable.add_function("q_mul",f_q_mul);
		symTable.add_function("q_inverse",f_q_inverse);
		symTable.add_function("q_length",f_q_length);
		return symTable;
	}
};
