![Logo](resources/autosim_logo_scaled.png)

# INFO

Automotive simulator application, built with gtk.

Currently for Linux OS only, tested on Raspberry CM5.

## Development Information

### Version Information:

| Version | Date | Notes |
|---|---|---|
| 0.01 | 29/09/2025 | Initial workspace interface |
| 0.10 | 02/10/2025 | Workspace interface with macros, demonstration workspace, no backend |
| 0.20 | 03/10/2025 | Sample backend GPIO development, configuration external. Working on backend config and windows app version development |
| 0.30 | 06/10/2025 | Configuration window for GPIO config. More description of use. |

### Development list

For V1:

| __Developed__ | Testing | In Development (no particular order) | Future work (no particular order) |
|---|---|---|---|
| Makefile | Logo for app, taskbar | Settings interface | Ability to edit workspaces in-app |
| Basic frontend UI | Backend interfaces (linked to below) | Button/ switch identifier | Macro creation in app |
| Loading worksapces from easy file format | Windows compatibility | Relevant documentation | Macro recording |
| Controls interface |  | EOL tester app | Licence management |
| Workspace interface/ menu bar |  |  | Docker container build |
| Descriptive files |  |  | GitLab integration |
| Macro controls (with headless macro-runner) |  |  | Logs |
| Initial backend integration |  |  | PWM control |
| Demonstration workspace(s) and macro |  |  | CAN command control |
| Communication interface |  |  | Crank level control |

## Build Information

Install dependencies with:
```
make install-deps
```
Build application with: 
```
make
```
This will also build:
 - The independent macro_runner, review [Readme_macro.md](ind_macro_app/Readme_macro.md) for further explanation.
 - The independent eol_tester, review [Readme_eol.md](eol_app/Readme_eol.md) for further explanation.

Run with:
```
./autosim2
```

## User Information

 - New workspace will load a default workspace and interface.

 - Load workspace is used to load a .csv defined workspace.

 - To run Macro it must be loaded in the workspace with corresponding controls.

 - Controls can be interacted with and will de logged in the terminal for checking/ debugging.

## GPIO configuration

Field Breakdown:
 - ControlName: Exact name from your workspace (e.g., "Speed Control")
 - ControlType: 0=Button, 1=HSlider, 2=VSlider, 3=Switch
 - GPIOPin: BCM GPIO pin number (0-27)
 - MinValue: Control's minimum value
 - MaxValue: Control's maximum value
 - InvertLogic: 0=Normal, 1=Inverted HIGH/LOW

How It Works:
 - User moves "Speed Control" slider to 75%
 - Frontend calls backend with the new value
 - Backend looks up "Speed Control" in mapping file
 - Finds it, maps to GPIO pin 18 with range 0-100
 - Converts 75% to PWM duty cycle on GPIO 18
 - External hardware responds

## Licence/ Declaration

All code is developed by John Savill, not for public use.
