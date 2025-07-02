// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <udm.hpp>
#include <mathutil/umath.h>

module panima;

import :animation;
import :channel;

panima::Channel *panima::Animation::AddChannel(std::string path, udm::Type valueType)
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

void panima::Animation::RemoveChannel(std::string path)
{
	auto it = FindChannelIt(std::move(path));
	if(it == m_channels.end())
		return;
	m_channels.erase(it);
}

void panima::Animation::RemoveChannel(const Channel &channel)
{
	auto it = std::find_if(m_channels.begin(), m_channels.end(), [&channel](const std::shared_ptr<Channel> &channelOther) { return &channel == channelOther.get(); });
	if(it == m_channels.end())
		return;
	m_channels.erase(it);
}

void panima::Animation::AddChannel(Channel &channel)
{
	auto it = std::find_if(m_channels.begin(), m_channels.end(), [&channel](const std::shared_ptr<Channel> &channelOther) { return channelOther->targetPath == channel.targetPath; });
	if(it != m_channels.end()) {
		*it = channel.shared_from_this();
		return;
	}
	m_channels.push_back(channel.shared_from_this());
}

std::vector<std::shared_ptr<panima::Channel>>::iterator panima::Animation::FindChannelIt(std::string path)
{
	ChannelPath channelPath {std::move(path)};
	return std::find_if(m_channels.begin(), m_channels.end(), [&channelPath](const std::shared_ptr<Channel> &channel) { return channel->targetPath == channelPath; });
}

panima::Channel *panima::Animation::FindChannel(std::string path)
{
	auto it = FindChannelIt(std::move(path));
	if(it == m_channels.end())
		return nullptr;
	return it->get();
}

void panima::Animation::Merge(const Animation &other)
{
	for(auto &channelOther : other.GetChannels()) {
		auto *channel = FindChannel(channelOther->targetPath);
		if(!channel)
			channel = AddChannel(channelOther->targetPath, channelOther->GetValueType());
		if(!channel)
			continue;
		channel->MergeValues(*channelOther);
	}
}

bool panima::Animation::Save(udm::LinkedPropertyWrapper &prop) const
{
	auto udmChannels = prop.AddArray("channels", m_channels.size());
	for(auto i = decltype(m_channels.size()) {0u}; i < m_channels.size(); ++i) {
		auto udmChannel = udmChannels[i];
		m_channels[i]->Save(udmChannel);
	}

	prop["speedFactor"] = m_speedFactor;
	prop["duration"] = m_duration;
	prop["flags"] = udm::flags_to_string(m_flags);
	return true;
}
bool panima::Animation::Load(udm::LinkedPropertyWrapper &prop)
{
	auto udmChannels = prop["channels"];
	auto numChannels = udmChannels.GetSize();
	m_channels.reserve(numChannels);
	for(auto udmChannel : udmChannels) {
		m_channels.push_back(std::make_shared<Channel>());
		m_channels.back()->Load(udmChannel);
	}

	prop["speedFactor"](m_speedFactor);
	prop["duration"](m_duration);
	udm::to_flags<Flags>(prop["flags"], m_flags);
	return true;
}

std::ostream &operator<<(std::ostream &out, const panima::Animation &o)
{
	out << "Animation";
	out << "[Dur:" << o.GetDuration() << "]";
	out << "[Channels:" << o.GetChannelCount() << "]";
	out << "[AnimSpeedFactor:" << o.GetAnimationSpeedFactor() << "]";
	return out;
}
