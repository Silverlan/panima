// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

export module panima:animation;

import :channel;

export namespace panima {
	class Animation : public std::enable_shared_from_this<Animation> {
	  public:
		enum class Flags : uint32_t { None = 0u, LoopBit = 1u };
		Animation() = default;
		void AddChannel(Channel &channel);
		Channel *AddChannel(std::string path, udm::Type valueType);
		void RemoveChannel(std::string path);
		void RemoveChannel(const Channel &channel);
		const std::vector<std::shared_ptr<Channel>> &GetChannels() const { return const_cast<Animation *>(this)->GetChannels(); }
		std::vector<std::shared_ptr<Channel>> &GetChannels() { return m_channels; }
		uint32_t GetChannelCount() const { return m_channels.size(); }
		void Merge(const Animation &other);

		bool Save(udm::LinkedPropertyWrapper &prop) const;
		bool Load(udm::LinkedPropertyWrapper &prop);

		Channel *FindChannel(std::string path);
		const Channel *FindChannel(std::string path) const { return const_cast<Animation *>(this)->FindChannel(std::move(path)); }

		float GetAnimationSpeedFactor() const { return m_speedFactor; }
		void SetAnimationSpeedFactor(float f) { m_speedFactor = f; }

		void SetName(std::string name) { m_name = std::move(name); }
		const std::string &GetName() const { return m_name; }

		Flags GetFlags() const { return m_flags; }
		bool HasFlags(Flags flags) const { return umath::is_flag_set(m_flags, flags); }

		float GetDuration() const { return m_duration; }
		void SetDuration(float duration) { m_duration = duration; }

		bool operator==(const Animation &other) const { return this == &other; }
		bool operator!=(const Animation &other) const { return !operator==(other); }
	  private:
		std::vector<std::shared_ptr<Channel>>::iterator FindChannelIt(std::string path);
		std::vector<std::shared_ptr<Channel>> m_channels;
		std::string m_name;
		float m_speedFactor = 1.f;
		float m_duration = 0.f;
		Flags m_flags = Flags::None;
	};
	using namespace umath::scoped_enum::bitwise;
};

export {
	namespace umath::scoped_enum::bitwise {
		template<>
		struct enable_bitwise_operators<panima::Animation::Flags> : std::true_type {};
	}

	std::ostream &operator<<(std::ostream &out, const panima::Animation &o);
};
