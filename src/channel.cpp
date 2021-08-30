/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "panima/channel.hpp"
#include "value_expression.hpp"
#include "mathutil/perlin_noise.hpp"
#include <udm.hpp>

static constexpr auto VALUE_EPSILON = 0.001f;

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
static double noise(double a,double b,double c)
{
	umath::PerlinNoise p;
	return p.GetNoise(a,b,c);
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
	p.DefineFun(
		new panima::MupFunGeneric<[](mup::ptr_val_type &ret, const mup::ptr_val_type * a_pArg, int a_iArgc) {
			auto a = a_pArg[0]->GetFloat();
			auto b = a_pArg[1]->GetFloat();
			auto c = a_pArg[2]->GetFloat();
			*ret = noise(a,b,c);
		}>{"noise",3}
	);
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

	p.DefineVar("value",mup.valueVar);
	p.DefineVar("time",mup.timeVar);
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

void panima::ValueExpression::Apply(double time,double &inOutValue)
{
	mup.value = inOutValue;
	mup.time = time;
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

panima::Channel::Channel()
	: m_times{::udm::Property::Create(udm::Type::ArrayLz4)},m_values{::udm::Property::Create(udm::Type::ArrayLz4)}
{
	GetTimesArray().SetValueType(udm::Type::Float);
}
panima::Channel::~Channel() {}
bool panima::Channel::Save(udm::LinkedPropertyWrapper &prop) const
{
	prop["interpolation"] = interpolation;
	prop["targetPath"] = targetPath.GetString();

	prop["times"] = m_times;
	prop["values"] = m_values;
	return true;
}
bool panima::Channel::Load(udm::LinkedPropertyWrapper &prop)
{
	prop["interpolation"](interpolation);
	std::string targetPath;
	prop["targetPath"](targetPath);
	this->targetPath = std::move(targetPath);

	prop["times"](m_times);
	prop["values"](m_values);
	return true;
}
uint32_t panima::Channel::GetSize() const {return GetTimesArray().GetSize();}
void panima::Channel::Resize(uint32_t numValues)
{
	m_times->GetValue<udm::Array>().Resize(numValues);
	m_values->GetValue<udm::Array>().Resize(numValues);
}
bool panima::Channel::ApplyValueExpression(double time,double &inOutVal) const
{
	if(!m_valueExpression)
		return false;
	m_valueExpression->Apply(time,inOutVal);
	return true;
}
bool panima::Channel::SetValueExpression(std::string expression,std::string &outErr)
{
	m_valueExpression = nullptr;

	auto expr = std::make_unique<ValueExpression>();
	expr->expression = std::move(expression);
	if(!expr->Initialize(outErr))
		return false;
	m_valueExpression = std::move(expr);
	return true;
}
const std::string *panima::Channel::GetValueExpression() const
{
	if(m_valueExpression)
		return &m_valueExpression->expression;
	return nullptr;
}
uint32_t panima::Channel::AddValue(float t,const void *value)
{
	float interpFactor;
	auto indices = FindInterpolationIndices(t,interpFactor);
	if(indices.first == std::numeric_limits<decltype(indices.first)>::max())
	{
		auto size = GetSize() +1;
		Resize(size);
		GetTimesArray()[size -1] = t;
		GetValueArray()[size -1] = value;
		return 0;
	}
	if(umath::abs(t -*GetTime(indices.first)) < VALUE_EPSILON)
	{
		// Replace value at first index with new value
		GetTimesArray()[indices.first] = t;
		GetValueArray()[indices.first] = value;
		return 0;
	}
	if(umath::abs(t -*GetTime(indices.second)) < VALUE_EPSILON)
	{
		// Replace value at second index with new value
		GetTimesArray()[indices.second] = t;
		GetValueArray()[indices.second] = value;
		return 0;
	}
	// Insert new value between the two indices
	
	//	udm::PProperty m_times = nullptr;
	//	udm::PProperty m_values = nullptr;


	// This is a stub
	throw std::runtime_error{"Not yet implemented"};
	return 0;
	//float interpFactor;
	//auto indices = FindInterpolationIndices(t,interpFactor);
	//values.insert(values.begin() +indices.first);
	//values.
}
udm::Array &panima::Channel::GetTimesArray() {return m_times->GetValue<udm::Array>();}
udm::Array &panima::Channel::GetValueArray() {return m_values->GetValue<udm::Array>();}
udm::Type panima::Channel::GetValueType() const {return GetValueArray().GetValueType();}
void panima::Channel::SetValueType(udm::Type type) {GetValueArray().SetValueType(type);}
std::pair<uint32_t,uint32_t> panima::Channel::FindInterpolationIndices(float t,float &interpFactor,uint32_t pivotIndex,uint32_t recursionDepth) const
{
	constexpr uint32_t MAX_RECURSION_DEPTH = 2;
	auto &times = GetTimesArray();
	if(pivotIndex >= times.GetSize() || times.GetSize() < 2 || recursionDepth == MAX_RECURSION_DEPTH)
		return FindInterpolationIndices(t,interpFactor);
	// We'll use the pivot index as the starting point of our search and check out the times immediately surrounding it.
	// If we have a match, we can return immediately. If not, we'll slightly broaden the search until we've reached the max recursion depth or found a match.
	// If we hit the max recusion depth, we'll just do a regular binary search instead.
	auto tPivot = times.GetValue<float>(pivotIndex);
	if(t >= tPivot)
	{
		if(pivotIndex == times.GetSize() -1)
		{
			interpFactor = 0.f;
			return {static_cast<uint32_t>(GetValueArray().GetSize() -1),static_cast<uint32_t>(GetValueArray().GetSize() -1)};
		}
		auto tPivotNext = times.GetValue<float>(pivotIndex +1);
		if(t < tPivotNext)
		{
			// Most common case
			interpFactor = (t -tPivot) /(tPivotNext -tPivot);
			return {pivotIndex,pivotIndex +1};
		}
		return FindInterpolationIndices(t,interpFactor,pivotIndex +1,recursionDepth +1);
	}
	if(pivotIndex == 0)
	{
		interpFactor = 0.f;
		return {0u,0u};
	}
	return FindInterpolationIndices(t,interpFactor,pivotIndex -1,recursionDepth +1);
}
std::pair<uint32_t,uint32_t> panima::Channel::FindInterpolationIndices(float t,float &interpFactor,uint32_t pivotIndex) const
{
	return FindInterpolationIndices(t,interpFactor,pivotIndex,0u);
}

// TODO: Move this to UDM library
class ArrayFloatIterator
{
public:
	using iterator_category = std::forward_iterator_tag;
	using value_type = float;
	using difference_type = std::ptrdiff_t;
	using pointer = float*;
	using reference = float&;
	
	ArrayFloatIterator(float *data)
		: m_data{data}
	{}
	ArrayFloatIterator &operator++()
	{
		m_data++;
		return *this;
	}
	ArrayFloatIterator operator++(int)
	{
		++m_data;
		return *this;
	}
	ArrayFloatIterator operator+(uint32_t n)
	{
		m_data += n;
		return *this;
	}
	int32_t operator-(const ArrayFloatIterator &other) const {return m_data -other.m_data;}
	ArrayFloatIterator operator-(int32_t idx) const {return ArrayFloatIterator{m_data -idx};}
	reference operator*() {return *m_data;}
	const reference operator*() const {return const_cast<ArrayFloatIterator*>(this)->operator*();}
	pointer operator->() {return m_data;}
	const pointer operator->() const {return const_cast<ArrayFloatIterator*>(this)->operator->();}
	bool operator==(const ArrayFloatIterator &other) const {return m_data == other.m_data;}
	bool operator!=(const ArrayFloatIterator &other) const {return !operator==(other);}
private:
	float *m_data = nullptr;
};
static ArrayFloatIterator begin(const udm::Array &a)
{
	return ArrayFloatIterator{static_cast<float*>(&const_cast<udm::Array&>(a).GetValue<float>(0))};
}
static ArrayFloatIterator end(const udm::Array &a)
{
	return ArrayFloatIterator{static_cast<float*>(&const_cast<udm::Array&>(a).GetValue<float>(0)) +a.GetSize()};
}
std::pair<uint32_t,uint32_t> panima::Channel::FindInterpolationIndices(float t,float &interpFactor) const
{
	auto &times = GetTimesArray();
	if(times.GetSize() == 0)
	{
		interpFactor = 0.f;
		return {std::numeric_limits<uint32_t>::max(),std::numeric_limits<uint32_t>::max()};
	}
	// Binary search
	auto it = std::upper_bound(begin(times),end(times),t);
	if(it == end(times))
	{
		interpFactor = 0.f;
		return {static_cast<uint32_t>(times.GetSize() -1),static_cast<uint32_t>(times.GetSize() -1)};
	}
	if(it == begin(times))
	{
		interpFactor = 0.f;
		return {0u,0u};
	}
	auto itPrev = it -1;
	interpFactor = (t -*itPrev) /(*it -*itPrev);
	return {static_cast<uint32_t>(itPrev -begin(times)),static_cast<uint32_t>(it -begin(times))};
}

uint32_t panima::Channel::GetTimeCount() const {return GetTimesArray().GetSize();}
uint32_t panima::Channel::GetValueCount() const {return GetValueArray().GetSize();}
std::optional<float> panima::Channel::GetTime(uint32_t idx) const
{
	auto &times = GetTimesArray();
	auto n = times.GetSize();
	return (idx < n) ? times.GetValue<float>(idx) : std::optional<float>{};
}

std::ostream &operator<<(std::ostream &out,const panima::Channel &o)
{
	out<<"Channel";
	out<<"[Path:"<<o.targetPath<<"]";
	out<<"[Interp:"<<magic_enum::enum_name(o.interpolation)<<"]";
	out<<"[Values:"<<o.GetValueArray().GetSize()<<"]";
	
	std::pair<float,float> timeRange {0.f,0.f};
	auto n = o.GetTimeCount();
	if(n > 0)
		out<<"[TimeRange:"<<*o.GetTime(0)<<","<<*o.GetTime(n -1)<<"]";
	return out;
}
