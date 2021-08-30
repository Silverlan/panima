/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PANIMA_VALUE_EXPRESSION_HPP__
#define __PANIMA_VALUE_EXPRESSION_HPP__

#include <mpParser.h>
#include <mathutil/uvec.h>
#include <mathutil/perlin_noise.hpp>

namespace panima
{
	struct Channel;
	struct ValueExpression
	{
		ValueExpression(Channel &channel)
			: channel{channel}
		{}
		Channel &channel;
		std::string expression;
		struct {
			mup::ParserX parser;

			mup::Value value {0.f};
			mup::Variable valueVar {&value};

			mup::Value time {0.f};
			mup::Variable timeVar {&time};

			mup::Value timeIndex {0};
			mup::Variable timeIndexVar {&timeIndex};
		} mup;

		bool Initialize(std::string &outErr);
		void Apply(double time,uint32_t timeIndex,double &inOutValue);
		void Apply(double time,uint32_t timeIndex,Vector3 &inOutValue);
		void Apply(double time,uint32_t timeIndex,Quat &inOutValue);
	};

	template<void(*TEval)(mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc)>
		class MupFunGeneric : public mup::ICallback
	{
	public:

		MupFunGeneric(const char *name,uint32_t numArgs)
			: ICallback(mup::cmFUNC, name,numArgs)
		{}

		MupFunGeneric(const MupFunGeneric &other)=default;

		virtual void Eval(mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) override
		{
			TEval(ret,a_pArg,a_iArgc);
		}

		virtual const mup::char_type* GetDesc() const override
		{
			return _T("");
		}

		virtual IToken* Clone() const override {return new MupFunGeneric(*this);}
	};

	class MupFunPerlinNoise : public mup::ICallback
	{
	public:

		MupFunPerlinNoise()
			: ICallback(mup::cmFUNC, "noise",3)
		{}

		MupFunPerlinNoise(const MupFunPerlinNoise &other)=default;

		virtual void Eval(mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) override;

		virtual const mup::char_type* GetDesc() const override
		{
			return _T("noise");
		}

		virtual IToken* Clone() const override {return new MupFunPerlinNoise(*this);}
	private:
		umath::PerlinNoise m_noise {static_cast<uint32_t>(umath::random(std::numeric_limits<int>::lowest(),std::numeric_limits<int>::max()))};
	};

	class MupFunValueAt : public mup::ICallback
	{
	public:

		MupFunValueAt(ValueExpression &valueExpression)
			: ICallback(mup::cmFUNC, "value_at",1),m_valueExpression{valueExpression}
		{}

		MupFunValueAt(const MupFunValueAt &other)
			: ICallback{other},m_valueExpression{other.m_valueExpression}
		{}

		virtual void Eval(mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) override;

		virtual const mup::char_type* GetDesc() const override
		{
			return _T("value_at");
		}

		virtual IToken* Clone() const override {return new MupFunValueAt(*this);}
	private:
		ValueExpression &m_valueExpression;
	};
};

#endif
