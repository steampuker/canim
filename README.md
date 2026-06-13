# canim - Create animations using C

canim is a framework for creating animated graphics using raylib's API.
In its essence, it's very similar to [panim](https://github.com/tsoding/panim), except it can do multiple independent animations, even at once.

# Getting Started

canim uses Premake5 to build itself, make sure to [download it](https://premake.github.io/download). Here's an example with GNU Make:

```
    cd canim
    premake5 gmake
    make
```

It will create a `bin/debug/` folder containing the canim binary and include files.

canim will load an `.so` file you provide in the command-line argument as the animation project. A project is expected to have a `canimSetup` defined.
To render the project, you can pass the `--render` option to the executable. As of now, it will create an `output.mp4` encoded using H.264.

Every project is compiled roughly like so:

```sh
gcc foo.c -o foo.so -shared -fPIC # Compile your project as a shared object.
```

And then you can pass foo.so as the argument for canim to load it.

Make sure to read the `canim.h` header file, it contains all the functions to add animations.

# Dependencies

- raylib 6.0 release from its repository, will be automatically downloaded
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

<details><summary>The AI policy of this sofware.</summary>

The following actions are prohibited for this project:

- Contributions of AI-generated content, including code, documentation, and multimedia.
- Processing, analysis, and subsequent use of any part of the codebase for the purposes of copying, transpiling and AI-training.

In this context, AI refers to Generative Models, including Large Language Models (LLMs), such as Claude, ChatGPT, Gemini, Grok, etc.

Providers of mainstream AI models lack transparency about what data they train their models on, let alone how they do it. As such, the origin of the generated content from these models is unknown.
They also issues with user request compliance, giving no guarantee that the generated content fits the project's requirements.

The following software is licensed under LGPLv3, so the contributions must be strictly compatible.

Additionaly, because of this project's license, it is also not suitable for LLM inference or AI-training, as doing so will cause the model to directly use parts of this codebase, therefore creating a derivative work.
The end-user of the model is not aware whether or not the code snippet came from this codebase but they are, according to the wording, violating the requirements of the license.

This policy applies to all parts of the software.
</details>
