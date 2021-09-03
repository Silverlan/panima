/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PANIMA_ANIMATION_MANAGER_HPP__
#define __PANIMA_ANIMATION_MANAGER_HPP__

#include "panima/types.hpp"
#include "panima/slice.hpp"
#include <sharedutils/util_string_hash.hpp>
#include <udm.hpp>
#include <vector>
#include <memory>

namespace panima
{
	struct AnimationPlayerCallbackInterface
	{
		std::function<bool(const panima::AnimationSet&,panima::AnimationId,PlaybackFlags)> onPlayAnimation = nullptr;
		std::function<void()> onStopAnimation = nullptr;
		std::function<void(const panima::AnimationSet&,panima::AnimationId&,PlaybackFlags&)> translateAnimation = nullptr;
	};
	class AnimationManager
		: public std::enable_shared_from_this<AnimationManager>
	{
	public:
		using AnimationSetIndex = uint32_t;
		using AnimationReference = std::pair<AnimationSetIndex,panima::AnimationId>;
		static constexpr auto INVALID_ANIMATION_SET_INDEX = std::numeric_limits<AnimationSetIndex>::max();
		static constexpr auto INVALID_ANIMATION_INDEX = panima::INVALID_ANIMATION;
		static constexpr auto INVALID_ANIMATION_REFERENCE = AnimationReference{INVALID_ANIMATION_SET_INDEX,INVALID_ANIMATION_INDEX};
		static std::shared_ptr<AnimationManager> Create(const AnimationManager &other);
		static std::shared_ptr<AnimationManager> Create(AnimationManager &&other);
		static std::shared_ptr<AnimationManager> Create();
		
		panima::AnimationId GetCurrentAnimationId() const {return m_currentAnimation;}
		panima::Animation *GetCurrentAnimation() const;

		void PlayAnimation(const std::string &setName,panima::AnimationId animation,PlaybackFlags flags=PlaybackFlags::Default);
		void PlayAnimation(const std::string &setName,const std::string &animation,PlaybackFlags flags=PlaybackFlags::Default);
		void PlayAnimation(const std::string &animation,PlaybackFlags flags=PlaybackFlags::Default);
		void StopAnimation();

		panima::Slice &GetPreviousSlice() {return m_prevAnimSlice;}
		const panima::Slice &GetPreviousSlice() const {return const_cast<AnimationManager*>(this)->GetPreviousSlice();}

		AnimationManager &operator=(const AnimationManager &other);
		AnimationManager &operator=(AnimationManager &&other);

		panima::Player *operator->() {return m_player.get();}
		const panima::Player *operator->() const {return const_cast<AnimationManager*>(this)->operator->();}

		panima::Player &operator*() {return *m_player;}
		const panima::Player &operator*() const {return const_cast<AnimationManager*>(this)->operator*();}

		panima::Player &GetPlayer() {return *m_player;}
		const panima::Player &GetPlayer() const {return const_cast<AnimationManager*>(this)->GetPlayer();}

		std::vector<ChannelValueSubmitter> &GetChannelValueSubmitters() {return m_channelValueSubmitters;}

		void AddAnimationSet(std::string name,panima::AnimationSet &animSet);
		const std::vector<panima::PAnimationSet> &GetAnimationSets() const {return m_animationSets;}
		void RemoveAnimationSet(const std::string_view &name);

		panima::AnimationSet *GetAnimationSet(AnimationSetIndex idx);
		const panima::AnimationSet *GetAnimationSet(AnimationSetIndex idx) const {return const_cast<AnimationManager*>(this)->GetAnimationSet(idx);}
		panima::AnimationSet *FindAnimationSet(const std::string &name);
		const panima::AnimationSet *FindAnimationSet(const std::string &name) const {return const_cast<AnimationManager*>(this)->FindAnimationSet(name);}

		void SetCallbackInterface(const AnimationPlayerCallbackInterface &i) {m_callbackInterface = i;}
	private:
		std::optional<AnimationSetIndex> FindAnimationSetIndex(const std::string &name) const;
		AnimationReference FindAnimation(AnimationSetIndex animSetIndex,panima::AnimationId animation,PlaybackFlags flags) const;
		AnimationReference FindAnimation(AnimationSetIndex animSetIndex,const std::string &animation,PlaybackFlags flags) const;
		AnimationReference FindAnimation(const std::string &setName,panima::AnimationId animation,PlaybackFlags flags) const;
		AnimationReference FindAnimation(const std::string &setName,const std::string &animation,PlaybackFlags flags) const;
		AnimationReference FindAnimation(const std::string &animation,PlaybackFlags flags) const;
		void PlayAnimation(AnimationSetIndex animSetIndex,panima::AnimationId animIdx,PlaybackFlags flags=PlaybackFlags::Default);
		AnimationManager(const AnimationManager &other);
		AnimationManager(AnimationManager &&other);
		AnimationManager();
		static void ApplySliceInterpolation(const panima::Slice &src,panima::Slice &dst,float f);
		panima::PPlayer m_player = nullptr;

		std::vector<panima::PAnimationSet> m_animationSets;
		util::StringMap<AnimationSetIndex> m_setNameToIndex;

		std::weak_ptr<panima::AnimationSet> m_currentAnimationSet {};
		panima::AnimationId m_currentAnimation = std::numeric_limits<panima::AnimationId>::max();

		PlaybackFlags m_currentFlags = PlaybackFlags::None;
		std::vector<ChannelValueSubmitter> m_channelValueSubmitters {};
		
		panima::Slice m_prevAnimSlice;
		mutable AnimationPlayerCallbackInterface m_callbackInterface {};
	};
	using PAnimationManager = std::shared_ptr<AnimationManager>;
};
REGISTER_BASIC_BITWISE_OPERATORS(panima::PlaybackFlags)

std::ostream &operator<<(std::ostream &out,const panima::AnimationManager &o);

#endif
