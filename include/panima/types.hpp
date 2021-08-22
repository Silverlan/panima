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
	using AnimationChannelId = uint16_t;
	class Animation;
	class Channel;
	class Manager;
	class Player;
	class Pose;
	struct Slice;
	using BoneId = uint16_t;
};

#endif
