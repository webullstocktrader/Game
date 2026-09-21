# MetaHuman + Quixel (milestone 1)

Code-capsule bodies are no longer the hero look. This slice prefers **one assembled MetaHuman (Mara)** and a **Quixel/FAB grass–trees–dirt patch**. Downloads happen on your Windows PC. This repo never ships those binaries.

God camera, the eight adults, weather, and “they never look at you” are unchanged. Intimate content is out of scope.

If the folders below are empty, PLAY.bat still runs: Mara falls back to the procedural villager, and the valley ground is a brown dirt material instance (BaseColor 0.30, 0.18, 0.09), never the blue engine default. Rain darkens that dirt. Real Quixel/Megascans/PN dirt and grass replace it only when the asset is not WorldGrid, BasicShape, or a blue-named default.

---

## Folders (put downloads here)

| Disk | Unreal path | Put this here |
|---|---|---|
| `Content/EditableMetahumans/Mara/` | `/Game/EditableMetahumans/Mara/` | Preferred. Assemble editable MetaHuman **Mara** here (`BP_Mara`) |
| `Content/MetaHumans/Mara/` | `/Game/MetaHumans/Mara/` | Older assemble folder. Used when the editable Blueprint is absent |
| `Content/PN_GrassLibrary/` | `/Game/PN_GrassLibrary/` | Grass clump meshes / grass materials |
| `Content/Megascans/` | `/Game/Megascans/` | Fab/Quixel default download root |
| `Content/Fab/` | `/Game/Fab/` | Alternate Fab drop folder (also scanned) |
| `Content/MSPresets/` | `/Game/MSPresets/` | Quixel/Megascans preset materials |
| `Content/Quixel/` | `/Game/Quixel/` | Alternate Quixel drop folder |
| `Content/ValleySlice/` | `/Game/ValleySlice/` | Optional aliases if you want to pick exact assets |
| `Content/Maps/ValleySlice` | `/Game/Maps/ValleySlice` | Playable map (ValleyPrep still creates this empty map) |

Downloaded `.uasset` trees under those folders are gitignored. Keep the `.gitkeep` files.

### Optional aliases (rename/copy after download)

These are checked first, so you can pin a specific pack:

- `/Game/ValleySlice/BP_Mara` — Mara Actor Blueprint
- `/Game/ValleySlice/MI_Dirt`, `MI_Grass`, `MI_DirtWet`
- `/Game/ValleySlice/SM_Tree`, `SM_Grass`, `SM_Rock`
- `/Game/PN_GrassLibrary/Meshes/SM_Grass` (and `Materials/MI_Grass`)

If aliases are missing, the game also scans `Content/PN_GrassLibrary`, `Content/Megascans`, `Content/Fab`, `Content/MSPresets`, and `Content/Quixel` and classifies names (`dirt`, `beech`, `grass`, `rock`, `pine`, `tuft`, …). Documented aliases merge with the scan so extra trees/grass/rocks still load. Real static meshes are preferred over capsule trees. Grass, saplings, and rocks instance densely (HISM). The camp clearing stays open.

Mara Blueprint candidates (first file that exists wins):

1. `/Game/EditableMetahumans/Mara/BP_Mara`
2. `/Game/EditableMetahumans/Mara/Mara`
3. `/Game/MetaHumans/Mara/BP_Mara`
4. `/Game/MetaHumans/Mara/Mara`
5. `/Game/MetaHumans/Mara/BP_MetaHuman`
6. `/Game/MetaHumans/Mara/Blueprints/BP_Mara`
7. `/Game/ValleySlice/BP_Mara`
8. Any Actor Blueprint under `Content/EditableMetahumans/Mara/`, then `Content/MetaHumans/Mara/`

Only **Mara** (cast slot 0) uses a MetaHuman this milestone. Nima, Lira, Sable, Flint, Oak, Reed, and Bram stay procedural until a later pass.

---

## Exact steps on Windows (UE 5.8.2)

### 0. Engine extras

In Epic Games Launcher, Unreal Engine 5.8 → **Options**, enable **MetaHuman Creator Core Data** (or the similarly named MetaHuman core pack). Without it, Creator opens but cannot finish a character.

Install Visual Studio 2022 with **Game development with C++** if you have not already.

### 1. Open the project

1. Double-click `ValleyGod.uproject` (or `PLAY.bat` once so it compiles).
2. If Unreal asks to rebuild modules, **Yes**.
3. If it warns that Fab / MetaHuman / Bridge / MegascansPlugin / HairStrands need enabling, **Enable**. Those names are already on in `ValleyGod.uproject`.
4. Sign in with your Epic account when the editor asks.

### 2. Fab sign-in and Quixel valley pack

1. Window → **Fab** (or the Fab button in the toolbar).
2. Sign in.
3. Search Quixel Megascans for a **forest / grassland** set that includes:
   - a **dirt** or forest-floor surface
   - a **grass** surface and/or grass clump 3D plant
   - at least one **tree** 3D plant (beech, pine, oak, willow — any temperate tree)
4. **Add to project** / download into **this** ValleyGod project. Fab should land files under `Content/Megascans/` (that is the path we scan).
5. If you have **PN_GrassLibrary**, add it under `Content/PN_GrassLibrary/`.
6. Do not need to place them in the level by hand. Play will scatter trees/grass/rocks densely and swap ground materials if it finds them.

If Fab puts meshes in a differently named folder, either move/copy the pack into `Content/Megascans/` or copy the ones you want to the `Content/ValleySlice/SM_*` / `MI_*` aliases above.

### 3. MetaHuman Creator → Mara

1. Enable plugins if prompted: **MetaHuman Character**, **MetaHuman SDK**, **HairStrands**.
2. Create a MetaHuman Character asset. Name the character **Mara** (adult woman, stone-age hide clothing — covered torso, no intimate content).
3. Use the Creator **Assemble** tab (not DCC-only export). Assemble into **`Content/MetaHumans/Mara`**.
4. Confirm a Blueprint appears in that folder (`BP_Mara`, `Mara`, or under a `Blueprints/` subfolder). Play scans `Content/MetaHumans/Mara` recursively for an Actor Blueprint.

If the assembled Blueprint fails to open, enable **Control Rig** (and Live Link / IK Rig if the editor lists them) in Plugins. Those are engine plugins, not extra game-module dependencies.

Hide clothing: use any fully covering hide/leather wardrobe item you have in Fab, or assemble in a closed tunic and ignore fashion polish for this milestone.

### 4. PLAY

1. Double-click `PLAY.bat`, or in the editor press **Alt+P**.
2. Bottom-left HUD line should read something like:
   - `Look  Mara MetaHuman  ·  ground Quixel  ·  foliage Quixel` when downloads are present
   - `Look  Mara procedural  ·  ground brown  ·  foliage procedural` when folders are still empty
3. Output Log (`Window → Developer Tools → Output Log`) also prints `Valley God assets: Mara=...`.

Mara may idle-slide until you assign a walk AnimBP. That is expected for this scaffolding milestone. Click-to-pin still works (capsule on the villager actor).

---

## What this does **not** change

- Invisible god camera (WASD, Q/E, mouse look).
- Eight adults only: Mara, Nima, Lira, Sable, Flint, Oak, Reed, Bram.
- Weather keys 1–4 / 0 clear.
- Villagers never acknowledge the watcher.

---

## Troubleshooting

**HUD still says Mara procedural after assemble**  
The Blueprint is not at a candidate path. Prefer `Content/EditableMetahumans/Mara/BP_Mara`. `Content/MetaHumans/Mara/` and `Content/ValleySlice/BP_Mara` still work. Watch the Output Log for `Mara MetaHuman class`.

**HUD still says foliage procedural after Fab download**  
Files are not under `Content/Megascans`, `Content/Fab`, `Content/PN_GrassLibrary`, `Content/MSPresets`, or `Content/Quixel`, or names do not contain dirt/grass/tree/rock keywords. Copy chosen meshes to `Content/ValleySlice/SM_Tree` (and `SM_Grass`, `MI_Dirt`, …).

**Plugin missing on project open**  
Install 5.8.2 with MetaHuman extras. Enable the plugin from the warning dialog. Do not remove ProceduralMeshComponent or EnhancedInput.

**Hair missing on Mara**  
HairStrands is enabled in the `.uproject`. Also enable **Groom** / **Alembic Groom Importer** in the Plugins window if the editor lists them separately, and confirm MetaHuman Core Data is installed.

**`-game` / PLAY.bat with empty Content**  
This is supported. The ground is brown (HUD `ground brown`), not the blue template floor. Villagers and capsule trees still spawn. Chopping, staged builds, and the tech tree are unchanged.

**Ground is still blue**  
You are on an old build, or a material named like a real dirt scan is actually an engine default. Delete `Intermediate` and `Binaries`, run `PLAY.bat` again, and check the log for `Valley MID M_Dirt BaseColor=(0.30, 0.18, 0.09)`. A rejected scan logs `Valley ground rejected`.
