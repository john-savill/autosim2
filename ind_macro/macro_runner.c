#include "macro_runner.h"

bool load_macro_from_csv(MacroDef *macro, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open macro file: %s\n", filename);
        return false;
    }
    
    char line[512];
    int line_number = 0;
    
    // Initialize macro
    memset(macro, 0, sizeof(MacroDef));
    
    printf("Loading macro from: %s\n", filename);
    
    // Skip header line if it exists
    if (fgets(line, sizeof(line), file)) {
        line_number++;
        // Check if it's a header line
        if (strstr(line, "MacroName") || strstr(line, "ActionType")) {
            printf("Skipping header line\n");
        } else {
            // Process this line as it's not a header
            rewind(file);
            line_number = 0;
        }
    }
    
    while (fgets(line, sizeof(line), file) && macro->action_count < MAX_ACTIONS) {
        line_number++;
        
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Skip empty lines and comments
        if (strlen(line) == 0 || line[0] == '#') {
            continue;
        }
        
        // Parse CSV line: MacroName,ActionType,TargetName,Value,DelayMS,Message
        char macro_name[MAX_NAME_LENGTH] = "";
        char action_type[64] = "";
        char target_name[MAX_NAME_LENGTH] = "";
        char value_str[64] = "";
        char delay_str[64] = "";
        char message[MAX_NAME_LENGTH] = "";
        
        // Simple CSV parsing with support for optional message field
        int parsed = sscanf(line, "%99[^,],%63[^,],%99[^,],%63[^,],%63[^,],%99[^\n]", 
                           macro_name, action_type, target_name, value_str, delay_str, message);
        
        if (parsed >= 4) { // At least 4 fields required
            // Set macro name if not set yet
            if (strlen(macro->name) == 0) {
                strcpy(macro->name, macro_name);
                printf("Macro name: %s\n", macro->name);
            }
            
            MacroAction *action = &macro->actions[macro->action_count];
            
            // Parse action type
            if (strcmp(action_type, "BUTTON_CLICK") == 0) {
                action->type = MACRO_BUTTON_CLICK;
            } else if (strcmp(action_type, "SLIDER_SET") == 0) {
                action->type = MACRO_SLIDER_SET;
            } else if (strcmp(action_type, "WAIT") == 0) {
                action->type = MACRO_WAIT;
            } else if (strcmp(action_type, "SWITCH_ON") == 0) {
                action->type = MACRO_SWITCH_ON;
            } else if (strcmp(action_type, "SWITCH_OFF") == 0) {
                action->type = MACRO_SWITCH_OFF;
            } else if (strcmp(action_type, "PRINT_MESSAGE") == 0) {
                action->type = MACRO_PRINT_MESSAGE;
            } else {
                fprintf(stderr, "Warning: Unknown action type '%s' on line %d\n", action_type, line_number);
                continue;
            }
            
            // Copy target name
            strcpy(action->target_name, target_name);
            
            // Parse value
            if (strlen(value_str) > 0 && strcmp(value_str, "-") != 0) {
                action->value = atof(value_str);
            } else {
                action->value = 0.0;
            }
            
            // Parse delay
            if (strlen(delay_str) > 0 && strcmp(delay_str, "-") != 0) {
                action->delay_ms = atoi(delay_str);
            } else {
                action->delay_ms = 100; // Default 100ms delay
            }
            
            // Copy message if provided
            if (parsed >= 6 && strlen(message) > 0) {
                strcpy(action->message, message);
            }
            
            macro->action_count++;
            printf("Loaded action %d: %s -> %s\n", macro->action_count, action_type, target_name);
        } else {
            fprintf(stderr, "Warning: Invalid line format on line %d: %s\n", line_number, line);
        }
    }
    
    fclose(file);
    
    if (macro->action_count == 0) {
        fprintf(stderr, "Error: No valid actions found in macro file\n");
        return false;
    }
    
    printf("Successfully loaded macro '%s' with %d actions\n", macro->name, macro->action_count);
    return true;
}

void msleep(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

void execute_action(MacroAction *action) {
    switch (action->type) {
        case MACRO_BUTTON_CLICK:
            printf("[ACTION] Clicking button: '%s'\n", action->target_name);
            // In a real implementation, this would interface with hardware/system
            break;
            
        case MACRO_SLIDER_SET:
            printf("[ACTION] Setting slider '%s' to: %.2f\n", action->target_name, action->value);
            // In a real implementation, this would interface with hardware/system
            break;
            
        case MACRO_SWITCH_ON:
            printf("[ACTION] Turning switch '%s' ON\n", action->target_name);
            // In a real implementation, this would interface with hardware/system
            break;
            
        case MACRO_SWITCH_OFF:
            printf("[ACTION] Turning switch '%s' OFF\n", action->target_name);
            // In a real implementation, this would interface with hardware/system
            break;
            
        case MACRO_WAIT:
            printf("[ACTION] Waiting %.0f ms\n", action->value);
            msleep((int)action->value);
            break;
            
        case MACRO_PRINT_MESSAGE:
            if (strlen(action->message) > 0) {
                printf("[MESSAGE] %s\n", action->message);
            } else {
                printf("[MESSAGE] %s\n", action->target_name);
            }
            break;
            
        default:
            fprintf(stderr, "[ERROR] Unknown action type\n");
            break;
    }
}

void execute_macro(MacroDef *macro) {
    printf("\n=== Starting macro execution: %s ===\n", macro->name);
    printf("Total actions: %d\n\n", macro->action_count);
    
    time_t start_time = time(NULL);
    
    for (int i = 0; i < macro->action_count; i++) {
        MacroAction *action = &macro->actions[i];
        
        printf("[%d/%d] ", i + 1, macro->action_count);
        execute_action(action);
        
        // Apply delay after action (unless it's the last action)
        if (i < macro->action_count - 1 && action->delay_ms > 0) {
            printf("[DELAY] Waiting %d ms...\n", action->delay_ms);
            msleep(action->delay_ms);
        }
        
        printf("\n");
    }
    
    time_t end_time = time(NULL);
    printf("=== Macro execution completed ===\n");
    printf("Total execution time: %ld seconds\n", end_time - start_time);
}

void print_usage(const char *program_name) {
    printf("Usage: %s <macro_file.csv> [options]\n\n", program_name);
    printf("Options:\n");
    printf("  -h, --help     Show this help message\n");
    printf("  -v, --verbose  Enable verbose output\n");
    printf("  -d, --dry-run  Show what would be executed without actually doing it\n\n");
    printf("Example:\n");
    printf("  %s my_macro.csv\n", program_name);
    printf("  %s my_macro.csv --verbose\n", program_name);
    printf("  %s my_macro.csv --dry-run\n\n", program_name);
    printf("CSV Format:\n");
    printf("  MacroName,ActionType,TargetName,Value,DelayMS,Message\n");
    printf("  Demo,BUTTON_CLICK,Start Button,-,1000,\n");
    printf("  Demo,SLIDER_SET,Speed Control,75,500,\n");
    printf("  Demo,SWITCH_ON,Main Power,-,1000,\n");
    printf("  Demo,WAIT,-,2000,100,\n");
    printf("  Demo,PRINT_MESSAGE,-,-,0,System Ready\n");
}

int main(int argc, char *argv[]) {
    bool verbose = false;
    bool dry_run = false;
    char *macro_file = NULL;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose = true;
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dry-run") == 0) {
            dry_run = true;
        } else if (argv[i][0] != '-') {
            // This is the macro file
            macro_file = argv[i];
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    if (!macro_file) {
        fprintf(stderr, "Error: No macro file specified\n\n");
        print_usage(argv[0]);
        return 1;
    }
    
    printf("Headless Macro Runner v1.0\n");
    printf("============================\n\n");
    
    if (verbose) {
        printf("Verbose mode enabled\n");
    }
    
    if (dry_run) {
        printf("Dry-run mode enabled (no actual actions will be performed)\n");
    }
    
    MacroDef macro;
    
    // Load macro from CSV file
    if (!load_macro_from_csv(&macro, macro_file)) {
        fprintf(stderr, "Failed to load macro from file: %s\n", macro_file);
        return 1;
    }
    
    if (dry_run) {
        printf("\n=== DRY RUN - No actions will be executed ===\n");
        printf("Macro: %s\n", macro.name);
        printf("Actions that would be executed:\n\n");
        
        for (int i = 0; i < macro.action_count; i++) {
            MacroAction *action = &macro.actions[i];
            printf("[%d] ", i + 1);
            
            switch (action->type) {
                case MACRO_BUTTON_CLICK:
                    printf("BUTTON_CLICK: %s\n", action->target_name);
                    break;
                case MACRO_SLIDER_SET:
                    printf("SLIDER_SET: %s -> %.2f\n", action->target_name, action->value);
                    break;
                case MACRO_SWITCH_ON:
                    printf("SWITCH_ON: %s\n", action->target_name);
                    break;
                case MACRO_SWITCH_OFF:
                    printf("SWITCH_OFF: %s\n", action->target_name);
                    break;
                case MACRO_WAIT:
                    printf("WAIT: %.0f ms\n", action->value);
                    break;
                case MACRO_PRINT_MESSAGE:
                    printf("PRINT_MESSAGE: %s\n", 
                           strlen(action->message) > 0 ? action->message : action->target_name);
                    break;
            }
            
            if (action->delay_ms > 0) {
                printf("    -> Delay: %d ms\n", action->delay_ms);
            }
        }
        
        printf("\n=== End of dry run ===\n");
    } else {
        // Execute the macro
        execute_macro(&macro);
    }
    
    return 0;
}