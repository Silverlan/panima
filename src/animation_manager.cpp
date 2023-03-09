/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "panima/animation_manager.hpp"
#include "panima/animation_set.hpp"
#include "panima/animation.hpp"
#include "panima/player.hpp"
//#include <type_traits>

std::shared_ptr<panima::AnimationManager> panima::AnimationManager::Create(const AnimationManager &other) { return std::shared_ptr<AnimationManager> {new AnimationManager {other}}; }
std::shared_ptr<panima::AnimationManager> panima::AnimationManager::Create(AnimationManager &&other) { return std::shared_ptr<AnimationManager> {new AnimationManager {std::move(other)}}; }
std::shared_ptr<panima::AnimationManager> panima::AnimationManager::Create() { return std::shared_ptr<AnimationManager> {new AnimationManager {}}; }
panima::AnimationManager::AnimationManager(const AnimationManager &other)
    : m_player {panima::Player::Create(*other.m_player)}, m_animationSets {other.m_animationSets}, m_currentAnimation {other.m_currentAnimation}, m_setNameToIndex {other.m_setNameToIndex}, m_currentAnimationSet {other.m_currentAnimationSet}, m_prevAnimSlice {other.m_prevAnimSlice}
/*,m_channelValueSubmitters{m_channelValueSubmitters}*/
{
#ifdef _MSC_VER
	static_assert(sizeof(*this) == 384, "Update this implementation when class has changed!");
#endif
}
panima::AnimationManager::AnimationManager(AnimationManager &&other)
    : m_player {panima::Player::Create(*other.m_player)}, m_animationSets {std::move(other.m_animationSets)}, m_currentAnimation {other.m_currentAnimation}, m_setNameToIndex {std::move(other.m_setNameToIndex)}, m_currentAnimationSet {other.m_currentAnimationSet},
      m_prevAnimSlice {std::move(other.m_prevAnimSlice)} /*,m_channelValueSubmitters{std::move(m_channelValueSubmitters)}*/
{
#ifdef _MSC_VER
	static_assert(sizeof(*this) == 384, "Update this implementation when class has changed!");
#endif
}
panima::AnimationManager::AnimationManager() : m_player {panima::Player::Create()} {}
panima::AnimationManager &panima::AnimationManager::operator=(const AnimationManager &other)
{
	m_player = panima::Player::Create(*other.m_player);
	m_animationSets = other.m_animationSets;
	m_currentAnimation = other.m_currentAnimation;
	m_currentAnimationSet = other.m_currentAnimationSet;
	m_setNameToIndex = other.m_setNameToIndex;

	m_prevAnimSlice = other.m_prevAnimSlice;
	// m_channelValueSubmitters = other.m_channelValueSubmitters;
#ifdef _MSC_VER
	static_assert(sizeof(*this) == 384, "Update this implementation when class has changed!");
#endif
	return *this;
}
panima::AnimationManager &panima::AnimationManager::operator=(AnimationManager &&other)
{
	m_player = panima::Player::Create(*other.m_player);
	m_animationSets = std::move(other.m_animationSets);
	m_currentAnimation = other.m_currentAnimation;
	m_currentAnimationSet = other.m_currentAnimationSet;
	m_setNameToIndex = std::move(other.m_setNameToIndex);

	m_prevAnimSlice = std::move(other.m_prevAnimSlice);
	// m_channelValueSubmitters = std::move(other.m_channelValueSubmitters);

#ifdef _MSC_VER
	static_assert(sizeof(*this) == 384, "Update this implementation when class has changed!");
#endif
	return *this;
}

panima::Animation *panima::AnimationManager::GetCurrentAnimation() const { return const_cast<panima::Animation *>(m_player->GetAnimation()); }

void panima::AnimationManager::RemoveAnimationSet(const std::string_view &name)
{
	auto it = m_setNameToIndex.find(name);
	if(it == m_setNameToIndex.end())
		return;
	auto idx = it->second;
	auto &set = m_animationSets[idx];
	if(m_currentAnimationSet.lock().get() == set.get())
		StopAnimation();
	m_animationSets.erase(m_animationSets.begin() + idx);
	m_setNameToIndex.erase(it);
}
void panima::AnimationManager::AddAnimationSet(std::string name, panima::AnimationSet &animSet)
{
	RemoveAnimationSet(name);
	m_animationSets.push_back(animSet.shared_from_this());
	m_setNameToIndex[name] = m_animationSets.size() - 1;
}

void panima::AnimationManager::PlayAnimation(const std::string &animation, PlaybackFlags flags)
{
	auto p = FindAnimation(animation, flags);
	if(p.first == INVALID_ANIMATION_SET_INDEX) {
		StopAnimation();
		return;
	}
	PlayAnimation(p.first, p.second, flags);
}

panima::AnimationSet *panima::AnimationManager::GetAnimationSet(AnimationSetIndex idx)
{
	if(idx >= m_animationSets.size())
		return nullptr;
	return m_animationSets[idx].get();
}

panima::AnimationSet *panima::AnimationManager::FindAnimationSet(const std::string &name)
{
	auto it = m_setNameToIndex.find(name);
	if(it == m_setNameToIndex.end())
		return nullptr;
	return m_animationSets[it->second].get();
}

panima::AnimationManager::AnimationReference panima::AnimationManager::FindAnimation(AnimationSetIndex animSetIndex, panima::AnimationId animation, PlaybackFlags flags) const
{
	auto *set = GetAnimationSet(animSetIndex);
	if(!set)
		return INVALID_ANIMATION_REFERENCE;
	if(m_callbackInterface.translateAnimation)
		m_callbackInterface.translateAnimation(*set, animation, flags);
	return {animSetIndex, animation};
}
panima::AnimationManager::AnimationReference panima::AnimationManager::FindAnimation(AnimationSetIndex animSetIndex, const std::string &animation, PlaybackFlags flags) const
{
	auto *set = GetAnimationSet(animSetIndex);
	if(!set)
		return INVALID_ANIMATION_REFERENCE;
	auto id = set->LookupAnimation(animation);
	if(!id.has_value())
		return INVALID_ANIMATION_REFERENCE;
	return FindAnimation(animSetIndex, *id, flags);
}
std::optional<panima::AnimationManager::AnimationSetIndex> panima::AnimationManager::FindAnimationSetIndex(const std::string &name) const
{
	auto it = m_setNameToIndex.find(name);
	if(it == m_setNameToIndex.end())
		return {};
	return it->second;
}
panima::AnimationManager::AnimationReference panima::AnimationManager::FindAnimation(const std::string &setName, panima::AnimationId animation, PlaybackFlags flags) const
{
	auto setIdx = FindAnimationSetIndex(setName);
	if(!setIdx.has_value())
		return INVALID_ANIMATION_REFERENCE;
	return FindAnimation(*setIdx, animation, flags);
}
panima::AnimationManager::AnimationReference panima::AnimationManager::FindAnimation(const std::string &setName, const std::string &animation, PlaybackFlags flags) const
{
	auto setIdx = FindAnimationSetIndex(setName);
	if(!setIdx.has_value())
		return INVALID_ANIMATION_REFERENCE;
	return FindAnimation(*setIdx, animation, flags);
}
panima::AnimationManager::AnimationReference panima::AnimationManager::FindAnimation(const std::string &animation, PlaybackFlags flags) const
{
	for(auto it = m_animationSets.begin(); it != m_animationSets.end(); ++it) {
		auto &set = **it;
		auto animId = set.LookupAnimation(animation);
		if(!animId.has_value())
			continue;
		return FindAnimation(it - m_animationSets.begin(), *animId, flags);
	}
	return INVALID_ANIMATION_REFERENCE;
}

void panima::AnimationManager::PlayAnimation(AnimationSetIndex animSetIndex, panima::AnimationId animIdx, PlaybackFlags flags)
{
	if(animSetIndex == INVALID_ANIMATION_SET_INDEX || animIdx == panima::INVALID_ANIMATION) {
		StopAnimation();
		return;
	}
	auto &set = m_animationSets[animSetIndex];
	auto reset = (flags & PlaybackFlags::ResetBit) != PlaybackFlags::None;
	if(!reset && set.get() == m_currentAnimationSet.lock().get() && m_currentAnimation == animIdx) {
		if(set->GetAnimation(animIdx)->HasFlags(panima::Animation::Flags::LoopBit))
			return;
	}

	if(!reset && (*this)->GetCurrentTime() == 0.f && m_currentFlags == flags)
		return;
	if(m_callbackInterface.onPlayAnimation && m_callbackInterface.onPlayAnimation(*set, animIdx, flags) == false)
		return;
	m_currentAnimationSet = set;
	m_currentAnimation = animIdx;
	(*this)->Reset();
	m_currentFlags = flags;
#ifdef PRAGMA_ENABLE_ANIMATION_SYSTEM_2
	auto &channels = anim->GetChannels();
	m_currentSlice.channelValues.resize(channels.size());
	m_lastChannelTimestampIndices.resize(channels.size(), 0u);
	for(auto i = decltype(channels.size()) {0u}; i < channels.size(); ++i) {
		auto &channel = channels[i];
		auto &sliceValue = m_currentSlice.channelValues[i];
		sliceValue = udm::Property::Create(channel->valueType);
	}
	SetCurrentTime(0.f, true);
#endif
}

void panima::AnimationManager::PlayAnimation(const std::string &setName, panima::AnimationId animation, PlaybackFlags flags)
{
	auto p = FindAnimation(setName, animation, flags);
	if(p.first == INVALID_ANIMATION_SET_INDEX) {
		StopAnimation();
		return;
	}
	PlayAnimation(p.first, p.second, flags);
}
void panima::AnimationManager::PlayAnimation(const std::string &setName, const std::string &animation, PlaybackFlags flags)
{
	auto p = FindAnimation(setName, animation, flags);
	if(p.first == INVALID_ANIMATION_SET_INDEX) {
		StopAnimation();
		return;
	}
	PlayAnimation(p.first, p.second, flags);
}
void panima::AnimationManager::StopAnimation()
{
	if(m_currentAnimation == panima::INVALID_ANIMATION)
		return;
	if(m_callbackInterface.onStopAnimation)
		m_callbackInterface.onStopAnimation();
	m_currentAnimation = panima::INVALID_ANIMATION;
	(*this)->Reset();
	m_currentFlags = PlaybackFlags::None;
}
void panima::AnimationManager::ApplySliceInterpolation(const panima::Slice &src, panima::Slice &dst, float f)
{
	// TODO
	/*if(f == 1.f)
		return;
	auto n = src.channelValues.size();
	for(auto i=decltype(n){0u};i<n;++i)
	{
		//auto &channel = m_channels[i];
		pragma::AnimationChannel &channel ;// TODO
		// TODO: How to translate channels?
		auto &srcVal = src.channelValues[i];
		auto &dstVal = dst.channelValues[i];
		// TODO
		auto interp = channel.GetInterpolationFunction<Vector3>();
		dstVal = interp(srcVal.GetValue<Vector3>(),dstVal.GetValue<Vector3>(),f);
	}*/
}

std::ostream &operator<<(std::ostream &out, const panima::AnimationManager &o)
{
	out << "AnimationManager";
	out << "[Player:" << *o << "]";
	return out;
}
