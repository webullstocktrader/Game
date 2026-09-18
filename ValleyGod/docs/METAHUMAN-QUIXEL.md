# MetaHuman + valley art (all eight adults)

Code-capsule bodies are the fallback, not the hero look. This slice aims for **photoreal MetaHuman adults** and a **Quixel-level wet cinematic valley** (Lumen, volumetric fog, dense instanced grass/trees) when those downloads exist.

God camera, weather, and “they never look at you” are unchanged. Intimate content is out of scope. PLAY.bat / `-game` still run if the folders below are empty: missing Blueprints fall back to the procedural body, and the valley keeps procedural dirt, tufts, and trees.

Population may grow later. Graphics spawn whoever the sim currently has. A missing Blueprint never blocks a villager from appearing.

---

## Folders (put downloads here)

| Disk | Unreal path | Put this here |
|---|---|---|
| `Content/EditableMetahumans/` | `/Game/EditableMetahumans/` | MetaHuman Creator assemble root (recursive scan) |
| `Content/MetaHumans/` | `/Game/MetaHumans/` | Alternate assemble root (`MetaHumans/Mara`, `MetaHumans/Nima`, …) |
| `Content/PN_GrassLibrary/` | `/Game/PN_GrassLibrary/` | Grass clump meshes / grass materials |
| `Content/Megascans/` | `/Game/Megascans/` | Fab/Quixel default download root |
| `Content/Fab/` | `/Game/Fab/` | Alternate Fab drop folder |
| `Content/ValleySlice/` | `/Game/ValleySlice/` | Optional aliases if you want to pin exact assets |
| `Content/Maps/ValleySlice` | `/Game/Maps/ValleySlice` | Playable map (ValleyPrep still creates this empty map) |

Downloaded `.uasset` trees under those folders are gitignored. Keep the `.gitkeep` files.

### MetaHuman Blueprint candidates (per adult)

For each name (`Mara`, `Nima`, `Lira`, `Sable`, `Flint`, `Oak`, `Reed`, `Bram`), the loader tries documented paths first, then recursively scans `EditableMetahumans`, `MetaHumans`, and `ValleySlice` for an Actor Blueprint whose package is named for that villager (`BP_Mara`, `BP_Nima`, …).

Documented paths (Name = villager):

1. `/Game/EditableMetahumans/{Name}/BP_{Name}`
2. `/Game/EditableMetahumans/{Name}/{Name}`
3. `/Game/MetaHumans/{Name}/BP_{Name}`
4. `/Game/MetaHumans/{Name}/{Name}`
5. `/Game/MetaHumans/{Name}/BP_MetaHuman`
6. `/Game/ValleySlice/BP_{Name}`

Mara also still accepts the Creator layout:

7. `/Game/EditableMetahumans/MHC_Hannah/Mara/BP_Mara`

Any `BP_*` Actor under `Content/EditableMetahumans` whose folder or asset name matches the villager works. A leftover generic `BP_MetaHuman` is assigned to the next adult who has no named Blueprint.

**Drop-in recipe:** assemble (or copy) eight covered-torso adult MetaHumans and leave them as `BP_Mara` … `BP_Bram` somewhere under `Content/EditableMetahumans/` or `Content/MetaHumans/{Name}/`. Hide clothing only. No intimate content.

If a slot has no class, that adult keeps the procedural body. Other adults still use theirs.

### Optional foliage aliases

Checked first so you can pin a pack:

- `/Game/ValleySlice/MI_Dirt`, `MI_Grass`, `MI_DirtWet`
- `/Game/ValleySlice/SM_Tree`, `SM_Grass`, `SM_Rock`
- `/Game/PN_GrassLibrary/Meshes/SM_Grass` (and `Materials/MI_Grass`)

If aliases are missing, the game scans `Content/PN_GrassLibrary`, `Content/Megascans`, `Content/Fab`, `Content/MSPresets`, and `Content/Quixel` and classifies names (`dirt`, `beech`, `grass`, `rock`, `pine`, `tuft`, …). Real static meshes are preferred over capsule trees. Grass and rocks instance densely (HISM). The camp clearing stays open.

---

## Exact steps on Windows (UE 5.8.2)

### 0. Engine extras

In Epic Games Launcher, Unreal Engine 5.8 → **Options**, enable **MetaHuman Creator Core Data**. Install Visual Studio 2022 with **Game development with C++**.

### 1. Open the project

1. Double-click `ValleyGod.uproject` (or `PLAY.bat` once so it compiles).
2. If Unreal asks to rebuild modules, **Yes**.
3. Enable Fab / MetaHuman / Bridge / MegascansPlugin / HairStrands if prompted.
4. Sign in with your Epic account when the editor asks.

### 2. Fab sign-in and valley packs

1. Window → **Fab**.
2. Sign in.
3. Add a **forest / grassland** Quixel set (dirt or forest floor, grass, at least one temperate tree). Download into this project (`Content/Megascans/` or `Content/Fab/`).
4. If you have **PN_GrassLibrary**, add it under `Content/PN_GrassLibrary/`.
5. You do not need to place meshes in the level. Play scatters trees/grass/rocks and swaps ground materials when it finds them.

### 3. MetaHuman Creator → eight adults

1. Enable **MetaHuman Character**, **MetaHuman SDK**, **HairStrands** if prompted.
2. Assemble each adult (21+, stone-age hide clothing, covered torso) into `Content/EditableMetahumans` or `Content/MetaHumans/{Name}`.
3. Confirm an Actor Blueprint appears (`BP_Mara`, `BP_Nima`, …). Play scans those trees recursively.

If an assembled Blueprint fails to open, enable **Control Rig** (and Live Link / IK Rig if listed).

### 4. PLAY

1. Double-click `PLAY.bat`, or in the editor press **Alt+P**.
2. Bottom-left HUD:
   - `Look  8 MetaHuman  ·  ground Quixel  ·  foliage Quixel` when all eight Blueprints and packs are present
   - `Look  1 MetaHuman / 7 procedural  ·  …` when only some Blueprints exist
   - `Look  people procedural  ·  ground procedural  ·  foliage procedural` when folders are empty
3. Output Log prints `Valley God assets: MetaHumans=N/8 …`.

Idle-slide until a walk AnimBP is assigned is expected. Click-to-pin still walks `GetOwner()` so nested MetaHuman primitives pin the villager.

---

## What this does **not** change

- Invisible god camera (WASD, Q/E, mouse look).
- Weather keys 1–4 / 0 clear.
- Villagers never acknowledge the watcher.
- Sim population rules (this pass does not clamp the watchable spawn list back to 8).

---

## Troubleshooting

**HUD still says people procedural after assemble**  
Blueprints are not under `Content/EditableMetahumans`, `Content/MetaHumans`, or `Content/ValleySlice`, or they are not Actor Blueprints named `BP_{Name}`. Watch the Output Log for `MetaHuman class`.

**Only Mara became a MetaHuman**  
The other seven Blueprints were not found. Copy `BP_Nima` … `BP_Bram` into `Content/EditableMetahumans/{Name}/` or `Content/MetaHumans/{Name}/`.

**HUD still says foliage procedural after Fab download**  
Files are not under `Content/Megascans`, `Content/Fab`, or `Content/PN_GrassLibrary`, or names do not contain dirt/grass/tree/rock keywords. Copy chosen meshes to `Content/ValleySlice/SM_Tree` (and `SM_Grass`, `MI_Dirt`, …).

**Plugin missing on project open**  
Install 5.8.2 with MetaHuman extras. Do not remove ProceduralMeshComponent or EnhancedInput.

**`-game` / PLAY.bat with empty Content**  
Supported. Procedural terrain, tufts, camp, animals, and villagers spawn as before.
