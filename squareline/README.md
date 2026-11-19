# Working on the DashDisplay UI

Clone DashDisplay (or download and unzip the whole repo) and open
`squareline/dash-ui.spj` in SquareLine Studio 1.6.1. Keep the
`squareline/` folder together so the images and fonts load. The project is
already set up for the Pi's 800 × 480 display, with LVGL 8.3.11 and 16-bit
RGB565 with byte swapping off.

## Make a change and export it

1. Edit the dash or the loading screen. Hit **Play** to see how it looks, then
   save the project.
2. Choose **Export → Export UI Files** and point it at the repo's
   [`ui/` folder](../ui/). SquareLine remembers that folder on your machine.
   The Makefile builds from `ui/`. The `final_ui*` folders hold earlier versions
   and alternate exports.
3. Take a quick look at `git diff` before you commit. Keep widget names like
   `GEAR`, `RPMVAL`, and `SPEEDMPH` as they are unless you're also updating
   [the C++ code that fills them in](../src/dashboard_values.cpp).

## Try it out

Once the LVGL dependencies are set up, run `make clean && make` on the Pi.
If you're on Linux with SDL2, you can also preview the dash on your desktop.

```sh
make UI_BACKEND=sdl clean && make UI_BACKEND=sdl
./fsdash
```

Check the live values and warnings on the Pi or a CAN test setup. When it
looks right, commit your saved SquareLine project and the generated `ui/`
files together so the next person has both.

[SquareLine's export guide](https://docs.squareline.io/docs/introduction/typical_dev/)
