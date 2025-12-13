// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module panima:animation_manager;

import :animation_set;
import :slice;
import :player;

export namespace panima {
	struct AnimationPlayerCallbackInterface {
		std::function<bool(const AnimationSet &, AnimationId, PlaybackFlags)> onPlayAnimation = nullptr;
		std::function<void()> onStopAnimation = nullptr;
		std::function<void(const AnimationSet &, AnimationId &, PlaybackFlags &)> translateAnimation = nullptr;
	};
	class AnimationManager : public std::enable_shared_from_this<AnimationManager> {
	  public:
		using AnimationSetIndex = uint32_t;
		using AnimationReference = std::pair<AnimationSetIndex, AnimationId>;
		static constexpr auto INVALID_ANIMATION_SET_INDEX = std::numeric_limits<AnimationSetIndex>::max();
		static constexpr auto INVALID_ANIMATION_INDEX = INVALID_ANIMATION;
		static constexpr auto INVALID_ANIMATION_REFERENCE = AnimationReference {INVALID_ANIMATION_SET_INDEX, INVALID_ANIMATION_INDEX};
		static std::shared_ptr<AnimationManager> Create(const AnimationManager &other);
		static std::shared_ptr<AnimationManager> Create(AnimationManager &&other);
		static std::shared_ptr<AnimationManager> Create();

		AnimationId GetCurrentAnimationId() const { return m_currentAnimation; }
		Animation *GetCurrentAnimation() const;

		void PlayAnimation(const std::string &setName, AnimationId animation, PlaybackFlags flags = PlaybackFlags::Default);
		void PlayAnimation(const std::string &setName, const std::string &animation, PlaybackFlags flags = PlaybackFlags::Default);
		void PlayAnimation(const std::string &animation, PlaybackFlags flags = PlaybackFlags::Default);
		void StopAnimation();

		Slice &GetPreviousSlice() { return m_prevAnimSlice; }
		const Slice &GetPreviousSlice() const { return const_cast<AnimationManager *>(this)->GetPreviousSlice(); }

		AnimationManager &operator=(const AnimationManager &other);
		AnimationManager &operator=(AnimationManager &&other);

		Player *operator->() { return m_player.get(); }
		const Player *operator->() const { return const_cast<AnimationManager *>(this)->operator->(); }

		Player &operator*() { return *m_player; }
		const Player &operator*() const { return const_cast<AnimationManager *>(this)->operator*(); }

		Player &GetPlayer() { return *m_player; }
		const Player &GetPlayer() const { return const_cast<AnimationManager *>(this)->GetPlayer(); }

		std::vector<ChannelValueSubmitter> &GetChannelValueSubmitters() { return m_channelValueSubmitters; }
		const std::vector<ChannelValueSubmitter> &GetChannelValueSubmitters() const { return const_cast<AnimationManager *>(this)->GetChannelValueSubmitters(); }

		void AddAnimationSet(std::string name, AnimationSet &animSet);
		const std::vector<PAnimationSet> &GetAnimationSets() const { return m_animationSets; }
		void RemoveAnimationSet(const std::string_view &name);

		AnimationSet *GetAnimationSet(AnimationSetIndex idx);
		const AnimationSet *GetAnimationSet(AnimationSetIndex idx) const { return const_cast<AnimationManager *>(this)->GetAnimationSet(idx); }
		AnimationSet *FindAnimationSet(const std::string &name);
		const AnimationSet *FindAnimationSet(const std::string &name) const { return const_cast<AnimationManager *>(this)->FindAnimationSet(name); }

		void SetCallbackInterface(const AnimationPlayerCallbackInterface &i) { m_callbackInterface = i; }

		int32_t GetPriority() const { return m_priority; }

		bool operator==(const AnimationManager &other) const { return this == &other; }
		bool operator!=(const AnimationManager &other) const { return !operator==(other); }
	  private:
		std::optional<AnimationSetIndex> FindAnimationSetIndex(const std::string &name) const;
		AnimationReference FindAnimation(AnimationSetIndex animSetIndex, AnimationId animation, PlaybackFlags flags) const;
		AnimationReference FindAnimation(AnimationSetIndex animSetIndex, const std::string &animation, PlaybackFlags flags) const;
		AnimationReference FindAnimation(const std::string &setName, AnimationId animation, PlaybackFlags flags) const;
		AnimationReference FindAnimation(const std::string &setName, const std::string &animation, PlaybackFlags flags) const;
		AnimationReference FindAnimation(const std::string &animation, PlaybackFlags flags) const;
		void PlayAnimation(AnimationSetIndex animSetIndex, AnimationId animIdx, PlaybackFlags flags = PlaybackFlags::Default);
		AnimationManager(const AnimationManager &other);
		AnimationManager(AnimationManager &&other);
		AnimationManager();
		static void ApplySliceInterpolation(const Slice &src, Slice &dst, float f);
		PPlayer m_player = nullptr;

		int32_t m_priority = 0;

		std::vector<PAnimationSet> m_animationSets;
		pragma::util::StringMap<AnimationSetIndex> m_setNameToIndex;

		std::weak_ptr<AnimationSet> m_currentAnimationSet {};
		AnimationId m_currentAnimation = std::numeric_limits<AnimationId>::max();

		PlaybackFlags m_currentFlags = PlaybackFlags::None;
		std::vector<ChannelValueSubmitter> m_channelValueSubmitters {};

		Slice m_prevAnimSlice;
		mutable AnimationPlayerCallbackInterface m_callbackInterface {};
	};
	using PAnimationManager = std::shared_ptr<AnimationManager>;
	using namespace pragma::math::scoped_enum::bitwise;
};

export {
	REGISTER_ENUM_FLAGS(panima::PlaybackFlags)

	std::ostream &operator<<(std::ostream &out, const panima::AnimationManager &o);
};
