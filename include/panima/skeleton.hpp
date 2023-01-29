/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PANIMA_SKELETON_HPP__
#define __PANIMA_SKELETON_HPP__

#include <cinttypes>
#include <memory>
#include <unordered_map>
#include <mathutil/transform.hpp>

namespace udm {
	struct AssetData;
	using AssetDataArg = const AssetData &;
};
namespace panima {
	struct Bone;
	class Skeleton {
	  public:
		static constexpr uint32_t FORMAT_VERSION = 1u;
		static constexpr auto PSKEL_IDENTIFIER = "PSKEL";
		static std::shared_ptr<Skeleton> Load(const udm::AssetData &data, std::string &outErr);
		Skeleton() = default;
		Skeleton(const Skeleton &other);
		uint32_t AddBone(Bone *bone);
		uint32_t GetBoneCount() const;
		bool IsRootBone(uint32_t boneId) const;
		int32_t LookupBone(const std::string &name) const;
		std::weak_ptr<Bone> GetBone(uint32_t id) const;
		const std::unordered_map<uint32_t, std::shared_ptr<Bone>> &GetRootBones() const;
		std::unordered_map<uint32_t, std::shared_ptr<Bone>> &GetRootBones();
		const std::vector<std::shared_ptr<Bone>> &GetBones() const;
		std::vector<std::shared_ptr<Bone>> &GetBones();
		std::vector<umath::ScaledTransform> &GetBonePoses() { return m_referencePoses; }
		const std::vector<umath::ScaledTransform> &GetBonePoses() const { return const_cast<Skeleton *>(this)->GetBonePoses(); }

		void Merge(Skeleton &other);
		bool Save(udm::AssetDataArg outData, std::string &outErr);

		bool operator==(const Skeleton &other) const;
		bool operator!=(const Skeleton &other) const { return !operator==(other); }
	  private:
		bool LoadFromAssetData(const udm::AssetData &data, std::string &outErr);
		std::vector<std::shared_ptr<Bone>> m_bones;
		std::unordered_map<uint32_t, std::shared_ptr<Bone>> m_rootBones;
		std::vector<umath::ScaledTransform> m_referencePoses;
	};
};

#endif
