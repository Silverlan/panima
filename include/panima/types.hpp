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
#include <limits>
#include <sharedutils/magic_enum.hpp>
#include <udm_types.hpp>
#include <udm_trivial_types.hpp>

namespace panima {
	enum class PlaybackFlags : uint32_t {
		None = 0u,
		ResetBit = 1u,
		LoopBit = ResetBit << 1u,

		Default = None
	};

	enum class ChannelInterpolation : uint8_t { Linear = 0, Step, CubicSpline };

	struct TimeFrame {
		float startOffset = 0.f;
		float scale = 1.f;
		float duration = -1.f;
	};

	using AnimationId = uint32_t;
	constexpr auto INVALID_ANIMATION = std::numeric_limits<AnimationId>::max();
	using AnimationChannelId = uint16_t;
	class Animation;
	struct Channel;
	class Manager;
	class Player;
	struct Slice;

	using PPlayer = std::shared_ptr<Player>;

	class AnimationManager;
	using PAnimationManager = std::shared_ptr<AnimationManager>;

	class AnimationSet;
	using PAnimationSet = std::shared_ptr<AnimationSet>;
	struct ChannelValueSubmitter {
		ChannelValueSubmitter() = default;
		ChannelValueSubmitter(const std::function<void(Channel &, uint32_t &, double)> &submitter);
		std::function<void(Channel &, uint32_t &, double)> submitter;
		bool enabled = true;
		operator bool() const;
		bool operator==(const std::nullptr_t &t) const;
		bool operator!=(const std::nullptr_t &t) const;
		void operator()(Channel &channel, uint32_t &timestampIndex, double t);
	};

	struct ChannelPath;

	constexpr bool is_animatable_type(udm::Type type) { return !udm::is_non_trivial_type(type) && type != udm::Type::HdrColor && type != udm::Type::Srgba && type != udm::Type::Transform && type != udm::Type::ScaledTransform && type != udm::Type::Nil && type != udm::Type::Half; }
};

#endif
