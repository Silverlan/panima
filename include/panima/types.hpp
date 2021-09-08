/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PANIMA_TYPES_HPP__
#define __PANIMA_TYPES_HPP__

#include <vector>
#include <iostream>
#include <functional>
#include <udm_types.hpp>

namespace panima
{
	enum class PlaybackFlags : uint32_t
	{
		None = 0u,
		ResetBit = 1u,
		LoopBit = ResetBit<<1u,

		Default = None
	};
	
	using AnimationId = uint32_t;
	constexpr auto INVALID_ANIMATION = std::numeric_limits<AnimationId>::max();
	using AnimationChannelId = uint16_t;
	class Animation;
	struct Channel;
	class Manager;
	class Player;
	class Pose;
	struct Slice;
	using BoneId = uint16_t;
	struct Bone;
	class Skeleton;

	using PPlayer = std::shared_ptr<Player>;

	class AnimationManager;
	using PAnimationManager = std::shared_ptr<AnimationManager>;

	class AnimationSet;
	using PAnimationSet = std::shared_ptr<AnimationSet>;
	using ChannelValueSubmitter = std::function<void(Channel&,uint32_t&,double)>;

	struct ChannelPath;
};

#endif
