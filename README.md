# Biscuitwm
Just another xcb window manager.

This project is but a simple window manager, at the time of writing it does not even support ewmh, it does not even have workspaces yet.

This window manager s maintained by only one person right now, so it is quite prone to bugs, please do report them.

Anyways, on to installing the WM;
 
 The installation will be quite gruesome, as the author is not yet planning on adding installation to the makefile, that`s why you have to do quite a lot.
 
First, make sure you install all the neede packages, those are:
  libxcb1-dev libxcb-shape0-dev libxcb-randr0-dev libxcb-keysyms1-dev
  
Next, compile with make, move the result to whichever directory you want and create a desktop entry for the MW in /usr/share/xessions

While to the experienced user the process may seem quite simple, it is, in fact complicated, and for that the author apologises.

Thanks to [xwm](https://github.com/mcpcpc/xwm), [sowm](https://github.com/dylanaraps/sowm) and [bspwm](https://github.com/baskerville/bspwm)
