#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <stdio.h>
#include <sys/types.h>

using namespace std;

extern bool is_background_command;
extern bool is_foreground_command;
extern pid_t current_pid; ////// check
extern bool is_curr_cmd_done;
extern bool is_timeout;
extern bool is_current_internal;
extern Command *currCommand;
extern pid_t * signal_pid;
extern char fg_cmd_line[COMMAND_LINE_MAX_LENGTH];
extern bool is_fg_command;
extern bool is_cp;
extern JobsList *global_jobs_list;
extern bool is_cat;
extern char cp_cmd_line_for_ctlr_z[COMMAND_LINE_MAX_LENGTH];
extern char cat_cmd_line_for_ctlr_z[COMMAND_LINE_MAX_LENGTH];


void ctrlCHandler(int sig_num) {
    // TODO: Add your implementation
    cout << "smash: got ctrl-C" << endl;
    if (is_curr_cmd_done || is_current_internal || current_pid == -1) {
        if (signal(SIGINT, ctrlCHandler) == SIG_ERR) {
            perror("smash error: failed to set ctrl-C handler");
        }
        return;
    }
    if (kill(current_pid, SIGINT) == -1) {
        perror("smash error: kill failed");
    } else {
       // is_curr_cmd_done = true;
        cout << "smash: process " << current_pid << " was killed" << endl;
        is_curr_cmd_done = true;
        current_pid = -1;
    }
}

