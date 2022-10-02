/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "panima/channel.hpp"
#include "panima/channel_t.hpp"
#include "value_expression.hpp"

static constexpr auto VALUE_EPSILON = 0.001f;

panima::expression::ExprScalar panima::expression::ExprFuncPerlinNoise::operator()(const ExprScalar& v1,const ExprScalar& v2,const ExprScalar& v3)
{
	return m_noise.GetNoise(v1,v2,v3);
}
template<typename T>
	panima::expression::ExprScalar panima::expression::ExprFuncValueAtArithmetic<T>::operator()(const ExprScalar& v)
{
	assert(m_valueExpression);
	uint32_t pivotIndex = m_valueExpression->expr.timeIndex;
	return m_valueExpression->channel.GetInterpolatedValue<T>(v,pivotIndex);
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
	if constexpr(std::is_same_v<udm::underlying_numeric_type<T>,ExprScalar>)
		*reinterpret_cast<T*>(&out[0]) = m_valueExpression->channel.GetInterpolatedValue<T>(t(),pivotIndex);
	else
	{
		auto value =  m_valueExpression->channel.GetInterpolatedValue<T>(t(),pivotIndex);
		auto *ptrValue = reinterpret_cast<udm::underlying_numeric_type<T>*>(&value);
		auto *ptrOut = &out[0];
		for(auto i=decltype(n){0u};i<n;++i)
			ptrOut[i] = ptrValue[i];
	}
	return {};
}

static panima::expression::ExprScalar ramp(const panima::expression::ExprScalar &x,const panima::expression::ExprScalar &a,const panima::expression::ExprScalar &b)
{
	if(a == b)
		return a;
	return (x -a) /(b -a);
}
static panima::expression::ExprScalar expr_lerp(const panima::expression::ExprScalar &x,const panima::expression::ExprScalar &a,const panima::expression::ExprScalar &b)
{
	return a +(b -a) *x;
}
static panima::expression::ExprScalar rescale(
	const panima::expression::ExprScalar &X,const panima::expression::ExprScalar &Xa,const panima::expression::ExprScalar &Xb,
	const panima::expression::ExprScalar &Ya,const panima::expression::ExprScalar &Yb
)
{
    return expr_lerp(ramp(X,Xa,Xb),Ya,Yb);
}

namespace panima::expression
{
	static ExprScalar sqr(const ExprScalar &v) {return v *v;}
	static ExprScalar cramp(const ExprScalar &v1,const ExprScalar &v2,const ExprScalar &v3) {return umath::clamp(ramp(v1,v2,v3),ExprScalar{0},ExprScalar{1});}
    static ExprScalar clerp(const ExprScalar &v1,const ExprScalar &v2,const ExprScalar &v3) {return umath::clamp(expr_lerp(v1,v2,v3),v2,v3);}
	static ExprScalar elerp(const ExprScalar &v1,const ExprScalar &v2,const ExprScalar &v3) {return ramp(3 *v1 *v1 -2 *v1 *v1 *v1,v2,v3);}
	static ExprScalar crescale(const ExprScalar &v1,const ExprScalar &v2,const ExprScalar &v3,const ExprScalar &v4,const ExprScalar &v5) {return umath::clamp(rescale(v1,v2,v3,v4,v5),v4,v5);}

	extern exprtk::symbol_table<ExprScalar> &get_quaternion_symbol_table();
	static exprtk::symbol_table<ExprScalar> &get_base_symbol_table()
	{
		static exprtk::symbol_table<panima::expression::ExprScalar> symTable;
		static auto initialized = false;
		if(initialized)
			return symTable;
		initialized = true;

		static ExprFuncGeneric1Param<ExprScalar,sqr> f_sqr {};
		static ExprFuncGeneric3Param<ExprScalar,ramp> f_ramp {};
		static ExprFuncGeneric3Param<ExprScalar,cramp> f_cramp {};
        static ExprFuncGeneric3Param<ExprScalar,expr_lerp> f_lerp {};
		static ExprFuncGeneric3Param<ExprScalar,clerp> f_clerp {};
		static ExprFuncGeneric3Param<ExprScalar,elerp> f_elerp {};
		static ExprFuncGeneric5Param<ExprScalar,rescale> f_rescale {};
		static ExprFuncGeneric5Param<ExprScalar,crescale> f_crescale {};

		symTable.add_function("sqr",f_sqr);
		symTable.add_function("ramp",f_ramp);
		symTable.add_function("cramp",f_cramp);
		symTable.add_function("lerp",f_lerp);
		symTable.add_function("clerp",f_clerp);
		symTable.add_function("elerp",f_elerp);
		symTable.add_function("rescale",f_rescale);
		symTable.add_function("crescale",f_crescale);

		static ExprFuncPrint<ExprScalar> f_print {};
		symTable.add_function("print",f_print);
		symTable.add_constants();
		return symTable;
	}
};

bool panima::expression::ValueExpression::Initialize(udm::Type type,std::string &outErr)
{
	auto success = udm::visit_ng(type,[this](auto tag) {
        using T = typename decltype(tag)::type;
		if constexpr(!is_supported_expression_type_v<T>)
			return false;
		else
		{
			using TExpr = TExprType<T>;
			if constexpr(std::is_same_v<TExpr,Single>)
			{
				expr.value = Single{0};
				auto &v = std::get<Single>(expr.value);
				expr.symbolTable.add_variable("value",v[0]);

				auto valueAt = std::make_unique<ExprFuncValueAtArithmetic<T>>();
				expr.symbolTable.add_function("value_at",*valueAt);
				expr.f_valueAt = std::move(valueAt);
			}
			else
			{
				expr.value = TExpr{};
				auto &v = std::get<TExpr>(expr.value);
				expr.symbolTable.add_vector("value",v.data(),v.size());
				
				auto valueAt = std::make_unique<ExprFuncValueAtVector<T>>();
				expr.symbolTable.add_function("value_at",*valueAt);
				expr.f_valueAt = std::move(valueAt);
			}
			return true;
		}
	});
	if(!success)
	{
		outErr = "Unsupported type '" +std::string{magic_enum::enum_name(type)} +"'!";
		return false;
	}
	m_type = type;

	expr.symbolTable.add_variable("time",expr.time);
	expr.symbolTable.add_variable("timeIndex",expr.timeIndex);
	
	expr.symbolTable.add_variable("startOffset",expr.startOffset);
	expr.symbolTable.add_variable("timeScale",expr.timeScale);
	expr.symbolTable.add_variable("duration",expr.duration);

	assert(expr.f_valueAt != nullptr);
	expr.f_valueAt->SetValueExpression(*this);
	
	expr.symbolTable.add_function("noise",expr.f_perlinNoise);
	
	expr.expression.register_symbol_table(get_base_symbol_table());
	expr.expression.register_symbol_table(get_quaternion_symbol_table());
	expr.expression.register_symbol_table(expr.symbolTable);
	if(expr.parser.compile(expression,expr.expression) == false)
	{
		outErr = expr.parser.error();
		return false;
	}
	return true;
}

panima::expression::ValueExpression::~ValueExpression()
{
    expr.f_valueAt = nullptr;
}

template<typename T>
	void panima::expression::ValueExpression::DoApply(double time,uint32_t timeIndex,const TimeFrame &timeFrame,T &inOutValue)
{
	expr.time = time;
	expr.timeIndex = static_cast<double>(timeIndex);

	expr.startOffset = timeFrame.startOffset;
	expr.timeScale = timeFrame.scale;
	expr.duration = timeFrame.duration;

	constexpr auto n = udm::get_numeric_component_count(udm::type_to_enum<T>());
	using type_t = exprtk::results_context<ExprScalar>::type_store_t;
	if constexpr(std::is_same_v<udm::underlying_numeric_type<T>,ExprScalar>)
	{
		static_assert(sizeof(TExprType<T>) == sizeof(T));
		std::get<TExprType<T>>(expr.value) = reinterpret_cast<TExprType<T>&>(inOutValue);
		if constexpr(n == 1)
			inOutValue = expr.expression.value();
		else
		{
			// Vector type
			auto floatVal = expr.expression.value();
			auto &results = expr.expression.results();
			if(results.count() == 1 && results[0].type == type_t::e_vector)
			{
				typename type_t::vector_view vv {results[0]};
				if(vv.size() == n)
					inOutValue = *reinterpret_cast<T*>(&vv[0]);
			}
			else
			{
				// Expression returned a single float value?
				// We'll assume it's intended to be interpreted as a vector
				if constexpr(udm::is_vector_type<T>)
                    inOutValue = T{static_cast<typename T::value_type>(floatVal)};
			}
		}
	}
	else
	{
		// Base type mismatch, we'll have to copy
		auto &exprValue = std::get<TExprType<T>>(expr.value);
		static_assert(std::tuple_size_v<std::remove_reference_t<decltype(exprValue)>> == n);
		auto *ptrStart = reinterpret_cast<udm::underlying_numeric_type<T>*>(&inOutValue);
		auto *ptr = ptrStart;
		for(auto i=decltype(n){0u};i<n;++i)
		{
			exprValue[i] = *ptr;
			++ptr;
		}

		if constexpr(n == 1)
			inOutValue = expr.expression.value();
		else
		{
			// Vector type
			auto floatVal = expr.expression.value();
			auto &results = expr.expression.results();
			if(results.count() == 1 && results[0].type == type_t::e_vector)
			{
				typename type_t::vector_view vv {results[0]};
				if(vv.size() == n)
				{
					ptr = ptrStart;
					for(auto i=decltype(n){0u};i<n;++i)
					{
						*ptr = vv[i];
						++ptr;
					}
				}
			}
			else
			{
				// Expression returned a single float value?
				// We'll assume it's intended to be interpreted as a vector
				if constexpr(udm::is_vector_type<T>)
                    inOutValue = T{static_cast<typename T::value_type>(floatVal)};
			}
		}
	}
}

template void panima::expression::ValueExpression::DoApply(double,uint32_t,const TimeFrame&,udm::Int8&);
template void panima::expression::ValueExpression::DoApply(double,uint32_t,const TimeFrame&,udm::UInt8&);
template void panima::expression::ValueExpression::DoApply(double,uint32_t,const TimeFrame&,udm::Int16&);
template void panima::expression::ValueExpression::DoApply(double,uint32_t,const TimeFrame&,udm::UInt16&);
template void panima::expression::ValueExpression::DoApply(double,uint32_t,const TimeFrame&,udm::Int32&);
template void panima::expression::ValueExpression::DoApply(double,uint32_t,const TimeFrame&,udm::UInt32&);
template void panima::expression::ValueExpression::DoApply(double,uint32_t,const TimeFrame&,udm::Int64&);
template void panima::expression::ValueExpression::DoApply(double,uint32_t,const TimeFrame&,udm::UInt64&);
template void panima::expression::ValueExpression::DoApply(double,uint32_t,const TimeFrame&,udm::Float&);
template void panima::expression::ValueExpression::DoApply(double,uint32_t,const TimeFrame&,udm::Double&);
template void panima::expression::ValueExpression::DoApply(double,uint32_t,const TimeFrame&,udm::Boolean&);
template void panima::expression::ValueExpression::DoApply(double,uint32_t,const TimeFrame&,udm::Vector2&);
template void panima::expression::ValueExpression::DoApply(double,uint32_t,const TimeFrame&,udm::Vector3&);
template void panima::expression::ValueExpression::DoApply(double,uint32_t,const TimeFrame&,udm::Vector4&);
template void panima::expression::ValueExpression::DoApply(double,uint32_t,const TimeFrame&,udm::Quaternion&);
template void panima::expression::ValueExpression::DoApply(double,uint32_t,const TimeFrame&,udm::EulerAngles&);
template void panima::expression::ValueExpression::DoApply(double,uint32_t,const TimeFrame&,udm::Srgba&);
template void panima::expression::ValueExpression::DoApply(double,uint32_t,const TimeFrame&,udm::HdrColor&);
template void panima::expression::ValueExpression::DoApply(double,uint32_t,const TimeFrame&,udm::Mat4&);
template void panima::expression::ValueExpression::DoApply(double,uint32_t,const TimeFrame&,udm::Mat3x4&);
template void panima::expression::ValueExpression::DoApply(double,uint32_t,const TimeFrame&,udm::Vector2i&);
template void panima::expression::ValueExpression::DoApply(double,uint32_t,const TimeFrame&,udm::Vector3i&);
template void panima::expression::ValueExpression::DoApply(double,uint32_t,const TimeFrame&,udm::Vector4i&);
