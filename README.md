
# Qwy2 - A wannabe Minecraft-like game

<p align="center"><img src="./pics/band_round.png" width="95%"/></>

Very early alpha indev work-in-progress v0.0.0 state, there is not much to see yet. For now all you can do here is move around in a voxel world and place/remove blocks.

## Build and run on Linux

### Dependencies

For now, Qwy2 depends on [GLM](http://glm.g-truc.net/0.9.8/index.html) (`libglm-dev`) and [SDL2](https://www.libsdl.org/download-2.0.php) (`libsdl2-dev`).

Qwy2 uses the [OpenGL](https://www.khronos.org/opengl/wiki/FAQ#What_is_OpenGL.3F) 4.3 graphics API, there should be nothing to install on Linux for this (except maybe some drivers in case the game does not work). [GLEW](http://glew.sourceforge.net/) (a cross-platform OpenGL extension loader) can be linked and used in Qwy2, but this is disabled by default, and Qwy2 does not make use of OpenGL extensions (and does not plan to), thus this is not a dependency.

Qwy2 is a [C++17](https://en.cppreference.com/w/cpp/compiler_support/17) project that uses the C++ standard library, which is thus a dependency (an implementation of which should be installed by default, but in case it is not then any recent implementation should do).

C++ compilers tested and supported by the build system include [GCC](https://gcc.gnu.org/) (`g++`) and [Clang](https://clang.llvm.org/) (`clang`). The build system is written in [Python 3](https://www.python.org/downloads/) (`python3`) (Python 3.9 or higher should do (but not older versions as this script uses some dictionary operators added in 3.9), it was out in 2020 so it seems reasonable).

The building process has only been tested on Linux (Ubuntu 18.04 LTS in the past and Ubuntu 22.04 LTS for the last commit by me) yet, it is likely to fail on very different systems for now.

### Build

The build system is in `buildsystem/`, written in Python 3, and can be invoked with the `bs.py` Python script in the repo's root folder.

For a *release build*, simply run it:

```sh
python3 bs.py
```

It can take some command line arguments such as `-d` for a *debug build*. Use `-h` instead to get a help message containing the possible arguments.

The compiled binary will be `bin/Qwy2`, build artifacts will be in the `build` directory.

If the build fails (on Linux) for some reason, please post an issue, I want it to be as portable as possible (but only on Linux for now).

### Build and run

The `-l` command line argument given to `bs.py` will make it run the compiled binary (with `bin` as the current directory) if the compilation is successful, and arguments that follow `-l` are forwarded to the compiled binary (so arguments that are for the build system must be before the `-l`).

Thus the most useful command during development is the following:

```sh
python3 bs.py -l
```

### Clearing build artifacts

If the build system breaks or something, `--clear` is the way to go, the great eraser:

```sh
python3 bs.py --clear --dont-build
```

## The game

It is in a so early state that everything is subject to change anytime soon.

### Some command line options

These are given here with their default value (at the time of writing this).

- **Performances and memory usage:** `--loaded-radius=160` sets the radius (in blocks) of the visible spherical zone of the world that is generated, bigger means more chunks to generate and load in memory and to manage, smaller means you see less cool stuff. `--loading-threads=2` sets the number of threads used for generating the terrain (setting it to about the number of physical cores of your computer seem to work fine, try and see what works best (and get these fans running, you didn't buy all these CPU cores to let them idle!)). `--chunk-side=25` sets the length (in blocks) of the edges of the chunks (must be an odd number btw), bigger means more time to generate and mesh (and remesh (which can cause the game to freeze if chunks are too big when modifying their meshes)), smaller means more chunks to manage (see what works best on your machine, maybe try odd values in the range 13~39).
- **Saving and loading:** `--load-save=false` disables/enables the saving and loading of the world data (set it to `true` if you want whatever you do in the world to be saved when quitting the game). `--save-name=the` selects the save directory to save to and to load from (and creates it if it does not exist), accepted characters in names are currently restricted to `a`-`z`, `A`-`Z`, `0`-`9`, `_` and `-`. `--save-only-modified=true` keeps untouched chunks from being saved to the disk, which saves a lot of disk space, but these unsaved chunks will have to be regenerated next time (generation of a chunk takes longer than loading from disk).
- **Terrain generation:** `--terrain-generator=classic` selects a terrain generator. The list of terrain generator names is in the implementation of `plain_terrain_generator_from_name` in `src/terrain_gen.cpp`. The ones to check (at the time of writing) are `octaves_2`, `funky_2` and maybe `planes`. There are also `flat` and `hills` that are less fancy (for quick tests). Some parameters that influence some terrain generators are `--noise-size=15.0`, `--density=0.5`, `--terrain-param-a=1.0`, `--terrain-param-b=1.0`, `--terrain-param-c=1.0` and`--seed=9`. `--structures=true` enables/disables the generation of structures.
- **Graphics:** `--shadow-map-resolution=4096` sets the length (in fragments (pixels)) of the side of the square shadow map framebuffer (it must be a power of two (2)), bigger means better shadows (try `8192`, `16384` or even `32768` if your graphics card is gaming enough ^^), smaller means quicker shadow mapping. `--fovy=1.05` sets the FOV (Field Of View) parameter of the camera (the default value is actually whatever `TAU / 6.0f` approximates to with `float`s), lower (like `0.6`) is more zommed-in and higer (like `2.0`) is more "Quake Pro"-y.

Here is a base example (stuff may take some time to appear, try pressing F10 to display chunk borders and see that stuff is actually generating (just that it may be empty for a time until you hit ground)):

```sh
python3 bs.py -l --loading-threads=6 --loaded-radius=200 --terrain-generator=octaves_2 --noise-size=35 --load-save=true --save-name=gaming-moment
```

Here is the same example with saving everything for a faster loading the second time:

```sh
python3 bs.py -l --loading-threads=6 --loaded-radius=200 --terrain-generator=octaves_2 --noise-size=35 --load-save=true --save-name=gaming-moment --save-only-modified=false
```

### Controls and commands

When running the game, a `commands.qwy2` file will be created in the current directory (which will be `bin` if the game is run via the recommended `python3 bs.py -l` command) and filled with default commands, if this file did not already exist. These commands are run at the beginning of execution of the game, and some of these commands bind keyboard keys and mouse buttons to other commands. The idiomatic Qwy2 way of configuring the controls is to modify this file to bind whatever you want to whatever commands you want. For example, one of the commands that is generated when creating `commands.qwy2` at the first execution is `bind_control KD:space [player_jump]`, it binds the event `space` `K`ey `D`own to the command `player_jump`, so that when the keyboard `K`ey named `space` is pressed (`D`down), the command `player_jump` is run (which makes the player jump, there is no trap here). The syntax for commands may change, but for now: one line per command, empty lines and lines starting with `#` are ignored, a command consists of a command name followed by its space-separated arguments. An event that `bind_control` accepts must match `(K|M)(U|D):([a-z0-9_]+)` with `K` being for keyboard keys and `M` for mouse buttons, `U` for up (released) and `D` for down (pressed), and the name of the key/button in snake case, no spaces in this. Stuff between brackets like `[player_jump]` is actually a command that can be passed as an argument to an other command (as is done with each `bind_control`), any command can be in brackets and passed like that, even an other `bind_control` command. Note that a key can be bound to multiple commands (by running multiple `bind_control`s on the same key), this might (or might not) change.

The list of command names and their soure code can be found in `src/command.cpp`. The list of key/button names can be found in `keycode.cpp`.

If your keyboard layout is QWERTY for example, you want to exchange `z`s with `w`s and `q`s with `a`s in `bin/commands.qwy2` (run the game once if this file does not exist yet) after `KD:`s and `KU:`s, because the default commands configure controls for AZERTY keyboards. Default controls adapting to keyboard layouts shall be added in the future.

### What this project will become (if my motivation does not disappear into the void too soon)?

<details>
<summary>Bottom text</summary>

Minecraft is very cool, but like all games that generate the world (or anything) procedurally, we eventually grow accustomed to what does not change (the block types, the biomes, the entities, the interface, etc.). I used to install a lot of mods to add content to the world generation, new entities, etc. to keep alive the possibility of discovering new content when exploring and experimenting. This is what turns me on: discovering new, unexpected stuff. Qwy2 is meant to eventually get a powerful procedural generation that will be able to generate new biome types, new block types, new entities, new laws of physics, new dimensions, new structures, new items, etc. Imagine how cool would that be! Obviously, some stuff would sill have to be hardcoded, but I want the hardcoded border to be one level farther than Minecraft's about a lot of in-game concepts to make Qwy2 worlds and parts of worlds feel more unique.
</details>

Also the build height limits are extremely frustrating, so Qwy2 has infinite world generating in all 3 axis (6 directions).

Note: For now, these are nothing more than hopes and dreams that everyone could have. I hope the Force will help me keep focusing on this project until these become fully implemented features!

A [TODO list](TODO.md) is available and contains some entries for both short-term implementation details as well as long-term dream features.

## FAQ

### Why C++? Why not in Rust?!

I know I know. Don't get me wrong: Rust is awesome. But I need to know C++ better to get more precise arguments against it. If there is ever a Qwy3, it will be in Rust.

### Why C++17? Why not C++20 or even newer?!

I know I know. Don't get me wrong: C++20 seems better than C++17. But the language server that my setup uses crashes when certain features of C++20 are used... It is probably too young for proper tooling, and C++17 is good enough for now. (This was written in April 2022.)

### Why OpenGL? Why not Vulkan?!

I know I know. Don't get me wrong: Vulkan seems awesome and will probably slowly but surely replace OpenGL in the years to come (it seems this process is already on its way). But right now I don't know Vulkan. When this changes, Vulkan support will probably be added to Qwy2. It could be neat to have a support for both.

### Why the SDL2? Why not something else?!

I know I know. Don't get me wrong: something else could be awesome too. But I feel comfortable with the SDL2. At some point, it could be cool to also support native window handling APIs.

### Why a custom build system written in Python? Why not CMake?!

I know I know. Don't get me wrong: CMake is awesome. Wait, no, I disagree here. Although a subjective (but somewhat popular) opinion, I dislike CMake to a point where I prefer a custom-made build system. Maybe everything that the build system here does could be done via CMake with less effort, but I don't know about that. Maybe, in the future, if a lot of people complain about CMake not being the build system here, then maybe, maybe, it will be supported as a plan-B build system, maybe. Maybe not.

### Why Python? Why not Bash/Ruby/Pearl/JavaScript/Befunge-93/whatever?!

I know I know. Don't get me wrong: [Befunge-93](https://youtu.be/dQw4w9WgXcQ) is awesome. But I am more comfortable in Python 3, and it is quite widespread. People are likely to have it installed or install it easily, and know enough of it to fix/adapt the build system if needed.

### Why tabs? Why not 4 spaces?!

I know I know. Don't get me wrong: 4 spaces are the most widely used convention out there, so much so that it is the default for the Rust formatter. But I don't care, we could talk about this for hours, in the end it does not matter, just configure your IDE or something. I feel more comfortable with tabs.

## FAQ - Part 2

### What to expect from this project?

I don't even know. On the one hand, I have abandoned many projects in the past, on the other hand, "making Minecraft" was pretty much what had me started programming 10 years ago. Now that I have the power to do it, it would be a shame to not do it.

### Reporting a bug or an issue, suggesting a feature, etc.?

Sure sure go ahead and post an issue!

### Contributing?

Well I dunno, bug fixes are welcome, but this project is kinda my dream project. I want to be able to show this repo as an achievment, and I have some strong visions about what this should become.

Feel free to clone/fork it and do your stuff, and we can talk about cool ideas, that is for sure!

### License?

[Apache License 2.0](LICENSE)

Pretty permissive. It is like the MIT license, but different. Harder to read, but there are [plenty of](https://www.whitesourcesoftware.com/resources/blog/top-10-apache-license-questions-answered/) [explanations](https://fossa.com/blog/open-source-licenses-101-apache-license-2-0/) [of what it says](https://en.wikipedia.org/wiki/Apache_License). Basically (if I understand correctly): you can do what you want with this project, the modifications are to be summed up somewhere with a copy of the license, don't sue me, can be sold, etc.

Unless stated otherwise, the code and assets in this repo are original work to which this license applies. If code or specific algorithms are "stolen", the source would be mentionned in comments. Feel free to steal bits of code too, its open source after all!

### If this is Qwy2, it means there was a Qwy1 before, right?

Yep, [here](https://github.com/anima-libera/qwy), but it is quite dusty by now. It died due to not enough time and me being not powerful enough at the time (I mean, look at the code, ugh..).

Basically Qwy1 does not exist, and the story starts at Qwy2. Like Python.

### Do you know about this other Minecraft-like stuff?

Probably haha. I mean maybe not, there are so many of them. If you are intrested in Minecraft-likes, you should keep an eye on [Hytale](https://hytale.com/) (read all their blog entries!), it looks extremely cool (but only exists in the future, for now).

Here are some YouTube channels *of interest*, some of them have done devlogs on Minecraft-likes:

- [Hopson](https://www.youtube.com/c/Hopsonn)
- [jdh](https://www.youtube.com/c/jdhvideo)
- [GamesWithGabe](https://www.youtube.com/c/GamesWithGabe) (also on Twitch)
- [Henrik Kniberg](https://www.youtube.com/c/henrikkniberg) (Minecraft dev!!)
- [Gabe Rundlett](https://youtube.com/channel/UCTceODz7ynphUfT8QnFduZQ)

Here are some GitHub repos of Minecraft-likes, (not looked at the code in detail yet, might steal multiplayer stuff some day):

- [Minetest](https://github.com/minetest/minetest) (C++, [Irrlicht](https://irrlicht.sourceforge.io/?page_id=45), big projet)
- [Terasology](https://github.com/MovingBlocks/Terasology) (Java, seems big too)
- [A Tiny Minecraft Clone](https://github.com/swr06/Minecraft) (C++, Modern OpenGL)
- [Craft](https://github.com/fogleman/Craft) (C, Modern OpenGL)
- [AltCraft](https://github.com/LaG1924/AltCraft) (C++, Modern OpenGL)
- [Freeminer](https://github.com/freeminer/freeminer) (C++, [Irrlicht](https://irrlicht.sourceforge.io/?page_id=45), medium?)
- [ClassiCube](https://github.com/UnknownShadow200/ClassiCube) (C, OpenGL 1 and 2 and Direct3D 9 and 11)
- [Minecraft-Clone](https://github.com/Aidanhouk/Minecraft-Clone) (C++, Modern OpenGL)
