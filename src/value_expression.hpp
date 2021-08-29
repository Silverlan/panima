/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PANIMA_VALUE_EXPRESSION_HPP__
#define __PANIMA_VALUE_EXPRESSION_HPP__

#include <mpParser.h>
#include <mathutil/uvec.h>

namespace panima
{
	struct ValueExpression
	{
		std::string expression;
		struct {
			mup::ParserX parser;

			mup::Value value {0.f};
			mup::Variable valueVar {&value};

			mup::Value time {0.f};
			mup::Variable timeVar {&time};
		} mup;

		bool Initialize(std::string &outErr);
		void Apply(double time,double &inOutValue);
		void Apply(double time,Vector3 &inOutValue);
		void Apply(double time,Quat &inOutValue);
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
};

#endif
