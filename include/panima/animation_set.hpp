/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PANIMA_ANIMATION_SET_HPP__
#define __PANIMA_ANIMATION_SET_HPP__

#include "panima/types.hpp"
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <sharedutils/util_string_hash.hpp>

namespace panima
{
	class AnimationSet
		: public std::enable_shared_from_this<AnimationSet>
	{
	public:
		static std::shared_ptr<AnimationSet> Create();
		void Clear();
		void AddAnimation(Animation &anim);
		void RemoveAnimation(const std::string_view &animName);
		void RemoveAnimation(const Animation &anim);
		void RemoveAnimation(AnimationId id);
		std::optional<AnimationId> LookupAnimation(const std::string_view &animName) const;
		Animation *GetAnimation(AnimationId id);
		const Animation *GetAnimation(AnimationId id) const {return const_cast<AnimationSet*>(this)->GetAnimation(id);}

		std::vector<std::shared_ptr<Animation>> &GetAnimations() {return m_animations;}
		const std::vector<std::shared_ptr<Animation>> &GetAnimations() const {return const_cast<AnimationSet*>(this)->GetAnimations();}

		Animation *FindAnimation(const std::string_view &animName);
		const Animation *FindAnimation(const std::string_view &animName) const {return const_cast<AnimationSet*>(this)->FindAnimation(animName);}

		void Reserve(uint32_t count);
		uint32_t GetSize() const;

		bool operator==(const AnimationSet &other) const {return this == &other;}
		bool operator!=(const AnimationSet &other) const {return !operator==(other);}
	private:
		AnimationSet();
		std::vector<std::shared_ptr<Animation>> m_animations;
		std::unordered_map<size_t,size_t> m_nameToId;
	};
};

std::ostream &operator<<(std::ostream &out,const panima::AnimationSet &o);

#endif
