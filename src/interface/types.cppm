// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <vector>
#include <iostream>
#include <functional>
#include <limits>
#include <sharedutils/magic_enum.hpp>
#include <udm_types.hpp>
#include <udm_trivial_types.hpp>

export module panima:types;

export namespace panima {
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

	constexpr bool is_animatable_type(udm::Type type) { return !udm::is_non_trivial_type(type) && type != udm::Type::HdrColor && type != udm::Type::Srgba && type != udm::Type::Transform && type != udm::Type::ScaledTransform && type != udm::Type::Nil && type != udm::Type::Half; }

	template<typename T>
	T make_value()
	{
		// We can't use the default empty constructor for Vector2 and Vector4 due to
		// a clang compiler bug (?), so we'll construct them explicitely.
		if constexpr(std::is_same_v<T,Vector2>)
			return Vector2 {0.f, 0.f};
		else if constexpr(std::is_same_v<T,Vector4>)
			return Vector4 {0.f, 0.f, 0.f, 0.f};
		else
			return T{};
	}
};
