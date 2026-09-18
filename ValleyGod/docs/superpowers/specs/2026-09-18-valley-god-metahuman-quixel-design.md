# Valley God — MetaHuman + Quixel first slice (2026-09-18)

## Approved
MetaHuman Creator + Quixel/FAB. First delivery: one MetaHuman adult + a Quixel foliage/landscape valley patch that looks real; then deepen to all eight (Mara, Nima, Lira, Sable, Flint, Oak, Reed, Bram).

## People
- MetaHuman-quality adults; distinct faces/bodies.
- Stone-age hide clothing for this slice.
- God camera; villagers never acknowledge the player.
- Intimate content out of scope for this slice.

## Valley
- Quixel/FAB dirt, grass, trees, rocks, water on a real landscape so grass and forest read clearly.

## Delivery order
1. One MetaHuman + Quixel valley patch (this milestone).
2. Full eight + denser foliage.
3. Later trailer deepen.

## Success criteria (milestone 1)
- Project has Fab/Bridge/MetaHuman plugins enabled.
- At least one MetaHuman character asset in the project **when the user has assembled Mara** (this repo does not invent that binary).
- Quixel/FAB grass + trees placed in a playable valley map **when those downloads are present**; otherwise the procedural valley still plays.
- Play shows a real-looking patch, not code capsules as the hero look.

## Milestone-1 lock (implementation)
This spec is the approved art path after rejecting code-capsule hero art. Assets are **not** invented in git: the user downloads Fab/MetaHuman packs on Windows into documented folders. The C++ project must keep `-game` / PLAY.bat working when those folders are empty.

### Plugins (UE 5.8)
Enable in `ValleyGod.uproject` (keep `EngineAssociation` `"5.8"`):

- `Fab`
- `Bridge`
- `MegascansPlugin`
- `MetaHumanCharacter`
- `MetaHumanSDK`
- `HairStrands` (valid UE 5.8 plugin id for grooms; keep enabled)
- Keep existing `ProceduralMeshComponent` and `EnhancedInput`

Do **not** add MetaHuman modules to `ValleyGod.Build.cs`. Runtime code loads optional Blueprints/meshes through Engine `LoadClass` / `LoadObject` so the game module still compiles when those plugins have no content yet.

### Content layout (downloads land here)
| Disk folder | Unreal path | What the user puts there |
|---|---|---|
| `Content/MetaHumans/Mara/` | `/Game/MetaHumans/Mara/...` | Assembled Mara MetaHuman (Blueprint + meshes/grooms) |
| `Content/Megascans/` | `/Game/Megascans/...` | Fab/Quixel default download root (grass, trees, dirt) |
| `Content/ValleySlice/` | `/Game/ValleySlice/...` | Optional aliases (`BP_Mara`, `MI_Dirt`, `SM_Tree`, …) |
| `Content/Maps/ValleySlice` | `/Game/Maps/ValleySlice` | Playable slice map (still spawned in code at BeginPlay) |

### Runtime preference (empty-folder safe)
- **Mara only** (sim slot 0, name `Mara`): if a MetaHuman Actor Blueprint exists at a documented path, spawn that presentation and skip the procedural body for that slot. The other seven adults stay on the current procedural villager.
- **Ground / foliage:** if Quixel/FAB materials or static meshes are present (canonical aliases, then a name scan under `Content/Megascans` / `Content/Fab`), apply/place those. Otherwise keep the current procedural dirt bowl, grass sections, and capsule trees so PLAY.bat still runs.
- God camera, eight-adult cast, weather, and “they never look at you” stay. No intimate content.

### User play loop after Windows downloads
1. Sign into Fab in the editor.
2. Download a Quixel grass/forest/dirt pack into this project.
3. Assemble MetaHuman **Mara** into `Content/MetaHumans/Mara`.
4. PLAY (PLAY.bat or Alt+P).
