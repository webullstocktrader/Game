# Valley God first slice — architecture

Ships as `ValleyGod/`, a self-contained Unreal Engine 5.8 C++ project. It does not replace or rewrite any other slice in this repo.

## Approach

The valley is spawned in C++ at BeginPlay (same pattern as a content-free first playable). An editor module writes Lumen-friendly materials and an empty map the first time the project opens. `PLAY.bat` compiles, runs the prep commandlet, and launches.

Simulation (day clock, hunger/energy, weather reactions, English lines) lives in `Source/ValleyGod/Sim/ValleySim.*` with **no Unreal types**. Unreal actors only present that state. `Tests/run_tests.sh` compiles the sim with g++.

## Pieces

| Unit | Job |
|---|---|
| `vg::World` | 14 adults, 4 animals, 5 shelters, weather, countdowns |
| `AValleyTerrain` | Dirt bowl, grass ridges, river, flood offset, wet material swap |
| `AValleyWorld` | Sky/Lumen lights, trees, lean-tos, fire, rain streaks, tornado column |
| `AValleyVillager` | Hide-tunic body (cylinders/spheres), speech bubble, walk bob |
| `AValleyGodPawn` | Invisible `DefaultPawn`. No mesh. Villagers never query it |
| `AValleyHUD` | Weather keys, event clock, voices, pinned needs |

## Out of scope (enforced)

No child characters (ages 24–44). No marriage/pregnancy. No god/player/camera words in speech. No multiplayer.
