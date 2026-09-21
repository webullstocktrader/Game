# Apply on the Desktop parent

This slice is the expansion pass on top of the eight-tribe Earth build. Host tests do not need Unreal. The playable check is UE 5.8 on the Desktop machine.

## Files to copy

Copy these over the Desktop `ValleyGod` tree. Keep the same relative paths. Do not replace `Content/` MetaHuman or Quixel binaries.

- `Source/ValleyGod/Sim/ValleySim.h`
- `Source/ValleyGod/Sim/ValleySim.cpp`
- `Source/ValleyGod/Sim/ValleyLookPaths.cpp`
- `Tests/test_valley_sim.cpp`
- `Tests/run_tests.sh` (only if the Desktop copy is older)
- `docs/PLAYTEST-EIGHT-TRIBES.md`
- `docs/PROGRESSION-LOCK.md`
- `docs/METAHUMAN-QUIXEL.md`
- `docs/WORLD-LORE.md`
- `README.md`

`ValleyWorld.cpp` and `ValleyVillager.cpp` already spawn new people, hide a chopped tree, and show site / frame / walls / roof. Leave them in place if they already match the Earth branch. This pass does not replace them.

## Mara path

The first Blueprint tried is:

`/Game/EditableMetahumans/MHC_Hannah/Mara/BP_Mara`

Fallbacks stay in this order: `/Game/MetaHumans/Mara/BP_Mara`, then `Mara`, `BP_MetaHuman`, then `/Game/ValleySlice/BP_Mara`. Do not point LookPaths at `/Game/MetaHumans/Mara` only. The assemble folder scan is still `Content/MetaHumans/Mara` when none of those paths load.

## Host check (no Unreal)

From `ValleyGod/`:

```bash
bash Tests/run_tests.sh
```

Expect a line ending in `passed, 0 failed`.

## PLAY

1. Paste the files above into the Desktop project.
2. If the editor was open, close it. If a previous link failed with `LNK2005`, delete `ValleyGod/Intermediate` and rebuild.
3. Double-click `PLAY.bat`, or open `ValleyGod.uproject` in UE 5.8 and press Alt+P.
4. You start overhead: eight lands, ocean between them, one tribe and one teacher on each. They are Neutral.
5. Press **Q** to drop onto Willow basin. Stay there for about two minutes. Do not press 1–4 (weather pauses camp work).

What you should see on that camp:

- A timber tree is chopped. That trunk and crown leave the world. Wood goes up. The stump does not return.
- The tech line moves Fire → Stone tools → Shelter craft, and keeps moving toward later names. Computing stays locked.
- Someone at the fire crafts a stone spear. A spare shows by the hearth.
- A new build site appears out past the starter huts. Watch stone pad, log frame, thatch walls, thatch roof.
- The roof finishes, a new grove appears farther out, and a second site opens.
- A baby appears, smaller than the adults, and gets a little taller as the years tick (about one year every 6 seconds).
- At night they sleep. At dawn they get up and go back to the site.

**G** hops to another land. Those camps do the same work. Nobody is scripted into a war.

Animal husbandry is still only a riding flag. There is no horse.
