
# Qwy2 TODO list

## Build system

- **Optimization:** Avoid recompiling everything when all that changes is the linking command. This could be done by storing the object files and translation unit data in directories that only depend on the compiling command (and not on the linking command).
- **Feature:** Make sure that halting with Ctrl+C at any time during any step of the build process does not break and require `--clean`ing.
- **Optimization:** Support [precompiled header files](https://gcc.gnu.org/onlinedocs/gcc/Precompiled-Headers.html).
- **Optimization:** Support running multiple instances of the compiler at once via nonblocking calls to make use of more cores (up to some input number of cores).
- **Feature:** Support Windows (using something like [MinGW](https://www.mingw-w64.org) and maybe other compilers too).
- **Feature:** Ask for auto install missing dependencies.

## Qwy2

- **Optimization:** When the player is falling towards unloaded chunks, these chunks should be generated as fast as possible (put all the threads on these chunks) so that the player hits ground faster.
- **Optimization:** Do not load chunks that are completely covered, focus on the surface instead. 
- **Optimization:** Unload PTG and PTT fields that are no longer required for any chunk generation. 
- **Feature:** Save/load player position.
- **Feature:** Save/load configuration info such as chunk side, the terrain generator and its parameters, etc.
- **Feature:** Add simple entities that are displayed, have directional shadows, move around, collide with blocks, are saved, do not randomly dispawn.
- **Optimization:** Compress chunk data saved to the disk.
- **Feature:** Display 2D text (with some pixely font inspired by Minecraft's or PICO-8's?), and add modes to display all kinds of info in corners (such as number of chunks at each generating step, player coords, pointed block coords, player chunk coords, FPS count, etc.).
- **Feature:** Allow typing commands at run-time.
- **Feature (quality of life):** Auto-complete commands being typed.
- **Feature:** Add tree-like structures.
- **Feature:** Add procedurally generated *types* of structures.
- **Feature:** Add biomes to some terrain generators. Biomes should only be terrain-generation-related (meaning that the game should behave as if biome information was lost once the terrain is generated).
- **Feature:** Add procedurally generated *types* of biomes.
- **Feature:** Add procedurally generated *types* of blocs.
- **Feature:** Add some kind of "survival" game mode.
- **Fix:** Rewrite the player collision with blocks code entirely. It is currently so ugly it can be considered a bug.
- **Optimization?:** Generate the chunk meshes sooner (without the whole neighborhood) and update the edges of the meshes as neighbors are generated.
- **Optimisation:** Allow chunks to have multiple meshes so that chunks with too much faces could internally have regions with separate meshes to require less remeshing work when a block is changed.
- **Debugging feature:** Allow for chunks to have their PTG fields be given meshes to display as soon as these fields are generated.
- **Optimization:** When in normal view, don't render chunks with empty meshes.
- **Optimization:** When in normal view, don't render chunks that are completely covered.
- **Optimization:** When in normal view, don't render chunks that are outside of the view.
- **Optimization:** When in normal view, don't render chunks that are behind chunks that act as opaque walls.
- **Optimization:** Reduce the shadow mapping time.
- **Feature (visual):** Display some sort of star in the sun's direction.
- **Feature (visual):** Keep the low-resolution shadow map that covers a big area, and add a smaller (thus higer-resolution) shadow map to get better shadows in a smaller area around the player (or rather around the camera).
- **Feature (visual):** God rays.
- **Feature:** Block face texture generator that can be tested without launching the whole game to allow easier tweaking and debugging of this specific feature.

### Ideas for some far future (don't think too much about it for now)

- **Idea (gameplay):** Spawning and respawning happen by hatching from an egg. There would be plenty of eggs generated in the world at the beginning. Eggs could have some randomized weird properties and all. The body hatching from the egg could have some randomized properties too.
- **Idea (gameplay):** Procedural infinite magic system. A procedurally generated alphabet could be used via custom keybindings to type words of power in a textbox, allowing for magic to be performed. The grammar and rules that makes these words take effect would be procedurally generated, with plenty of exceptions, weird rules, etc. It would allow for virtually anything, use up mana or something, allow to place magic circles, etc.
- **Idea (gameplay):** Procedural alchemy system. A procedurally generated graph of possibilities allowing to transform certain materials into others via reactions.
- **Idea (gameplay):** Liquid physics would be less weird than Minecraft's. A cool idea would be to handle one body of water for example as one object with a fixed amount of water in it, and it would synchronize with the grid of blocks to maintain a coherent shape (a list of blocks and how much they are filled). While trying to enforce the best we can the conservation of the quantity of water, it could exit the body of water in the form of drops, flows, etc. I like the idea of a quantity-preserving liquid system so much! **Problem:** This way of handling liquids does not seem compatible with large bodies of water like oceans or rivers too large to fit in the generated zone of the world. That problem could be addressed later.
- **General idea (gameplay):** The game should not pretend to not be a game. We all know that while the player is somewhere, only a small zone around them is actually experiencing the passing of time. We all know that the "soul" of the player is not in the world of the game (as opposed to the soul of other entities). The game should use these kinds of terms (the *Passing of Time*, the *Soul of the Player*, etc.) and integrate these concepts in the game (like having some boss event happen because a powerful entity wanted to feel the warmth of the Passing of Time, chunk loaders being called artifacts that allow the Passing of Time, etc.).
- **Idea? (gameplay):** (Not so sure about this one anymore.) Two adjacent blocks are "touching" each other on one of their face. The liaison between them could be handled in the game as these being stuck together for a specific reason. For example, in a mountain, two adjacent blocks of stone are "linked" by the fact that they are part of a greater structure, the same goes for two adjacent wood blocks of a tree. But once a block has fallen to the ground, it is adjacent to the floor only because it lies on it (meaning a weaker link than two blocks of a mountain). A (very) small set of blocks not linked to anything else could fall.
- **Idea (gameplay):** Way less inventory space than in Minecraft. When mining in a rock wall, the stone you get have to be transported elsewhere, it cannot be compressed in a wierdly immense inventory space that could contain an entire mountain. Carrying a whole heavy block should not be easy in early-game, but carrying a small stone (item) obtained from a lager block that have scattered it more realistic (realism in absolutely not what this game should feel like, this decision is arbitrary). Digging or filling a 100x100x100 area are not to be reasonably feasible via bare hands, but mid-game mechanics such as the magic system and alchemy system might help doing stuff quicker (although it may require some creativity, for example the magic system may allow growing weird pillars for some material from the ground, these could then be sculpted a bit to get the desired form, etc.). The same goes for transporting huge amount of materials: it should be done in creative ways such as by physically filling a room to which the player has a quick way to teleport to and from.
- **Idea (gameplay):** Portals, dimensions, procedural, etc. There shall be an infinite amount of procedurally generated dimensions (3D grid of blocks such as the starting dimension) between which procedurally generated ways to travel shall be generated (such as naturally generated or buildable/craftable/castable portals, rituals, respawning, etc.). Fast ways to travel withing one dimension should also exist (same ways, magic, etc.). All these should not be too easy to find and use.
