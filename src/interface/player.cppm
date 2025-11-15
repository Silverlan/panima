// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#undef GetCurrentTime
// #define PRAGMA_ENABLE_ANIMATION_SYSTEM_2

export module panima:player;

import :slice;
import :types;
import :animation;

export namespace panima {
	class Player : public std::enable_shared_from_this<Player> {
	  public:
		enum class StateFlags : uint32_t { None = 0u, Looping = 1u, AnimationDirty = Looping << 1u };
		static std::shared_ptr<Player> Create();
		static std::shared_ptr<Player> Create(const Player &other);
		static std::shared_ptr<Player> Create(Player &&other);
		bool Advance(float dt, bool force = false);

		float GetDuration() const;
		float GetRemainingAnimationDuration() const;
		float GetCurrentTimeFraction() const;
		float GetCurrentTime() const { return m_currentTime; }
		void SetCurrentTimeFraction(float t, bool updateAnimation = false);
		float GetPlaybackRate() const { return m_playbackRate; }
		void SetPlaybackRate(float playbackRate) { m_playbackRate = playbackRate; }
		void SetCurrentTime(float t, bool updateAnimation = false);

		Slice &GetCurrentSlice() { return m_currentSlice; }
		const Slice &GetCurrentSlice() const { return const_cast<Player *>(this)->GetCurrentSlice(); }

		void SetLooping(bool looping);
		bool IsLooping() const;

		void SetAnimationDirty();
		void SetAnimation(const Animation &animation);
		void Reset();

		const Animation *GetAnimation() const { return m_animation.get(); }
		uint32_t &GetLastChannelTimestampIndex(AnimationChannelId channelId) { return m_lastChannelTimestampIndices[channelId]; }

		Player &operator=(const Player &other);
		Player &operator=(Player &&other);

		bool operator==(const Player &other) const { return this == &other; }
		bool operator!=(const Player &other) const { return !operator==(other); }
	  private:
		Player();
		Player(const Player &other);
		Player(Player &&other);
		static void ApplySliceInterpolation(const Slice &src, Slice &dst, float f);
		std::shared_ptr<const Animation> m_animation = nullptr;
		Slice m_currentSlice;
		float m_playbackRate = 1.f;
		float m_currentTime = 0.f;
		StateFlags m_stateFlags = StateFlags::None;

		std::vector<uint32_t> m_lastChannelTimestampIndices;
	};
	using PPlayer = std::shared_ptr<Player>;
	using namespace umath::scoped_enum::bitwise;
};

export {
	namespace umath::scoped_enum::bitwise {
		template<>
		struct enable_bitwise_operators<panima::Player::StateFlags> : std::true_type {};
	}

	std::ostream &operator<<(std::ostream &out, const panima::Player &o);
	std::ostream &operator<<(std::ostream &out, const panima::Slice &o);
};
