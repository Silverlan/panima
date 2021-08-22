/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "panima/bone.hpp"

panima::Bone::Bone()
	: parent(),ID(0)
{}

panima::Bone::Bone(const Bone &other)
	: ID(other.ID),name{other.name},parent{}
{
	for(auto &pair : other.children)
		children[pair.first] = std::make_shared<Bone>(*pair.second);
	static_assert(sizeof(Bone) == 136,"Update this function when making changes to this class!");
}

bool panima::Bone::IsAncestorOf(const Bone &other) const
{
	if(other.parent.expired())
		return false;
	auto *parent = other.parent.lock().get();
	return (parent == this) ? true : IsAncestorOf(*parent);
}
bool panima::Bone::IsDescendantOf(const Bone &other) const {return other.IsAncestorOf(*this);}
bool panima::Bone::operator==(const Bone &other) const
{
	static_assert(sizeof(Bone) == 136,"Update this function when making changes to this class!");
	if(!(name == other.name && ID == other.ID && children.size() == other.children.size() && parent.expired() == other.parent.expired()))
		return false;
	for(auto &pair : children)
	{
		if(other.children.find(pair.first) == other.children.end())
			return false;
	}
	return true;
}

std::ostream &operator<<(std::ostream &out,const panima::Bone &o)
{
	out<<"Bone";
	out<<"[Name:"<<o.name<<"]";
	out<<"[Id:"<<o.name<<"]";
	out<<"[Children:"<<o.children.size()<<"]";
	out<<"[Parent:";
	if(o.parent.expired())
		out<<"NULL";
	else
		out<<o.parent.lock()->name;
	out<<"]";
	return out;
}
