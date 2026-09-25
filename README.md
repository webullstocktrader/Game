# SILT COUNTY

Chief and Gooch. Best friends. Two trucks. One wet county.

This repo is the **first playable slice** — not the full 6×5 km county. You start in the shop, they say the only two lines that matter, then you drive. Mud, a washed-out lie of Highway 6, a river ford, a crate the county wants moved, and a winch so you can pull your brother out when you get stupid.

You do not need to write any game code.

---

## What you can do in this slice

1. Watch the garage intro. Gooch: *“Chief, you ready for this adventure?”* Chief: *“Hell yeah, brother, let’s get it.”*
2. Drive both trucks on the same machine (split screen). Player 1 is **Chief**. Player 2 is **Gooch**.
3. Throttle, brake, steer. Honest gears: Drive, Low when you air down or sink, Reverse when you back up.
4. Air the tires down for mud. Air them up for pavement.
5. Floor it in the soup and the mud **ruts and swallows you**.
6. Winch to the other truck, a stump, or the stranded county rig in the ford.
7. Pick up the crate at the ford (`F` / A) and haul it to the **Highway 6 lay-by**.

That is the whole job. No funeral campaign. No ranks. No rebuild tree.

---

## What you need (once)

This is an **Unreal Engine 5.8** project. Latest stable. Windows is the path that is meant to “just run.”

### 1. Epic Games Launcher + Unreal Engine 5.8

1. Install the [Epic Games Launcher](https://store.epicgames.com/en-US/download).
2. Open **Unreal Engine → Library → plus button**.
3. Install **5.8** (5.8.2 or whatever hotfix Epic is on is fine).
4. Wait it out. It is a large install.

### 2. Visual Studio 2022 (C++ game tools)

Unreal has to compile this project the first time.

1. Install [Visual Studio 2022 Community](https://visualstudio.microsoft.com/vs/community/).
2. In the installer, check **Game development with C++**.
3. Let it include the Windows 10/11 SDK.

Mac: install Unreal 5.8 and Xcode, then double-click `SiltCounty.uproject`.  
Linux: possible if you already have a source UE 5.8 build. Not the easy path.

---

## How to play

### Easiest

1. Unzip / clone this folder onto your PC.
2. Double-click **`PLAY.bat`**.
3. First run compiles. Go get coffee. Next runs are faster.
4. The garage loads. Wait for the lines, or press **Enter / Start** to skip.
5. Drive.

`PLAY.bat` finds Unreal 5.8, compiles, writes the wet materials, and launches the slice.

### Or open it yourself

1. Double-click **`SiltCounty.uproject`**.
2. If Windows asks which Unreal version, pick **5.8**.
3. If it asks to rebuild modules, say **Yes**.
4. When the editor appears, press **Alt+P** (or the Play button).

The county is built when the game starts. You do not place anything in the level.

---

## Controls

| | **Chief (Player 1)** | **Gooch (Player 2)** | **Gamepad** |
|---|---|---|---|
| Throttle / reverse | W / S | I / K | RT / LT |
| Steer | A / D | J / L | Left stick |
| Brake | Left Shift | Right Shift | LT |
| Handbrake | Space | Right Alt | B |
| Air down / air up | **[** / **]** | P / ; | D-pad left / right |
| Winch attach / cut | Q | U | LB |
| Winch in / out | E / C | O / , | RB / D-pad down |
| Grab crate | F | ' | A |
| Reset upright (hold) | R | / | X |
| Skip intro | Enter | Enter | Start |

Plug in two pads if you have them. Keyboard + one pad also works (Chief on keys, Gooch on the pad).

---

## How not to get stuck (and how to enjoy getting stuck)

- **Do not floor it in the mud.** High tire spin digs a grave. The ruts stay.
- **Air down** (into the teens) before the ford and the washout. Low PSI floats better in soup, worse on Highway 6.
- When you are buried, **winch Gooch** (or a stump). Hold winch-in. He pulls. You crawl.
- The crate is in the water by the stranded county rig. Back up to it, press grab, take it to the marked lay-by on Highway 6.
- Hold reset if you turtle.

---

## Two computers (optional)

Same-machine split screen is the default.

Listen-server if you want two PCs on a LAN:

1. Host: launch, open the console (**~**), type `open SiltCountySlice?listen`
2. Client: `open HOST_IP`

First player is Chief. Second is Gooch.

---

## What this slice is not

- Not the full county.
- Not every rank, rebuild, or story beat.
- Trucks are built in-engine (hero *feel*, wet Lumen metal, readable silhouettes) so the project runs without Marketplace packs. Swap in real body meshes later if you want.

---

## Town and shop look

You still start in the shop. The east bay stays open, the keys stay on the west wall, and both trucks stay in their stalls. The bay floor is oil-stained concrete, the walls are block, and the roof is corrugated tin.

East of the shop, past Highway 6, Main Street is a blockout of Silt County: County Trust Bank, Town Hall with corner turrets, Millard's, Tate's Western, Silt County High, a water tower, and a grain elevator. The feed store, chapel, and gas shed are still there. The names are fiction. There are no street numbers.

The editor writes these patterned materials the first time it opens the project, the same way it writes the truck paints. Until then, the shapes use a flat engine material.

---

## If something goes wrong

**“Unreal Engine 5.8 was not found”**  
Install 5.8 from the Epic launcher. Run `PLAY.bat` again.

**Compile errors / “missing modules”**  
Install Visual Studio 2022 with **Game development with C++**. Open `SiltCounty.uproject` and let Unreal rebuild.

**Black level / missing map**  
Open the project in the editor once (not `-game`). The editor writes `/Game/Maps/SiltCountySlice` and the wet materials on startup. Then press Play.

**Only one truck / no split screen**  
Start with **Alt+P** from the editor, or use `PLAY.bat`. Make sure **Windowed** or a wide fullscreen so the horizontal split is usable.

**Looks dry / gray**  
You launched before the editor wrote materials. Open the editor once. Wet green (Chief), rust red (Gooch), and the town brick, tin, and shop floor should show after that.

---

## Folder map (you can ignore this)

| Path | What it is |
|---|---|
| `SiltCounty.uproject` | The Unreal project |
| `PLAY.bat` | Double-click this |
| `Config/` | Lumen, split screen, project name |
| `Source/SiltCounty/` | The game: trucks, mud, winch, garage, contract |
| `Source/SiltCountyEditor/` | Writes wet materials, town and shop patterns, and the empty slice map the first time |

Unreal 5.8. Latest stable. Co-op first.
