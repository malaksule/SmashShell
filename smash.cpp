#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"
#include <ctime>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
using namespace std;

extern bool is_curr_cmd_done;
extern Command *currCommand;
extern pid_t smash_pid;


int main(int argc, char* argv[]) {
   umask(022);
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }

    //TODO: setup sig alarm handler

    SmallShell& smash = SmallShell::getInstance();
    std::string prompt = string("smash");
    while(true) {
        std::cout << prompt << "> ";
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        char input_command[COMMAND_LINE_MAX_LENGTH];
        if (cmd_line.empty())
            continue;
        strcpy(input_command, cmd_line.c_str());
        smash.executeCommand(input_command);
        if(currCommand){
            if (currCommand->isChpromptFlag()) {
                if (currCommand->getArgumentNum() > 1) {
                    prompt=string(currCommand->getArgument()[1]);
                } else {
                    prompt=string("smash");
                }
            }}
        if(smash_pid!= getpid()) return 0;
        is_curr_cmd_done=true;
    }
    return 0;
}