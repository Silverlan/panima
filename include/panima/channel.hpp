/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PANIMA_CHANNEL_HPP__
#define __PANIMA_CHANNEL_HPP__

#include "panima/expression.hpp"
#include "panima/types.hpp"
#include <sharedutils/util_path.hpp>
#include <udm_types.hpp>

namespace panima {
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
		std::optional<size_t> FindValueIndex(float time, float epsilon = panima::Channel::VALUE_EPSILON) const;
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
		template<typename T, bool ENABLE_VALIDATION = true>
		T GetInterpolatedValue(float t, uint32_t &inOutPivotTimeIndex, T (*interpFunc)(const T &, const T &, float) = nullptr) const;
		template<typename T, bool ENABLE_VALIDATION = true>
		T GetInterpolatedValue(float t, uint32_t &inOutPivotTimeIndex, void (*interpFunc)(const void *, const void *, double, void *)) const;
		template<typename T, bool ENABLE_VALIDATION = true>
		T GetInterpolatedValue(float t, T (*interpFunc)(const T &, const T &, float) = nullptr) const;
		template<typename T, bool ENABLE_VALIDATION = true>
		T GetInterpolatedValue(float t, void (*interpFunc)(const void *, const void *, double, void *)) const;

		template<typename T>
		void GetDataInRange(float tStart, float tEnd, std::vector<float> &outTimes, std::vector<T> &outValues) const;

		void Decimate(float tStart, float tEnd, float error = 0.03f);
		void Decimate(float error = 0.03f);

		std::optional<uint32_t> InsertSample(float t);
		void ScaleTimeInRange(float tStart, float tEnd, double scale);
		void ShiftTimeInRange(float tStart, float tEnd, float shiftAmount);

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

		bool operator==(const Channel &other) const { return this == &other; }
		bool operator!=(const Channel &other) const { return !operator==(other); }

		template<typename T>
		static void MergeDataArrays(const std::vector<float> &times0, const std::vector<T> values0, const std::vector<float> &times1, const std::vector<T> &values1, std::vector<float> &outTimes, std::vector<T> &outValues)
		{
			MergeDataArrays(
			  times0.size(), times0.data(), reinterpret_cast<const uint8_t *>(values0.data()), times1.size(), times1.data(), reinterpret_cast<const uint8_t *>(values1.data()), outTimes,
			  [&outValues](size_t size) -> uint8_t * {
				  outValues.resize(size);
				  return reinterpret_cast<uint8_t *>(outValues.data());
			  },
			  sizeof(T));
		}
	  private:
		static void MergeDataArrays(uint32_t n0, const float *times0, const uint8_t *values0, uint32_t n1, const float *times1, const uint8_t *values1, std::vector<float> &outTimes, const std::function<uint8_t *(size_t)> &fAllocateValueData, size_t valueStride);
		void TimeToLocalTimeFrame(float &inOutT) const;
		template<typename T>
		bool DoApplyValueExpression(double time, uint32_t timeIndex, T &inOutVal) const;
		uint32_t AddValue(float t, const void *value);
		uint32_t InsertValues(uint32_t n, const float *times, const void *values, size_t valueStride, float offset, InsertFlags flags = InsertFlags::ClearExistingDataInRange);
		std::pair<uint32_t, uint32_t> FindInterpolationIndices(float t, float &outInterpFactor, uint32_t pivotIndex, uint32_t recursionDepth) const;
		void GetDataInRange(float tStart, float tEnd, std::vector<float> &outTimes, const std::function<void *(size_t)> &fAllocateValueData) const;
		udm::PProperty m_times = nullptr;
		udm::PProperty m_values = nullptr;
		std::unique_ptr<expression::ValueExpression> m_valueExpression; //default constructor is sufficient
		TimeFrame m_timeFrame {};
		TimeFrame m_effectiveTimeFrame {};
	};

	class ArrayFloatIterator {
	  public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = float;
		using difference_type = std::ptrdiff_t;
		using pointer = float *;
		using reference = float &;

		ArrayFloatIterator(float *data);
		ArrayFloatIterator &operator++();
		ArrayFloatIterator operator++(int);
		ArrayFloatIterator operator+(uint32_t n);
		int32_t operator-(const ArrayFloatIterator &other) const;
		ArrayFloatIterator operator-(int32_t idx) const;
		reference operator*();
		const reference operator*() const;
		pointer operator->();
		const pointer operator->() const;
		bool operator==(const ArrayFloatIterator &other) const;
		bool operator!=(const ArrayFloatIterator &other) const;
	  private:
		float *m_data = nullptr;
	};
	ArrayFloatIterator begin(const udm::Array &a);
	ArrayFloatIterator end(const udm::Array &a);
};

REGISTER_BASIC_BITWISE_OPERATORS(panima::Channel::InsertFlags)

std::ostream &operator<<(std::ostream &out, const panima::Channel &o);
std::ostream &operator<<(std::ostream &out, const panima::TimeFrame &o);
std::ostream &operator<<(std::ostream &out, const panima::ChannelPath &o);

#endif
