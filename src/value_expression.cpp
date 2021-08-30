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

void panima::MupFunValueAt::Eval(mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc)
{
	uint32_t pivotIndex = m_valueExpression.mup.timeIndex.GetInteger();
	*ret = m_valueExpression.channel.GetInterpolatedValue<float>(a_pArg[0]->GetFloat(),pivotIndex);
}

void panima::MupFunPerlinNoise::Eval(mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc)
{
	*ret = m_noise.GetNoise(a_pArg[0]->GetFloat(),a_pArg[1]->GetFloat(),a_pArg[2]->GetFloat());
}

static double ramp(double x,double a,double b)
{
	if(a == b)
		return a;
	return (x -a) /(b -a);
}
static double lerp(double x,double a,double b)
{
	return a +(b -a) *x;
}
static double rescale(double X,double Xa,double Xb,double Ya,double Yb)
{
	return lerp(ramp(X,Xa,Xb),Ya,Yb);
}

bool panima::ValueExpression::Initialize(std::string &outErr)
{
	mup.parser = {};
	auto &p = mup.parser;
	p.ClearFun();
	
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			*ret = umath::deg_to_rad(a_pArg[0]->GetFloat());
		}>{"rad",1}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			*ret = umath::rad_to_deg(a_pArg[0]->GetFloat());
		}>{"deg",1}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			*ret = umath::abs(a_pArg[0]->GetFloat());
		}>{"abs",1}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			*ret = umath::floor(a_pArg[0]->GetFloat());
		}>{"floor",1}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			*ret = umath::ceil(a_pArg[0]->GetFloat());
		}>{"ceil",1}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			*ret = umath::round(a_pArg[0]->GetFloat());
		}>{"round",1}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			*ret = umath::sign(static_cast<float>(a_pArg[0]->GetFloat()));
		}>{"sign",1}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			*ret = umath::pow2(static_cast<float>(a_pArg[0]->GetFloat()));
		}>{"sqr",1}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			*ret = umath::sqrt(static_cast<float>(a_pArg[0]->GetFloat()));
		}>{"sqrt",1}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			*ret = umath::sin(umath::deg_to_rad(a_pArg[0]->GetFloat()));
		}>{"sin",1}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			*ret = umath::asin(umath::deg_to_rad(a_pArg[0]->GetFloat()));
		}>{"asin",1}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			*ret = umath::cos(umath::deg_to_rad(a_pArg[0]->GetFloat()));
		}>{"cos",1}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			*ret = umath::acos(umath::deg_to_rad(a_pArg[0]->GetFloat()));
		}>{"acos",1}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			*ret = umath::tan(umath::deg_to_rad(a_pArg[0]->GetFloat()));
		}>{"tan",1}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			*ret = exp(a_pArg[0]->GetFloat());
		}>{"exp",1}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			*ret = log(a_pArg[0]->GetFloat());
		}>{"log",1}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			*ret = std::min(a_pArg[0]->GetFloat(),a_pArg[1]->GetFloat());
		}>{"min",2}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			*ret = std::max(a_pArg[0]->GetFloat(),a_pArg[1]->GetFloat());
		}>{"max",2}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			*ret = atan2f(a_pArg[0]->GetFloat(),a_pArg[1]->GetFloat());
		}>{"atan2",2}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			*ret = pow(a_pArg[0]->GetFloat(),a_pArg[1]->GetFloat());
		}>{"pow",2}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			auto x = a_pArg[0]->GetFloat();
			auto a = a_pArg[1]->GetFloat();
			auto b = a_pArg[2]->GetFloat();
			*ret = (x >= a && x <= b) ? 1 : 0;
		}>{"inrange",3}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			auto x = a_pArg[0]->GetFloat();
			auto a = a_pArg[1]->GetFloat();
			auto b = a_pArg[2]->GetFloat();
			*ret = umath::clamp(x,a,b);
		}>{"clamp",3}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			auto x = a_pArg[0]->GetFloat();
			auto a = a_pArg[1]->GetFloat();
			auto b = a_pArg[2]->GetFloat();
			*ret = ramp(x,a,b);
		}>{"ramp",3}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			auto x = a_pArg[0]->GetFloat();
			auto a = a_pArg[1]->GetFloat();
			auto b = a_pArg[2]->GetFloat();
			*ret = umath::clamp(ramp(x,a,b),0.0,1.0);
		}>{"cramp",3}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			auto x = a_pArg[0]->GetFloat();
			auto a = a_pArg[1]->GetFloat();
			auto b = a_pArg[2]->GetFloat();
			*ret = lerp(x,a,b);
		}>{"lerp",3}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			auto x = a_pArg[0]->GetFloat();
			auto a = a_pArg[1]->GetFloat();
			auto b = a_pArg[2]->GetFloat();
			*ret = umath::clamp(lerp(x,a,b),a,b);
		}>{"clerp",3}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			auto x = a_pArg[0]->GetFloat();
			auto a = a_pArg[1]->GetFloat();
			auto b = a_pArg[2]->GetFloat();
			*ret = ramp(3 *x *x -2 *x *x *x,a,b);
		}>{"elerp",3}
	);
	p.DefineFun(new MupFunPerlinNoise{});
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			auto X = a_pArg[0]->GetFloat();
			auto Xa = a_pArg[1]->GetFloat();
			auto Xb = a_pArg[2]->GetFloat();
			auto Ya = a_pArg[3]->GetFloat();
			auto Yb = a_pArg[4]->GetFloat();
			*ret = rescale(X,Xa,Xb,Ya,Yb);
		}>{"rescale",5}
	);
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			auto X = a_pArg[0]->GetFloat();
			auto Xa = a_pArg[1]->GetFloat();
			auto Xb = a_pArg[2]->GetFloat();
			auto Ya = a_pArg[3]->GetFloat();
			auto Yb = a_pArg[4]->GetFloat();
			*ret = umath::clamp(rescale(X,Xa,Xb,Ya,Yb),Ya,Yb);
		}>{"crescale",5}
	);
	p.DefineFun(new MupFunValueAt{*this});

	// For debugging purposes
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			auto &val = *a_pArg[0];
			if(val.IsInteger())
			{
				std::cout<<val.GetInteger()<<std::endl;
				*ret = val.GetInteger();
			}
			else if(val.IsScalar())
			{
				std::cout<<val.GetFloat()<<std::endl;
				*ret = val.GetFloat();
			}
			else
			{
				if(val.IsString())
					std::cout<<val.GetString()<<std::endl;
				*ret = 1;
			}
		}>{"print",1}
	);

	p.DefineVar("value",mup.valueVar);
	p.DefineVar("time",mup.timeVar);
	p.DefineVar("timeIndex",mup.timeIndexVar);
	p.SetExpr(expression);
	try
	{
		// Eval once, which will initialize the RPN for faster execution in the future
		p.Eval();
	}
	catch(const mup::ParserError &err)
	{
		outErr = err.GetMsg();
		return false;
	}
	return true;
}

void panima::ValueExpression::Apply(double time,uint32_t timeIndex,double &inOutValue)
{
	mup.value = inOutValue;
	mup.time = time;
	mup.timeIndex = static_cast<int>(timeIndex);
	try
	{
		auto &res = mup.parser.Eval();
		inOutValue = res.GetFloat();
	}
	catch(const mup::ParserError &err)
	{
		// TODO
	}
}
