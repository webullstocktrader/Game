# VALLEY GOD

Spectator god-mode. Stone-age valley **on Earth** — our sun, our sky, the planet that will later be called Earth. You are invisible. The villagers never know a watcher exists. Weather is weather.

This slice is a **miniature Earth**: the sculpted valley plus seven placeholder continents, with ocean between them. Eight tribes start there, one on each land. Each tribe has one teacher. See `docs/WORLD-LORE.md`, `docs/CAST.md`, and `docs/PLAYTEST-EIGHT-TRIBES.md`.

You do not need to write any game code.

---

## What you can do in this slice

1. Free-fly an invisible camera (WASD, Q/E up/down, mouse look). There is no in-world body for them to see.
2. Watch eight tribes. Each land starts with one teacher (Mara, Nima, Lira, Sable, Flint, Oak, Reed, Bram) and three learners. They walk, talk, hunt, eat, sleep, teach, and build. Hunger and energy still drive them.
3. Fire weather: **1 Rain**, **2 Tornado**, **3 Hurricane**, **4 Flood**. They seek shelter, panic, or climb. **0** clears the sky.
4. Watch the day turn. Default is a **75 second day**. `[` / `]` stretch or compress it.
5. Pin someone (left click / Tab). The card shows tribe, Teacher or Learning, knowledge, shelters, and claim.
6. Press **G** to hop the camera to the next continent, or hold **Shift** and fly across the ocean yourself.

**Teachers (all 21+):** Mara, Nima, Lira, Sable · Flint, Oak, Reed, Bram. Learners and later kin are adults too. This slice does not place child bodies. Pairing only runs at 21 or older, and it is not an intimacy scene.

No myths about you. No space travel.

---

## What you need (once)

This is an **Unreal Engine 5.8** C++ project. Windows is the path that is meant to “just run.”

### 1. Epic Games Launcher + Unreal Engine 5.8

1. Install the [Epic Games Launcher](https://store.epicgames.com/en-US/download).
2. Open **Unreal Engine → Library → plus button**.
3. Install **5.8** (5.8.2 or whatever hotfix Epic is on is fine).

### 2. Visual Studio 2022 (C++ game tools)

1. Install [Visual Studio 2022 Community](https://visualstudio.microsoft.com/vs/community/).
2. Check **Game development with C++**.
3. Let it include the Windows 10/11 SDK.

Mac: Unreal 5.8 + Xcode, then double-click `ValleyGod.uproject`.  
Linux: possible with a source UE 5.8 build. Not the easy path.

---

## How to play

### Easiest

1. Open the `ValleyGod` folder (this folder).
2. Double-click **`PLAY.bat`**.
3. First run compiles and writes wet-dirt materials. Go get water. Next runs are faster.
4. You spawn above the camp at mid-morning. Villagers are already moving and talking.

`PLAY.bat` finds Unreal 5.8, compiles, writes materials, and launches the slice.

### Or open it yourself

1. Double-click **`ValleyGod.uproject`**.
2. If Windows asks which Unreal version, pick **5.8**.
3. If it asks to rebuild modules, say **Yes**.
4. When the editor appears, press **Alt+P**.

The valley is built when the game starts. You do not place anything in the level.

---

## Controls

| Action | Key |
|---|---|
| Fly | **W A S D** |
| Up / down | **E** / **Q** |
| Look | Mouse |
| Faster fly | **Left Shift** |
| Zoom (FOV + speed) | Mouse wheel |
| Rain / Tornado / Hurricane / Flood | **1** / **2** / **3** / **4** |
| Clear weather | **0** |
| Pause / resume sim | **P** |
| Faster / slower day | **]** / **[** |
| Pin villager under crosshair | **Left mouse** |
| Cycle pinned villager | **Tab** |
| Unpin / overview | **F** |
| Next continent | **G** |

---

## What this slice is not

- Not a full-size Earth. Continents past the valley are placeholder discs.
- Not a sex scene. New people arrive as adults (21+). Nobody under 21 can pair.
- Not bronze, iron, or anything past knapped stone.
- Not a visible god, a cult, or villager awareness of the camera.
- Not space, planet travel, or a fantasy planet. Same solar system, later — not here.
- Characters wear hide tunics. No intimate content.

Meshes are built in-engine (cylinders, spheres, layered canopies) **until** you drop MetaHuman + Quixel downloads into the folders in `docs/METAHUMAN-QUIXEL.md`. Lighting is Lumen with volumetric fog, virtual shadows, TSR, and cinematic post. Dirt goes wet when it rains. PLAY.bat bakes `Content/Materials/M_*` when Unreal prep succeeds; if Content materials are empty, the game module builds the same wet materials at runtime so `-game` does not fall back to WorldGrid or BasicShape.

---

## If something goes wrong

**“Unreal Engine 5.8 was not found”**  
Install 5.8 from the Epic launcher. Run `PLAY.bat` again.

**Compile errors / “missing modules”**  
Install Visual Studio 2022 with **Game development with C++**. Open `ValleyGod.uproject` and let Unreal rebuild.

**LNK2005 `Valley::Material` / `FallbackMaterial` already defined**  
Those functions live only in `ValleyMaterials.cpp` now. A stale `ValleyTypes.cpp.obj` (unity/incremental) still has the old copies. Delete `ValleyGod\Intermediate` and rebuild, or run PLAY.bat again after this pull (the module definition bump forces a recompile).

**“ValleyPrepCommandlet looked like a commandlet, but we could not find the class”**  
The editor module must load at Default (not PostEngineInit) so `-run=` can find the class. PLAY.bat calls `-run=ValleyGodEditor.ValleyPrepCommandlet` to load the module by name. If you invoke Unreal yourself:
```
UnrealEditor.exe "ValleyGod.uproject" -run=ValleyGodEditor.ValleyPrepCommandlet -unattended -nopause -nosplash -log
```

**Black level / missing map**  
Open the project in the editor once (not `-game`). The editor writes `/Game/Maps/ValleySlice` and the materials on startup. Then press Play.

**Looks dry / gray / checkerboard**  
PLAY.bat now launches `-game` even when prep failed. Runtime materials should still show wet dirt, grass, bark, hide, and skin. To bake assets onto disk: run PLAY.bat once (it calls `-run=ValleyPrep`) or open the editor. After a successful prep, `Content/Materials` contains `M_Dirt`, `M_DirtWet`, `M_Grass`, `M_Water`, `M_Bark`, `M_Foliage`, `M_Wood`, `M_Hide`, `M_SkinWarm`, and the rest of the `M_*` set.

**How to verify this graphics pass**
1. Double-click `PLAY.bat` (quotes handle spaces in the project path).
2. Confirm the valley is brown dirt + green patches + dark water, not a gray grid.
3. Fly to camp: hide roofs, wood poles, stone fire ring. Eight named adults in hide tunics. Trees have thick trunks and layered canopies.
4. Press **1** — dirt darkens (wet). Press **0** to clear.

**MetaHuman + Quixel (after Fab / Creator downloads)**  
Full steps: [`docs/METAHUMAN-QUIXEL.md`](docs/METAHUMAN-QUIXEL.md).

1. Sign into **Fab**, download a Quixel grass/forest/dirt pack (`Content/Megascans/` or `Content/PN_GrassLibrary/`).
2. In MetaHuman Creator, assemble adult **Mara** into `Content/MetaHumans/Mara`.
3. PLAY. Bottom-left HUD: `Look  Mara MetaHuman  ·  ground Quixel  ·  foliage Quixel` when those assets exist.
4. Empty folders are OK — HUD stays `procedural` and `-game` still runs.

**Sim tests (no Unreal required)**  
From this folder: `bash Tests/run_tests.sh`

---

## Folder map (you can ignore this)

| Path | What it is |
|---|---|
| `ValleyGod.uproject` | The Unreal project |
| `PLAY.bat` | Double-click this |
| `Config/` | Lumen, project name |
| `Source/ValleyGod/` | Valley, villagers, weather, invisible camera |
| `Source/ValleyGod/Sim/` | Engine-free day/hunger/weather brain + MetaHuman/Quixel path table |
| `Source/ValleyGodEditor/` | Writes materials and the empty slice map the first time |
| `Content/MetaHumans/Mara/` | Drop assembled Mara here (not committed) |
| `Content/PN_GrassLibrary/` | Grass pack meshes/materials (not committed) |
| `Content/Megascans/` | Fab/Quixel downloads (not committed) |
| `docs/METAHUMAN-QUIXEL.md` | Fab sign-in, Mara assemble, dense valley foliage, PLAY |
| `docs/WORLD-LORE.md` | Earth / this solar system. No fantasy planet. No space in this slice |
| `docs/CAST.md` | Eight teachers and the learners on their lands |
| `docs/PLAYTEST-EIGHT-TRIBES.md` | How to watch dialogue, teaching, and the other continents |

Unreal 5.8. Latest stable. Watch first.
