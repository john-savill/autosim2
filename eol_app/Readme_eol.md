# INFO

EOL tester

Intend to develop an End of line testing application that will communicate to an ECU via CAN to ensure sensor functionality is correct.

Currently empty application, opens to empty workspace.

### Temporary local build command:
```
gcc `pkg-config --cflags gtk+-3.0` -Wall -g -o eol_app welcome_screen.c eol_app.c main.c workspace_view.c `pkg-config --libs gtk+-3.0`
```
