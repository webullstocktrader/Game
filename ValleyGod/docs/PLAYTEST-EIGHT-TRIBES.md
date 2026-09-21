# Playtest — eight tribes on a miniature Earth

Host checks (no Unreal):

```bash
bash Tests/run_tests.sh
```

That binary covers dialogue pools, no back-to-back repeats, eight tribes with one `TeacherId` each, continent gaps, unclaimed land, teaching, building, and the 21+ birth rule.

## In Unreal

1. Double-click `PLAY.bat`, or open `ValleyGod.uproject` in UE 5.8 and press Alt+P.
2. You start over **Willow basin** (the sculpted valley). Mara is the teacher. Three learners share her camp.
3. Bottom-left hint: **G** jumps the god camera to the next continent. **Shift + WASD** flies the ocean between lands. The other seven lands are flat placeholder discs (grass or dirt) with a hearth and two shelters.
4. Pin someone (click or Tab). A teacher’s card says **Teacher** and a high know value. A learner says **Learning**. Stand near a teacher for a few seconds and the learner’s know number climbs.
5. Listen. Hunt, eat, sleep, talk, weather, teach, and build each pull from their own pool. The same person should not repeat the identical line back to back.
6. Weather keys **1–4** are still global. **0** clears. People on every land react.
7. After a tribe’s know climbs, someone walks off and another shelter appears. The pin card’s claim grows. The outer part of each continent stays unclaimed.
8. Two adults (21+) of one tribe who stay fed and close can add one more adult at the fire. The top bar’s **kin** count goes up. There is no child mesh and no intimacy scene.

`Content/MetaHumans/` is unchanged. Mara is still the only MetaHuman slot when that Blueprint is present.
