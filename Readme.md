![Logo](resources/autosim_logo_scaled.png)

# INFO

Automotive simulator application, built with gtk.

Currently for Linux OS only

## Development Information

### Version Information:

| Version | Date | Notes |
|---|---|---|
| 0.01 | 29/09/2025 | Initial workspace interface |
| 0.10 | 02/10/2025 | Workspace interface with macros, demonstration workspace, no backend |
|0.20 | 03/10/2025 | Sample backend GPIO development, configuration external. Working on backend config and windows app version development |

### Development list

For V1:

| __Developed__ | Testing | In Development (no particular order) | Future work (no particular order) |
|---|---|---|---|
| Makefile | Logo for app, taskbar | Settings interface | Ability to edit workspaces in-app |
| Basic frontend UI | Backend interfaces (linked to below) | Button/ switch identifier | Macro creation in app |
| Loading worksapces from easy file format | Communication interface | Relevant documentation | Macro recording |
| Controls interface | Windows compatibility |  | Licence management |
| Workspace interface/ menu bar |  |  | Docker container build |
| Descriptive files |  |  | GitLab integration |
| Macro controls (with headless macro-runner) |  |  | EOL tester app |
| Initial backend integration |  |  | Logs |
| Demonstration workspace(s) and macro |  |  |  |

## Build Information

Install dependencies with:
```
make install-deps
```
Build application with: 
```
make
```
(will also make the independent macro_runner, review [Readme_macro.md](ind_macro/Readme_macro.md) for some more explanation)

Run with:
```
./autosim2
```

## User Information

 - New workspace will load a default workspace and interface.

 - Load workspace is used to load a .csv defined workspace.

 - To run Macro it must be loaded in the workspace with corresponding controls.

 - Controlscan be interacted with and will de logged in the terminal fo checking/ debugging.

## Licence/ Declaration

All code is developed by John Savill, not for public use.