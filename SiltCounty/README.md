# Silt County

Early playable co-op slice. Two friends, two trucks, one flooded county.

You are **Chief** (faded red). Your best friend is **Gooch** (dirty gold). After a short skippable intro, both of you drive the mud, hook a stranded van, and drag it back to the county garage. Bridges, power, and culverts are on the map as later rebuild jobs. This is not a single-player campaign.

The county is **8 km × 8 km**. The garage, flooded town, causeway, and south slough are built with denser ground. The rest of the county is a coarser wet floodplain so the map stays large without a huge first-play hitch.

## What you need

- Unreal Engine **5.4 or newer** (5.5 through 5.8 are fine)
- Visual Studio 2022 with the **Desktop development with C++** workload, so the editor can compile the game module
- Windows is the desktop target. Quote every path. Windows usernames and folders often contain spaces.

Quixel / Megascans / Fab kits are not bundled. The first time the editor opens, it generates the mud, water, and wet-ground materials under `Content/SiltCounty/Materials`. Lumen and Nanite are turned on in the project. DBuffer decals are on so standing puddles can darken and gloss the ground. The runtime terrain is a procedural mesh, so it is not Nanite; Lumen still lights the wet surfaces, water, and rain.

If you already opened an older build, restart the editor after pulling this change. The wet materials rebuild themselves when their revision does not match. The Output Log should include `Silt wet-ground, floodwater, puddle, and physical materials are revision 4`.

## Open the project

From File Explorer, double-click:

`SiltCounty\SiltCounty.uproject`

Or, from `SiltCounty\`, run `OpenSiltCounty.bat`. It searches `C:\Program Files\Epic Games` for UE 5.4–5.7 and quotes the project path.

If the editor lives somewhere else:

```bat
"D:\Game\SiltCounty\OpenSiltCounty.bat" "C:\Program Files\Epic Games\UE_5.4\Engine\Binaries\Win64\UnrealEditor.exe"
```

Manual launch, with a username that contains a space:

```bat
"C:\Program Files\Epic Games\UE_5.4\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\Alex Morgan\Game\SiltCounty\SiltCounty.uproject"
```

The first open compiles C++ and shaders. Let that finish. The Output Log should include `Silt County materials are ready`. If a material graph fails, the county still loads with an engine fallback material.

Ignore any “lighting needs to be rebuilt” note. The county is spawned at runtime and uses dynamic Lumen lighting.

## Play (one truck)

1. Press **Play** (Alt+P). The default mode is a single window.
2. Wait on the first Play. The county collision is built when the session starts. The Output Log prints `Silt County world built in ...s`.
3. Optional intro, about 15 seconds:
   - Flood overlook
   - Garage, two trucks, keys in them
   - Gooch: “Chief, you ready for this adventure?”
   - Chief: “Hell yeah, brother, let’s get it.”
4. Press **Enter** to skip the intro. Or launch with `-silt.skipintro`.
5. You are in Chief’s truck. Gooch’s truck sits parked in the yard until a second player joins.

### Drive

| Input | Action |
| --- | --- |
| W / S or Right / Left trigger | Throttle / reverse |
| A / D or Left stick | Steer |
| Space or gamepad B | Brake |
| Shift or right bumper | Handbrake |
| Right mouse (or right stick) | Look around; recenters when you let go |
| R or gamepad Y | Flip the truck back onto its wheels |
| E or gamepad A | Hook the tow when you are close |
| Enter | Skip the intro |
| F9 | Local splitscreen co-op on the host |

The causeway crown is wet asphalt and has the most grip. The gravel shoulder is slower. Mud tracks off the shoulder, the town basin, and the south slough sink, drag, and slide. Floodwater adds buoyancy and drag. If the hull bottoms out, keep the wheels turning or press R.

## Test two players

Both trucks exist as soon as a session starts. A second player possesses Gooch.

### Local splitscreen (fastest)

1. Press Play in a single window (standalone or listen server).
2. Press **F9**, or open the console with the tilde key and run `silt.AddLocalPlayer`.
3. The view splits top and bottom. Top is Chief. Bottom is Gooch.
4. Each side uses its own keyboard/controller. A gamepad is the comfortable way to drive the second truck. Skip the intro with Enter if it is still running.

F9 only works on the host. It does nothing in a remote client window, and it does nothing if two players are already in the session.

### Editor listen server (two windows)

1. Click the **three dots** next to the Play button.
2. Set **Net Mode** to **Play As Listen Server**.
3. Set **Number of Players** to **2**.
4. Press Play.

Window 1 is the host: Chief. Window 2 is the client: Gooch. Enter skips the intro in the window that has focus. The host truck is the smoothest. Gooch’s truck is simulated on the host and replicated back, so it has a little network delay even on one machine.

Console on the host: `silt.SkipIntro`.

### Second process (host / join)

Quote paths that contain spaces.

Host:

```bat
"C:\Program Files\Epic Games\UE_5.4\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\Alex Morgan\Game\SiltCounty\SiltCounty.uproject" -game -log -listen -silt.skipintro
```

Join, after the host’s log shows the world is built:

```bat
"C:\Program Files\Epic Games\UE_5.4\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\Alex Morgan\Game\SiltCounty\SiltCounty.uproject" -game -log 127.0.0.1
```

Default port is **7777**. From a running client console you can also run:

```text
open 127.0.0.1
```

## Sample contract

**South Slough Pull.** A service van and its driver are stuck off the causeway, southwest of the garage.

1. Follow the raised gravel causeway south out of the garage, through the flooded town.
2. Leave the road into the slough. The ground gets slow.
3. Pull up to the van (about 14 m). The HUD says to press **E**.
4. A tow tether pulls the van behind your hitch. This is a stub, not a winch simulation: the van drags in mud and rolls easier once it reaches gravel.
5. Bring it to the yellow **DROP ZONE** beam in the garage yard.
6. Stop there. The HUD reads **CONTRACT COMPLETE**.

The broken bridge and the culvert are set dressing for later rebuild jobs. The causeway is the way through for this slice.

## Map

```text
north
  garage + drop zone     y ≈ +800 m     Chief and Gooch spawn here
  flooded south town     y ≈ +520 m
  bridge out             y ≈ +220 m     cosmetic, road still goes through
  culvert                y ≈ +400 m
        |  gravel causeway
        |
  south slough van       x ≈ -300 m, y ≈ -240 m

county bounds: 8 km × 8 km, centered on the origin
```

## Layout

```text
SiltCounty/
  SiltCounty.uproject
  Config/                 map, Lumen, input, splitscreen
  Content/SiltCounty/     generated wet materials (created on editor launch)
  Source/SiltCounty/      trucks, county, contract, HUD, listen-server rules
  Source/SiltCountyEditor/ material bootstrap
```

The world is not a saved map. On Play, `USiltWorldSubsystem` clears the template level and builds terrain, water, town, garage, weather, and rain. `ASiltCountyGameMode` spawns both trucks and the contract. Each machine builds the same terrain from a fixed seed so collision matches.

Driving is server-authoritative. Local splitscreen and the listen host simulate directly. A remote client sends throttle, steer, and brake each tick.

## Mud, water, and wet ground

This is a material and dressing pass. Garage start, town blocks, bridges, and the culvert are unchanged. Wheel grip is still the surface query in `SiltTruckPawn` (the HUD label is the authority). Physical materials carry density and a zero bounce; their friction stays at the engine default or higher so chassis contact does not get looser.

What you should see:

- **Wet asphalt crown** — black (`RoadDist` under 700), vertex roughness 0.22, clear-coat sheen and mirror puddles. HUD: `WET ASPHALT`.
- **Wet gravel shoulder** — crushed stone (`RoadDist` under 1100). HUD: `WET GRAVEL`. Same grip as a leftover `Road` sample.
- **Wet soil** — olive upland, the default above the mud band. HUD: `WET SOIL`. Dirt and mud near the garage (Y about 80000), the first contract, or within 20 m of the road get a +0.15 wetness boost.
- **Mud tracks and deep mud** — olive silt. Tracks are `RoadDist` under 1600 where `ValueNoise` exceeds 0.35, and only on ground above the deep-mud line. The basin and slough still go to `DEEP MUD` and `FLOODWATER`. HUD on the ruts: `MUD TRACK`.
- **Wetness** — one 0–1 value from `SiltTerrain::SampleWetness`. Asphalt and gravel use the road curve (0.55). It darkens vertex color and lowers roughness, and each ground chunk writes that average into `M_WetGround` `Wetness` and `WetnessBias`. Puddle decals stay.
- **Floodwater** — opaque tea-brown sheet with a slow ripple normal, sharper reflections in the middle, and a broken dirty foam edge where the bank meets the water.
- **Rain** — cooler, thinner streaks. They do not change the ground; the ground is already built wet.

Generated assets, all under `Content/SiltCounty/Materials`:

| Asset | Role |
| --- | --- |
| `M_WetGround` | Clear-coat master. Parameters: `PuddleAmount`, `NormalStrength`, `WpoAmplitude`, `ClearCoatBias`, `PuddleRoughness`, `WaterZ` (keep at 720), `ShoreBand`, `Wetness`, `WetnessBias` (both 0–1, set per chunk at play) |
| `MI_WetRoad`, `MI_WetSoil`, `MI_Mud`, `MI_DeepMud`, `MI_SiltBed` | Per-surface instances and physical materials |
| `PM_WetRoad`, `PM_WetSoil`, `PM_Mud`, `PM_DeepMud`, `PM_StandingWater` | Density and restitution. Friction is intentionally not slippery |
| `M_FloodWater` | Standing flood sheet. `RippleSpeed`, `FoamStrength`, `WaterRoughness` |
| `M_PuddleDecal` | DBuffer puddle. `PuddleOpacity` |

`WpoAmplitude` is 0 on the instances so neighboring surface sections do not split open. Turn it up on `M_WetGround` only if the whole chunk uses that one material.

### Verify in editor / PIE

On the Desktop machine, rebuild with `PLAY.bat` (it compiles C++ and opens the editor). Let shaders finish. The Output Log should include `Silt wet-ground, floodwater, puddle, and physical materials are revision 4`. Then press Play.

1. Open `SiltCounty.uproject` in Unreal 5.4 or newer (5.8 is fine) if you are not using `PLAY.bat`. Confirm the revision log line above. `M_WetGround` must list `Wetness` and `WetnessBias` in addition to the puddle parameters.
2. In the Content Browser, open `M_WetGround` and `M_FloodWater`. You should see the parameters listed above, not a single fresnel lerp. `M_PuddleDecal` should still be there.
3. Press Play and skip the intro (Enter). You spawn in the garage yard. Soil around the garage should read darker and glossier than open upland.
4. Look at the gravel around the trucks: damp sheen and a few dark puddles. The garage and the flooded town blocks should still be there.
5. Drive south through **SOUTH TOWN — FLOODED**. Water should ripple, shores should foam, and the houses should still stand in the basin.
6. Leave the causeway into the slough. The HUD should step from `WET ASPHALT` (crown, under 700 cm) to `WET GRAVEL` (shoulder, under 1100 cm), then broken `MUD TRACK` patches and `WET SOIL`, and into `DEEP MUD` / `FLOODWATER`. Shallow puddle decals stay on asphalt and gravel.
7. Optional: on `MI_WetRoad`, raise `PuddleAmount` toward 1 and play again. The causeway should get more mirror patches without moving the town.

## Limits of this slice

- Towing is a soft tether, not a cable, winch, or trailer hitch.
- Rebuild jobs are signs and props only.
- No engine audio, no Quixel meshes, and procedural ground is not Nanite.
- First Play stalls while terrain collision is created.
- Remote-client driving has replication delay. Splitscreen and the host window are the feel test.
