# Valley God — eight MetaHumans + denser valley (2026-09-18)

## Goal
Raise visual fidelity of the first watchable slice so all eight starting adults can use MetaHuman Blueprint presentations, and valley foliage/ground/trees/camp/animals look denser. Missing assets still fall back to procedural art. PLAY.bat stays empty-folder safe.

## People
- Per-slot MetaHuman class discovery (named `BP_{Name}` under `EditableMetahumans` / `MetaHumans` / `ValleySlice`).
- `UsesMetaHumanSlot` is true for every non-negative slot; spawn uses the discovered class or the procedural body.
- Generic leftover `BP_MetaHuman` Blueprints fill unmatched adults.
- Graphics spawn `Brain.VillagerCount` (do not clamp the watchable list back to 8).

## Valley
- Path tables include `PN_GrassLibrary`, extra Megascans/Fab aliases, and broader name classification.
- `DiscoverOptionalAssets` always merges documented aliases with a recursive scan so one pin does not hide pack variety.
- `SpawnTreesAndRocks` prefers real meshes, instances grass/rocks/saplings with HISM (1600 grass / 90 rocks / 140 trees), and keeps unique tree components for wind.
- Ground uses world-space UVs and smooth height-field normals so Quixel surfaces light correctly.
- Cinematic UE5: volumetric fog, Lumen, virtual shadows, TSR, epic scalability, wet dirt.
- Modest camp props and animal antlers/tusks from existing primitive meshes.

## Out of scope
Intimate content. Inventing binary MetaHuman/Quixel assets. Changing sim day length or population growth rules.
