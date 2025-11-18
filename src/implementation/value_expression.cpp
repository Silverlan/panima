// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <exprtk.hpp>

module panima;

import :expression;

static constexpr auto VALUE_EPSILON = 0.001f;

panima::expression::ExprScalar panima::expression::ExprFuncPerlinNoise::operator()(const ExprScalar &v1, const ExprScalar &v2, const ExprScalar &v3) { return m_noise.GetNoise(v1, v2, v3); }
template<typename T>
panima::expression::ExprScalar panima::expression::ExprFuncValueAtArithmetic<T>::operator()(const ExprScalar &v)
{
	assert(m_valueExpression);
	uint32_t pivotIndex = m_valueExpression->expr.timeIndex;
	return m_valueExpression->channel.GetInterpolatedValue<T>(v, pivotIndex);
}

template<typename T>
panima::expression::ExprScalar panima::expression::ExprFuncValueAtVector<T>::operator()(exprtk::igeneric_function<ExprScalar>::parameter_list_t parameters)
{
	using generic_type = exprtk::igeneric_function<ExprScalar>::generic_type;
	assert(parameters.size() == 2);
	typename generic_type::scalar_view t {parameters[0]};
	typename generic_type::vector_view out {parameters[1]};
	assert(m_valueExpression);
	uint32_t pivotIndex = m_valueExpression->expr.timeIndex;
	auto n = udm::get_numeric_component_count(udm::type_to_enum<T>());
	assert(out.size() == n);
	if constexpr(std::is_same_v<udm::underlying_numeric_type<T>, ExprScalar>)
		*reinterpret_cast<T *>(&out[0]) = m_valueExpression->channel.GetInterpolatedValue<T>(t(), pivotIndex);
	else {
		auto value = m_valueExpression->channel.GetInterpolatedValue<T>(t(), pivotIndex);
		auto *ptrValue = reinterpret_cast<udm::underlying_numeric_type<T> *>(&value);
		auto *ptrOut = &out[0];
		for(auto i = decltype(n) {0u}; i < n; ++i)
			ptrOut[i] = ptrValue[i];
	}
	return {};
}

static panima::expression::ExprScalar ramp(const panima::expression::ExprScalar &x, const panima::expression::ExprScalar &a, const panima::expression::ExprScalar &b)
{
	if(a == b)
		return a;
	return (x - a) / (b - a);
}
static panima::expression::ExprScalar expr_lerp(const panima::expression::ExprScalar &x, const panima::expression::ExprScalar &a, const panima::expression::ExprScalar &b) { return a + (b - a) * x; }
static panima::expression::ExprScalar rescale(const panima::expression::ExprScalar &X, const panima::expression::ExprScalar &Xa, const panima::expression::ExprScalar &Xb, const panima::expression::ExprScalar &Ya, const panima::expression::ExprScalar &Yb)
{
	return expr_lerp(ramp(X, Xa, Xb), Ya, Yb);
}

namespace panima::expression {
	static ExprScalar sqr(const ExprScalar &v) { return v * v; }
	static ExprScalar cramp(const ExprScalar &v1, const ExprScalar &v2, const ExprScalar &v3) { return umath::clamp(ramp(v1, v2, v3), ExprScalar {0}, ExprScalar {1}); }
	static ExprScalar clerp(const ExprScalar &v1, const ExprScalar &v2, const ExprScalar &v3) { return umath::clamp(expr_lerp(v1, v2, v3), v2, v3); }
	static ExprScalar elerp(const ExprScalar &v1, const ExprScalar &v2, const ExprScalar &v3) { return ramp(3 * v1 * v1 - 2 * v1 * v1 * v1, v2, v3); }
	static ExprScalar crescale(const ExprScalar &v1, const ExprScalar &v2, const ExprScalar &v3, const ExprScalar &v4, const ExprScalar &v5) { return umath::clamp(rescale(v1, v2, v3, v4, v5), v4, v5); }

	static ExprScalar hsv_to_rgb(exprtk::igeneric_function<ExprScalar>::parameter_list_t parameters)
	{
		using generic_type = exprtk::igeneric_function<ExprScalar>::generic_type;
		typename generic_type::vector_view hsv {parameters[0]};
		typename generic_type::vector_view out {parameters[1]};

		auto rgb = util::hsv_to_rgb(hsv[0], hsv[1], hsv[2]);
		*reinterpret_cast<::Vector3 *>(&out[0]) = rgb;
		return ExprScalar {};
	}
	static ExprScalar rgb_to_hsv(exprtk::igeneric_function<ExprScalar>::parameter_list_t parameters)
	{
		using generic_type = exprtk::igeneric_function<ExprScalar>::generic_type;
		typename generic_type::vector_view rgb {parameters[0]};
		typename generic_type::vector_view out {parameters[1]};

		double h, s, v;
		util::rgb_to_hsv(::Vector3 {rgb[0], rgb[1], rgb[2]}, h, s, v);
		*reinterpret_cast<::Vector3 *>(&out[0]) = ::Vector3 {static_cast<float>(h), static_cast<float>(s), static_cast<float>(v)};
		return ExprScalar {};
	}

	extern exprtk::symbol_table<ExprScalar> &get_quaternion_symbol_table();
	static exprtk::symbol_table<ExprScalar> &get_base_symbol_table()
	{
		static exprtk::symbol_table<panima::expression::ExprScalar> symTable;
		static auto initialized = false;
		if(initialized)
			return symTable;
		initialized = true;

		static ExprFuncGeneric1Param<ExprScalar, sqr> f_sqr {};
		static ExprFuncGeneric3Param<ExprScalar, ramp> f_ramp {};
		static ExprFuncGeneric3Param<ExprScalar, cramp> f_cramp {};
		static ExprFuncGeneric3Param<ExprScalar, expr_lerp> f_lerp {};
		static ExprFuncGeneric3Param<ExprScalar, clerp> f_clerp {};
		static ExprFuncGeneric3Param<ExprScalar, elerp> f_elerp {};
		static ExprFuncGeneric5Param<ExprScalar, rescale> f_rescale {};
		static ExprFuncGeneric5Param<ExprScalar, crescale> f_crescale {};

		symTable.add_function("sqr", f_sqr);
		symTable.add_function("ramp", f_ramp);
		symTable.add_function("cramp", f_cramp);
		symTable.add_function("lerp", f_lerp);
		symTable.add_function("clerp", f_clerp);
		symTable.add_function("elerp", f_elerp);
		symTable.add_function("rescale", f_rescale);
		symTable.add_function("crescale", f_crescale);

		static ExprFuncGeneric<ExprScalar, hsv_to_rgb> f_hsv_to_rgb {};
		static ExprFuncGeneric<ExprScalar, rgb_to_hsv> f_rgb_to_hsv {};
		symTable.add_function("hsv_to_rgb", f_hsv_to_rgb);
		symTable.add_function("rgb_to_hsv", f_rgb_to_hsv);

		static ExprFuncPrint<ExprScalar> f_print {};
		symTable.add_function("print", f_print);

		symTable.add_constants();
		return symTable;
	}
};

bool panima::expression::ValueExpression::Initialize(udm::Type type, std::string &outErr)
{
	auto success = udm::visit_ng(type, [this](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(!is_supported_expression_type_v<T>)
			return false;
		else {
			using TExpr = TExprType<T>;
			if constexpr(std::is_same_v<TExpr, Single>) {
				expr.value = Single {0};
				auto &v = std::get<Single>(expr.value);
				expr.symbolTable.add_variable("value", v[0]);

				auto valueAt = std::make_unique<ExprFuncValueAtArithmetic<T>>();
				expr.symbolTable.add_function("value_at", *valueAt);
				expr.f_valueAt = std::move(valueAt);
			}
			else {
				expr.value = TExpr {};
				auto &v = std::get<TExpr>(expr.value);
				expr.symbolTable.add_vector("value", v.data(), v.size());

				auto valueAt = std::make_unique<ExprFuncValueAtVector<T>>();
				expr.symbolTable.add_function("value_at", *valueAt);
				expr.f_valueAt = std::move(valueAt);
			}
			return true;
		}
	});
	if(!success) {
		outErr = "Unsupported type '" + std::string {magic_enum::enum_name(type)} + "'!";
		return false;
	}
	m_type = type;

	expr.symbolTable.add_variable("time", expr.time);
	expr.symbolTable.add_variable("timeIndex", expr.timeIndex);

	expr.symbolTable.add_variable("startOffset", expr.startOffset);
	expr.symbolTable.add_variable("timeScale", expr.timeScale);
	expr.symbolTable.add_variable("duration", expr.duration);

	assert(expr.f_valueAt != nullptr);
	expr.f_valueAt->SetValueExpression(*this);

	expr.symbolTable.add_function("noise", expr.f_perlinNoise);

	expr.expression.register_symbol_table(get_base_symbol_table());
	expr.expression.register_symbol_table(get_quaternion_symbol_table());
	expr.expression.register_symbol_table(expr.symbolTable);
	if(expr.parser.compile(expression, expr.expression) == false) {
		outErr = expr.parser.error();
		return false;
	}
	return true;
}

panima::expression::ValueExpression::ValueExpression(const ValueExpression &other) : ValueExpression {other.channel}
{
	expression = other.expression;
	m_type = other.m_type;
	std::string err;
	Initialize(other.m_type, err);
}

panima::expression::ValueExpression::~ValueExpression() { expr.f_valueAt = nullptr; }
