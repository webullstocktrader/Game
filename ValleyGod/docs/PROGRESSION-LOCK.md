# Progression lock (2026-09-21)

This is the rule for harvest, buildings, people, and the look of the world. The miniature Earth and the eight tribes ship first. This lock rides along with that scaffold. It does not wait on a finished graphics pass.

## Research

The teacher runs `TickResearch` on `vg::Tech` (`ValleyTechTree.h`). The watchable start is Fire, Stone tools, Shelter craft. One tier unlocks at a time, and research keeps ticking toward Farming, Metal, and later names. Stone tools spends wood to craft a spear the person carries, and a spare shows by the fire. Shelter craft sends an adult into the Build activity on a site. A camp that has a live site keeps a builder on it after dawn. Animal husbandry sets `bRidingUnlocked` and does not spawn a horse.

## Harvest

Chopping a tree removes **that** world tree instance and adds one wood to the tribe stock. The sim marks the timber down. The world destroys the trunk and crown components for that tree. They do not grow back. Each camp starts with five of these trees. When a roof finishes, the camp plants a new grove farther out. Those are new instances. The chopped ones stay gone.

Scenic forest scatter is dressing. It is not the wood stock. The trees that count are the ones a person can chop.

## Buildings

Construction is modular and staged. The player can watch it:

1. **Site** — stone pad
2. **Frame** — log posts and beams
3. **Walls** — thatch or hide panels
4. **Roof** — thatch slopes

An adult uses the **Build** activity at the site. Frame, walls, and roof spend wood. The roof finishes a shelter and grows the claim. The next site opens farther from the starter huts, so the camp expands instead of stacking on one pad. People sleep at night and wake at dawn so the work continues. Outer land on the continent stays unclaimed.

Do not import a unique finished building for each structure. Log, thatch, and stone kit pieces are the building. Procedural kit pieces are the right stand-in until a real kit pack exists.

## Characters

MetaHuman, or an equivalent photoreal character pack, is the preferred body. Procedural capsules exist so a machine with empty `Content/` folders can still run the sim. They are not the art direction.

Mara is the one MetaHuman slot wired today. The other people stay on the fallback body until Desktop has packs for them. Babies are smaller people. They are not a sexual body, and they cannot pair until 21.

## Graphics

The north star is GTA-6-level photoreal. The honest path is Quixel (or the same class of scan), Nanite, and Lumen. That work is the Desktop environment and character lane. Land this sim, the Earth, the tribes, the wood, and the staged kit without waiting for that lane to finish.
