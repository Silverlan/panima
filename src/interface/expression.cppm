// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <exprtk.hpp>

export module panima:expression;

import :channel;
export import pragma.udm;

export namespace panima {
	struct TimeFrame;
	namespace expression {
		struct ValueExpression;

		using ExprScalar = float;
		using Single = std::array<ExprScalar, 1>;
		using Vector2 = std::array<ExprScalar, 2>;
		using Vector3 = std::array<ExprScalar, 3>;
		using Vector4 = std::array<ExprScalar, 4>;
		using Mat3x4 = std::array<ExprScalar, 12>;
		using Mat4 = std::array<ExprScalar, 16>;
		template<typename T>
		using TExprType = std::conditional_t<std::is_same_v<T, udm::Vector2> || std::is_same_v<T, udm::Vector2i>, Vector2,
		  std::conditional_t<std::is_same_v<T, udm::Vector3> || std::is_same_v<T, udm::Vector3i> || std::is_same_v<T, udm::EulerAngles> || std::is_same_v<T, udm::HdrColor>, Vector3,
		    std::conditional_t<std::is_same_v<T, udm::Vector4> || std::is_same_v<T, udm::Vector4i> || std::is_same_v<T, udm::Quaternion> || std::is_same_v<T, udm::Srgba>, Vector4,
		      std::conditional_t<std::is_same_v<T, udm::Mat3x4>, Mat3x4, std::conditional_t<std::is_same_v<T, udm::Mat4>, Mat4, Single>>>>>;

		constexpr bool is_supported_expression_type(udm::Type type)
		{
			return udm::visit(type, [](auto tag) { return is_supported_expression_type_v<typename decltype(tag)::type>; });
		}

		template<typename T, T (*TEval)(const T &)>
		struct ExprFuncGeneric1Param : public exprtk::ifunction<T> {
			using exprtk::ifunction<T>::operator();

			ExprFuncGeneric1Param() : exprtk::ifunction<T>(1) { exprtk::disable_has_side_effects(*this); }

			inline T operator()(const T &v) { return TEval(v); }
		};
		template<typename T, T (*TEval)(const T &, const T &)>
		struct ExprFuncGeneric2Param : public exprtk::ifunction<T> {
			using exprtk::ifunction<T>::operator();

			ExprFuncGeneric2Param() : exprtk::ifunction<T>(2) { exprtk::disable_has_side_effects(*this); }

			inline T operator()(const T &v0, const T &v1) { return TEval(v0, v1); }
		};
		template<typename T, T (*TEval)(const T &, const T &, const T &)>
		struct ExprFuncGeneric3Param : public exprtk::ifunction<T> {
			using exprtk::ifunction<T>::operator();

			ExprFuncGeneric3Param() : exprtk::ifunction<T>(3) { exprtk::disable_has_side_effects(*this); }

			inline T operator()(const T &v0, const T &v1, const T &v2) { return TEval(v0, v1, v2); }
		};
		template<typename T, T (*TEval)(const T &, const T &, const T &, const T &)>
		struct ExprFuncGeneric4Param : public exprtk::ifunction<T> {
			using exprtk::ifunction<T>::operator();

			ExprFuncGeneric4Param() : exprtk::ifunction<T>(4) { exprtk::disable_has_side_effects(*this); }

			inline T operator()(const T &v0, const T &v1, const T &v2, const T &v3) { return TEval(v0, v1, v2, v3); }
		};
		template<typename T, T (*TEval)(const T &, const T &, const T &, const T &, const T &)>
		struct ExprFuncGeneric5Param : public exprtk::ifunction<T> {
			using exprtk::ifunction<T>::operator();

			ExprFuncGeneric5Param() : exprtk::ifunction<T>(5) { exprtk::disable_has_side_effects(*this); }

			inline T operator()(const T &v0, const T &v1, const T &v2, const T &v3, const T &v4) { return TEval(v0, v1, v2, v3, v4); }
		};
		struct ExprFuncPerlinNoise : public exprtk::ifunction<ExprScalar> {
			using ifunction<ExprScalar>::operator();

			ExprFuncPerlinNoise() : ifunction<ExprScalar>(3) {}

			ExprScalar operator()(const ExprScalar &v1, const ExprScalar &v2, const ExprScalar &v3) override;
		  private:
			pragma::math::PerlinNoise m_noise {static_cast<uint32_t>(pragma::math::random(std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max()))};
		};
		struct BaseExprFuncValueAt {
			BaseExprFuncValueAt() = default;
			~BaseExprFuncValueAt() {}

			void SetValueExpression(ValueExpression &expr) { m_valueExpression = &expr; }
		  protected:
			ValueExpression *m_valueExpression = nullptr;
		};
		template<typename T>
		struct ExprFuncValueAtArithmetic : public BaseExprFuncValueAt, public exprtk::ifunction<ExprScalar> {
			using ifunction<ExprScalar>::operator();

			ExprFuncValueAtArithmetic() : ifunction<ExprScalar>(1) {}
			~ExprFuncValueAtArithmetic() {}

			ExprScalar operator()(const ExprScalar &v) override;
		};
		template<typename T>
		struct ExprFuncValueAtVector : public BaseExprFuncValueAt, public exprtk::igeneric_function<ExprScalar> {
			using igeneric_function<ExprScalar>::operator();

			ExprFuncValueAtVector() : igeneric_function<ExprScalar> {} {}
			~ExprFuncValueAtVector() {}

			ExprScalar operator()(parameter_list_t parameters) override;
		};

		template<typename T, T (*TEval)(typename exprtk::igeneric_function<T>::parameter_list_t)>
		struct ExprFuncGeneric : public exprtk::igeneric_function<T> {
			using exprtk::igeneric_function<T>::operator();

			ExprFuncGeneric() : exprtk::igeneric_function<T> {} {}

			inline T operator()(typename exprtk::igeneric_function<T>::parameter_list_t parameters) { return TEval(parameters); }
		};

		template<typename T>
		struct ExprFuncPrint : public exprtk::igeneric_function<T> {
			using exprtk::igeneric_function<T>::operator();

			ExprFuncPrint() : exprtk::igeneric_function<T> {} {}

			inline T operator()(typename exprtk::igeneric_function<T>::parameter_list_t parameters)
			{
				using generic_type = typename exprtk::igeneric_function<T>::generic_type;
				for(std::size_t i = 0; i < parameters.size(); ++i) {
					if(i > 0)
						std::cout << "\t";
					auto &param = parameters[i];
					switch(param.type) {
					case generic_type::e_scalar:
						{
							typename generic_type::scalar_view sv {param};
							std::cout << sv.operator()();
							break;
						}
					case generic_type::e_vector:
						{
							typename generic_type::vector_view vv {param};
							auto first = true;
							for(auto &v : vv) {
								if(first)
									first = false;
								else
									std::cout << ",";
								std::cout << v;
							}
							break;
						}
					case generic_type::e_string:
						{
							typename generic_type::string_view sv {param};
							std::string str;
							str.reserve(sv.size());
							for(auto &c : sv)
								str += c;
							std::cout << str << std::endl;
							break;
						}
					default:
						break;
					}
				}
				std::cout << std::endl;
				return T {0};
			}
		};

		struct ValueExpression {
			ValueExpression(Channel &channel) : channel {channel} {}
			ValueExpression(const ValueExpression &other);
			~ValueExpression();
			Channel &channel;
			std::string expression;
			struct {
				exprtk::symbol_table<ExprScalar> symbolTable;
				exprtk::expression<ExprScalar> expression;
				exprtk::parser<ExprScalar> parser;
				ExprFuncPerlinNoise f_perlinNoise {};
				std::shared_ptr<BaseExprFuncValueAt> f_valueAt = nullptr;

				std::variant<Single, Vector2, Vector3, Vector4, Mat3x4, Mat4> value;
				ExprScalar time {0.0};
				ExprScalar timeIndex {0};

				ExprScalar startOffset {0.0};
				ExprScalar timeScale {1.0};
				ExprScalar duration {0.0};
			} expr;

			bool Initialize(udm::Type type, std::string &outErr);
			template<typename T>
			    requires(is_supported_expression_type_v<T>)
			void Apply(double time, uint32_t timeIndex, const TimeFrame &timeFrame, T &inOutValue)
			{
				DoApply<T>(time, timeIndex, timeFrame, inOutValue);
			}
		  private:
			template<typename T>
			void DoApply(double time, uint32_t timeIndex, const TimeFrame &timeFrame, T &inOutValue);
			udm::Type m_type = udm::Type::Invalid;
		};
	};
};

template<typename T>
void panima::expression::ValueExpression::DoApply(double time, uint32_t timeIndex, const TimeFrame &timeFrame, T &inOutValue)
{
	expr.time = time;
	expr.timeIndex = static_cast<double>(timeIndex);

	expr.startOffset = timeFrame.startOffset;
	expr.timeScale = timeFrame.scale;
	expr.duration = timeFrame.duration;

	constexpr auto n = udm::get_numeric_component_count(udm::type_to_enum<T>());
	using type_t = exprtk::results_context<ExprScalar>::type_store_t;
	if constexpr(std::is_same_v<udm::underlying_numeric_type<T>, ExprScalar>) {
		static_assert(sizeof(TExprType<T>) == sizeof(T));
		std::get<TExprType<T>>(expr.value) = reinterpret_cast<TExprType<T> &>(inOutValue);
		if constexpr(n == 1)
			inOutValue = expr.expression.value();
		else {
			// Vector type
			auto floatVal = expr.expression.value();
			auto &results = expr.expression.results();
			if(results.count() == 1 && results[0].type == type_t::e_vector) {
				typename type_t::vector_view vv {results[0]};
				if(vv.size() == n)
					inOutValue = *reinterpret_cast<T *>(&vv[0]);
			}
			else {
				// Expression returned a single float value?
				// We'll assume it's intended to be interpreted as a vector
				if constexpr(udm::is_vector_type<T>)
					inOutValue = T {static_cast<typename T::value_type>(floatVal)};
			}
		}
	}
	else {
		// Base type mismatch, we'll have to copy
		auto &exprValue = std::get<TExprType<T>>(expr.value);
		static_assert(std::tuple_size_v<std::remove_reference_t<decltype(exprValue)>> == n);
		auto *ptrStart = reinterpret_cast<udm::underlying_numeric_type<T> *>(&inOutValue);
		auto *ptr = ptrStart;
		for(auto i = decltype(n) {0u}; i < n; ++i) {
			exprValue[i] = *ptr;
			++ptr;
		}

		if constexpr(n == 1)
			inOutValue = expr.expression.value();
		else {
			// Vector type
			auto floatVal = expr.expression.value();
			auto &results = expr.expression.results();
			if(results.count() == 1 && results[0].type == type_t::e_vector) {
				typename type_t::vector_view vv {results[0]};
				if(vv.size() == n) {
					ptr = ptrStart;
					for(auto i = decltype(n) {0u}; i < n; ++i) {
						*ptr = vv[i];
						++ptr;
					}
				}
			}
			else {
				// Expression returned a single float value?
				// We'll assume it's intended to be interpreted as a vector
				if constexpr(udm::is_vector_type<T>)
					inOutValue = T {static_cast<typename T::value_type>(floatVal)};
			}
		}
	}
}

export {
	//Fixed bug: value_expression.cpp defines all of these for common use, but no one used them, making instead their own versions.
	extern template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Int8 &);
	extern template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::UInt8 &);
	extern template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Int16 &);
	extern template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::UInt16 &);
	extern template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Int32 &);
	extern template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::UInt32 &);
	extern template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Int64 &);
	extern template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::UInt64 &);
	extern template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Float &);
	extern template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Double &);
	extern template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Boolean &);
	extern template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Vector2 &);
	extern template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Vector3 &);
	extern template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Vector4 &);
	extern template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Quaternion &);
	extern template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::EulerAngles &);
	extern template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Srgba &);
	extern template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::HdrColor &);
	extern template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Mat4 &);
	extern template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Mat3x4 &);
	extern template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Vector2i &);
	extern template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Vector3i &);
	extern template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Vector4i &);

	template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Int8 &);
	template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::UInt8 &);
	template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Int16 &);
	template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::UInt16 &);
	template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Int32 &);
	template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::UInt32 &);
	template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Int64 &);
	template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::UInt64 &);
	template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Float &);
	template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Double &);
	template void panima::expression::ValueExpression::DoApply<bool>(double, uint32_t, const TimeFrame &, bool &);
	template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Vector2 &);
	template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Vector3 &);
	template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Vector4 &);
	template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Quaternion &);
	template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::EulerAngles &);
	template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Srgba &);
	template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::HdrColor &);
	template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Mat4 &);
	template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Mat3x4 &);
	template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Vector2i &);
	template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Vector3i &);
	template void panima::expression::ValueExpression::DoApply(double, uint32_t, const TimeFrame &, udm::Vector4i &);
};
