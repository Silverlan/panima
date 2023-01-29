/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PANIMA_SLICE_HPP__
#define __PANIMA_SLICE_HPP__

#include <vector>
#include <iostream>
#include <udm_types.hpp>

namespace panima {
	struct Slice {
		Slice() = default;
		Slice(const Slice &) = default;
		Slice(Slice &&other) = default;
		Slice &operator=(const Slice &) = default;
		Slice &operator=(Slice &&) = default;
		std::vector<udm::PProperty> channelValues;
	};
};

#endif
