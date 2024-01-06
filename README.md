# Chess Bot

This is a simple but powerful chess AI built using C++ and SFML. The bot uses the negamax algorithm with a depth of 3 moves.

## Platform

The bot has currently only been tested on MacOS, but it will also most likely work with minor command adjustments on Linux and Windows(WSL).

## Dependencies

The only dependencies you will need to compile and run this project are CMake and OpenGL. You do not need to install SFML, as the makefile will install it locally for you.

## Compile Instructions

(Note: These instructions are for MacOS. Similar commands may work for other operating systems, but will need to be adjusted slightly.)

After cloning the repository, open a terminal window and navigate into the `ChessBot` folder. Now, run the following commands:

```zsh
ChessBot % cd build
build % cmake ..
```

Note: The second command will take a while to run, as it builds the SFML src files. This step only needs to be done once while configuring the project.

After this step, run `make`. There should be an executable inside `build` named Chess. Simply double-click on the file or run `./Chess` to start the game. You will always be white and the bot will respond as black.
