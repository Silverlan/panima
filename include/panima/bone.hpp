/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PANIMA_BONE_HPP__
#define __PANIMA_BONE_HPP__

#include "panima/types.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <optional>
#include <sharedutils/util_path.hpp>

namespace panima
{
	struct Bone
		: public std::enable_shared_from_this<Bone>
	{
		Bone();
		Bone(const Bone &other); // Parent has to be updated by caller!
		std::string name;
		std::unordered_map<uint32_t,std::shared_ptr<Bone>> children;
		std::weak_ptr<Bone> parent;
		BoneId ID;

		bool IsAncestorOf(const Bone &other) const;
		bool IsDescendantOf(const Bone &other) const;

		bool operator==(const Bone &other) const;
		bool operator!=(const Bone &other) const {return !operator==(other);}
	};

	enum class ChannelInterpolation : uint8_t
	{
		Linear = 0,
		Step,
		CubicSpline
	};
	struct Channel
		: public std::enable_shared_from_this<Channel>
	{
		template<typename T>
			class Iterator
		{
		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = T&;
			using difference_type = std::ptrdiff_t;
			using pointer = T*;
			using reference = T&;
	
			Iterator(std::vector<uint8_t> &values,bool end);
			Iterator &operator++();
			Iterator operator++(int);
			reference operator*();
			pointer operator->();
			bool operator==(const Iterator<T> &other) const;
			bool operator!=(const Iterator<T> &other) const;
		private:
			std::vector<uint8_t>::iterator m_it;
		};

		template<typename T>
			struct IteratorWrapper
		{
			IteratorWrapper(std::vector<uint8_t> &values);
			IteratorWrapper()=default;
			Iterator<T> begin();
			Iterator<T> end();
		private:
			std::vector<uint8_t> &m_values;
		};

		Channel();
		Channel(const Channel &other)=default;
		Channel(Channel &&other)=default;
		Channel &operator=(const Channel&)=default;
		Channel &operator=(Channel&&)=default;
		ChannelInterpolation interpolation = ChannelInterpolation::Linear;
		util::Path targetPath;

		template<typename T>
			uint32_t AddValue(float t,const T &value);

		udm::Array &GetTimesArray();
		const udm::Array &GetTimesArray() const {return const_cast<Channel*>(this)->GetTimesArray();}
		udm::Array &GetValueArray();
		const udm::Array &GetValueArray() const {return const_cast<Channel*>(this)->GetValueArray();}
		udm::Type GetValueType() const;
		void SetValueType(udm::Type type);

		uint32_t GetTimeCount() const;
		uint32_t GetValueCount() const;
		std::optional<float> GetTime(uint32_t idx) const;

		bool Save(udm::LinkedPropertyWrapper &prop) const;
		bool Load(udm::LinkedPropertyWrapper &prop);
		
		udm::Property &GetTimesProperty() {return *m_times;}
		const udm::Property &GetTimesProperty() const {return const_cast<Channel*>(this)->GetTimesProperty();}

		udm::Property &GetValueProperty() {return *m_values;}
		const udm::Property &GetValueProperty() const {return const_cast<Channel*>(this)->GetValueProperty();}

		std::pair<uint32_t,uint32_t> FindInterpolationIndices(float t,float &outInterpFactor,uint32_t pivotIndex) const;
		std::pair<uint32_t,uint32_t> FindInterpolationIndices(float t,float &outInterpFactor) const;
		template<typename T>
			bool IsValueType() const;
		template<typename T>
			IteratorWrapper<T> It();
		template<typename T>
            IteratorWrapper<T> It() const {return const_cast<Channel*>(this)->It<T>();}
		template<typename T>
			T &GetValue(uint32_t idx);
		template<typename T>
			const T &GetValue(uint32_t idx) const {return const_cast<Channel*>(this)->GetValue<T>(idx);}
		template<typename T>
			auto GetInterpolationFunction() const;
		template<typename T,bool ENABLE_VALIDATION=true>
			T GetInterpolatedValue(float t,uint32_t &inOutPivotTimeIndex,T(*interpFunc)(const T&,const T&,float)=nullptr) const;
		template<typename T,bool ENABLE_VALIDATION=true>
			T GetInterpolatedValue(float t,uint32_t &inOutPivotTimeIndex,void(*interpFunc)(const void*,const void*,double,void*)) const;
		template<typename T,bool ENABLE_VALIDATION=true>
			T GetInterpolatedValue(float t,T(*interpFunc)(const T&,const T&,float)=nullptr) const;
		template<typename T,bool ENABLE_VALIDATION=true>
			T GetInterpolatedValue(float t,void(*interpFunc)(const void*,const void*,double,void*)) const;

		void Resize(uint32_t numValues);
		uint32_t GetSize() const;
	private:
		uint32_t AddValue(float t,const void *value);
		std::pair<uint32_t,uint32_t> FindInterpolationIndices(float t,float &outInterpFactor,uint32_t pivotIndex,uint32_t recursionDepth) const;
		udm::PProperty m_times = nullptr;
		udm::PProperty m_values = nullptr;
	};
};

std::ostream &operator<<(std::ostream &out,const panima::Bone &o);

#endif
