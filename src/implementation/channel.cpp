// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "mathutil/glmutil.h"
#include "sharedutils/magic_enum.hpp"
#include <exprtk.hpp>

module panima;

import bezierfit;

import :channel;
import :expression;

panima::ChannelPath::ChannelPath(const std::string &ppath)
{
	auto pathWithoutScheme = ppath;
	auto colon = pathWithoutScheme.find(':');
	if(colon != std::string::npos) {
		auto scheme = pathWithoutScheme.substr(0, colon);
		if(scheme != "panima")
			return; // Invalid panima URI
		pathWithoutScheme = pathWithoutScheme.substr(colon + 1);
	}
	auto escapedPath = uriparser::escape(pathWithoutScheme);
	uriparser::Uri uri {escapedPath};
	auto scheme = uri.scheme();
	if(!scheme.empty() && scheme != "panima")
		return; // Invalid panima URI
	auto strPath = uri.path();
	if(!strPath.empty() && strPath.front() == '/')
		strPath.erase(strPath.begin());
	ustring::replace(strPath, "%20", " ");
	path = std::move(uriparser::unescape(strPath));
	auto strQueries = uri.query();
	std::vector<std::string> queries;
	ustring::explode(strQueries, "&", queries);
	for(auto &queryParam : queries) {
		std::vector<std::string> query;
		ustring::explode(queryParam, "=", query);
		if(query.size() < 2)
			continue;
		if(query.front() == "components") {
			m_components = std::make_unique<std::vector<std::string>>();
			ustring::explode(query[1], ",", *m_components);
			for(auto &str : *m_components)
				str = uriparser::unescape(str);
		}
	}
}
panima::ChannelPath::ChannelPath(const ChannelPath &other) { operator=(other); }
bool panima::ChannelPath::operator==(const ChannelPath &other) const { return path == other.path && ((!m_components && !other.m_components) || (m_components && other.m_components && *m_components == *other.m_components)); }
panima::ChannelPath &panima::ChannelPath::operator=(const ChannelPath &other)
{
	path = other.path;
	m_components = nullptr;
	if(other.m_components)
		m_components = std::make_unique<std::vector<std::string>>(*other.m_components);
	return *this;
}
std::string panima::ChannelPath::ToUri(bool includeScheme) const
{
	std::string uri;
	if(includeScheme)
		uri = "panima:";
	uri += path.GetString();
	if(m_components) {
		std::string strComponents;
		for(auto first = true; auto &c : *m_components) {
			if(first)
				first = false;
			else
				strComponents += ",";
			strComponents += c;
		}
		if(!strComponents.empty())
			uri += "?" + strComponents;
	}
	return uri;
}

////////////////

panima::ArrayFloatIterator::ArrayFloatIterator(float *data) : m_data {data} {}
panima::ArrayFloatIterator &panima::ArrayFloatIterator::operator++()
{
	m_data++;
	return *this;
}
panima::ArrayFloatIterator panima::ArrayFloatIterator::operator++(int)
{
	++m_data;
	return *this;
}
panima::ArrayFloatIterator panima::ArrayFloatIterator::operator+(uint32_t n)
{
	m_data += n;
	return *this;
}
int32_t panima::ArrayFloatIterator::operator-(const ArrayFloatIterator &other) const { return m_data - other.m_data; }
panima::ArrayFloatIterator panima::ArrayFloatIterator::operator-(int32_t idx) const { return ArrayFloatIterator {m_data - idx}; }
panima::ArrayFloatIterator::reference panima::ArrayFloatIterator::operator*() { return *m_data; }
panima::ArrayFloatIterator::const_reference panima::ArrayFloatIterator::operator*() const { return const_cast<ArrayFloatIterator *>(this)->operator*(); }
panima::ArrayFloatIterator::pointer panima::ArrayFloatIterator::operator->() { return m_data; }
const panima::ArrayFloatIterator::pointer panima::ArrayFloatIterator::operator->() const { return const_cast<ArrayFloatIterator *>(this)->operator->(); }
bool panima::ArrayFloatIterator::operator==(const ArrayFloatIterator &other) const { return m_data == other.m_data; }
bool panima::ArrayFloatIterator::operator!=(const ArrayFloatIterator &other) const { return !operator==(other); }

panima::ArrayFloatIterator panima::begin(const udm::Array &a) { return ArrayFloatIterator {static_cast<float *>(&const_cast<udm::Array &>(a).GetValue<float>(0))}; }
panima::ArrayFloatIterator panima::end(const udm::Array &a) { return ArrayFloatIterator {static_cast<float *>(&const_cast<udm::Array &>(a).GetValue<float>(0)) + a.GetSize()}; }

////////////////

panima::Channel::Channel() : m_times {::udm::Property::Create(udm::Type::ArrayLz4)}, m_values {::udm::Property::Create(udm::Type::ArrayLz4)}
{
	UpdateLookupCache();
	GetTimesArray().SetValueType(udm::Type::Float);
}
panima::Channel::Channel(const udm::PProperty &times, const udm::PProperty &values) : m_times {times}, m_values {values} { UpdateLookupCache(); }
panima::Channel::Channel(Channel &other) : Channel {} { operator=(other); }
panima::Channel::~Channel() {}
panima::Channel &panima::Channel::operator=(Channel &other)
{
	interpolation = other.interpolation;
	targetPath = other.targetPath;
	m_times = other.m_times->Copy(true);
	m_values = other.m_values->Copy(true);
	m_valueExpression = nullptr;
	if(other.m_valueExpression)
		m_valueExpression = std::make_unique<expression::ValueExpression>(*other.m_valueExpression);
	m_timeFrame = other.m_timeFrame;
	m_effectiveTimeFrame = other.m_effectiveTimeFrame;
	UpdateLookupCache();
	return *this;
}
bool panima::Channel::Save(udm::LinkedPropertyWrapper &prop) const
{
	prop["interpolation"] = interpolation;
	prop["targetPath"] = targetPath.ToUri();
	if(m_valueExpression)
		prop["expression"] = m_valueExpression->expression;

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

	auto *el = prop.GetValuePtr<udm::Element>();
	if(!el)
		return false;
	auto itTimes = el->children.find("times");
	auto itValues = el->children.find("values");
	if(itTimes == el->children.end() || itValues == el->children.end())
		return false;
	m_times = itTimes->second;
	m_values = itValues->second;
	UpdateLookupCache();

	// Note: Expression has to be loaded *after* the values, because
	// it's dependent on the value type
	auto udmExpression = prop["expression"];
	if(udmExpression) {
		std::string expr;
		udmExpression(expr);
		std::string err;
		if(SetValueExpression(expr, err) == false)
			; // TODO: Print warning?
	}
	return true;
}
uint32_t panima::Channel::GetSize() const { return GetTimesArray().GetSize(); }
void panima::Channel::Resize(uint32_t numValues)
{
	m_times->GetValue<udm::Array>().Resize(numValues);
	m_values->GetValue<udm::Array>().Resize(numValues);
	UpdateLookupCache();
}
void panima::Channel::Update()
{
	m_effectiveTimeFrame = m_timeFrame;
	if(m_effectiveTimeFrame.duration < 0.f)
		m_effectiveTimeFrame.duration = GetMaxTime();
}
size_t panima::Channel::Optimize()
{
	auto numTimes = GetTimeCount();
	constexpr auto EPSILON = 0.001f;
	size_t numRemoved = 0;
	if(numTimes > 2) {
		for(int32_t i = numTimes - 2; i >= 1; --i) {
			auto tPrev = *GetTime(i - 1);
			auto t = *GetTime(i);
			auto tNext = *GetTime(i + 1);

			auto shouldRemove = udm::visit_ng(GetValueType(), [this, tPrev, t, tNext, i](auto tag) -> bool {
				using T = typename decltype(tag)::type;
				if constexpr(is_animatable_type(udm::type_to_enum<T>())) {
					uint32_t pivotTimeIndex = i - 1;
					auto valPrev = GetInterpolatedValue<T>(tPrev, pivotTimeIndex);

					pivotTimeIndex = i;
					auto val = GetInterpolatedValue<T>(t, pivotTimeIndex);

					pivotTimeIndex = i + 1;
					auto valNext = GetInterpolatedValue<T>(tNext, pivotTimeIndex);

					auto f = (t - tPrev) / (tNext - tPrev);
					auto expectedVal = GetInterpolationFunction<T>()(valPrev, valNext, f);
					return uvec::is_equal(val, expectedVal, EPSILON);
				}
				return false;
			});

			if(shouldRemove) {
				// This value is just linearly interpolated between its neighbors,
				// we can remove it.
				RemoveValueAtIndex(i);
				++numRemoved;
			}
		}
	}

	numTimes = GetTimeCount();
	if(numTimes == 2) {
		// If only two values are remaining, we may be able to collapse into a single value (if they are the same)
		udm::visit_ng(GetValueType(), [this, &numRemoved](auto tag) {
			using T = typename decltype(tag)::type;
			if constexpr(is_animatable_type(udm::type_to_enum<T>())) {
				auto &val0 = GetValue<T>(0);
				auto &val1 = GetValue<T>(1);
				if(!uvec::is_equal(val0, val1, EPSILON))
					return;
				RemoveValueAtIndex(1);
				++numRemoved;
			}
		});
	}

	return numRemoved;
}
template<typename T>
bool panima::Channel::DoApplyValueExpression(double time, uint32_t timeIndex, T &inOutVal) const
{
	if(!m_valueExpression)
		return false;
	m_valueExpression->Apply<T>(time, timeIndex, m_effectiveTimeFrame, inOutVal);
	return true;
}
template bool panima::Channel::DoApplyValueExpression(double, uint32_t, udm::Int8 &) const;
template bool panima::Channel::DoApplyValueExpression(double, uint32_t, udm::UInt8 &) const;
template bool panima::Channel::DoApplyValueExpression(double, uint32_t, udm::Int16 &) const;
template bool panima::Channel::DoApplyValueExpression(double, uint32_t, udm::UInt16 &) const;
template bool panima::Channel::DoApplyValueExpression(double, uint32_t, udm::Int32 &) const;
template bool panima::Channel::DoApplyValueExpression(double, uint32_t, udm::UInt32 &) const;
template bool panima::Channel::DoApplyValueExpression(double, uint32_t, udm::Int64 &) const;
template bool panima::Channel::DoApplyValueExpression(double, uint32_t, udm::UInt64 &) const;
template bool panima::Channel::DoApplyValueExpression(double, uint32_t, udm::Float &) const;
template bool panima::Channel::DoApplyValueExpression(double, uint32_t, udm::Double &) const;
template bool panima::Channel::DoApplyValueExpression(double, uint32_t, udm::Boolean &) const;
template bool panima::Channel::DoApplyValueExpression(double, uint32_t, udm::Vector2 &) const;
template bool panima::Channel::DoApplyValueExpression(double, uint32_t, udm::Vector3 &) const;
template bool panima::Channel::DoApplyValueExpression(double, uint32_t, udm::Vector4 &) const;
template bool panima::Channel::DoApplyValueExpression(double, uint32_t, udm::Quaternion &) const;
template bool panima::Channel::DoApplyValueExpression(double, uint32_t, udm::EulerAngles &) const;
template bool panima::Channel::DoApplyValueExpression(double, uint32_t, udm::Mat4 &) const;
template bool panima::Channel::DoApplyValueExpression(double, uint32_t, udm::Mat3x4 &) const;
template bool panima::Channel::DoApplyValueExpression(double, uint32_t, udm::Vector2i &) const;
template bool panima::Channel::DoApplyValueExpression(double, uint32_t, udm::Vector3i &) const;
template bool panima::Channel::DoApplyValueExpression(double, uint32_t, udm::Vector4i &) const;

float panima::Channel::GetMinTime() const
{
	if(GetTimeCount() == 0)
		return 0.f;
	return *GetTimesArray().GetFront<float>();
}
float panima::Channel::GetMaxTime() const
{
	if(GetTimeCount() == 0)
		return 0.f;
	return *GetTimesArray().GetBack<float>();
}
void panima::Channel::MergeValues(const Channel &other)
{
	if(!udm::is_convertible(other.GetValueType(), GetValueType()))
		return;
	auto startTime = other.GetMinTime();
	auto endTime = other.GetMaxTime();
	if(!ClearRange(startTime, endTime))
		return;
	auto indicesStart = FindInterpolationIndices(startTime, startTime);
	auto startIdx = indicesStart.second;
	if(startIdx == std::numeric_limits<uint32_t>::max()) {
		if(!GetTimesArray().IsEmpty())
			return;
		startIdx = 0;
	}
	auto &times = GetTimesArray();
	auto &timesOther = other.GetTimesArray();
	auto &values = GetValueArray();
	auto &valuesOther = other.GetValueArray();
	times.AddValueRange(startIdx, other.GetValueCount());
	values.AddValueRange(startIdx, other.GetValueCount());
	UpdateLookupCache();
	memcpy(times.GetValuePtr(startIdx), const_cast<udm::Array &>(other.GetTimesArray()).GetValuePtr(0), timesOther.GetSize() * timesOther.GetValueSize());
	if(other.GetValueType() == GetValueType()) {
		// Same value type, just copy
		memcpy(values.GetValuePtr(startIdx), const_cast<udm::Array &>(other.GetValueArray()).GetValuePtr(0), valuesOther.GetSize() * valuesOther.GetValueSize());
		return;
	}
	// Values have to be converted
	udm::visit_ng(GetValueType(), [this, &other, startIdx, &values, &valuesOther](auto tag) {
		using T = typename decltype(tag)::type;
		udm::visit_ng(other.GetValueType(), [this, &other, startIdx, &values, &valuesOther](auto tag) {
			using TOther = typename decltype(tag)::type;
			if constexpr(udm::is_convertible<TOther, T>()) {
				auto n = other.GetValueCount();
				for(auto i = decltype(n) {0u}; i < n; ++i)
					values.GetValue<T>(startIdx + i) = udm::convert<TOther, T>(valuesOther.GetValue<TOther>(i));
			}
		});
	});
}
void panima::Channel::ClearAnimationData()
{
	GetTimesArray().Resize(0);
	GetValueArray().Resize(0);
	UpdateLookupCache();
}
bool panima::Channel::ClearRange(float startTime, float endTime, bool addCaps)
{
	if(GetTimesArray().IsEmpty())
		return true;
	auto minTime = *GetTime(0);
	auto maxTime = *GetTime(GetTimeCount() - 1);
	if(endTime < startTime || (startTime < minTime - TIME_EPSILON && endTime < minTime - TIME_EPSILON) || (startTime > maxTime + TIME_EPSILON && endTime > maxTime + TIME_EPSILON))
		return false;
	startTime = umath::clamp(startTime, minTime, maxTime);
	endTime = umath::clamp(endTime, minTime, maxTime);
	float t;
	auto indicesStart = FindInterpolationIndices(startTime, t);
	auto startIdx = (t < TIME_EPSILON) ? indicesStart.first : indicesStart.second;
	auto indicesEnd = FindInterpolationIndices(endTime, t);
	auto endIdx = (t > (1.f - TIME_EPSILON)) ? indicesEnd.second : indicesEnd.first;
	if(startIdx == std::numeric_limits<uint32_t>::max() || endIdx == std::numeric_limits<uint32_t>::max() || endIdx < startIdx)
		return false;
	udm::visit_ng(GetValueType(), [this, startTime, endTime, startIdx, endIdx, addCaps](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(is_animatable_type(udm::type_to_enum<T>())) {
			auto startVal = GetInterpolatedValue<T>(startTime);
			auto endVal = GetInterpolatedValue<T>(endTime);

			auto &times = GetTimesArray();
			auto &values = GetValueArray();
			times.RemoveValueRange(startIdx, (endIdx - startIdx) + 1);
			values.RemoveValueRange(startIdx, (endIdx - startIdx) + 1);
			UpdateLookupCache();

			if(addCaps) {
				AddValue<T>(startTime, startVal);
				AddValue<T>(endTime, endVal);
				UpdateLookupCache();
			}
		}
	});
	return true;
}
void panima::Channel::ClearValueExpression() { m_valueExpression = nullptr; }
bool panima::Channel::TestValueExpression(std::string expression, std::string &outErr)
{
	m_valueExpression = nullptr;

	auto expr = std::make_unique<expression::ValueExpression>(*this);
	expr->expression = std::move(expression);
	if(!expr->Initialize(GetValueType(), outErr))
		return false;
	return true;
}
bool panima::Channel::SetValueExpression(std::string expression, std::string &outErr)
{
	m_valueExpression = nullptr;

	auto expr = std::make_unique<expression::ValueExpression>(*this);
	expr->expression = std::move(expression);
	if(!expr->Initialize(GetValueType(), outErr))
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
void panima::Channel::MergeDataArrays(uint32_t n0, const float *times0, const uint8_t *values0, uint32_t n1, const float *times1, const uint8_t *values1, std::vector<float> &outTimes, const std::function<uint8_t *(size_t)> &fAllocateValueData, size_t valueStride)
{
	outTimes.resize(n0 + n1);
	auto *times = outTimes.data();
	auto *values = fAllocateValueData(outTimes.size());
	size_t idx0 = 0;
	size_t idx1 = 0;
	size_t outIdx = 0;
	while(idx0 < n0 || idx1 < n1) {
		if(idx0 < n0) {
			if(idx1 >= n1 || times0[idx0] < times1[idx1]) {
				if(outIdx == 0 || umath::abs(times0[idx0] - times[outIdx - 1]) > TIME_EPSILON) {
					times[outIdx] = times0[idx0];
					memcpy(values + outIdx * valueStride, values0 + idx0 * valueStride, valueStride);
					++outIdx;
				}
				++idx0;
				continue;
			}
		}

		assert(idx1 < n1);
		if(outIdx == 0 || umath::abs(times1[idx1] - times[outIdx - 1]) > TIME_EPSILON) {
			times[outIdx] = times1[idx1];
			memcpy(values + outIdx * valueStride, values1 + idx1 * valueStride, valueStride);
			++outIdx;
		}
		++idx1;
	}
	outTimes.resize(outIdx);
	fAllocateValueData(outIdx);
}
void panima::Channel::GetDataInRange(float tStart, float tEnd, std::vector<float> *optOutTimes, const std::function<void *(size_t)> &optAllocateValueData) const
{
	if(tEnd < tStart)
		return;
	::udm::visit_ng(GetValueType(), [this, tStart, tEnd, optOutTimes, &optAllocateValueData](auto tag) {
		using T = typename decltype(tag)::type;

		auto n = GetValueCount();
		auto getInterpolatedValue = [this, n](float t, uint32_t &outIdx, bool prefix) -> std::optional<std::pair<float, T>> {
			float f;
			auto indices = FindInterpolationIndices(t, f);
			if(indices.first == std::numeric_limits<decltype(indices.first)>::max()) {
				if(t <= *GetTime(0)) {
					indices = {0, 0};
					f = 0.f;
				}
				else {
					indices = {n - 1, n - 1};
					f = 0.f;
				}
			}

			if(f == 0.f)
				outIdx = indices.first;
			else if(f == 1.f)
				outIdx = indices.second;
			else {
				outIdx = prefix ? indices.second : indices.first;

				auto time0 = *GetTime(indices.first);
				auto time1 = *GetTime(indices.second);
				auto &value0 = GetValue<T>(indices.first);
				auto &value1 = GetValue<T>(indices.second);
				auto result = make_value<T>();
				udm::lerp_value(value0, value1, f, result, udm::type_to_enum<T>());
				return std::pair<float, T> {umath::lerp(time0, time1, f), result};
			}
			return {};
		};

		if(n > 0) {
			uint32_t idxStart;
			auto prefixValue = getInterpolatedValue(tStart, idxStart, true);

			uint32_t idxEnd;
			auto postfixValue = getInterpolatedValue(tEnd, idxEnd, false);

			auto count = idxEnd - idxStart + 1;
			if(prefixValue)
				++count;
			if(postfixValue)
				++count;

			float *times = nullptr;
			if(optOutTimes) {
				optOutTimes->resize(count);
				times = optOutTimes->data();
			}
			T *values = nullptr;
			if(optAllocateValueData)
				values = static_cast<T *>(optAllocateValueData(count));
			size_t idx = 0;
			if(prefixValue) {
				if(times)
					times[idx] = prefixValue->first;
				if(values)
					values[idx] = prefixValue->second;
				++idx;
			}
			for(auto i = idxStart; i <= idxEnd; ++i) {
				if(times)
					times[idx] = *GetTime(i);
				if(values)
					values[idx] = GetValue<T>(i);
				++idx;
			}
			if(postfixValue) {
				if(times)
					times[idx] = postfixValue->first;
				if(values)
					values[idx] = postfixValue->second;
				++idx;
			}
		}
	});
}
void panima::Channel::GetTimesInRange(float tStart, float tEnd, std::vector<float> &outTimes) const { GetDataInRange(tStart, tEnd, &outTimes, nullptr); }
void panima::Channel::Decimate(float error)
{
	auto n = GetTimeCount();
	if(n < 2)
		return;
	Decimate(*GetTime(0), *GetTime(n - 1), error);
}
std::optional<uint32_t> panima::Channel::InsertSample(float t)
{
	if(GetTimeCount() == 0)
		return {};
	float f;
	auto idx = FindValueIndex(t);
	if(idx)
		return *idx; // There already is a sample at this timestamp
	return udm::visit_ng(GetValueType(), [this, t](auto tag) -> std::optional<uint32_t> {
		using T = typename decltype(tag)::type;
		if constexpr(is_animatable_type(udm::type_to_enum<T>())) {
			auto val = GetInterpolatedValue<T>(t);
			return AddValue<T>(t, val);
		}
		return {};
	});
}
void panima::Channel::TransformGlobal(const umath::ScaledTransform &transform)
{
	auto valueType = GetValueType();
	auto numTimes = GetTimeCount();
	switch(valueType) {
	case udm::Type::Vector3:
		{
			for(size_t i = 0; i < numTimes; ++i) {
				auto t = *GetTime(i);
				auto &val = GetValue<Vector3>(i);
				val = transform * val;
			}
			break;
		}
	case udm::Type::Quaternion:
		{
			for(size_t i = 0; i < numTimes; ++i) {
				auto t = *GetTime(i);
				auto &val = GetValue<Quat>(i);
				val = transform * val;
			}
			break;
		}
	default:
		break;
	}
}
void panima::Channel::RemoveValueAtIndex(uint32_t idx)
{
	auto &times = GetTimesArray();
	times.RemoveValue(idx);

	auto &values = GetValueArray();
	values.RemoveValue(idx);
	UpdateLookupCache();
}
void panima::Channel::ResolveDuplicates(float t)
{
	for(;;) {
		auto idx = FindValueIndex(t);
		if(!idx)
			break;
		auto t = *GetTime(*idx);
		auto numTimes = GetTimeCount();
		if(*idx > 0) {
			auto tPrev = *GetTime(*idx - 1);
			if(umath::abs(t - tPrev) <= TIME_EPSILON) {
				RemoveValueAtIndex(*idx - 1);
				continue;
			}
		}
		if(numTimes > 1 && *idx < numTimes - 1) {
			auto tNext = *GetTime(*idx + 1);
			if(umath::abs(t - tNext) <= TIME_EPSILON) {
				RemoveValueAtIndex(*idx + 1);
				continue;
			}
		}
		break;
	}
}
std::pair<std::optional<uint32_t>, std::optional<uint32_t>> panima::Channel::GetBoundaryIndices(float tStart, float tEnd, bool retainBoundaries)
{
	if(!retainBoundaries) {
		float f;
		auto indicesStart = FindInterpolationIndices(tStart, f, 0);
		if(indicesStart.first == std::numeric_limits<decltype(indicesStart.first)>::max())
			return {};
		f *= (*GetTime(indicesStart.second) - *GetTime(indicesStart.first));
		auto startIdx = (f < TIME_EPSILON) ? indicesStart.first : indicesStart.second;

		auto indicesEnd = FindInterpolationIndices(tEnd, f, startIdx);
		if(indicesEnd.first == std::numeric_limits<decltype(indicesEnd.first)>::max())
			return {};
		f *= (*GetTime(indicesEnd.second) - *GetTime(indicesEnd.first));
		auto endIdx = (umath::abs(f - *GetTime(indicesEnd.second)) < TIME_EPSILON) ? indicesEnd.second : indicesEnd.first;
		auto tStartTs = *GetTime(startIdx);
		auto tEndTs = *GetTime(endIdx);
		auto tMin = tStart - TIME_EPSILON;
		auto tMax = tEnd + TIME_EPSILON;
		if(tStartTs < tMin || tStartTs > tMax || tEndTs < tMin || tEndTs > tMax)
			return {};
		return {startIdx, endIdx};
	}
	std::optional<uint32_t> idxStart = 0;
	auto t0 = GetTime(*idxStart);
	if(!t0)
		return {};
	if(*t0 < tStart - TIME_EPSILON)
		idxStart = InsertSample(tStart);

	std::optional<uint32_t> idxEnd = GetTimeCount() - 1;
	auto t1 = GetTime(*idxEnd);
	if(!t1)
		return {};
	if(*t1 > tEnd + TIME_EPSILON)
		idxEnd = InsertSample(tEnd);
	return {idxStart, idxEnd};
}
void panima::Channel::ShiftTimeInRange(float tStart, float tEnd, float shiftAmount, bool retainBoundaryValues)
{
	if(umath::abs(shiftAmount) <= TIME_EPSILON * 1.5f)
		return;
	auto [idxStart, idxEnd] = GetBoundaryIndices(tStart, tEnd, retainBoundaryValues);
	if(!idxStart || !idxEnd)
		return;
	tStart = *GetTime(*idxStart);
	tEnd = *GetTime(*idxEnd);
	// After shifting, we need to restore the value at the opposite boundary of the shift
	// (e.g. if we're shifting left, we have to restore the right-most value and vice versa.)
	std::shared_ptr<void> boundaryValue = nullptr;
	if(retainBoundaryValues) {
		auto valIdxStart = *idxStart;
		auto valIdxEnd = *idxEnd;
		udm::visit_ng(GetValueType(), [this, &boundaryValue, shiftAmount, valIdxStart, valIdxEnd](auto tag) {
			using T = typename decltype(tag)::type;
			auto val = GetValue<T>((shiftAmount < 0.f) ? valIdxEnd : valIdxStart);
			boundaryValue = std::make_shared<T>(val);
		});
		if(shiftAmount < 0) {
			// Clear the shift range, but keep the value at tStart. We use epsilon *1.5 to prevent
			// potential edge cases because of precision errors
			ClearRange(tStart + shiftAmount - TIME_EPSILON * 1.5f, tStart - TIME_EPSILON * 1.5f, false);
		}
		else
			ClearRange(tEnd + TIME_EPSILON * 1.5f, tEnd + shiftAmount + TIME_EPSILON * 1.5f, false);

		// Indices may have changed due to cleared ranges
		idxStart = FindValueIndex(tStart);
		idxEnd = FindValueIndex(tEnd);
		assert(idxStart.has_value() && idxEnd.has_value());
	}

	auto &times = GetTimesArray();
	if(!idxStart || !idxEnd)
		return;
	for(auto idx = *idxStart; idx <= *idxEnd; ++idx) {
		auto &t = times.GetValue<float>(idx);
		t += shiftAmount;
	}
	ResolveDuplicates(*GetTime(*idxStart));
	ResolveDuplicates(*GetTime(*idxEnd));
	if(retainBoundaryValues) {
		// Restore boundary value
		udm::visit_ng(GetValueType(), [this, &boundaryValue, shiftAmount, tStart, tEnd](auto tag) {
			using T = typename decltype(tag)::type;
			AddValue<T>((shiftAmount < 0.f) ? tEnd : tStart, *static_cast<T *>(boundaryValue.get()));
		});
		ResolveDuplicates(tStart);
		ResolveDuplicates(tEnd);
	}
}
void panima::Channel::ScaleTimeInRange(float tStart, float tEnd, float tPivot, double scale, bool retainBoundaryValues)
{
	auto [idxStart, idxEnd] = GetBoundaryIndices(tStart, tEnd, retainBoundaryValues);
	if(!idxStart || !idxEnd)
		return;
	tStart = *GetTime(*idxStart);
	tEnd = *GetTime(*idxEnd);

	// After scaling, we need to restore the value at the boundaries where we are scaling
	// away from the rest of the animation
	std::shared_ptr<void> boundaryValueStart = nullptr;
	std::shared_ptr<void> boundaryValueEnd = nullptr;
	if(retainBoundaryValues) {
		auto valIdxStart = *idxStart;
		auto valIdxEnd = *idxEnd;
		udm::visit_ng(GetValueType(), [this, valIdxStart, valIdxEnd, &boundaryValueStart, &boundaryValueEnd](auto tag) {
			using T = typename decltype(tag)::type;
			boundaryValueStart = std::make_shared<T>(GetValue<T>(valIdxStart));
			boundaryValueEnd = std::make_shared<T>(GetValue<T>(valIdxEnd));
		});
	}

	auto rescale = [tPivot, scale](double t) {
		t -= tPivot;
		t *= scale;
		t += tPivot;
		return t;
	};
	if(retainBoundaryValues) {
		auto scaledStart = rescale(tStart);
		if(scaledStart < tStart)
			ClearRange(scaledStart, tStart - TIME_EPSILON * 1.5f, false);

		auto scaledEnd = rescale(tEnd);
		if(scaledEnd > tEnd)
			ClearRange(tEnd + TIME_EPSILON * 1.5f, scaledEnd, false);

		// Indices may have changed due to cleared ranges
		idxStart = FindValueIndex(tStart);
		idxEnd = FindValueIndex(tEnd);
		assert(idxStart.has_value() && idxEnd.has_value());
	}

	auto &times = GetTimesArray();
	if(!idxStart || !idxEnd)
		return;
	// Scale all times within the range [tStart,tEnd]
	for(auto idx = *idxStart; idx <= *idxEnd; ++idx) {
		auto &t = times.GetValue<float>(idx);
		t = rescale(t);
	}
	ResolveDuplicates(*GetTime(*idxStart));
	ResolveDuplicates(*GetTime(*idxEnd));

	if(retainBoundaryValues) {
		// Restore boundary values
		udm::visit_ng(GetValueType(), [this, &boundaryValueStart, boundaryValueEnd, &times, tStart, tEnd, tPivot, scale](auto tag) {
			using T = typename decltype(tag)::type;
			// If the scale is smaller than 1, we'll be pulled towards the pivot, otherwise we will be
			// pushed away from it. In some cases this will create a 'hole' near the boundary that we have to plug.
			// (e.g. If the pivot lies to the right of the start time, and the start time is being pulled towards it (i.e. pivot < 1.0),
			// a hole will be left to the left of the start time.)
			if((scale < 1.0 && tPivot >= tStart) || (scale > 1.0 && tPivot <= tStart))
				AddValue<T>(tStart, *static_cast<T *>(boundaryValueStart.get()));
			if((scale < 1.0 && tPivot <= tEnd) || (scale > 1.0 && tPivot >= tEnd))
				AddValue<T>(tEnd, *static_cast<T *>(boundaryValueEnd.get()));
		});
		ResolveDuplicates(tStart);
		ResolveDuplicates(tEnd);
	}
}
void panima::Channel::Decimate(float tStart, float tEnd, float error)
{
	return udm::visit_ng(GetValueType(), [this, tStart, tEnd, error](auto tag) {
		using T = typename decltype(tag)::type;
		using TValue = std::conditional_t<std::is_same_v<T, bool>, uint8_t, T>;
		if constexpr(is_animatable_type(udm::type_to_enum<TValue>())) {
			std::vector<float> times;
			std::vector<TValue> values;
			GetDataInRange<TValue>(tStart, tEnd, times, values);

			// We need to decimate each component of the value separately, then merge the reduced values
			auto valueType = GetValueType();
			auto numComp = udm::get_numeric_component_count(valueType);
			std::vector<std::vector<float>> newTimes;
			std::vector<std::vector<TValue>> newValues;
			newTimes.resize(numComp);
			newValues.resize(numComp);
			for(auto c = decltype(numComp) {0u}; c < numComp; ++c) {
				std::vector<bezierfit::VECTOR> tmpValues;
				tmpValues.reserve(times.size());
				for(auto i = decltype(times.size()) {0u}; i < times.size(); ++i)
					tmpValues.push_back({times[i], udm::get_numeric_component(values[i], c)});
				auto reduced = bezierfit::reduce(tmpValues, error);

				// Calculate interpolated values for the reduced timestamps
				auto &cValues = newValues[c];
				auto &cTimes = newTimes[c];
				cValues.reserve(reduced.size());
				cTimes.reserve(reduced.size());
				for(auto &v : reduced) {
					auto value = GetInterpolatedValue<TValue>(v.x);
					udm::set_numeric_component(value, c, v.y);
					cTimes.push_back(v.x);
					cValues.push_back(value);
				}
			}

			// Clear values in the target range
			ClearRange(tStart, tEnd, true);

			// Merge components back together
			for(auto c = decltype(numComp) {0u}; c < numComp; ++c) {
				auto &cValues = newValues[c];
				auto &cTimes = newTimes[c];
				InsertValues<TValue>(cTimes.size(), cTimes.data(), cValues.data(), 0.f, InsertFlags::None);
			}
		}
	});
}
uint32_t panima::Channel::InsertValues(uint32_t n, const float *times, const void *values, size_t valueStride, float offset, InsertFlags flags)
{
	if(n == 0)
		return std::numeric_limits<uint32_t>::max();
	if(offset != 0.f) {
		std::vector<float> timesWithOffset;
		timesWithOffset.resize(n);
		for(auto i = decltype(n) {0u}; i < n; ++i)
			timesWithOffset[i] = times[i] + offset;
		return InsertValues(n, timesWithOffset.data(), values, valueStride, 0.f);
	}
	if(umath::is_flag_set(flags, InsertFlags::ClearExistingDataInRange) == false) {
		auto tStart = times[0];
		auto tEnd = times[n - 1];
		return udm::visit_ng(GetValueType(), [this, tStart, tEnd, n, times, values, valueStride, offset, flags](auto tag) {
			using T = typename decltype(tag)::type;
			using TValue = std::conditional_t<std::is_same_v<T, bool>, uint8_t, T>;
			std::vector<float> newTimes;
			std::vector<TValue> newValues;
			GetDataInRange(tStart, tEnd, newTimes, newValues);

			std::vector<float> mergedTimes;
			std::vector<TValue> mergedValues;
			MergeDataArrays(
			  newTimes.size(), newTimes.data(), reinterpret_cast<uint8_t *>(newValues.data()), n, times, static_cast<const uint8_t *>(values), mergedTimes,
			  [&mergedValues](size_t size) -> uint8_t * {
				  mergedValues.resize(size, make_value<TValue>());
				  return reinterpret_cast<uint8_t *>(mergedValues.data());
			  },
			  sizeof(TValue));

			auto newFlags = flags;
			umath::set_flag(newFlags, InsertFlags::ClearExistingDataInRange);
			return InsertValues(mergedTimes.size(), mergedTimes.data(), mergedValues.data(), valueStride, offset, newFlags);
		});
	}
	auto startTime = times[0];
	auto endTime = times[n - 1];
	ClearRange(startTime - TIME_EPSILON, endTime + TIME_EPSILON, false);
	float f;
	auto indices = FindInterpolationIndices(times[0], f);
	auto startIndex = indices.second;
	if(startIndex == std::numeric_limits<decltype(startIndex)>::max() || startTime > *GetTime(indices.second))
		startIndex = GetValueCount();
	auto numCurValues = GetValueCount();
	auto numNewValues = numCurValues + n;
	Resize(numCurValues + n);

	auto &timesArray = GetTimesArray();
	auto &valueArray = GetValueArray();
	auto *pValues = static_cast<const uint8_t *>(values);

	// Move up current values
	for(auto i = static_cast<int64_t>(numNewValues - 1); i >= startIndex + n; --i) {
		auto iSrc = i - n;
		auto iDst = i;
		timesArray.SetValue(iDst, static_cast<const void *>(timesArray.GetValuePtr(iSrc)));
		valueArray.SetValue(iDst, static_cast<const void *>(valueArray.GetValuePtr(iSrc)));
	}

	// Assign new values
	for(auto i = startIndex; i < (startIndex + n); ++i) {
		auto iSrc = i - startIndex;
		auto iDst = i;
		timesArray.SetValue(iDst, times[i - startIndex]);
		valueArray.SetValue(iDst, static_cast<const void *>(pValues));
		pValues += valueStride;
	}

	if(umath::is_flag_set(flags, InsertFlags::DecimateInsertedData))
		Decimate(startTime, endTime);
	return startIndex;
}
uint32_t panima::Channel::AddValue(float t, const void *value)
{
	float interpFactor;
	auto indices = FindInterpolationIndices(t, interpFactor);
	if(indices.first == std::numeric_limits<decltype(indices.first)>::max()) {
		auto size = GetSize() + 1;
		Resize(size);
		auto idx = size - 1;
		GetTimesArray()[idx] = t;
		GetValueArray()[idx] = value;
		return idx;
	}
	if(umath::abs(t - *GetTime(indices.first)) < VALUE_EPSILON) {
		// Replace value at first index with new value
		auto idx = indices.first;
		GetTimesArray()[idx] = t;
		GetValueArray()[idx] = value;
		return idx;
	}
	if(umath::abs(t - *GetTime(indices.second)) < VALUE_EPSILON) {
		// Replace value at second index with new value
		auto idx = indices.second;
		GetTimesArray()[idx] = t;
		GetValueArray()[idx] = value;
		return idx;
	}
	auto &times = GetTimesArray();
	auto &values = GetValueArray();
	if(indices.first == indices.second) {
		if(indices.first == 0 && t < times.GetValue<float>(0)) {
			// New time value preceeds first time value in time array, push front
			auto idx = indices.first;
			times.InsertValue(idx, t);
			udm::visit_ng(GetValueType(), [&values, idx, value](auto tag) {
				using T = typename decltype(tag)::type;
				values.InsertValue(idx, *static_cast<const T *>(value));
			});
			UpdateLookupCache();
			return idx;
		}
		// New time value exceeds last time value in time array, push back
		auto idx = indices.second + 1;
		times.InsertValue(idx, t);
		udm::visit_ng(GetValueType(), [&values, idx, value](auto tag) {
			using T = typename decltype(tag)::type;
			values.InsertValue(idx, *static_cast<const T *>(value));
		});
		UpdateLookupCache();
		return idx;
	}
	// Insert new value between the two indices
	auto idx = indices.second;
	times.InsertValue(idx, t);
	udm::visit_ng(GetValueType(), [&values, idx, value](auto tag) {
		using T = typename decltype(tag)::type;
		values.InsertValue(idx, *static_cast<const T *>(value));
	});
	UpdateLookupCache();
	return idx;
}
void panima::Channel::UpdateLookupCache()
{
	m_timesArray = m_times->GetValuePtr<udm::Array>();
	m_valueArray = m_values->GetValuePtr<udm::Array>();
	m_timesData = !m_timesArray->IsEmpty() ? m_timesArray->GetValuePtr<float>(0) : nullptr;
	m_valueData = !m_valueArray->IsEmpty() ? m_valueArray->GetValuePtr(0) : nullptr;

	if(m_timesArray->GetArrayType() == udm::ArrayType::Compressed)
		static_cast<udm::ArrayLz4 *>(m_timesArray)->SetUncompressedMemoryPersistent(true);
	if(m_valueArray->GetArrayType() == udm::ArrayType::Compressed)
		static_cast<udm::ArrayLz4 *>(m_valueArray)->SetUncompressedMemoryPersistent(true);
}
udm::Array &panima::Channel::GetTimesArray() { return *m_timesArray; }
udm::Array &panima::Channel::GetValueArray() { return *m_valueArray; }
udm::Type panima::Channel::GetValueType() const { return GetValueArray().GetValueType(); }
void panima::Channel::SetValueType(udm::Type type) { GetValueArray().SetValueType(type); }
bool panima::Channel::Validate() const
{
	std::vector<float> times;
	auto numTimes = GetTimeCount();
	times.reserve(numTimes);
	for(auto i = decltype(numTimes) {0u}; i < numTimes; ++i)
		times.push_back(*GetTime(i));
	if(times.size() <= 1)
		return true;
	for(auto i = decltype(times.size()) {1u}; i < times.size(); ++i) {
		auto t0 = times[i - 1];
		auto t1 = times[i];
		if(t0 >= t1) {
			throw std::runtime_error {"Time values are not in order!"};
			const_cast<panima::Channel *>(this)->ResolveDuplicates(t0);
			return false;
		}
		auto diff = t1 - t0;
		if(fabsf(diff) < TIME_EPSILON * 0.5f) {
			throw std::runtime_error {"Time values are too close!"};
			return false;
		}
	}
	return true;
}
void panima::Channel::TimeToLocalTimeFrame(float &inOutT) const
{
	inOutT -= m_timeFrame.startOffset;
	if(m_timeFrame.duration >= 0.f)
		inOutT = umath::min(inOutT, m_timeFrame.duration);
	inOutT *= m_timeFrame.scale;
}
std::pair<uint32_t, uint32_t> panima::Channel::FindInterpolationIndices(float t, float &interpFactor, uint32_t pivotIndex, uint32_t recursionDepth) const
{
	constexpr uint32_t MAX_RECURSION_DEPTH = 2;
	auto &times = GetTimesArray();
	if(pivotIndex >= times.GetSize() || times.GetSize() < 2 || recursionDepth == MAX_RECURSION_DEPTH)
		return FindInterpolationIndices(t, interpFactor);
	// We'll use the pivot index as the starting point of our search and check out the times immediately surrounding it.
	// If we have a match, we can return immediately. If not, we'll slightly broaden the search until we've reached the max recursion depth or found a match.
	// If we hit the max recusion depth, we'll just do a regular binary search instead.
	auto tPivot = times.GetValue<float>(pivotIndex);
	auto tLocal = t;
	TimeToLocalTimeFrame(tLocal);
	if(tLocal >= tPivot) {
		if(pivotIndex == times.GetSize() - 1) {
			interpFactor = 0.f;
			return {static_cast<uint32_t>(GetValueArray().GetSize() - 1), static_cast<uint32_t>(GetValueArray().GetSize() - 1)};
		}
		auto tPivotNext = times.GetValue<float>(pivotIndex + 1);
		if(tLocal < tPivotNext) {
			// Most common case
			interpFactor = (tLocal - tPivot) / (tPivotNext - tPivot);
			return {pivotIndex, pivotIndex + 1};
		}
		return FindInterpolationIndices(t, interpFactor, pivotIndex + 1, recursionDepth + 1);
	}
	if(pivotIndex == 0) {
		interpFactor = 0.f;
		return {0u, 0u};
	}
	return FindInterpolationIndices(t, interpFactor, pivotIndex - 1, recursionDepth + 1);
}
std::pair<uint32_t, uint32_t> panima::Channel::FindInterpolationIndices(float t, float &interpFactor, uint32_t pivotIndex) const { return FindInterpolationIndices(t, interpFactor, pivotIndex, 0u); }

std::pair<uint32_t, uint32_t> panima::Channel::FindInterpolationIndices(float t, float &interpFactor) const
{
	auto &times = GetTimesArray();
	auto numTimes = times.GetSize();
	if(numTimes == 0) {
		interpFactor = 0.f;
		return {std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max()};
	}
	// Binary search
	TimeToLocalTimeFrame(t);
	auto it = std::upper_bound(begin(times), end(times), t);
	if(it == end(times)) {
		interpFactor = 0.f;
		return {static_cast<uint32_t>(numTimes - 1), static_cast<uint32_t>(numTimes - 1)};
	}
	if(it == begin(times)) {
		interpFactor = 0.f;
		return {0u, 0u};
	}
	auto itPrev = it - 1;
	interpFactor = (t - *itPrev) / (*it - *itPrev);
	return {static_cast<uint32_t>(itPrev - begin(times)), static_cast<uint32_t>(it - begin(times))};
}

std::optional<size_t> panima::Channel::FindValueIndex(float time, float epsilon) const
{
	auto &t = GetTimesArray();
	auto size = t.GetSize();
	if(size == 0)
		return {};
	float interpFactor;
	auto indices = FindInterpolationIndices(time, interpFactor);
	if(indices.first == std::numeric_limits<decltype(indices.first)>::max())
		return {};
	if(interpFactor == 0.f && indices.first == indices.second) {
		if(indices.first == 0 && umath::abs(t.GetValue<float>(0) - time) >= epsilon)
			return {};
		if(indices.first == size - 1 && umath::abs(t.GetValue<float>(size - 1) - time) >= epsilon)
			return {};
	}
	interpFactor *= (*GetTime(indices.second) - *GetTime(indices.first));
	if(interpFactor < epsilon)
		return indices.first;
	if(interpFactor > 1.f - epsilon)
		return indices.second;
	return {};
}

uint32_t panima::Channel::GetTimeCount() const { return GetTimesArray().GetSize(); }
uint32_t panima::Channel::GetValueCount() const { return GetValueArray().GetSize(); }
std::optional<float> panima::Channel::GetTime(uint32_t idx) const
{
	auto &times = GetTimesArray();
	auto n = times.GetSize();
	return (idx < n) ? times.GetValue<float>(idx) : std::optional<float> {};
}

std::ostream &operator<<(std::ostream &out, const panima::Channel &o)
{
	out << "Channel";
	out << "[Path:" << o.targetPath << "]";
	out << "[Interp:" << magic_enum::enum_name(o.interpolation) << "]";
	out << "[Values:" << o.GetValueArray().GetSize() << "]";

	std::pair<float, float> timeRange {0.f, 0.f};
	auto n = o.GetTimeCount();
	if(n > 0)
		out << "[TimeRange:" << *o.GetTime(0) << "," << *o.GetTime(n - 1) << "]";
	return out;
}
std::ostream &operator<<(std::ostream &out, const panima::TimeFrame &o)
{
	out << "TimeFrame";
	out << "[StartOffset:" << o.startOffset << "]";
	out << "[Scale:" << o.scale << "]";
	out << "[Duration:" << o.duration << "]";
	return out;
}
std::ostream &operator<<(std::ostream &out, const panima::ChannelPath &o)
{
	out << "ChannelPath";
	out << "[" << o.ToUri() << "]";
	return out;
}

//////////

panima::ChannelValueSubmitter::ChannelValueSubmitter(const std::function<void(Channel &, uint32_t &, double)> &submitter) : submitter {submitter} {}
panima::ChannelValueSubmitter::operator bool() const { return submitter && enabled; }
bool panima::ChannelValueSubmitter::operator==(const std::nullptr_t &t) const { return submitter == nullptr; }
bool panima::ChannelValueSubmitter::operator!=(const std::nullptr_t &t) const { return submitter != nullptr; }
void panima::ChannelValueSubmitter::operator()(Channel &channel, uint32_t &timestampIndex, double t) { submitter(channel, timestampIndex, t); }
