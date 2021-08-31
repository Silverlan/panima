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
#include <udm_types.hpp>

namespace panima
{
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
};

#endif
