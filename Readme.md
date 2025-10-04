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

### In-development list

(in no particular order)
For V1:
 - Logo for app, taskbar
 - Correct demo workspaces setup
 - Demonstration macros
 - Back-end interfaces
 - Settings interface
 - Communication interface
 - Windows compatibility
 - Button/ switch identifier
 - Relevant documentation

Beyond: 
 - Ability to edit workspaces in-app
 - Macro creation in app
 - Macro recording
 - Licence management
 - Docker container build
 - GitLab integration
 - EOL tester app

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
./autosim
```

## User Information

 - New workspace will load a default workspace and interface

 - Load workspace is used to load a .csv defined workspace

 - To run Macro it must be loaded in the workspace with corresponding controls

## Licence/ Declaration

All code is developed by John Savill, not for public use.