/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "panima/pose.hpp"
#include <functional>

panima::Pose::Pose(const std::vector<umath::ScaledTransform> &transforms)
	: m_transforms{transforms}
{}
panima::Pose::Pose(std::vector<umath::ScaledTransform> &&transforms)
	: m_transforms{std::move(transforms)}
{}
void panima::Pose::SetTransformCount(uint32_t c) {m_transforms.resize(c);}
void panima::Pose::SetBoneIndex(uint32_t channelId,BoneId boneId)
{
	if(boneId >= m_boneIdToChannelId.size())
		m_boneIdToChannelId.resize(boneId +1,std::numeric_limits<uint32_t>::max());
	m_boneIdToChannelId[boneId] = channelId;
}
umath::ScaledTransform *panima::Pose::GetTransform(BoneId idx)
{
	if(idx >= m_boneIdToChannelId.size())
		return nullptr;
	auto channelIdx = m_boneIdToChannelId[idx];
	if(channelIdx >= m_transforms.size())
		return nullptr;
	return &m_transforms[channelIdx];
}
void panima::Pose::SetTransform(BoneId idx,const umath::ScaledTransform &pose)
{
	if(idx >= m_boneIdToChannelId.size())
		return;
	auto channelIdx = m_boneIdToChannelId[idx];
	if(channelIdx >= m_transforms.size())
		return;
	m_transforms[channelIdx] = pose;
}
void panima::Pose::Clear()
{
	m_transforms.clear();
	m_boneIdToChannelId.clear();
}
uint32_t panima::Pose::GetChannelIdx(BoneId boneId) const
{
	return (boneId < m_boneIdToChannelId.size()) ? m_boneIdToChannelId[boneId] : std::numeric_limits<uint32_t>::max();
}
void panima::Pose::Lerp(const Pose &other,float f)
{
	for(auto boneId=decltype(m_boneIdToChannelId.size()){0u};boneId<m_boneIdToChannelId.size();++boneId)
	{
		auto channel0 = GetChannelIdx(boneId);
		auto channel1 = other.GetChannelIdx(boneId);
		if(channel0 == std::numeric_limits<uint32_t>::max() || channel1 == std::numeric_limits<uint32_t>::max())
			continue;
		m_transforms[channel0].Interpolate(other.m_transforms[channel1],f);
	}
}

std::ostream &operator<<(std::ostream &out,const panima::Pose &o)
{
	out<<"AnimatedPose";
	out<<"[Transforms:"<<o.GetTransforms().size()<<"]";
	return out;
}
