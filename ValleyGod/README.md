# VALLEY GOD

Spectator god-mode. Stone-age valley **on Earth** — our sun, our sky, the planet that will later be called Earth. You are invisible. The villagers never know a watcher exists. Weather is weather.

This is the **first watchable slice** — one valley, a compressed day, **eight adults** (four women, four men), four weather commands. No space. No other worlds. See `docs/WORLD-LORE.md`.

You do not need to write any game code.

---

## What you can do in this slice

1. Free-fly an invisible camera (WASD, Q/E up/down, mouse look). There is no in-world body for them to see.
2. Watch **8 adults** (4 women, 4 men) walk, talk in English, hunt, eat, and sleep. Each has a name, look, and habit. Hunger and energy drive it.
3. Fire weather: **1 Rain**, **2 Tornado**, **3 Hurricane**, **4 Flood**. They seek shelter, panic, or climb. **0** clears the sky.
4. Watch the day turn. Default is a **75 second day**. `[` / `]` stretch or compress it.
5. Pin a villager (left click / Tab) and read their name, habit, and the countdown panel (meal / sleep / dawn).

**Cast (all 21+):** Mara, Nima, Lira, Sable · Flint, Oak, Reed, Bram. No children.

No marriage, no children, no later tech, no myths about you.

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

---

## What this slice is not

- Not a full calendar of lives.
- Not marriage, pregnancy, or children.
- Not bronze, iron, or anything past knapped stone.
- Not a visible god, a cult, or villager awareness of the camera.
- Not space, planet travel, or a fantasy planet. Same solar system, later — not here.
- Characters wear hide tunics. No intimate content.

Meshes are built in-engine (capsules, cylinders, spheres) so the project runs without Marketplace packs. Lighting is Lumen. Dirt goes wet when it rains. Swap in real characters later if you want.

---

## If something goes wrong

**“Unreal Engine 5.8 was not found”**  
Install 5.8 from the Epic launcher. Run `PLAY.bat` again.

**Compile errors / “missing modules”**  
Install Visual Studio 2022 with **Game development with C++**. Open `ValleyGod.uproject` and let Unreal rebuild.

**Black level / missing map**  
Open the project in the editor once (not `-game`). The editor writes `/Game/Maps/ValleySlice` and the materials on startup. Then press Play.

**Looks dry / gray**  
You launched before the editor wrote materials. Open the editor once. Wet dirt and hide tunics should show after that.

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
| `Source/ValleyGod/Sim/` | Engine-free day/hunger/weather brain |
| `Source/ValleyGodEditor/` | Writes materials and the empty slice map the first time |
| `docs/WORLD-LORE.md` | Earth / this solar system. No fantasy planet. No space in this slice |
| `docs/CAST.md` | Eight named adults: four women, four men |

Unreal 5.8. Latest stable. Watch first.
