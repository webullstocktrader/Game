# MetaHuman + Quixel milestone-1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Scaffold Valley God so Mara can present as a MetaHuman and the valley can present Quixel/FAB grass/trees/dirt when those downloads exist, while empty Content still plays the current procedural slice.

**Architecture:** Keep an engine-free path table (`vg::` in `Sim/ValleyLookPaths.*`) that names Mara as the only MetaHuman slot and lists documented asset paths plus filename heuristics. Unreal `ValleyAssets` loads the first existing package. `AValleyVillager` / `AValleyWorld` / `AValleyTerrain` consume that loader and fall back to today’s procedural meshes and materials. No binary assets are committed.

**Tech Stack:** Unreal Engine 5.8 C++, existing ValleyGod runtime + editor prep, host `g++` sim tests.

## Global Constraints

- UE `EngineAssociation` stays `"5.8"`.
- Plugins: Fab, Bridge, MegascansPlugin, MetaHumanCharacter, MetaHumanSDK, HairStrands; keep ProceduralMeshComponent + EnhancedInput.
- Do not link MetaHuman/Fab modules from `ValleyGod.Build.cs`.
- Only Mara (slot 0) uses a MetaHuman presentation this milestone.
- Do not remove the god camera or the eight-adult cast.
- No intimate content.
- Do not invent `.uasset` / `.umap` binaries; gitignore downloaded MetaHuman/Megascans trees.
- `-game` / PLAY.bat must still run with empty download folders.

---

### Task 1: Engine-free look-path table

**Files:**
- Create: `ValleyGod/Source/ValleyGod/Sim/ValleyLookPaths.h`
- Create: `ValleyGod/Source/ValleyGod/Sim/ValleyLookPaths.cpp`
- Modify: `ValleyGod/Tests/test_valley_sim.cpp`
- Modify: `ValleyGod/Tests/run_tests.sh`

**Interfaces:**
- Consumes: `vg::kEarthHumans`, `PersonLookAt`
- Produces: `UsesMetaHumanSlot`, path-count/at accessors, `ClassifyContentPath`, folder constants

- [x] **Step 1: Write failing tests** for Mara-only MetaHuman, documented `/Game/MetaHumans/Mara` class paths, Quixel dirt/grass/tree heuristics, and “other seven slots stay procedural.”
- [x] **Step 2: Run tests** (`bash ValleyGod/Tests/run_tests.sh`) and confirm the new checks fail.
- [x] **Step 3: Implement `ValleyLookPaths`** so the tests pass.
- [x] **Step 4: Re-run tests** until the host suite is green.
- [x] **Step 5: Commit** the table + tests.

### Task 2: Project plugins, folders, ignore rules

**Files:**
- Modify: `ValleyGod/ValleyGod.uproject`
- Modify: `ValleyGod/.gitignore`
- Modify: `ValleyGod/Config/DefaultEngine.ini` (groom skin cache)
- Modify: `ValleyGod/Config/DefaultGame.ini` (cook optional folders)
- Create: `Content/MetaHumans/.gitkeep`, `Content/MetaHumans/Mara/.gitkeep`, `Content/Megascans/.gitkeep`, `Content/ValleySlice/.gitkeep`, `Content/Maps/.gitkeep`
- Modify: `ValleyGod/Source/ValleyGodEditor/ValleyContentFactory.cpp` (ensure those directories)

- [x] Enable the locked plugin list.
- [x] Layout folders + gitignore downloaded binaries.
- [x] Commit.

### Task 3: Runtime loader + actor wiring

**Files:**
- Create: `ValleyGod/Source/ValleyGod/ValleyAssets.h`
- Create: `ValleyGod/Source/ValleyGod/ValleyAssets.cpp`
- Modify: `ValleyGod/Source/ValleyGod/ValleyVillager.h/.cpp`
- Modify: `ValleyGod/Source/ValleyGod/ValleyWorld.h/.cpp`
- Modify: `ValleyGod/Source/ValleyGod/ValleyTerrain.h/.cpp`
- Modify: `ValleyGod/Source/ValleyGod/ValleyHUD.cpp`

**Interfaces:**
- Consumes: `vg::ValleyLookPaths` + `FPackageName::DoesPackageExist`
- Produces: `Valley::FindMaraMetaHumanClass`, `FindPreferred*Material/Mesh`, world `GraphicsStatusLine`

- [x] Loader tries documented paths, then scans `Content/Megascans` and `Content/Fab` with `ClassifyContentPath`.
- [x] Mara: spawn MetaHuman Blueprint when present; else procedural body. Always keep a visibility capsule so click-to-pin works.
- [x] Trees/rocks/grass: Quixel static meshes when present; else current procedural scatter.
- [x] Ground sections: Quixel dirt/grass materials when present; else `M_Dirt` / `M_Grass`. Wet swap still works.
- [x] HUD/log reports which presentation is active.

### Task 4: Docs

**Files:**
- Create: `ValleyGod/docs/METAHUMAN-QUIXEL.md`
- Modify: `ValleyGod/README.md`
- Modify: `ValleyGod/docs/CAST.md` (Mara is the first MetaHuman slot)

- [x] Exact Fab sign-in, Quixel download, Mara assemble, PLAY steps.
- [x] Document fallback so empty Content is not a failure.

### Task 5: Verify and PR

- [x] `bash ValleyGod/Tests/run_tests.sh`
- [x] Attempt UE compile if an editor exists; otherwise record that this VM has no UE.
- [x] Open PR against `cursor/valley-god-graphics-pass-b7e4` with summary + verify steps.
