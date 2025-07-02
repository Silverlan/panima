// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <vector>
#include <iostream>
#include <udm_types.hpp>

export module panima:slice;

export namespace panima {
	struct Slice {
		Slice() = default;
		Slice(const Slice &) = default;
		Slice(Slice &&other) = default;
		Slice &operator=(const Slice &) = default;
		Slice &operator=(Slice &&) = default;
		std::vector<udm::PProperty> channelValues;
	};
};
