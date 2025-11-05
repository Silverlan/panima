// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <string>
#include <string_view>
#include <vector>
#include <stdexcept>
#include <memory>
#include <optional>
#include <functional>

export module panima:channel;

import :types;
export import pragma.udm;

export namespace panima {
	template<typename T>
	concept is_supported_expression_type_v = (udm::is_numeric_type(udm::type_to_enum<T>()) && !std::is_same_v<T, udm::Half>) || udm::is_vector_type<T> || udm::is_matrix_type<T> || std::is_same_v<T, Quat> || std::is_same_v<T, EulerAngles>;

	constexpr std::string_view ANIMATION_CHANNEL_PATH_POSITION = "position";
	constexpr std::string_view ANIMATION_CHANNEL_PATH_ROTATION = "rotation";
	constexpr std::string_view ANIMATION_CHANNEL_PATH_SCALE = "scale";

	// Example URI: panima:ec/color/color?components=red,blue
	struct ChannelPath {
		ChannelPath() = default;
		ChannelPath(const std::string &path);
		ChannelPath(const ChannelPath &other);

		bool operator==(const ChannelPath &other) const;
		bool operator!=(const ChannelPath &other) const { return !const_cast<ChannelPath *>(this)->operator==(other); }
		ChannelPath &operator=(const ChannelPath &other);

		std::vector<std::string> *GetComponents() { return m_components.get(); }
		const std::vector<std::string> *GetComponents() const { return const_cast<ChannelPath *>(this)->GetComponents(); }
		util::Path path;

		operator std::string() const { return ToUri(); }
		std::string ToUri(bool includeScheme = true) const;
	  private:
		std::unique_ptr<std::vector<std::string>> m_components = nullptr;
	};

	namespace expression {
		struct ValueExpression;
	};
	struct Channel : public std::enable_shared_from_this<Channel> {
		template<typename T>
		class Iterator {
		  public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = T &;
			using difference_type = std::ptrdiff_t;
			using pointer = T *;
			using reference = T &;

			Iterator(std::vector<uint8_t> &values, bool end);
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
		struct IteratorWrapper {
			IteratorWrapper(std::vector<uint8_t> &values);
			IteratorWrapper() = default;
			Iterator<T> begin();
			Iterator<T> end();
		  private:
			std::vector<uint8_t> &m_values;
		};

		enum class InsertFlags : uint8_t {
			None = 0u,
			ClearExistingDataInRange = 1u,
			DecimateInsertedData = ClearExistingDataInRange << 1u,
		};

		static constexpr auto VALUE_EPSILON = 0.001f;
		static constexpr float TIME_EPSILON = 0.0001f;
		static constexpr bool ENABLE_VALIDATION = true;
		Channel();
		Channel(const udm::PProperty &times, const udm::PProperty &values);
		//Channel(const Channel &other)=default;
		Channel(Channel &&other) = default;
		Channel(Channel &other);
		//Channel &operator=(const Channel&)=default;
		Channel &operator=(Channel &&) = default;
		Channel &operator=(Channel &other);
		~Channel();
		ChannelInterpolation interpolation = ChannelInterpolation::Linear;
		ChannelPath targetPath;

		template<typename T>
		uint32_t AddValue(float t, const T &value);
		template<typename T>
		uint32_t InsertValues(uint32_t n, const float *times, const T *values, float offset = 0.f, InsertFlags flags = InsertFlags::ClearExistingDataInRange);
		void RemoveValueAtIndex(uint32_t idx);

		udm::Array &GetTimesArray();
		const udm::Array &GetTimesArray() const { return const_cast<Channel *>(this)->GetTimesArray(); }
		udm::Array &GetValueArray();
		const udm::Array &GetValueArray() const { return const_cast<Channel *>(this)->GetValueArray(); }
		udm::Type GetValueType() const;
		void SetValueType(udm::Type type);
		bool Validate() const;

		float GetMinTime() const;
		float GetMaxTime() const;

		uint32_t GetTimeCount() const;
		uint32_t GetValueCount() const;
		std::optional<float> GetTime(uint32_t idx) const;
		bool ClearRange(float startTime, float endTime, bool addCaps = true);
		void ClearAnimationData();
		void MergeValues(const Channel &other);

		bool Save(udm::LinkedPropertyWrapper &prop) const;
		bool Load(udm::LinkedPropertyWrapper &prop);

		udm::Property &GetTimesProperty() { return *m_times; }
		const udm::Property &GetTimesProperty() const { return const_cast<Channel *>(this)->GetTimesProperty(); }

		udm::Property &GetValueProperty() { return *m_values; }
		const udm::Property &GetValueProperty() const { return const_cast<Channel *>(this)->GetValueProperty(); }

		std::pair<uint32_t, uint32_t> FindInterpolationIndices(float t, float &outInterpFactor, uint32_t pivotIndex) const;
		std::pair<uint32_t, uint32_t> FindInterpolationIndices(float t, float &outInterpFactor) const;
		std::optional<size_t> FindValueIndex(float time, float epsilon = panima::Channel::TIME_EPSILON) const;
		template<typename T>
		bool IsValueType() const;
		template<typename T>
		IteratorWrapper<T> It();
		template<typename T>
		IteratorWrapper<T> It() const
		{
			return const_cast<Channel *>(this)->It<T>();
		}
		template<typename T>
		T &GetValue(uint32_t idx);
		template<typename T>
		const T &GetValue(uint32_t idx) const
		{
			return const_cast<Channel *>(this)->GetValue<T>(idx);
		}
		template<typename T>
		auto GetInterpolationFunction() const;
		template<typename T, bool VALIDATE = ENABLE_VALIDATION>
		T GetInterpolatedValue(float t, uint32_t &inOutPivotTimeIndex, T (*interpFunc)(const T &, const T &, float) = nullptr) const;
		template<typename T, bool VALIDATE = ENABLE_VALIDATION>
		T GetInterpolatedValue(float t, uint32_t &inOutPivotTimeIndex, void (*interpFunc)(const void *, const void *, double, void *)) const;
		template<typename T, bool VALIDATE = ENABLE_VALIDATION>
		T GetInterpolatedValue(float t, T (*interpFunc)(const T &, const T &, float) = nullptr) const;
		template<typename T, bool VALIDATE = ENABLE_VALIDATION>
		T GetInterpolatedValue(float t, void (*interpFunc)(const void *, const void *, double, void *)) const;

		template<typename T>
		void GetDataInRange(float tStart, float tEnd, std::vector<float> &outTimes, std::vector<T> &outValues) const;
		void GetTimesInRange(float tStart, float tEnd, std::vector<float> &outTimes) const;

		void Decimate(float tStart, float tEnd, float error = 0.03f);
		void Decimate(float error = 0.03f);

		std::optional<uint32_t> InsertSample(float t);
		void ScaleTimeInRange(float tStart, float tEnd, float tPivot, double scale, bool retainBoundaryValues = true);
		void ShiftTimeInRange(float tStart, float tEnd, float shiftAmount, bool retainBoundaryValues = true);

		void ResolveDuplicates(float t);

		void TransformGlobal(const umath::ScaledTransform &transform);

		// Note: It is the caller's responsibility to ensure that the type matches the channel type
		template<typename T>
		    requires(is_supported_expression_type_v<T>)
		bool ApplyValueExpression(double time, uint32_t timeIndex, T &inOutVal) const
		{
			return DoApplyValueExpression<T>(time, timeIndex, inOutVal);
		}
		void ClearValueExpression();
		bool SetValueExpression(std::string expression, std::string &outErr);
		bool TestValueExpression(std::string expression, std::string &outErr);
		const std::string *GetValueExpression() const;

		void SetTimeFrame(TimeFrame timeFrame) { m_timeFrame = std::move(timeFrame); }
		TimeFrame &GetTimeFrame() { return m_timeFrame; }
		const TimeFrame &GetTimeFrame() const { return const_cast<Channel *>(this)->GetTimeFrame(); }

		void Resize(uint32_t numValues);
		uint32_t GetSize() const;
		void Update();

		size_t Optimize();

		bool operator==(const Channel &other) const { return this == &other; }
		bool operator!=(const Channel &other) const { return !operator==(other); }

		template<typename T>
		static void MergeDataArrays(const std::vector<float> &times0, const std::vector<T> values0, const std::vector<float> &times1, const std::vector<T> &values1, std::vector<float> &outTimes, std::vector<T> &outValues)
		{
			MergeDataArrays(
			  times0.size(), times0.data(), reinterpret_cast<const uint8_t *>(values0.data()), times1.size(), times1.data(), reinterpret_cast<const uint8_t *>(values1.data()), outTimes,
			  [&outValues](size_t size) -> uint8_t * {
				  outValues.resize(size, make_value<T>());
				  return reinterpret_cast<uint8_t *>(outValues.data());
			  },
			  sizeof(T));
		}
	  private:
		static void MergeDataArrays(uint32_t n0, const float *times0, const uint8_t *values0, uint32_t n1, const float *times1, const uint8_t *values1, std::vector<float> &outTimes, const std::function<uint8_t *(size_t)> &fAllocateValueData, size_t valueStride);
		std::pair<std::optional<uint32_t>, std::optional<uint32_t>> GetBoundaryIndices(float tStart, float tEnd, bool retainBoundaries = true);
		void TimeToLocalTimeFrame(float &inOutT) const;
		template<typename T>
		bool DoApplyValueExpression(double time, uint32_t timeIndex, T &inOutVal) const;
		uint32_t AddValue(float t, const void *value);
		uint32_t InsertValues(uint32_t n, const float *times, const void *values, size_t valueStride, float offset, InsertFlags flags = InsertFlags::ClearExistingDataInRange);
		std::pair<uint32_t, uint32_t> FindInterpolationIndices(float t, float &outInterpFactor, uint32_t pivotIndex, uint32_t recursionDepth) const;
		void GetDataInRange(float tStart, float tEnd, std::vector<float> *optOutTimes, const std::function<void *(size_t)> &optAllocateValueData) const;
		udm::PProperty m_times = nullptr;
		udm::PProperty m_values = nullptr;
		std::unique_ptr<expression::ValueExpression> m_valueExpression; //default constructor is sufficient
		TimeFrame m_timeFrame {};
		TimeFrame m_effectiveTimeFrame {};

		// Cached variables for faster lookup
		void UpdateLookupCache();
		udm::Array *m_timesArray = nullptr;
		udm::Array *m_valueArray = nullptr;
		float *m_timesData = nullptr;
		void *m_valueData = nullptr;
	};

	class ArrayFloatIterator {
	  public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = float;
		using difference_type = std::ptrdiff_t;
		using pointer = float *;
		using reference = float &;
		using const_reference = const value_type &;

		ArrayFloatIterator(float *data);
		ArrayFloatIterator &operator++();
		ArrayFloatIterator operator++(int);
		ArrayFloatIterator operator+(uint32_t n);
		int32_t operator-(const ArrayFloatIterator &other) const;
		ArrayFloatIterator operator-(int32_t idx) const;
		reference operator*();
		const_reference operator*() const;
		pointer operator->();
		const pointer operator->() const;
		bool operator==(const ArrayFloatIterator &other) const;
		bool operator!=(const ArrayFloatIterator &other) const;
	  private:
		float *m_data = nullptr;
	};
	ArrayFloatIterator begin(const udm::Array &a);
	ArrayFloatIterator end(const udm::Array &a);

	struct ChannelValueSubmitter {
		ChannelValueSubmitter() = default;
		ChannelValueSubmitter(const std::function<void(Channel &, uint32_t &, double)> &submitter);
		std::function<void(Channel &, uint32_t &, double)> submitter;
		bool enabled = true;
		operator bool() const;
		bool operator==(const std::nullptr_t &t) const;
		bool operator!=(const std::nullptr_t &t) const;
		void operator()(Channel &channel, uint32_t &timestampIndex, double t);
	};
	using namespace umath::scoped_enum::bitwise;
};

export
{
	namespace umath::scoped_enum::bitwise {
		template<>
		struct enable_bitwise_operators<panima::Channel::InsertFlags> : std::true_type {};
	}

	std::ostream &operator<<(std::ostream &out, const panima::Channel &o);
	std::ostream &operator<<(std::ostream &out, const panima::TimeFrame &o);
	std::ostream &operator<<(std::ostream &out, const panima::ChannelPath &o);
};

namespace panima {
	constexpr auto ANIMATION_CHANNEL_TYPE_POSITION = udm::Type::Vector3;
	constexpr auto ANIMATION_CHANNEL_TYPE_ROTATION = udm::Type::Quaternion;
	constexpr auto ANIMATION_CHANNEL_TYPE_SCALE = udm::Type::Vector3;

	template<typename T0, typename T1>
	concept is_binary_compatible_type_v = (std::is_same_v<T0, T1>) || (std::is_same_v<T0, bool> && (std::is_same_v<T1, int8_t> || std::is_same_v<T1, uint8_t>)) || (std::is_same_v<T1, bool> && (std::is_same_v<T0, int8_t> || std::is_same_v<T0, uint8_t>));

	constexpr bool is_binary_compatible_type(udm::Type t0, udm::Type t1)
	{
		static_assert(sizeof(bool) == sizeof(udm::Int8) && sizeof(bool) == sizeof(udm::UInt8));
		return (t0 == t1) || (t0 == udm::Type::Boolean && (t1 == udm::Type::Int8 || t1 == udm::Type::UInt8)) || (t1 == udm::Type::Boolean && (t0 == udm::Type::Int8 || t0 == udm::Type::UInt8));
	}
};

template<typename T>
bool panima::Channel::IsValueType() const
{
	return udm::type_to_enum<T>() == GetValueType();
}

template<typename T>
uint32_t panima::Channel::AddValue(float t, const T &value)
{
	if(!is_binary_compatible_type(udm::type_to_enum<T>(), GetValueType()))
		throw std::invalid_argument {"Value type mismatch!"};
	return AddValue(t, static_cast<const void *>(&value));
}

template<typename T>
uint32_t panima::Channel::InsertValues(uint32_t n, const float *times, const T *values, float offset, InsertFlags flags)
{
	if(!is_binary_compatible_type(udm::type_to_enum<T>(), GetValueType()))
		throw std::invalid_argument {"Value type mismatch!"};
	return InsertValues(n, times, values, sizeof(T), offset, flags);
}

template<typename T>
void panima::Channel::GetDataInRange(float tStart, float tEnd, std::vector<float> &outTimes, std::vector<T> &outValues) const
{
	if(!is_binary_compatible_type(udm::type_to_enum<T>(), GetValueType()))
		throw std::invalid_argument {"Requested data type does not match channel value type!"};
	GetDataInRange(tStart, tEnd, &outTimes, [&outValues](size_t size) -> void * {
		outValues.resize(size, make_value<T>());
		return outValues.data();
	});
}

/////////////////////

template<typename T>
panima::Channel::Iterator<T>::Iterator(std::vector<uint8_t> &values, bool end) : m_it {end ? values.end() : values.begin()}
{
}
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
	return reinterpret_cast<T *>(&*m_it);
}
template<typename T>
bool panima::Channel::Iterator<T>::operator==(const Iterator<T> &other) const
{
	return m_it == other.m_it;
}
template<typename T>
bool panima::Channel::Iterator<T>::operator!=(const Iterator<T> &other) const
{
	return !operator==(other);
}

/////////////////////

template<typename T>
panima::Channel::IteratorWrapper<T>::IteratorWrapper(std::vector<uint8_t> &values) : m_values {values}
{
}
template<typename T>
panima::Channel::Iterator<T> begin()
{
	// TODO: This is a stub
	throw std::runtime_error {"Not yet implemented!"};
	return {};
	//return Iterator<T>{m_values,false};
}
template<typename T>
panima::Channel::Iterator<T> end()
{
	// TODO: This is a stub
	throw std::runtime_error {"Not yet implemented!"};
	return {};
	//return Iterator<T>{m_values,true};
}

template<typename T>
panima::Channel::IteratorWrapper<T> panima::Channel::It()
{
	// TODO: This is a stub
	throw std::runtime_error {"Not yet implemented!"};
	//static std::vector<uint8_t> empty;
	//return IsValueType<T>() ? IteratorWrapper<T>{values} : IteratorWrapper<T>{empty};
	return {};
}

/////////////////////

template<typename T>
T &panima::Channel::GetValue(uint32_t idx)
{
	return *(static_cast<T *>(m_valueData) + idx);
}

template<typename T>
auto panima::Channel::GetInterpolationFunction() const
{
	constexpr auto type = udm::type_to_enum<T>();
	if constexpr(std::is_same_v<T, Vector3>)
		return &uvec::lerp;
	else if constexpr(std::is_same_v<T, Quat>)
		return &uquat::slerp;
	else if constexpr(std::is_same_v<T, Vector2i> || std::is_same_v<T, Vector3i> || std::is_same_v<T, Vector4i>) {
		using Tf = std::conditional_t<std::is_same_v<T, Vector2i>, Vector2, std::conditional_t<std::is_same_v<T, Vector3i>, Vector3, Vector4>>;
		return [](const T &v0, const T &v1, float f) -> T { return static_cast<T>(static_cast<Tf>(v0) + f * (static_cast<Tf>(v1) - static_cast<Tf>(v0))); };
	}
	// TODO: How should we interpolate integral values?
	// else if constexpr(std::is_integral_v<T>)
	// 	return [](const T &v0,const T &v1,float f) -> T {return static_cast<T>(round(static_cast<double>(v0) +f *(static_cast<double>(v1) -static_cast<double>(v0))));};
	else
		return [](const T &v0, const T &v1, float f) -> T { return (v0 + f * (v1 - v0)); };
}

template<typename T, bool VALIDATE>
T panima::Channel::GetInterpolatedValue(float t, uint32_t &inOutPivotTimeIndex, T (*interpFunc)(const T &, const T &, float)) const
{
	if constexpr(VALIDATE) {
		auto &times = GetTimesArray();
		if(udm::type_to_enum<T>() != GetValueType() || times.IsEmpty())
			return make_value<T>();
	}
	float factor;
	auto indices = FindInterpolationIndices(t, factor, inOutPivotTimeIndex);
	inOutPivotTimeIndex = indices.first;
	auto &v0 = GetValue<T>(indices.first);
	auto &v1 = GetValue<T>(indices.second);
	return interpFunc ? interpFunc(v0, v1, factor) : GetInterpolationFunction<T>()(v0, v1, factor);
}

template<typename T, bool VALIDATE>
T panima::Channel::GetInterpolatedValue(float t, uint32_t &inOutPivotTimeIndex, void (*interpFunc)(const void *, const void *, double, void *)) const
{
	if(!interpFunc)
		return GetInterpolatedValue<T, VALIDATE>(t, inOutPivotTimeIndex);
	if constexpr(VALIDATE) {
		auto &times = GetTimesArray();
		if(udm::type_to_enum<T>() != GetValueType() || times.IsEmpty())
			return {};
	}
	float factor;
	auto indices = FindInterpolationIndices(t, factor, inOutPivotTimeIndex);
	inOutPivotTimeIndex = indices.first;
	auto &v0 = GetValue<T>(indices.first);
	auto &v1 = GetValue<T>(indices.second);
	auto v = make_value<T>();
	interpFunc(&v0, &v1, factor, &v);
	return v;
}

template<typename T, bool VALIDATE>
T panima::Channel::GetInterpolatedValue(float t, T (*interpFunc)(const T &, const T &, float)) const
{
	if constexpr(VALIDATE) {
		auto &times = GetTimesArray();
		if(udm::type_to_enum<T>() != GetValueType() || times.IsEmpty())
			return make_value<T>();
	}
	float factor;
	auto indices = FindInterpolationIndices(t, factor);
	auto &v0 = GetValue<T>(indices.first);
	auto &v1 = GetValue<T>(indices.second);
	return interpFunc ? interpFunc(v0, v1, factor) : GetInterpolationFunction<T>()(v0, v1, factor);
}

template<typename T, bool VALIDATE>
T panima::Channel::GetInterpolatedValue(float t, void (*interpFunc)(const void *, const void *, double, void *)) const
{
	if(!interpFunc)
		return GetInterpolatedValue<T, VALIDATE>(t);
	if constexpr(VALIDATE) {
		auto &times = GetTimesArray();
		if(udm::type_to_enum<T>() != GetValueType() || times.IsEmpty())
			return {};
	}
	float factor;
	auto indices = FindInterpolationIndices(t, factor);
	auto &v0 = GetValue<T>(indices.first);
	auto &v1 = GetValue<T>(indices.second);
	auto v = make_value<T>();
	interpFunc(&v0, &v1, factor, &v);
	return v;
}
