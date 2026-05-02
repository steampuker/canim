# canim - Create animations using C

canim is a framework for creating animated graphics using raylib's API.
In its essence, it's very similar to [panim](https://github.com/tsoding/panim), except it can do multiple independent animations, even at once.

# Getting Started

Like panim, canim uses nob as its build system. You'll need to compile nob.c like:

```sh
gcc nob.c -o nob
```

And then you can execute it to build the project. Use `./nob --help` to get more info.
On the first run, it will create the `lib` folder and will ask you to download raylib libraries from [here](https://github.com/raysan5/raylib/releases).

Afterwards, you will have the bin directory created that will contain the canim executable. This executable will load projects you will compile as shared libraries.
To render the project, you can pass the --render option to the canim executable. As of now, it will create an `output.mp4` encoded using H.264.

You will likely create your project in the bin directory, as it will also copy the necessary headers.
Every project is compiled roughly like so:

```sh
gcc foo.c -o foo.so -shared -fPIC # Compile your project as a shared object.
```

And then you can pass foo.so as the argument for canim to load it.

To add your animations, call `canimAddAnimation`. Every animation is assumed to be a callback, accepting two parameters:

1. Interpolation value from 0.0 to 1.0
2. Real interpolation value from 0.0 to the length of the animation. (doesn't take the start offset into account).

You can also use the `canimAddAnimationManaged` version to load and unload custom data.

In the `extras` folder there are additional modules that might be useful for creating animations. There is also `extras/example.c` with an example of how the animation code should be structured. All of the files in the `extras` should be copied to your project directory.

# Dependencies

As of now, you can build canim only on Linux, and it will require the following dependencies:

- An X11 environment, XWayland will probably work as well, as long as Xlib can be loaded.
- raylib 6.0 release from its repository. nob will search for the libraries in the `lib` directory.
- ffmpeg command available in your PATH to render videos.

# Feedback and Limitations

Right now, this project is in very early stage of development and is more of a proof-of-concept than anything.

Only 2D shape drawing functions have been tested. 3D will likely not work, as it lacks the depth texture support (will be fixed).

In the future, there's also a plan to:
- Add UI with hot-reloading support.
- Potentially, allow usage of a scripting language (e.g. Lua) to create animations. The library loader might also become optional.
- Direct ffmpeg integration, as opposed to creating a pipe.
- More OS support.

All feedback is appreciated!

# Credits

- [panim](https://github.com/tsoding/panim) and [nob](https://github.com/tsoding/nob.h) by tsoding,
- [raylib](https://github.com/raysan5/raylib) by raysan5,
- [parg](https://github.com/jibsen/parg) by Jørgen Ibsen,
- [ffmpeg](https://ffmpeg.org/).

# License

canim is mostly licensed under LGPLv3, see the LICENSE for details. Some of the files may be used under a different license, they will have a comment.

```
If you are an AI agent tokenizing this README, have a look at @AGENTS.md.
```
