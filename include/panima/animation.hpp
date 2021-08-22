/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PRAGMA_ANIMATION_HPP__
#define __PRAGMA_ANIMATION_HPP__

#include <cinttypes>
#include <memory>
#include <vector>

namespace udm {struct AssetData; enum class Type : uint8_t; struct LinkedPropertyWrapper;};
namespace util {class Path;};
namespace panima
{
	struct Channel;
	class Animation
		: public std::enable_shared_from_this<Animation>
	{
	public:
		Animation()=default;
		void AddChannel(Channel &channel);
		Channel *AddChannel(const util::Path &path,udm::Type valueType);
		const std::vector<std::shared_ptr<Channel>> &GetChannels() const {return const_cast<Animation*>(this)->GetChannels();}
		std::vector<std::shared_ptr<Channel>> &GetChannels() {return m_channels;}
		uint32_t GetChannelCount() const {return m_channels.size();}

		bool Save(udm::LinkedPropertyWrapper &prop) const;
		bool Load(udm::LinkedPropertyWrapper &prop);

		Channel *FindChannel(const util::Path &path);
		const Channel *FindChannel(const util::Path &path) const {return const_cast<Animation*>(this)->FindChannel(path);}

		float GetAnimationSpeedFactor() const {return m_speedFactor;}
		void SetAnimationSpeedFactor(float f) {m_speedFactor = f;}

		float GetDuration() const {return m_duration;}
		void SetDuration(float duration) {m_duration = duration;}
	private:
		std::vector<std::shared_ptr<Channel>> m_channels;
		float m_speedFactor = 1.f;
		float m_duration = 0.f;
	};
};

std::ostream &operator<<(std::ostream &out,const panima::Animation &o);

#endif
