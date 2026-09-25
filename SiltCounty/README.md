# Silt County

Early playable co-op slice. Two friends, two trucks, one flooded county.

You are **Chief** (faded red). Your best friend is **Gooch** (dirty gold). After a short skippable intro, both of you drive the mud, hook a stranded van, and drag it back to the county garage. Bridges, power, and culverts are on the map as later rebuild jobs. This is not a single-player campaign.

The county is **8 km × 8 km**. The garage, flooded town, causeway, and south slough are built with denser ground. The rest of the county is a coarser wet floodplain so the map stays large without a huge first-play hitch.

## What you need

- Unreal Engine **5.4 or newer** (5.5 and 5.6 are fine)
- Visual Studio 2022 with the **Desktop development with C++** workload, so the editor can compile the game module
- Windows is the desktop target. Quote every path. Windows usernames and folders often contain spaces.

Quixel / Megascans / Fab kits are not bundled. The first time the editor opens, it generates wet-ground, floodwater, truck paint, beacon, rain, the town materials (`M_TownBrick`, `M_TownClapboard`, `M_ConcreteBlock`, `M_MunicipalPaint`), and the garage materials (`M_GarageFloor`, `M_GarageWall`, `M_GarageRoof`) under `Content/SiltCounty/Materials`. Lumen and Nanite are turned on in the project. The runtime terrain is a procedural mesh, so it is not Nanite; Lumen still lights the wet surfaces, water, and rain.

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

Gravel on the causeway has grip. Shoulders, the town basin, and the south slough sink, drag, and slide. Floodwater adds buoyancy and drag. If the hull bottoms out, keep the wheels turning or press R.

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

## Limits of this slice

- Towing is a soft tether, not a cable, winch, or trailer hitch.
- Rebuild jobs are signs and props only.
- No engine audio, no Quixel meshes, and procedural ground is not Nanite.
- First Play stalls while terrain collision is created.
- Remote-client driving has replication delay. Splitscreen and the host window are the feel test.
