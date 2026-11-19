# Editing the DashDisplay UI in SquareLine Studio

The editable project is `dash-ui.spj` in this folder. Keep it alongside
`dash-ui.sll`, `Themes.slt`, and `assets/` so SquareLine can find the project
settings, images, and fonts. The checked-in project was saved with
SquareLine Studio 1.6.1. It is configured for the Raspberry Pi at 800 × 480,
with LVGL 8.3.11 and 16-bit RGB565 without byte swapping.

1. Clone DashDisplay or download the default branch ZIP and extract it.
   In SquareLine Studio, choose **File → Open** or **Open → Import Project**
   from the launcher, then select `squareline/dash-ui.spj`.
   Edit the dash or loading screen, use **Play** to preview the layout,
   and save the project.
2. Set **UI Files Export Path** to a clean temporary folder and choose
   **Export → Export UI Files**. The output should include `ui.c`, `ui.h`,
   and the `screens/`, `fonts/`, `images/`, and `components/`
   subfolders.
3. Update the generated files under [`ui/`](../ui/) from the export, keeping
   that directory structure. Review `git diff` for stale generated files,
   accidental changes, and any hand edits that need to carry forward.
   The [Makefile](../Makefile) builds `ui/`. The two `final_ui*` folders
   are older or alternate exports. DashDisplay already has its own build
   and CAN integration, so use the UI export option.
4. Keep the names of widgets referenced by
   [`src/dashboard_values.cpp`](../src/dashboard_values.cpp), such as
   `RPMVAL`, `GEAR`, and `SPEEDMPH`. If you rename one, update the
   corresponding C++ reference as part of the same change. SquareLine
   defines the widgets and layout. The C++ code supplies live CAN data
   and handles display updates and warning states.
5. With the repo's LVGL dependencies available, rebuild with
   `make clean && make` on the Pi. On Linux with SDL2 and the LVGL
   dependencies, you can also use `make UI_BACKEND=sdl clean && make UI_BACKEND=sdl`
   and run `./fsdash` for a desktop check. Test live values and warnings
   on the Pi or a CAN test setup before relying on the new layout.
   Commit the saved SquareLine source and the matching `ui/` export
   together so the next person can reproduce the build.

SquareLine records each editor's export path in a local `.slp` file.
Set this path on your machine when you export.

See the [SquareLine project settings](https://docs.squareline.io/docs/1.6.1/dev_env/project_settings/)
and [SquareLine export workflow](https://docs.squareline.io/docs/introduction/typical_dev/).
