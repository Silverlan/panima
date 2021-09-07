/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PANIMA_PLAYER_HPP__
#define __PANIMA_PLAYER_HPP__

#include "slice.hpp"
#include "types.hpp"
#include <mathutil/umath.h>
#include <udm_types.hpp>
#include <vector>
#include <memory>

#undef GetCurrentTime
// #define PRAGMA_ENABLE_ANIMATION_SYSTEM_2

class Model;
namespace panima
{
	class Animation;
	class Player
		: public std::enable_shared_from_this<Player>
	{
	public:
		enum class StateFlags : uint32_t
		{
			None = 0u,
			Looping = 1u,
			AnimationDirty = Looping<<1u
		};
		static std::shared_ptr<Player> Create();
		static std::shared_ptr<Player> Create(const Player &other);
		static std::shared_ptr<Player> Create(Player &&other);
		bool Advance(float dt,bool force=false);

		float GetDuration() const;
		float GetRemainingAnimationDuration() const;
		float GetCurrentTimeFraction() const;
		float GetCurrentTime() const {return m_currentTime;}
		void SetCurrentTimeFraction(float t,bool updateAnimation=false);
		float GetPlaybackRate() const {return m_playbackRate;}
		void SetPlaybackRate(float playbackRate) {m_playbackRate = playbackRate;}
		void SetCurrentTime(float t,bool updateAnimation=false);
		
		Slice &GetCurrentSlice() {return m_currentSlice;}
		const Slice &GetCurrentSlice() const {return const_cast<Player*>(this)->GetCurrentSlice();}

		void SetLooping(bool looping);
		bool IsLooping() const;
		
		void SetAnimationDirty();
		void SetAnimation(const Animation &animation);
		void Reset();

		const Animation *GetAnimation() const {return m_animation.get();}
		uint32_t &GetLastChannelTimestampIndex(AnimationChannelId channelId) {return m_lastChannelTimestampIndices[channelId];}

		Player &operator=(const Player &other);
		Player &operator=(Player &&other);

		bool operator==(const Player &other) const {return this == &other;}
		bool operator!=(const Player &other) const {return !operator==(other);}
	private:
		Player();
		Player(const Player &other);
		Player(Player &&other);
		static void ApplySliceInterpolation(const Slice &src,Slice &dst,float f);
		std::shared_ptr<const Animation> m_animation = nullptr;
		Slice m_currentSlice;
		float m_playbackRate = 1.f;
		float m_currentTime = 0.f;
		StateFlags m_stateFlags = StateFlags::None;

		std::vector<uint32_t> m_lastChannelTimestampIndices;
	};
	using PPlayer = std::shared_ptr<Player>;
};
REGISTER_BASIC_BITWISE_OPERATORS(panima::Player::StateFlags)

std::ostream &operator<<(std::ostream &out,const panima::Player &o);
std::ostream &operator<<(std::ostream &out,const panima::Slice &o);

#endif
