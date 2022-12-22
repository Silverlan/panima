/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PANIMA_BONE_HPP__
#define __PANIMA_BONE_HPP__

#include "panima/types.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <optional>
#include <sharedutils/util_path.hpp>

namespace panima
{
	struct Bone
		: public std::enable_shared_from_this<Bone>
	{
		Bone();
		Bone(const Bone &other); // Parent has to be updated by caller!
		std::string name;
		std::unordered_map<uint32_t,std::shared_ptr<Bone>> children;
		std::weak_ptr<Bone> parent;
		BoneId ID;

		bool IsAncestorOf(const Bone &other) const;
		bool IsDescendantOf(const Bone &other) const;

		bool operator==(const Bone &other) const;
		bool operator!=(const Bone &other) const {return !operator==(other);}
	};
};

std::ostream &operator<<(std::ostream &out,const panima::Bone &o);

#endif
