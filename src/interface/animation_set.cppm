// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <sharedutils/util_string_hash.hpp>

export module panima:animation_set;

import :animation;
import :types;

export namespace panima {
	class AnimationSet : public std::enable_shared_from_this<AnimationSet> {
	  public:
		static std::shared_ptr<AnimationSet> Create();
		void Clear();
		void AddAnimation(Animation &anim);
		void RemoveAnimation(const std::string_view &animName);
		void RemoveAnimation(const Animation &anim);
		void RemoveAnimation(AnimationId id);
		std::optional<AnimationId> LookupAnimation(const std::string_view &animName) const;
		Animation *GetAnimation(AnimationId id);
		const Animation *GetAnimation(AnimationId id) const { return const_cast<AnimationSet *>(this)->GetAnimation(id); }

		std::vector<std::shared_ptr<Animation>> &GetAnimations() { return m_animations; }
		const std::vector<std::shared_ptr<Animation>> &GetAnimations() const { return const_cast<AnimationSet *>(this)->GetAnimations(); }

		Animation *FindAnimation(const std::string_view &animName);
		const Animation *FindAnimation(const std::string_view &animName) const { return const_cast<AnimationSet *>(this)->FindAnimation(animName); }

		void Reserve(uint32_t count);
		uint32_t GetSize() const;

		bool operator==(const AnimationSet &other) const { return this == &other; }
		bool operator!=(const AnimationSet &other) const { return !operator==(other); }
	  private:
		AnimationSet();
		std::vector<std::shared_ptr<Animation>> m_animations;
		std::unordered_map<size_t, size_t> m_nameToId;
	};
	using PAnimationSet = std::shared_ptr<AnimationSet>;
};

export { std::ostream &operator<<(std::ostream &out, const panima::AnimationSet &o); };
