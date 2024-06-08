# Tungsten Chess

This is a simple but powerful chess AI built using C++ and SFML. The bot uses the negamax algorithm with a minimum depth of 5 moves, and a maximum depth of 15 moves (for quiescence search).

## Platform

The bot is currently only compatible with MacOS. It is in the process of being ported to Windows.

## Dependencies

The only dependencies you will need to compile and run this project are CMake and OpenGL. You do not need to install SFML, as the makefile will install it locally for you.

If you already have SFML installed, the makefile will use the system version instead of installing a new one. (Installing SFML onto your system is recommended, as it will speed up the build process and reduce the size of the application bundle.)

## Compile Instructions

After cloning the repository, open a terminal window and navigate into the `TungstenChess` folder. Now, run the following commands:

```zsh
TungstenChess % cd build
build % cmake ..
```

Note: The second command may take a while to run if it needs to build SFML src files. This step only needs to be done once while configuring the project.

After this step, run `make`. It will create a MacOS application bundle called `TungstenChess.app`. You can run the application by double-clicking on the bundle or by running `open Chess.app` in the terminal.
