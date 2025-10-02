#ifndef MACRO_RUNNER_H
#define MACRO_RUNNER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

#define MAX_NAME_LENGTH 100
#define MAX_ACTIONS 100

// Macro action types
typedef enum {
    MACRO_BUTTON_CLICK,
    MACRO_SLIDER_SET,
    MACRO_WAIT,
    MACRO_SWITCH_ON,
    MACRO_SWITCH_OFF,
    MACRO_PRINT_MESSAGE
} MacroActionType;

// Macro action definition
typedef struct {
    MacroActionType type;
    char target_name[MAX_NAME_LENGTH];
    double value;
    int delay_ms;
    char message[MAX_NAME_LENGTH]; // For print messages
} MacroAction;

// Macro definition
typedef struct {
    char name[MAX_NAME_LENGTH];
    int action_count;
    MacroAction actions[MAX_ACTIONS];
} MacroDef;

// Function declarations
bool load_macro_from_csv(MacroDef *macro, const char *filename);
void execute_macro(MacroDef *macro);
void execute_action(MacroAction *action);
void print_usage(const char *program_name);
void msleep(int milliseconds);

#endif