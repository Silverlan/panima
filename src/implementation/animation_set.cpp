/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

module;

#include <string>
#include <memory>
#include <optional>

module panima;

import :animation_set;
import :animation;

static size_t get_anim_hash(const std::string &name) { return std::hash<std::string> {}(name); }
static size_t get_anim_hash(const std::string_view &name) { return std::hash<std::string_view> {}(name); }
static size_t get_anim_hash(const panima::Animation &anim) { return get_anim_hash(anim.GetName()); }
std::shared_ptr<panima::AnimationSet> panima::AnimationSet::Create() { return std::shared_ptr<AnimationSet> {new AnimationSet {}}; }
panima::AnimationSet::AnimationSet() {}
void panima::AnimationSet::Clear()
{
	m_animations.clear();
	m_nameToId.clear();
}
void panima::AnimationSet::AddAnimation(Animation &anim)
{
	auto hash = get_anim_hash(anim);
	auto it = m_nameToId.find(hash);
	if(it != m_nameToId.end())
		RemoveAnimation(anim);
	m_animations.push_back(anim.shared_from_this());
	m_nameToId.insert(std::make_pair(hash, m_animations.size() - 1));
}
void panima::AnimationSet::RemoveAnimation(const Animation &anim) { RemoveAnimation(anim.GetName()); }

void panima::AnimationSet::RemoveAnimation(AnimationId id)
{
	if(id >= m_animations.size())
		return;
	RemoveAnimation(*m_animations[id]);
}

void panima::AnimationSet::RemoveAnimation(const std::string_view &animName)
{
	auto it = m_nameToId.find(get_anim_hash(animName));
	if(it == m_nameToId.end())
		return;
	auto id = it->second;
	m_animations.erase(m_animations.begin() + id);
	m_nameToId.erase(it);
	for(auto &pair : m_nameToId) {
		if(id >= pair.second)
			--pair.second;
	}
}

void panima::AnimationSet::Reserve(uint32_t count)
{
	m_animations.reserve(count);
	m_nameToId.reserve(count);
}
uint32_t panima::AnimationSet::GetSize() const { return m_animations.size(); }

panima::Animation *panima::AnimationSet::GetAnimation(AnimationId id)
{
	if(id >= m_animations.size())
		return nullptr;
	return m_animations[id].get();
}

panima::Animation *panima::AnimationSet::FindAnimation(const std::string_view &animName)
{
	auto id = LookupAnimation(animName);
	if(!id.has_value())
		return nullptr;
	return GetAnimation(*id);
}

std::optional<panima::AnimationId> panima::AnimationSet::LookupAnimation(const std::string_view &animName) const
{
	auto it = m_nameToId.find(get_anim_hash(animName));
	if(it == m_nameToId.end())
		return {};
	return it->second;
}

std::ostream &operator<<(std::ostream &out, const panima::AnimationSet &o)
{
	out << "AnimationSet";
	out << "[Count:" << o.GetSize() << "]";
	return out;
}
