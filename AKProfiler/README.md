# AKProfiler
## Description
AKProfiler is a tool to profile Lite-c code. Lite-C is mainly used for games developed in the "3d game stuio" game engine.

## Important
This script was written in a short amount of time. MAKE A BACKUP OF YOUR PROJECT before using it, because it was pretty much not tested at all. Normally it shouldn't change any of your files, but you never know...

## How to use

Copy the "AKProifler.c" file into your project folder and include it anywhere in your project:
```
#include "AKProfiler.c"
```

The function calls will the be logged on game exit in AKP.log

If you want the functions of a specific file not to be profiled, yon add the following comment on the first line of the source code: ```//AKP_DO_NOT_PROFILE```

### TODO
* Start and stop parsing with buttons/keys
  * Very easy to implement, however need to decide how the user should interact with the plugin + see next point
* Make a graphical user-interface to show statistics of function calls
  * Ideas: generate html file, Google sheet template, draw stats inside the game window

### Restrictions
* Maximal file size to parse is by default a bit less than 100000 characters. You can change that by modifying the following line in "AKProfiler.c": ```#define AKP_MAX_SCRIPT_SIZE 100000```

### How it works
* AKProfiler.c has a startup function that execute immediatly when the engine starts.
* It immedialty pauses the game.
* It then looks for the main file of your project(First command line parameter).
* Next it parses that file and all files that are included into that file.
* It creates new temporary files for all these files with profiler functions injected at function starts/ends and wait's.
* Then it executes the main temporary file in a new acknex process.
* That process is now your game with profiling functions included in the scripts.
* When the process ends, it deletes all temporary files.
