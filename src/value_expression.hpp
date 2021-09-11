/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PANIMA_VALUE_EXPRESSION_HPP__
#define __PANIMA_VALUE_EXPRESSION_HPP__

#include "panima/expression.hpp"
#include <exprtk.hpp>
#include <mathutil/uvec.h>
#include <mathutil/perlin_noise.hpp>
#include <udm.hpp>

namespace panima
{
	struct Channel;
	struct TimeFrame;
	namespace expression
	{
		struct ValueExpression;

		using ExprScalar = float;
		using Single = std::array<ExprScalar,1>;
		using Vector2 = std::array<ExprScalar,2>;
		using Vector3 = std::array<ExprScalar,3>;
		using Vector4 = std::array<ExprScalar,4>;
		using Mat3x4 = std::array<ExprScalar,12>;
		using Mat4 = std::array<ExprScalar,16>;
		template<typename T>
			using TExprType = std::conditional_t<
				std::is_same_v<T,udm::Vector2> || std::is_same_v<T,udm::Vector2i>,Vector2,
				std::conditional_t<
					std::is_same_v<T,udm::Vector3> || std::is_same_v<T,udm::Vector3i> || std::is_same_v<T,udm::EulerAngles> || std::is_same_v<T,udm::HdrColor>,Vector3,
					std::conditional_t<
						std::is_same_v<T,udm::Vector4> || std::is_same_v<T,udm::Vector4i> || std::is_same_v<T,udm::Quaternion> || std::is_same_v<T,udm::Srgba>,Vector4,
						std::conditional_t<
							std::is_same_v<T,udm::Mat3x4>,Mat3x4,
							std::conditional_t<
								std::is_same_v<T,udm::Mat4>,Mat4,
								Single
				>>>>>;

		constexpr bool is_supported_expression_type(udm::Type type)
		{
			return udm::visit(type,[](auto tag) {
				return is_supported_expression_type_v<decltype(tag)::type>;
			});
		}

		template <typename T,T(*TEval)(const T&)>
			struct ExprFuncGeneric1Param : public exprtk::ifunction<T>
		{
			using exprtk::ifunction<T>::operator();

			ExprFuncGeneric1Param()
				: exprtk::ifunction<T>(1)
			{ exprtk::disable_has_side_effects(*this); }

			inline T operator()(const T& v)
			{
			return TEval(v);
			}
		};
		template <typename T,T(*TEval)(const T&,const T&)>
			struct ExprFuncGeneric2Param : public exprtk::ifunction<T>
		{
			using exprtk::ifunction<T>::operator();

			ExprFuncGeneric2Param()
				: exprtk::ifunction<T>(2)
			{ exprtk::disable_has_side_effects(*this); }

			inline T operator()(const T& v0,const T& v1)
			{
			return TEval(v0,v1);
			}
		};
		template <typename T,T(*TEval)(const T&,const T&,const T&)>
			struct ExprFuncGeneric3Param : public exprtk::ifunction<T>
		{
			using exprtk::ifunction<T>::operator();

			ExprFuncGeneric3Param()
				: exprtk::ifunction<T>(3)
			{ exprtk::disable_has_side_effects(*this); }

			inline T operator()(const T& v0,const T& v1,const T& v2)
			{
			return TEval(v0,v1,v2);
			}
		};
		template <typename T,T(*TEval)(const T&,const T&,const T&,const T&)>
			struct ExprFuncGeneric4Param : public exprtk::ifunction<T>
		{
			using exprtk::ifunction<T>::operator();

			ExprFuncGeneric4Param()
				: exprtk::ifunction<T>(4)
			{ exprtk::disable_has_side_effects(*this); }

			inline T operator()(const T& v0,const T& v1,const T& v2,const T& v3)
			{
			return TEval(v0,v1,v2,v3);
			}
		};
		template <typename T,T(*TEval)(const T&,const T&,const T&,const T&,const T&)>
			struct ExprFuncGeneric5Param : public exprtk::ifunction<T>
		{
			using exprtk::ifunction<T>::operator();

			ExprFuncGeneric5Param()
				: exprtk::ifunction<T>(5)
			{ exprtk::disable_has_side_effects(*this); }

			inline T operator()(const T& v0,const T& v1,const T& v2,const T& v3,const T& v4)
			{
			return TEval(v0,v1,v2,v3,v4);
			}
		};
		struct ExprFuncPerlinNoise : public exprtk::ifunction<ExprScalar>
		{
			using exprtk::ifunction<ExprScalar>::operator();

			ExprFuncPerlinNoise()
				: exprtk::ifunction<ExprScalar>(3)
			{}

			ExprScalar operator()(const ExprScalar& v1,const ExprScalar& v2,const ExprScalar& v3) override;
		private:
			umath::PerlinNoise m_noise {static_cast<uint32_t>(umath::random(std::numeric_limits<int>::lowest(),std::numeric_limits<int>::max()))};
		};
		struct BaseExprFuncValueAt
		{
			BaseExprFuncValueAt()=default;
			~BaseExprFuncValueAt() {}

			void SetValueExpression(ValueExpression &expr) {m_valueExpression = &expr;}
		protected:
			ValueExpression *m_valueExpression = nullptr;
		};
		template<typename T>
			struct ExprFuncValueAtArithmetic : public BaseExprFuncValueAt,public exprtk::ifunction<ExprScalar>
		{
			using exprtk::ifunction<ExprScalar>::operator();

			ExprFuncValueAtArithmetic()
				: exprtk::ifunction<ExprScalar>(1)
			{}
			~ExprFuncValueAtArithmetic()
			{}

			ExprScalar operator()(const ExprScalar& v) override;
		};
		template<typename T>
			struct ExprFuncValueAtVector : public BaseExprFuncValueAt,public exprtk::igeneric_function<ExprScalar>
		{
			using exprtk::igeneric_function<ExprScalar>::operator();

			ExprFuncValueAtVector()
				: exprtk::igeneric_function<ExprScalar>{}
			{}
			~ExprFuncValueAtVector()
			{}

			ExprScalar operator()(exprtk::igeneric_function<ExprScalar>::parameter_list_t parameters) override;
		};

		template<typename T,T(*TEval)(exprtk::igeneric_function<T>::parameter_list_t)>
		struct ExprFuncGeneric : public exprtk::igeneric_function<T>
		{
			using exprtk::igeneric_function<T>::operator();

			ExprFuncGeneric()
				: exprtk::igeneric_function<T>{}
			{}

			inline T operator()(exprtk::igeneric_function<T>::parameter_list_t parameters)
			{
				return TEval(parameters);
			}
		};

		template<typename T>
		struct ExprFuncPrint : public exprtk::igeneric_function<T>
		{
			using exprtk::igeneric_function<T>::operator();

			ExprFuncPrint()
				: exprtk::igeneric_function<T>{}
			{}

			inline T operator()(exprtk::igeneric_function<T>::parameter_list_t parameters)
			{
				using generic_type = exprtk::igeneric_function<T>::generic_type;
				for(std::size_t i = 0; i < parameters.size(); ++i)
				{
					if(i > 0)
						std::cout<<"\t";
					auto &param = parameters[i];
					switch(param.type)
					{
					case generic_type::e_scalar:
					{
						typename generic_type::scalar_view sv {param};
						std::cout<<sv.operator()();
						break;
					}
					case generic_type::e_vector:
					{
						typename generic_type::vector_view vv {param};
						auto first = true;
						for(auto &v : vv)
						{
							if(first)
								first = false;
							else
								std::cout<<",";
							std::cout<<v;
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
						std::cout<<str<<std::endl;
						break;
					}

					}
				}
				std::cout<<std::endl;
				return T{0};
			}
		};

		struct ValueExpression
		{
			ValueExpression(Channel &channel)
				: channel{channel}
			{}
			~ValueExpression();
			Channel &channel;
			std::string expression;
			struct {
				exprtk::symbol_table<ExprScalar> symbolTable;
				exprtk::expression<ExprScalar> expression;
				exprtk::parser<ExprScalar> parser;
				ExprFuncPerlinNoise f_perlinNoise {};
				std::shared_ptr<BaseExprFuncValueAt> f_valueAt = nullptr;

				std::variant<Single,Vector2,Vector3,Vector4,Mat3x4,Mat4> value;
				ExprScalar time {0.0};
				ExprScalar timeIndex{0};

				ExprScalar startOffset {0.0};
				ExprScalar timeScale {1.0};
				ExprScalar duration {0.0};
			} expr;
		
			bool Initialize(udm::Type type,std::string &outErr);
			template<typename T> requires(is_supported_expression_type_v<T>)
				void Apply(double time,uint32_t timeIndex,const TimeFrame &timeFrame,T &inOutValue)
			{
				DoApply<T>(time,timeIndex,timeFrame,inOutValue);
			}
		private:
			template<typename T>
				void DoApply(double time,uint32_t timeIndex,const TimeFrame &timeFrame,T &inOutValue);
			udm::Type m_type = udm::Type::Invalid;
		};
	};
};

#endif
