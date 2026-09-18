# Valley God — graphics polish pass (2026-09-18)

## Approved goal
Same god-mode stone-age valley and eight adults (Mara, Nima, Lira, Sable, Flint, Oak, Reed, Bram). Replace toy/capsule look with wet, grounded, cinematic Unreal 5.8 presentation. **No store-bought Megascans/MetaHumans.** No intimate content this pass.

## Approach (locked)
Do **both**:
1. **Bake** Material assets into `Content/Materials` via a fixed ValleyPrep / content factory path.
2. **Runtime fallback** that builds matching wet materials in code if Content is empty, so `-game` / PLAY.bat never goes black or WorldGrid again.

## People
- Keep code-built procedural bodies; reshape into clearer head, torso, hips, arms, legs, hide tunics.
- Soft flesh materials (not plastic); distinct skin + hair per cast member.
- Still clothed (hide tunics).

## Trees and ground
- Layered canopy, thicker bark trunks.
- Wet dirt (rain-responsive), grass patches, stone — not checkerboard.
- Water darker and more reflective.

## Camp and animals
- Shelter, fire ring, racks, tools with wood/hide materials.
- Animals with clearer body proportions and fur-tone materials.

## Lighting
- Keep cinematic wet look: stronger sun, soft sky, light non-volumetric fog, histogram exposure.
- Weather keys (1–4) still drive wetness.

## Out of scope
Megascans, MetaHumans, intimate systems, new gameplay systems, speech-pool expansion (unless trivial).

## Success criteria
- After rebuild + PLAY.bat (or equivalent), valley shows colored dirt/grass/water/bark/hide/skin without depending on missing Content.
- `Content/Materials` contains baked `M_*` assets when prep succeeds.
- People, trees, camp, animals read as intentional shapes, not capsules/lumps.
- God camera / eight adults / no awareness of player unchanged.
- Compiles on UE 5.8; PR opened on `webullstocktrader/Game` under `ValleyGod/`.

## Delivery
Work lives under `ValleyGod/` in https://github.com/webullstocktrader/Game. User play folder is Desktop `ValleyGod` — sync after PR.
