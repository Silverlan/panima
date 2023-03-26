/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "panima/channel.hpp"
#include "panima/channel_t.hpp"
#include "value_expression.hpp"
#include <udm.hpp>
#include <sharedutils/util_uri.hpp>

panima::ChannelPath::ChannelPath(const std::string &ppath)
{
	uriparser::Uri uri {ppath};
	auto scheme = uri.scheme();
	if(!scheme.empty() && scheme != "panima")
		return; // Invalid panima URI
	auto strPath = uri.path();
	if(!strPath.empty() && strPath.front() == '/')
		strPath.erase(strPath.begin());
	path = std::move(strPath);
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
		}
	}
}
bool panima::ChannelPath::operator==(const ChannelPath &other) const { return path == other.path && ((!m_components && !other.m_components) || (m_components && other.m_components && *m_components == *other.m_components)); }
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
const panima::ArrayFloatIterator::reference panima::ArrayFloatIterator::operator*() const { return const_cast<ArrayFloatIterator *>(this)->operator*(); }
panima::ArrayFloatIterator::pointer panima::ArrayFloatIterator::operator->() { return m_data; }
const panima::ArrayFloatIterator::pointer panima::ArrayFloatIterator::operator->() const { return const_cast<ArrayFloatIterator *>(this)->operator->(); }
bool panima::ArrayFloatIterator::operator==(const ArrayFloatIterator &other) const { return m_data == other.m_data; }
bool panima::ArrayFloatIterator::operator!=(const ArrayFloatIterator &other) const { return !operator==(other); }

panima::ArrayFloatIterator panima::begin(const udm::Array &a) { return ArrayFloatIterator {static_cast<float *>(&const_cast<udm::Array &>(a).GetValue<float>(0))}; }
panima::ArrayFloatIterator panima::end(const udm::Array &a) { return ArrayFloatIterator {static_cast<float *>(&const_cast<udm::Array &>(a).GetValue<float>(0)) + a.GetSize()}; }

////////////////

panima::Channel::Channel() : m_times {::udm::Property::Create(udm::Type::ArrayLz4)}, m_values {::udm::Property::Create(udm::Type::ArrayLz4)} { GetTimesArray().SetValueType(udm::Type::Float); }
panima::Channel::Channel(const udm::PProperty &times, const udm::PProperty &values) : m_times {times}, m_values {values} {}
panima::Channel::~Channel() {}
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
}
void panima::Channel::Update()
{
	m_effectiveTimeFrame = m_timeFrame;
	if(m_effectiveTimeFrame.duration < 0.f)
		m_effectiveTimeFrame.duration = GetMaxTime();
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
bool panima::Channel::ClearRange(float startTime, float endTime)
{
	if(GetTimesArray().IsEmpty())
		return true;
	float t;
	auto indicesStart = FindInterpolationIndices(startTime, t);
	auto indicesEnd = FindInterpolationIndices(endTime, t);
	auto startIdx = indicesStart.second;
	auto endIdx = indicesEnd.first;
	if(startIdx == std::numeric_limits<uint32_t>::max() || endIdx == std::numeric_limits<uint32_t>::max() || endIdx < startIdx)
		return false;
	udm::visit_ng(GetValueType(), [this, startTime, endTime, startIdx, endIdx](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(is_animatable_type(udm::type_to_enum<T>())) {
			auto startVal = GetInterpolatedValue<T>(startTime);
			auto endVal = GetInterpolatedValue<T>(endTime);

			auto &times = GetTimesArray();
			auto &values = GetValueArray();
			times.RemoveValueRange(startIdx, (endIdx - startIdx) + 1);
			values.RemoveValueRange(startIdx, (endIdx - startIdx) + 1);

			AddValue<T>(startTime, startVal);
			AddValue<T>(endTime, endVal);
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
			return idx;
		}
		// New time value exceeds last time value in time array, push back
		auto idx = indices.second + 1;
		times.InsertValue(idx, t);
		udm::visit_ng(GetValueType(), [&values, idx, value](auto tag) {
			using T = typename decltype(tag)::type;
			values.InsertValue(idx, *static_cast<const T *>(value));
		});
		return idx;
	}
	// Insert new value between the two indices
	auto idx = indices.second;
	times.InsertValue(idx, t);
	udm::visit_ng(GetValueType(), [&values, idx, value](auto tag) {
		using T = typename decltype(tag)::type;
		values.InsertValue(idx, *static_cast<const T *>(value));
	});
	return idx;
}
udm::Array &panima::Channel::GetTimesArray() { return m_times->GetValue<udm::Array>(); }
udm::Array &panima::Channel::GetValueArray() { return m_values->GetValue<udm::Array>(); }
udm::Type panima::Channel::GetValueType() const { return GetValueArray().GetValueType(); }
void panima::Channel::SetValueType(udm::Type type) { GetValueArray().SetValueType(type); }
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
	if(times.GetSize() == 0) {
		interpFactor = 0.f;
		return {std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max()};
	}
	// Binary search
	TimeToLocalTimeFrame(t);
	auto it = std::upper_bound(begin(times), end(times), t);
	if(it == end(times)) {
		interpFactor = 0.f;
		return {static_cast<uint32_t>(times.GetSize() - 1), static_cast<uint32_t>(times.GetSize() - 1)};
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
