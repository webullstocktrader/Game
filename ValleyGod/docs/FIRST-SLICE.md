# Valley God — First Watchable Slice

## Pitch
Spectator god-mode stone-age valley **on Earth** (our solar system, Milky Way). User is invisible. Villagers never know a watcher exists. No myths about a god pointing at the player. Weather is weather. This is not a fantasy planet.

See `WORLD-LORE.md`. First slice stays in the valley. No space content.

## First login (this Cloud Agent deliverable)
Playable Unreal Engine 5.8 project the user can open and watch.

Must have:
- One small stone-age valley (trees, river, dirt, a few shelters)
- Day / night cycle
- Invisible free-fly god camera (WASD + mouse look, or top-down toggle). Camera is never an in-world object villagers can see or talk about.
- 8 adult villagers (exactly 4 women, 4 men, all 21+). They are the **entire human population on Earth**. Distinct names, looks, and habits. Walk, talk (English), hunt, cook/eat, sleep. No children. No other tribes.
- Hunger / energy drives behavior
- God weather commands UI: Rain, Tornado, Hurricane, Flood (aimed or global). Villagers react (seek shelter, panic, resume).
- Event countdown panel (even if mostly placeholders): e.g. meal / sleep timers so the compressed calendar is visible
- Compressed time: day/night cycles fast enough to watch (configurable), not real-time human lifespan

## Explicitly NOT in this first login
- Nudity / sex / bathing intimate scenes (later pass, adults 21+ only)
- Marriage / pregnancy / children systems (next pass)
- Tech tree past stone age
- Space, planet travel, named cosmos, invented planets
- Invented AI, god-awareness
- Online multiplayer

## Graphics goal
Aim for grounded UE5 look (Lumen, Nanite where useful, wet dirt, soft night). First build may use simple meshes / Metahuman-lite or mannequins with clear silhouettes — better than gray cubes. Prefer quality lighting and materials over asset count.

## Controls
- Fly camera: WASD, Q/E up-down, mouse look, scroll zoom
- Number keys or sidebar: Rain / Tornado / Hurricane / Flood
- Pause / resume sim time
- Click a villager to follow / pin their status

## Success criteria
1. Project opens in UE 5.8 (or PLAY.bat) on Windows
2. Within 1 minute user sees villagers moving and talking in a valley with day/night
3. At least one weather command visibly changes the world and villagers react
4. README with how to run
5. No sexual content, no child characters in this slice
