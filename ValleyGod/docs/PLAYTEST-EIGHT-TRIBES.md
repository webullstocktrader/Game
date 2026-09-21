# Playtest — eight tribes on a miniature Earth

Host checks (no Unreal):

```bash
bash Tests/run_tests.sh
```

That binary covers dialogue pools, eight tribes, continent gaps, unclaimed land, neutral stances, babies, chopping, staged building, and the tech tree.

## In Unreal

1. Double-click `PLAY.bat`, or open `ValleyGod.uproject` in UE 5.8 and press Alt+P.
2. You start **looking straight down** at the miniature Earth. The square in the middle is Willow basin (sculpted valley). Blue around it is ocean. Seven colored discs are the other lands. Each land has a colored camp disc and a name: Willow, Red bluff, Salt, Dark wood, High stone, Reed water, Cold ridge, Ash shore. One tribe each. They start **Neutral**. A teacher can later choose Ally or Enemy when people actually meet. Ambition alone does not start a war.
3. **Q** drops you down. **G** hops to the next land. **F** returns to the overhead view. **Shift + WASD** flies. The other lands are placeholder discs with a hearth and two shelters.
4. Pin someone (click or Tab). A teacher’s card says **Teacher** and a high know value. A learner says **Learning**. Stand near a teacher for a few seconds and the learner’s know number climbs.
5. Listen. Hunt, eat, sleep, talk, weather, teach, and build each pull from their own pool. The same person should not repeat the identical line back to back.
6. Weather keys **1–4** are still global. **0** clears. People on every land react.
7. Each camp starts with five timber trees. Someone will **Chop**. That world tree instance is destroyed and that camp’s **wood** count goes up. A felled tree does not grow back. Scenic forest scatter is dressing, not this stock.
8. Stay on Willow basin. Within about two minutes you should see the teacher’s research leave **Fire**, a stone spear at the fire, then a new shelter out past the starter huts. Watch the kit: stone site, log frame, thatch walls, thatch roof. Frame and later stages spend wood. The roof finishes a shelter, the claim grows, and a new grove appears farther out so the next site can start. There is no unique finished mesh per building. People sleep at night and wake at dawn, then go back to the site. Outer land on the continent stays unclaimed.
9. Two adults (21+) of one tribe who stay fed near camp can have a **baby** (age 0). On Willow that shows up inside the same two-minute watch. The baby is smaller, stays by the fire, and ages about one year every 6 seconds. They cannot pair until 21. There is no intimacy scene. The top bar’s **kin** count goes up. Other lands do this too. The camp does not stay frozen at the starter huts.
10. The tech line starts at **Fire**. The teacher researches. **Stone tools** comes up first: someone at the fire crafts a stone spear and the camp’s spear count goes up. **Shelter craft** is next. By two minutes the line has moved through Farming toward Metal and Writing. **Computing** stays locked. Animal husbandry, when it finally unlocks, only flags riding. Nobody mounts a horse in this slice. Lie, cheat, and steal stay personal. Tribes start Neutral. Expansion does not declare war.

`Content/MetaHumans/` is unchanged. Mara is still the only MetaHuman slot when that Blueprint is present. Other people use the procedural fallback until a photoreal pack exists. The lock for harvest, the building kit, characters, and the Quixel/Nanite/Lumen north star is `PROGRESSION-LOCK.md`.
