
# Qwy2 TODO list

## Build system

- **Fix:** Build system and artifacts should take into account the fact that having compiled an object file in release mode does not make it usable in a debug build and vice-versa. **Workaround:** The current workaround is to `--clear` the previous builds when switching between build modes. **Fix idea:** having a `build/release` directory and a `build/debug` directory.
- **Optimization:** Save last successful link date to avoid unnecessary links.
- **Optimization:** Save last successful build date for each translation unit to avoid rebuilding those that already passed.
- **Optimization:** Scanning the header and source files for `#include` directives and obtain a file dependency graph to use to reduce the set of effectively changed source files when some header files are changed.
- **Optimization:** Support [precompiled header files](https://gcc.gnu.org/onlinedocs/gcc/Precompiled-Headers.html).
- **Optimization:** Support running multiple instances of the compiler at once via nonblocking calls to make use of more cores (up to some input number of cores).
- **Refactoring:** Split it into multiple scripts.
- **Refactoring:** Factorize the code that handles the permanent build data sync for some arbitrary Python object.
- **Feature:** Support Windows (using something like [MinGW](https://www.mingw-w64.org) and maybe other compilers too).
- **Feature:** Ask for auto install missing dependencies.

## Qwy2

- **Feature (gameplay):** Allow placing and breaking blocks.
- **Feature (gameplay):** When in first-person view, select a block via a raycast.
- **Feature (visual):** Display some sort of star in the sun's direction.
- **Feature (visual):** God rays.

- **Feature:** Block face texture generator that can be tested without launching the whole game to allow easier tweaking and debugging of this specific feature.
- **Design decision:** Entities will be handled via an Entity Component System (archetypal). The `Player` class will be removed.

### Ideas for some far future (don't think too much about it for now)

- **Idea (gameplay):** Spawning and respawning happen by hatching from an egg. There would be plenty of eggs generated in the world at the beginning. Eggs could have some randomized weird properties and all. The body hatching from the egg could have some randomized properties too.
- **Idea (gameplay):** Procedural infinite magic system. A procedurally generated alphabet could be used via custom keybindings to type words of power in a textbox, allowing for magic to be performed. The grammar and rules that makes these words take effect would be procedurally generated, with plenty of exceptions, weird rules, etc. It would allow for virtually anything, use up mana or something, allow to place magic circles, etc.
- **Idea (gameplay):** Procedural alchemy system. A procedurally generated graph of possibilities allowing to transform certain materials into others via reactions.
- **Idea (gameplay):** Liquid physics would be less weird than Minecraft's. A cool idea would be to handle one body of water for example as one object with a fixed amount of water in it, and it would synchronize with the grid of blocks to maintain a coherent shape (a list of blocks and how much they are filled). While trying to enforce the best we can the conservation of the quantity of water, it could exit the body of water in the form of drops, flows, etc. I like the idea of a quantity-preserving liquid system so much! **Problem:** This way of handling liquids does not seem compatible with large bodies of water like oceans or rivers too large to fit in the generated zone of the world. That problem could be addressed later.
- **General idea (gameplay):** The game should not pretend to not be a game. We all know that while the player is somewhere, only a small zone around them is actually experiencing the passing of time. We all know that the "soul" of the player is not in the world of the game (as opposed to the soul of other entities). The game should use these kinds of terms (the *Passing of Time*, the *Soul of the Player*, etc.) and integrate these concepts in the game (like having some boss event happen because a powerful entity wanted to feel the warmth of the Passing of Time, chunk loaders being called artifacts that allow the Passing of Time, etc.).
- **Idea (gameplay):** Two adjacent blocks are "touching" each other on one of their face. The liaison between them could be handled in the game as these being stuck together for a specific reason. For example, in a mountain, two adjacent blocks of stone are "linked" by the fact that they are part of a greater structure, the same goes for two adjacent wood blocks of a tree. But once a block has fallen to the ground, it is adjacent to the floor only because it lies on it (meaning a weaker link than two blocks of a mountain). A (very) small set of blocks not linked to anything else could fall.
- **Idea (gameplay):** Way less inventory space than in Minecraft. When mining in a rock wall, the stone you get have to be transported elsewhere, it cannot be compressed in a wierdly immense inventory space that could contain an entire mountain. Carrying a whole heavy block should not be easy in early-game, but carrying a small stone (item) obtained from a lager block that have scattered it more realistic (realism in absolutely not what this game should feel like, this decision is arbitrary). Digging or filling a 100x100x100 area are not to be reasonably feasible via bare hands, but mid-game mechanics such as the magic system and alchemy system might help doing stuff quicker (although it may require some creativity, for example the magic system may allow growing weird pillars for some material from the ground, these could then be sculpted a bit to get the desired form, etc.). The same goes for transporting huge amount of materials: it should be done in creative ways such as by physically filling a room to which the player has a quick way to teleport to and from.
- **Idea (gameplay):** Portals, dimensions, procedural, etc. There shall be an infinite amount of procedurally generated dimensions (3D grid of blocks such as the starting dimension) between which procedurally generated ways to travel shall be generated (such as naturally generated or buildable/craftable/castable portals, rituals, respawning, etc.). Fast ways to travel withing one dimension should also exist (same ways, magic, etc.). All these should not be too easy to find and use.
