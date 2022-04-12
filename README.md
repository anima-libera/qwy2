
# Qwy2 - A wannabe Minecraft-like game

<p align="center"><img src="./pics/band_round.png" width="70%"/></>

Very early alpha indev work-in-progress v0.0.0 state, there is not much to see yet. For now all you can do here is move around in a voxel world.

## Build and run on Linux

### Dependencies

For now, Qwy2 depends on [GLM](http://glm.g-truc.net/0.9.8/index.html) (`libglm-dev`) and [SDL2](https://www.libsdl.org/download-2.0.php) (`libsdl2-dev`).

Qwy2 uses the [OpenGL](https://www.khronos.org/opengl/wiki/FAQ#What_is_OpenGL.3F) 4.3 graphics API, there should be nothing to install on Linux for this (except maybe some drivers in case the game does not work). [GLEW](http://glew.sourceforge.net/) (a cross-platform OpenGL extention loader) can be linked and used in Qwy2, but this is disabled by default, and Qwy2 does not make use of OpenGL extentions (and does not plan to), thus this is not a dependency.

Qwy2 is a [C++17](https://en.cppreference.com/w/cpp/compiler_support/17) project that uses the C++ standard library, which is thus a dependency (an implementation of which should be installed by default, but in case it is not then any recent implementation should do).

C++ compilers tested and supported by the build system include [GCC](https://gcc.gnu.org/) (`g++`) and [Clang](https://clang.llvm.org/) (`clang`). The build system itself is a [Python 3](https://www.python.org/downloads/) (`python3`) script (Python 3.6 or higher should do (but not older versions as this script uses f-strings)).

The building process has only been tested on Linux (Ubuntu 18.04 LTS) yet, it is likely to fail on very different systems for now.

### Build

The build system is the `_comp.py` Python 3 script at the root of the project directory.

For a *release build*, simply run it:

```sh
python3 _comp.py
```

It can take some command line arguments such as `-d` for a *debug build*. Read its docstring or use `-h` to get a list of possible command line arguments.

The *release build* compiled binary will be `bin/Qwy2` and the *debug build* compiled binary will be `bin/Qwy2-debug`. Build artifacts will be in the `build` directory.

### Build and run

The `-l` command line argument given to `_comp.py` will make it run the compiled binary (from `bin`) if the compilation is successful, and arguments that follow `-l` are forwarded to the compiled binary.

Thus the most useful command durring development is the followig:

```sh
python3 _comp.py -l
```

## The game

It is in a so early state that everything is subject to change anytime soon.

### Controls

At the time of writing this README (that may not be quite kept in sync with the changes made to Qwy2), I have hardcoded some of my personal Minecraft controls, which is why *jumping is right-click* (don't ask), and forward/left/backward/right is ZQSD (AZERTY keyboard). This should change in the (near?) future and be configurable! The rest of the controls can be found either by searching the event loop in the source code or by trying all the keys (keys that have an effect on settings will result in some message being printed, that should help).

### What this project will become (if my motivation does not disappear into the void too soon)?

Minecraft is very cool, but like all games that generate the world (or anything) procedurally, we eventually grow accustomed to what does not change (the block types, the biomes, the entities, the interface, etc.). I used to install a lot of mods to add content to the world generation, new entities, etc. to keep alive the possibility of discovering new content when exploring and experimenting. This is what turns me on: discovering new, unexpected stuff. Qwy2 is meant to eventually get a powerful procedural generation that will be able to generate new biome types, new block types, new entities, new laws of physics, new dimensions, new structures, new items, etc. Imagine how cool would that be! Obviously, some stuff would sill have to be hardcoded, but I want the hardcoded border to be one level farther than Minecraft's about a lot of in-game concepts to make Qwy2 worlds and parts of worlds feel more unique.

Also the build height limits are extremely frustrating, so Qwy2 has infinite world generating in all 3 axis (6 directions).

Note: For now, these are nothing more than hopes and dreams that everyone could have. I hope the Force will help me keep focucing on this project until these become fully implemented features!
