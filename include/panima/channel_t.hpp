/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PANIMA_CHANNEL_T_HPP__
#define __PANIMA_CHANNEL_T_HPP__

#include "panima/channel.hpp"
#include <udm.hpp>

namespace panima
{
	constexpr auto ANIMATION_CHANNEL_TYPE_POSITION = udm::Type::Vector3;
	constexpr auto ANIMATION_CHANNEL_TYPE_ROTATION = udm::Type::Quaternion;
	constexpr auto ANIMATION_CHANNEL_TYPE_SCALE = udm::Type::Vector3;
};

template<typename T>
	bool panima::Channel::IsValueType() const {return udm::type_to_enum<T>() == GetValueType();}

template<typename T>
	uint32_t panima::Channel::AddValue(float t,const T &value)
{
	if(udm::type_to_enum<T>() != GetValueType())
		throw std::invalid_argument{"Value type mismatch!"};
	return AddValue(t,static_cast<const void*>(&value));
}

/////////////////////

template<typename T>
	panima::Channel::Iterator<T>::Iterator(std::vector<uint8_t> &values,bool end)
		: m_it{end ? values.end() : values.begin()}
{}
template<typename T>
	panima::Channel::Iterator<T> &panima::Channel::Iterator<T>::operator++()
{
	m_it += udm::size_of_base_type(udm::type_to_enum<T>());
	return *this;
}
template<typename T>
	panima::Channel::Iterator<T> panima::Channel::Iterator<T>::operator++(int)
{
	auto it = *this;
	it.operator++();
	return it;
}
template<typename T>
	typename panima::Channel::Iterator<T>::reference panima::Channel::Iterator<T>::operator*()
{
	return *operator->();
}
template<typename T>
	typename panima::Channel::Iterator<T>::pointer panima::Channel::Iterator<T>::operator->()
{
	return reinterpret_cast<T*>(&*m_it);
}
template<typename T>
	bool panima::Channel::Iterator<T>::operator==(const Iterator<T> &other) const {return m_it == other.m_it;}
template<typename T>
	bool panima::Channel::Iterator<T>::operator!=(const Iterator<T> &other) const {return !operator==(other);}

/////////////////////

template<typename T>
	panima::Channel::IteratorWrapper<T>::IteratorWrapper(std::vector<uint8_t> &values)
		: m_values{values}
{}
template<typename T>
	panima::Channel::Iterator<T> begin()
{
	// TODO: This is a stub
	throw std::runtime_error{"Not yet implemented!"};
	return {};
	//return Iterator<T>{m_values,false};
}
template<typename T>
	panima::Channel::Iterator<T> end()
{
	// TODO: This is a stub
	throw std::runtime_error{"Not yet implemented!"};
	return {};
	//return Iterator<T>{m_values,true};
}

template<typename T>
	panima::Channel::IteratorWrapper<T> panima::Channel::It()
{
	// TODO: This is a stub
	throw std::runtime_error{"Not yet implemented!"};
	//static std::vector<uint8_t> empty;
	//return IsValueType<T>() ? IteratorWrapper<T>{values} : IteratorWrapper<T>{empty};
	return {};
}

/////////////////////

template<typename T>
	T &panima::Channel::GetValue(uint32_t idx)
	{
		return GetValueArray().GetValue<T>(idx);
	}

template<typename T>
	auto panima::Channel::GetInterpolationFunction() const
{
	constexpr auto type = udm::type_to_enum<T>();
	if constexpr(std::is_same_v<T,Vector3>)
		return &uvec::lerp;
	else if constexpr(std::is_same_v<T,Quat>)
		return &uquat::slerp;
	else if constexpr(std::is_same_v<T,Vector2i> || std::is_same_v<T,Vector3i> || std::is_same_v<T,Vector4i>)
	{
		using Tf = std::conditional_t<std::is_same_v<T,Vector2i>,Vector2,std::conditional_t<std::is_same_v<T,Vector3i>,Vector3,Vector4>>;
		return [](const T &v0,const T &v1,float f) -> T {return static_cast<T>(static_cast<Tf>(v0) +f *(static_cast<Tf>(v1) -static_cast<Tf>(v0)));};
	}
	// TODO: How should we interpolate integral values?
	// else if constexpr(std::is_integral_v<T>)
	// 	return [](const T &v0,const T &v1,float f) -> T {return static_cast<T>(round(static_cast<double>(v0) +f *(static_cast<double>(v1) -static_cast<double>(v0))));};
	else
		return [](const T &v0,const T &v1,float f) -> T {return (v0 +f *(v1 -v0));};
}

template<typename T,bool ENABLE_VALIDATION>
	T panima::Channel::GetInterpolatedValue(float t,uint32_t &inOutPivotTimeIndex,T(*interpFunc)(const T&,const T&,float)) const
{
	if constexpr(ENABLE_VALIDATION)
	{
		auto &times = GetTimesArray();
		if(udm::type_to_enum<T>() != GetValueType() || times.IsEmpty())
			return {};
	}
	float factor;
	auto indices = FindInterpolationIndices(t,factor,inOutPivotTimeIndex);
	inOutPivotTimeIndex = indices.first;
	auto &v0 = GetValue<T>(indices.first);
	auto &v1 = GetValue<T>(indices.second);
	return interpFunc ? interpFunc(v0,v1,factor) : GetInterpolationFunction<T>()(v0,v1,factor);
}
	
template<typename T,bool ENABLE_VALIDATION>
	T panima::Channel::GetInterpolatedValue(float t,uint32_t &inOutPivotTimeIndex,void(*interpFunc)(const void*,const void*,double,void*)) const
{
	if(!interpFunc)
		return GetInterpolatedValue<T,ENABLE_VALIDATION>(t,inOutPivotTimeIndex);
	if constexpr(ENABLE_VALIDATION)
	{
		auto &times = GetTimesArray();
		if(udm::type_to_enum<T>() != GetValueType() || times.IsEmpty())
			return {};
	}
	float factor;
	auto indices = FindInterpolationIndices(t,factor,inOutPivotTimeIndex);
	inOutPivotTimeIndex = indices.first;
	auto &v0 = GetValue<T>(indices.first);
	auto &v1 = GetValue<T>(indices.second);
	T v;
	interpFunc(&v0,&v1,factor,&v);
	return v;
}

template<typename T,bool ENABLE_VALIDATION>
	T panima::Channel::GetInterpolatedValue(float t,T(*interpFunc)(const T&,const T&,float)) const
{
	if constexpr(ENABLE_VALIDATION)
	{
		auto &times = GetTimesArray();
		if(udm::type_to_enum<T>() != GetValueType() || times.IsEmpty())
			return {};
	}
	float factor;
	auto indices = FindInterpolationIndices(t,factor);
	auto &v0 = GetValue<T>(indices.first);
	auto &v1 = GetValue<T>(indices.second);
	return interpFunc ? interpFunc(v0,v1,factor) : GetInterpolationFunction<T>()(v0,v1,factor);
}

template<typename T,bool ENABLE_VALIDATION>
	T panima::Channel::GetInterpolatedValue(float t,void(*interpFunc)(const void*,const void*,double,void*)) const
{
	if(!interpFunc)
		return GetInterpolatedValue<T,ENABLE_VALIDATION>(t);
	if constexpr(ENABLE_VALIDATION)
	{
		auto &times = GetTimesArray();
		if(udm::type_to_enum<T>() != GetValueType() || times.IsEmpty())
			return {};
	}
	float factor;
	auto indices = FindInterpolationIndices(t,factor);
	auto &v0 = GetValue<T>(indices.first);
	auto &v1 = GetValue<T>(indices.second);
	T v;
	interpFunc(&v0,&v1,factor,&v);
	return v;
}

#endif
