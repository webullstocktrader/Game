#pragma once

// Teacher research tree. Fire, stone tools, and shelter craft are the
// watchable start. Later names stay on the tree for a long run.
// Animal husbandry only flips a riding flag. It does not spawn a mount.

namespace vg
{
	enum class TechId
	{
		Fire = 0,
		StoneTools,
		ShelterCraft,
		Farming,
		PotteryWeaving,
		Metal,
		Writing,
		Machines,
		Electricity,
		Computing,
		AnimalHusbandry,
		Count
	};

	constexpr int kTechCount = static_cast<int>(TechId::Count);

	struct TechTierDef
	{
		TechId Id;
		const char* Name;
		float ResearchNeed;
	};

	inline const TechTierDef& TechTierAt(int Index)
	{
		static const TechTierDef kTree[] = {
			{TechId::Fire, "Fire", 0.f},
			{TechId::StoneTools, "Stone tools", 8.f},
			{TechId::ShelterCraft, "Shelter craft", 22.f},
			{TechId::Farming, "Farming", 48.f},
			{TechId::PotteryWeaving, "Pottery and weaving", 90.f},
			{TechId::Metal, "Metal", 150.f},
			{TechId::Writing, "Writing", 230.f},
			{TechId::Machines, "Machines", 330.f},
			{TechId::Electricity, "Electricity", 450.f},
			{TechId::Computing, "Computing", 600.f},
			{TechId::AnimalHusbandry, "Animal husbandry", 800.f},
		};
		if (Index < 0 || Index >= kTechCount)
		{
			return kTree[0];
		}
		return kTree[Index];
	}
} // namespace vg
