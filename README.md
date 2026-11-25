# Input Buffer (Geode, GD 2.2091)

Buffers a jump press that arrives slightly before an orb or a landing, and fires it on the first physics step it can take effect.

## Behavior

- Press while airborne: the press is armed for N physics steps (default 6).
- While armed and the button is still held, the press is re-issued every step. Touching an orb or landing inside the window activates it immediately.
- Releasing the button, dying, or the window expiring disarms it.
- Landing consumes the buffered press (so ball/cube don't double-fire).
- Not applied to ship, UFO, wave or swing, where a press is a hold or a per-click impulse.
- Everything is counted in physics steps, so it is FPS independent. 1 step = 1/240 s at default TPS.

## Build

Requires Geode v6.0.0 (first release supporting 2.2091), Geode CLI 4.0.0+.

    geode sdk install
    geode sdk install-binaries
    geode build

Or push to GitHub and run the "Build Geode Mod" workflow, then download the `.geode` from the artifact and drop it in `geode/mods/`.

## Notes

Input buffers are treated as a cheat by the demonlist and most record lists. Do not use it for submissions.
