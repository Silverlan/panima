/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "panima/animation.hpp"
#include "panima/channel.hpp"
#include <udm.hpp>
#include <mathutil/umath.h>

panima::Channel *panima::Animation::AddChannel(std::string path,udm::Type valueType)
{
	ChannelPath channelPath {std::move(path)};
	auto *channel = FindChannel(path);
	if(channel)
		return (channel->targetPath == channelPath && channel->GetValueType() == valueType) ? channel : nullptr;
	m_channels.push_back(std::make_shared<Channel>());
	channel = m_channels.back().get();
	channel->SetValueType(valueType);
	channel->targetPath = path;
	return channel;
}

void panima::Animation::AddChannel(Channel &channel)
{
	auto it = std::find_if(m_channels.begin(),m_channels.end(),[&channel](const std::shared_ptr<Channel> &channelOther) {
		return channelOther->targetPath == channel.targetPath;
	});
	if(it != m_channels.end())
	{
		*it = channel.shared_from_this();
		return;
	}
	m_channels.push_back(channel.shared_from_this());
}

panima::Channel *panima::Animation::FindChannel(const util::Path &path)
{
	auto it = std::find_if(m_channels.begin(),m_channels.end(),[&path](const std::shared_ptr<Channel> &channel) {
		return channel->targetPath == path;
	});
	if(it == m_channels.end())
		return nullptr;
	return it->get();
}

bool panima::Animation::Save(udm::LinkedPropertyWrapper &prop) const
{
	auto udmChannels = prop.AddArray("channels",m_channels.size());
	for(auto i=decltype(m_channels.size()){0u};i<m_channels.size();++i)
	{
		auto udmChannel = udmChannels[i];
		m_channels[i]->Save(udmChannel);
	}

	prop["speedFactor"] = m_speedFactor;
	prop["duration"] = m_duration;
	return true;
}
bool panima::Animation::Load(udm::LinkedPropertyWrapper &prop)
{
	auto udmChannels = prop["channels"];
	auto numChannels = udmChannels.GetSize();
	m_channels.reserve(numChannels);
	for(auto udmChannel : udmChannels)
	{
		m_channels.push_back(std::make_shared<Channel>());
		m_channels.back()->Load(udmChannel);
	}

	prop["speedFactor"](m_speedFactor);
	prop["duration"](m_duration);
	return true;
}

std::ostream &operator<<(std::ostream &out,const panima::Animation &o)
{
	out<<"Animation";
	out<<"[Dur:"<<o.GetDuration()<<"]";
	out<<"[Channels:"<<o.GetChannelCount()<<"]";
	out<<"[AnimSpeedFactor:"<<o.GetAnimationSpeedFactor()<<"]";
	return out;
}
