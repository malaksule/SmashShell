#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <algorithm>    // std::min
#include "Commands.h"
#include <ctype.h>
#include <dirent.h>
#include <cerrno>
#include <cstdio>
#include "regex"
#include "set"
#include <fstream> // Needed for netinfo
#include <arpa/inet.h>  // for inet_ntoa
#include <ifaddrs.h>
#include <netinet/in.h>
#include <net/if.h>



#ifndef COMMANDS_H
#define COMMANDS_H
#define BUFFER_SIZE 515

///////////////////
using namespace std;
bool is_fg_command;
char fg_cmd_line[COMMAND_LINE_MAX_LENGTH];
int fg_job;
bool is_background_command;
bool is_foreground_command;
bool is_current_internal;
pid_t current_pid;
pid_t * signal_pid;
bool is_curr_cmd_done;
int duration;
bool is_son1;
bool is_son2;
bool is_cp;
char cp_cmd_line_for_ctlr_z[COMMAND_LINE_MAX_LENGTH];
char cat_cmd_line_for_ctlr_z[COMMAND_LINE_MAX_LENGTH];
char *prev_direc;
char cp_cmd_line[COMMAND_LINE_MAX_LENGTH];
char cat_cmd_line[COMMAND_LINE_MAX_LENGTH];
char *curr_direc;
pid_t smash_pid;
pid_t pipe_1;
pid_t pipe_2;
int fd[2];
int output_channel;
Command *currCommand;
Command *prevCommand;
Command *newCommand;
char pipe_with_bg[COMMAND_LINE_MAX_LENGTH];
JobsList *global_jobs_list;
///////////////////////////////////////

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

#define DEBUG_PRINT cerr << "DEBUG: "

#define EXEC(path, arg) \
  execvp((path), (arg));

/////////////////////////////////////////////
JobsCommand::JobsCommand(){}

BuiltInCommand::BuiltInCommand() {}

GetCurrDirCommand::GetCurrDirCommand() {}

ChangePrompt::ChangePrompt() {}
ChangePrompt::~ChangePrompt()  {}

ShowPidCommand::ShowPidCommand() {}


//////////////////////////////////////////////
string _ltrim(const std::string& s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for(std::string s; iss >> s; ) {
        args[i] = (char*)malloc(s.length()+1);
        memset(args[i], 0, s.length()+1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign( char* cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h


void ChangePrompt::execute() {}


JobsList::JobEntry::JobEntry() {}

int min(int a, int b) {
    if (a > b)
        return b;
    return a;
}

/////////////////
void GetCurrDirCommand::execute()  {
    cout << getcwd(nullptr, 0)<<endl;
}
///////////////
void ChangeDirCommand::execute() {
    if (newPath[0] == '-') {
        if (!(*lastDir)) {
            cerr << "smash error: cd: OLDPWD not set" << endl;
            return;
        }
        if (chdir(*(lastDir)) < 0) {
            perror("smash error: chdir failed");
            return;
        }
        char *temp = *lastDir;
        *lastDir = *currDir;
        *currDir = temp;
        return;
    }
    if (chdir(newPath) < 0) {
        perror("smash error: chdir failed");
        return;
    }
    free(*lastDir);
    *lastDir = *currDir;
    *currDir=getcwd(nullptr,0);
}

////////////////////
class sort_func{
public:
    bool operator()(char* i, char* j){
        return (strcoll(i,j)<0);
    }
}vecSort;


//////////////////////////////////////////
void JobsList::addJob(Command* cmd, bool isStopped,char* cmd_line,pid_t pid ){
    if (!is_son1 && !is_son2)
        removeFinishedJobs();
    int newId;
    if (!is_fg_command) {
        newId = global_jobs_list->maxJobId() +1;
    } else {
        newId = fg_job;
    }
    JobEntry newJob = JobEntry(newId, cmd,pid,
                               isStopped,cmd_line); ///// check id
    for(ListNode<JobEntry> *it = job_list.head ; it != job_list.tail ; it=it->next){
        if(it->next != job_list.tail && it->next->data.job_id < newJob.job_id)
            continue;
        job_list.pushBefore(it,it->next,newJob);
        break;
    }
}

void JobsList::printJobsList(){
    if(!is_son1 && !is_son2)
        removeFinishedJobs();
    if(global_jobs_list->isEmpty()) return;
    for(ListNode<JobEntry> *it = job_list.head->next; it != job_list.tail ; it=it->next){
        ExternalCommand* ext = dynamic_cast<ExternalCommand*>(it->data.command);
        cout << "[" << it->data.job_id << "] ";
        if(ext != nullptr && ext->alias_key_flag)
            cout << ext->alias_key;
        else
            cout<< it->data.cmd_line;
        cout << endl;
    }
}

void JobsList::killAllJobs() {
    if (!is_son1 && !is_son2)
        this->removeFinishedJobs();
    int killStatus;
    int jobsNum=0;
    List<JobEntry> tmpList;
    for (ListNode<JobEntry> *it = job_list.head->next; it != job_list.tail; it=it->next) {
        JobEntry tmp = JobEntry(it->data.job_id,it->data.command,it->data.process_id,it->data.isStopped,it->data.cmd_line);
        tmpList.pushLast(tmp);
        killStatus = kill(it->data.process_id, SIGKILL);
        if (killStatus < 0) {
            ListNode<JobEntry> *ptr = tmpList.getLast();
            tmpList.remove(ptr);
            perror("smash error: kill failed");
            return;
        }
        jobsNum++;
    }
    cout << "smash: sending SIGKILL signal to " << jobsNum << " jobs:" << endl;
    for (ListNode<JobEntry> *it = tmpList.head->next; it != tmpList.tail; it=it->next) {
        cout << it->data.process_id << ": " << it->data.cmd_line << endl;
    }
    job_list.clearList();
    delete job_list.head;
    delete job_list.tail;
}

void JobsList::removeFinishedJobs(){

    if(global_jobs_list->isEmpty()) return;

    pid_t returnVal = -55;
    for(ListNode<JobEntry> *it = job_list.head->next ; it != job_list.tail ; it=it->next){
        returnVal = waitpid(it->data.process_id, NULL, WNOHANG);
        if(returnVal == it->data.process_id || returnVal == -1){
            job_list.remove(it);
        }
    }
}
JobsList::JobEntry * JobsList::getJobById(int jobId){
    if (!is_son1 && !is_son2)
        this->removeFinishedJobs();
    for(ListNode<JobEntry> *it = job_list.head->next ; it!= job_list.tail ; it=it->next){
        if (it->data.job_id == jobId)
            return &(it->data);
    }
    return NULL;
}

void JobsList::removeJobById(int jobId){
    if (!is_son1 && !is_son2)
        this->removeFinishedJobs();
    for(ListNode<JobEntry> *it = job_list.head->next; it!= job_list.tail ; it=it->next){
        if (it->data.job_id == jobId)
            job_list.remove(it);
    }
}

JobsList::JobEntry * JobsList::getLastJob(int* lastJobId){
    return &(job_list.getLast()->data);
}

JobsList::JobEntry *JobsList::getLastStoppedJob(){
    for(ListNode<JobEntry> *it = job_list.tail->prev ; it != job_list.head ; it=it->prev){
        if (it->data.isStopped)
            return &(it->data);
    }
    return NULL;
}

bool JobsList::isEmpty(){

    return (job_list.head->next==job_list.tail);
}

int JobsList::maxJobId(){
    if (!is_son1 && !is_son2)
        this->removeFinishedJobs();
    int max=0;
    if(global_jobs_list->isEmpty()) return max;
    for(ListNode<JobEntry> *it = job_list.head->next ; it != job_list.tail ; it=it->next){
        if (it->data.job_id > max)
            max=it->data.job_id;
    }
    return max;
}
////////////////////
void backDifSignal() {
    for (int i = 1; i < 32; i++) {
        if (i == 9 || i == 19) {
            continue;
        }
        if (signal(i, SIG_DFL) == SIG_ERR) { ////// CHECK
            perror("smash error: failed to set handler");
        }
    }
}
bool isComplexExternalCommand(const char* cmd_line){
    for (int i = 0; i < (int)(strlen(cmd_line) - 1); i++) {
        if (cmd_line[i] == '?' || cmd_line[i] == '*') {
            return true;
        }
    }
    return false;
}
void ExternalCommand::execute() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("smash error: fork failed");
    } else if (pid == 0) {
        backDifSignal();
        if (is_son1) {
            dup2(fd[1], output_channel);
            close(fd[0]);
            close(fd[1]);
        }
        if (is_son2) {
            dup2(fd[0], 0);
            close(fd[0]);
            close(fd[1]);
        }

        if (!is_son1 && !is_son2) {
            if (setpgrp() == -1) {
                perror("smash error: setpgrp failed");
                return;
            }
        }
        umask(022);//////////////// new
        _removeBackgroundSign(cmd_line);
// הסרה של פקודות ריקות לגמרי או כאלה שמכילות רק רווחים
        std::string trimmed_cmd = std::string(cmd_line);
        trimmed_cmd.erase(0, trimmed_cmd.find_first_not_of(" \t\n\r"));
        if (trimmed_cmd.empty()) {
            exit(1);
        }
        string s="/bin/bash";
        char a[s.size()+1];
        strcpy(a,s.c_str());
        string ss="-c";
        char b[ss.size()+1];
        strcpy(b,ss.c_str());

        char *args[] = {a,b, cmd_line, NULL};
        char** arguments = (char **)malloc(sizeof(char *) * (COMMAND_MAX_ARGS+1));

        int argsNum = _parseCommandLine(cmd_line,arguments);

        if(!isComplexExternalCommand(cmd_line)){
            execvp(arguments[0], arguments);
        }else{
            execvp(args[0], args);
        }
        perror("smash error: execvp failed");
        return;
    } else {
        if (is_son1) {
            pipe_1 = pid;
        }
        if (is_son2) {
            pipe_2 = pid;
        }
        if (!is_son1 && !is_son2) {
            curr_pid = pid;
            current_pid=pid;
        }
        if (!is_back_ground) {
            while (!is_curr_cmd_done) {
                int status;
                pid_t res = waitpid(pid, &status, WNOHANG);  // לא לחסום
                if (res == 0) {
                    sleep(1);
                } else if (res == pid) {
                    break;  // התהליך הסתיים
                } else if (res == -1) {
                    perror("smash error: waitpid failed");
                    break;
                }
            }
        }
        else {
            global_jobs_list->addJob(currCommand, false, const_cast<char*>(this->getCmdLine()), curr_pid);
        }
    }
}
/////////////////////////////////////
void QuitCommand::execute() {
    if(withKill)
        global_jobs_list->killAllJobs();
    exit(0);

}
//////////////////////
void KillCommand::execute() {
    JobsList::JobEntry *tmp = global_jobs_list->getJobById(jobId);
    pid_t pid;
    if (tmp==NULL){
        cerr << "smash error: kill: job-id " << jobId << " does not exist" << endl;
        return;
    }
    pid=tmp->process_id;
    if (kill(pid, sigNum) == -1){
        perror("smash error: kill failed");
        return;
    }
    else cout << "signal number " << sigNum << " was sent to pid " << pid << endl;
}
////////////////////
void JobsCommand::execute() {
    global_jobs_list->printJobsList();
}
////////////////////
void ForegroundCommand::execute() {
    JobsList::JobEntry *tmp = global_jobs_list->getJobById(jobId);
    if(!tmp){
        cerr<< "smash error: fg: job-id " << jobId << " does not exist" <<endl;
        return;
    }
    else{
        pid_t pid = tmp->process_id;
        current_pid = pid;
        strcpy(fg_cmd_line, tmp->cmd_line);
        cout << tmp->cmd_line <<" " <<  pid << endl;
        global_jobs_list->removeJobById(jobId);
        is_current_internal = false;

        waitpid(pid, NULL, WUNTRACED);

    }
}
/////////////////////////

void BackgroundCommand::execute() {
    JobsList::JobEntry *tmp = global_jobs_list->getJobById(jobId);
    if(!tmp){
        cout << "smash error: bg: job-id " << jobId << " does not exist" << endl;
        return;
    }
    else{
        cout << "smash error: bg: job-id " << jobId << " is already running in the background" << endl;
        return;
    }
}
////////////////////////////
void ShowPidCommand::execute() {
    cout << "smash pid is " << getpid() <<  endl;
}
/////////////////////////////////////////////////////////////////////new


static bool variableExists(const char* var_name) {
int fd = open( ("/proc/" + to_string(smash_pid) + "/environ").c_str(), O_RDONLY );
    if (fd ==-1) {
        std::cerr << "smash error: open failed" << std::endl;
        return false;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    size_t var_len = strlen(var_name);
    std::string leftover;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        std::string data = leftover + std::string(buffer, bytes_read);
        size_t pos = 0;

        while (pos < data.size()) {
            size_t end = data.find('\0', pos);
            if (end == std::string::npos) {
                leftover = data.substr(pos);
                break;
            }

            std::string entry = data.substr(pos, end - pos);
            if (entry.compare(0, var_len, var_name) == 0 && entry[var_len] == '=') {
                close(fd);
                return true;
            }

            pos = end + 1;
        }
    }

    close(fd);
    return false;
}

void UnSetEnvCommand::execute() {
    extern char **__environ;

    int argc = getArgumentNum();
    char** argv = getArgument();

    if (argc < 2) {
        std::cerr << "smash error: unsetenv: not enough arguments" << std::endl;
        return;
    }

    for (int k = 1; k < argc; ++k) {
        const char* var = argv[k];

        // Check if variable exists
        if (!variableExists(var)) {
            std::cerr << "smash error: unsetenv: " << var << " does not exist" << std::endl;
            return;
        }

        // Length of variable name
        size_t var_len = strlen(var);

        // Look for the variable in environ
        for (int i = 0; __environ[i] != nullptr; ++i) {
            if (strncmp(__environ[i], var, var_len) == 0 && __environ[i][var_len] == '=') {
                // Remove the variable from the environment by shifting
                for (int j = i; __environ[j] != nullptr; ++j) {
                    __environ[j] = __environ[j + 1];
                }
                break;
            }
        }
    }
}


/////////////////////////////////////////////////////////////////////new
bool isFileExists(const char *name) {
    struct stat buffer;
    return (stat(name, &buffer) == 0);
}
void CopyCommand::execute() {
    fflush(stdout);
    pid_t pid = fork();
    if (pid < 0) {
        perror("smash error: fork failed");
        return;
    } else if (pid == 0) {
        fflush(stdout);
        backDifSignal();
        if (is_son1) {
            dup2(fd[1], output_channel);
            close(fd[0]);
            close(fd[1]);
        }
        if (is_son2) {
            dup2(fd[0], 0);
            close(fd[0]);
            close(fd[1]);
        }
        if (!is_son1 && !is_son2) {
            setpgrp();
        }
        char buf[100] = "";
        int file1 = open(orig_file, O_RDONLY);
        if (file1 < 0) {
            perror("smash error: open failed");
            return;
        }
        char *file_path_n = realpath(file_path, NULL);
        char *orig_file_n = realpath(orig_file, NULL);
        if (isFileExists(file_path) && strcmp(file_path_n, orig_file_n) == 0) {
            cout << "smash: " << orig_file << " was copied to " << file_path << ""
                 << endl;
            fflush(stdout);
            free(file_path_n);
            free(orig_file_n);
            close(file1);
            return;
        }
        fflush(stdout);
        free(file_path_n);
        free(orig_file_n);
        int file2 = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (file2 < 0) {
            close(file1);
            perror("smash error: open failed");
            return;
        }
        while (1) {
            int result1 = read(file1, buf, 100);
            if (result1 == 0) {
                break;
            }
            if (result1 == -1) {
                perror("smash error: read failed");
                close(file1);
                close(file2);
                return;
            }
            int result2 = write(file2, buf, result1);
            if (result1 > result2) {
                perror("smash error: write failed");
                close(file1);
                close(file2);
                return;
            }
        }
        cout << "smash: " << orig_file << " was copied to " << file_path << ""
             << endl;
        fflush(stdout);
        close(file1);
        close(file2);
    } else {
        if (is_son1) {
            pipe_1 = pid;
        }
        if (is_son2) {
            pipe_2 = pid;
        }
        if (!is_son1 && !is_son2) {
            *curr_pid = pid;
        }
        if (!is_back_ground)
            waitpid(pid, NULL, WUNTRACED);
        else {
            if(currCommand)
                global_jobs_list->addJob(currCommand, false,cp_cmd_line,current_pid);
        }
    }
}

int stringToMode(const char* str, mode_t* mode)
{
    char* last = NULL;
    *mode = (mode_t)strtol(str, &last, 8);
    if (!last)
        return 0;
    while(isspace(*last)) last++;
    return *last == '\0' && (unsigned)*mode < 010000;
}


// extra functions
void freeArgs(char **args) {
    int i = 0;
    while (*(args + i)) {
        free(*(args + i));
        i++;
    }
}

// checks if the command is a special command
int isCommandRedirection(const char *cmd_line, int *place) {
    for (int i = 0; i < (int)(strlen(cmd_line) - 1); i++) {
        if (cmd_line[i] == '>' && cmd_line[i + 1] == '>') {
            *place = i;
            return 2;
        }
    }
    for (int i = 0; i < (int)(strlen(cmd_line) - 1); i++) {
        if (cmd_line[i] == '|' && cmd_line[i + 1] == '&') {
            *place = i;
            return 4;
        }
    }
    for (int i = 0; i < (int)(strlen(cmd_line)); i++) {
        if (cmd_line[i] == '|') {
            *place = i;
            return 3;
        }
    }
    for (int i = 0; i <(int) (strlen(cmd_line)); i++) {
        if (cmd_line[i] == '>') {
            *place = i;
            return 1;
        }
    }
    return 0;
}

// checks the status of the special commands : 1+2 is IO redirection, 3+4 is pipe
int checkCommandStatus(const char *cmd_line, char *new_cmd1, char *new_cmd2) {
    int i;
    char copy[COMMAND_LINE_MAX_LENGTH];
    strcpy(copy, cmd_line);
    _removeBackgroundSign(copy);
    int stat = isCommandRedirection(copy, &i);
    if (stat == 0) {
        return stat;
    }
    char tmp[COMMAND_LINE_MAX_LENGTH];
    strcpy(tmp, copy);
    tmp[i] = 0;
    if (stat == 1 || stat == 3) {
        strcpy(new_cmd1, tmp);
        strcpy(new_cmd2, tmp + i + 1);
        return stat;
    }
    if (stat == 2 || stat == 4) {
        strcpy(new_cmd1, tmp);
        strcpy(new_cmd2, tmp + i + 2);
        return stat;
    }
    return stat;
}

// checks if the given command is a built-in command
bool isCommandInternal(const char *cmd_line) {
    char copy[COMMAND_LINE_MAX_LENGTH];
    strcpy(copy, cmd_line);
    char **arguments;
    arguments = (char **)malloc(sizeof(char *) * COMMAND_MAX_ARGS);
    for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
        arguments[i] = NULL;
    }
    if (_isBackgroundComamnd(copy)) {
        _removeBackgroundSign(copy);
    }
    _parseCommandLine(copy, arguments);
    if (strcmp(arguments[0], "chprompt") == 0 ||
        strcmp(arguments[0], "showpid") == 0 ||
        strcmp(arguments[0], "pwd") == 0 ||
        strcmp(arguments[0], "cd") == 0 ||
        strcmp(arguments[0], "jobs") == 0 ||
        strcmp(arguments[0], "kill") == 0 ||
        strcmp(arguments[0], "fg") == 0 ||
        strcmp(arguments[0], "quit") == 0
            ) {
        return true;
    }
    return false;
}

// checks if there is a number in the given str
bool isNumber(const char *str) {
    int i = 0;
    if (*(str + i) == '-' && strlen(str) != 1)
        i++;
    while (*(str + i)) {
        char c = *(str + i);
        if (c < '0' || c > '9')
            return false;
        i++;
    }
    return true;
}

// convert a char value to int
int charToInt(const char *str, bool is_with_minus) {
    int i = 0, num = 0;
    if (is_with_minus) {
        i++;
    }
    while (*(str + i)) {
        num *= 10;
        num += ((int)(str[i] - '0'));
        i++;
    }
    if (is_with_minus)
        return (-1) * num;
    return num;
}

//added: check if octal
bool isOctal(const char *str) {
    int i = 0;
    if (*(str + i) == '-' && strlen(str) != 1)
        i++;
    while (*(str + i)) {
        char c = *(str + i);
        if (c < '0' || c > '7')
            return false;
        i++;
    }
    return true;
}
//added: check if num is 4 or less digits or is octal
bool validNumForChmod(const char *str){
    int num= charToInt(str, false);
    if( (num>9999) || (!isOctal(str)) ){
        return false;
    }
    return true;
}

/// listDir
ListDirCommand::ListDirCommand(const char* cmd_line) {
    char* args[COMMAND_MAX_ARGS];
    int argc = _parseCommandLine(cmd_line, args);

    if (argc > 2) { // Too many arguments
        std::cerr << "smash error: listdir: too many arguments" << std::endl;
        this->setArgument(nullptr, 0);
        return;
    }

    this->setCmdLine((char*)cmd_line);

    if (argc == 1) {
        // No path provided, use current directory
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) == nullptr) {
            perror("smash error: getcwd failed");
            this->setArgument(nullptr, 0);
            return;
        }
        this->dir_path = cwd;
    } else {
        this->dir_path = args[1];
    }
    this->setArgument(args, argc);
}

void ListDirCommand::execute() {
    if (dir_path.empty()) {
        return;
    }
    listDirectory(dir_path, 0);
}
//////////////******************************************

void ListDirCommand::listDirectory(const std::string& path, int depth) {
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        perror("smash error: opendir failed");
        return;
    }
    struct dirent* entry;
    std::vector<std::string> directories;
    std::vector<std::string> files;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") { continue; }
        std::string full_path = path + "/" + name;
        struct stat sb;
        if (stat(full_path.c_str(), &sb) == -1) {
            perror("smash error: stat failed");
            continue;
        }
        if (S_ISDIR(sb.st_mode)) {
            directories.push_back(name); // It's a directory
        }
        else {
            files.push_back(name); // It's a file
        }
    }
    closedir(dir);
    std::sort(directories.begin(), directories.end());
    std::sort(files.begin(), files.end());
    for (const auto& dir_name : directories) {
        for (int i = 0; i < depth; ++i) std::cout << "\t";
        std::cout << dir_name << std::endl;
        listDirectory(path + "/" + dir_name, depth + 1); // Recurse
    }
    for (const auto& file_name : files) {
        for (int i = 0; i < depth; ++i) std::cout << "\t";
        std::cout << file_name << std::endl;
    }
}
//////////////////////////////////*************************** whoami
WhoAmICommand::WhoAmICommand(){}

void WhoAmICommand::execute() {
    const char* username = getenv("USER");
    const char* home_directory = getenv("HOME");

    if (username == nullptr || home_directory == nullptr) {
        return;
    }

    std::cout << username << " " << home_directory << std::endl;
}

///////////////////////////////////////////////// alias
bool AliasManager::isReservedKeyword(const std::string& name) const {
    static const std::set<std::string> reserved = {
            "quit", "cd", "chprompt", "showpid", "pwd", "jobs", "kill", "fg","unalias","alias"
    };
    return reserved.count(name) > 0;
}

void AliasManager::addAlias(const std::string& name, const std::string& command) {
    if (isReservedKeyword(name)) {
        std::cerr << "smash error: alias: " << name << " already exists or is a reserved command" << std::endl;
        return;
    }

    static const std::regex aliasRegex("^([a-zA-Z0-9_]+)=['\"]([^'\"]*)['\"]$");
    if (!std::regex_match(name + "='" + command +"'", aliasRegex)) {
        std::cerr << "smash error: alias: invalid alias format" << std::endl;
        return;
    }

    aliasMap[name] = command;
}

void AliasManager::removeAlias(const std::string& name) {
    if (aliasMap.find(name) == aliasMap.end()) {
        std::cerr << "smash error: unalias: " << name << " alias does not exist" << std::endl;
        return;
    }
    aliasMap.erase(name);
}

void AliasManager::listAliases() const {
    for (std::map<std::string, std::string>::const_iterator it = aliasMap.begin(); it != aliasMap.end(); ++it) {
        std::cout << it->first << "='" << it->second << "'" << std::endl;
    }
}

bool AliasManager::isAlias(const std::string& name) const {
    return aliasMap.find(name) != aliasMap.end();
}

std::string AliasManager::resolveAlias(const std::string& name) const {
    return isAlias(name) ? aliasMap.at(name) : name;
}

void SmallShell::handleAliasCommand(const std::string& cmdLine) {
    std::istringstream iss(cmdLine);
    std::string command, name, aliasCommand;
    iss >> command; // Skip "alias".

    if (!(iss >> name)) {
        aliasManager.listAliases();
        return;
    }

    size_t eqPos = name.find('=');
    if (eqPos == std::string::npos) {
        aliasManager.removeAlias(name);
    } else {
        std::string aliasName = name.substr(0, eqPos);
        std::string aliasCmd = name.substr(eqPos + 1);
        aliasManager.addAlias(aliasName, aliasCmd);
    }
}
////////////////////////////////////////// du and netinfo
DuCommand::DuCommand(const char* cmd_line) {
    char* args[COMMAND_MAX_ARGS];
    int argc = _parseCommandLine(cmd_line, args);
    if (argc > 2) {
        std::cerr << "smash error: du: too many arguments" << std::endl;
        this->setArgument(nullptr, 0);
        return;
    }
    if (argc == 2) {
        path = args[1];
    } else {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) == nullptr) {
            perror("smash error: getcwd failed");
            this->setArgument(nullptr, 0);
            return;
        }
        path = cwd;
    }
    this->setArgument(args, argc);
}

static long calculateDiskUsage(const std::string& path) {
    struct stat sb;
    if (lstat(path.c_str(), &sb) == -1) {
        return 0;
    }
    long total = sb.st_blocks / 2;

    if (S_ISDIR(sb.st_mode)) {
        DIR* dir = opendir(path.c_str());
        if (!dir) {
            return total;
        }
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            std::string full_path = path + "/" + entry->d_name;
            total += calculateDiskUsage(full_path);
        }
        closedir(dir);
    }
    return total;
}

void DuCommand::execute() {
    if (path.empty()) { return; }
    struct stat sb;
    if (stat(path.c_str(), &sb) == -1 || !S_ISDIR(sb.st_mode)) {
        std::cerr << "smash error: du: directory " << path << " does not exist" << std::endl;
        return;
    }

    long usage = calculateDiskUsage(path);
    std::cout << "Total disk usage: " << usage << " KB" << std::endl;
}

NetInfoCommand::NetInfoCommand(const char* cmd_line) {
    char* args[COMMAND_MAX_ARGS];
    int argc = _parseCommandLine(cmd_line, args);

    if (argc < 2) {
        std::cerr << "smash error: netinfo: interface not specified" << std::endl;
        this->setArgument(nullptr, 0);
        return;
    }
    if (argc > 2) {
        std::cerr << "smash error: netinfo: too many arguments" << std::endl;
        this->setArgument(nullptr, 0);
        return;
    }
    interface = args[1];
    this->setArgument(args, argc);
}
void printDNSList(const std::vector<std::string>& dns) {
    if (dns.empty()) {
        std::cout << "DNS Servers: None found" << std::endl;
    } else {
        std::cout << "DNS Servers: ";
        for (size_t i = 0; i < dns.size(); ++i) {
            std::cout << dns[i];
            if (i != dns.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << std::endl;
    }
}

void printDNSServers() {
    std::ifstream resolv("/etc/resolv.conf");
    if (!resolv) {
        return;
    }
    std::vector<std::string> dns;
    std::string line;
    while (getline(resolv, line)) {
        if (line.find("nameserver") == 0) {
            std::istringstream iss(line);
            std::string ns, ip;
            iss >> ns >> ip;
            dns.push_back(ip);
        }
    }
    printDNSList(dns);
}

bool extractIPv4Info(struct ifaddrs* ifa, const std::string& interface, std::string& ip, std::string& mask) {
    if (!ifa->ifa_addr) return false;
    if (interface != ifa->ifa_name) return false;

    if (ifa->ifa_addr->sa_family == AF_INET) {
        char addr[INET_ADDRSTRLEN];
        char netmask[INET_ADDRSTRLEN];

        void* in_addr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
        void* in_netmask = &((struct sockaddr_in*)ifa->ifa_netmask)->sin_addr;

        inet_ntop(AF_INET, in_addr, addr, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, in_netmask, netmask, INET_ADDRSTRLEN);

        ip = addr;
        mask = netmask;
        return true;
    }
    return false;
}

std::string getDefaultGateway() {
    std::ifstream f("/proc/net/route");
    if (!f) {
        return "unknown";
    }

    std::string line;
    getline(f, line);  // skip header

    while (getline(f, line)) {
        std::istringstream iss(line);
        std::string iface, destination, gateway;
        if (!(iss >> iface >> destination >> gateway)) continue;

        if (destination == "00000000") {  // default route
            unsigned int gw;
            std::stringstream ss;
            ss << std::hex << gateway;
            ss >> gw;
            struct in_addr addr;
            addr.s_addr = gw;
            return std::string(inet_ntoa(addr));
        }
    }
    return "unknown";
}

void NetInfoCommand::execute() {
    if (interface.empty()) return;

    std::string ip = "N/A";
    std::string mask = "N/A";

    // Get IP and Subnet Mask
    struct ifaddrs* ifaddr;
    if (getifaddrs(&ifaddr) == -1)
        return;

    for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
      if (extractIPv4Info(ifa, interface, ip, mask))
        break;

    freeifaddrs(ifaddr);
    std::cout << "IP Address: " << ip << std::endl;
    std::cout << "Subnet Mask: " << mask << std::endl;
    std::string default_gw = getDefaultGateway();
    std::cout << "Default Gateway: " << default_gw << std::endl;
    // DNS
    printDNSServers();
}

//////////////////////////////////////////****************************************************************
SmallShell::SmallShell() {
// TODO: add your implementation
    curr_direc=getcwd(nullptr,0);
    prevCommand=NULL;
    smash_pid=getpid();
    current_pid=getpid();
    signal_pid=&current_pid;
    is_curr_cmd_done=true;
    is_current_internal=false;
    global_jobs_list= new JobsList;
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(char* cmd_line) {
    string command;
    char **arguments;
    is_cp=false;
    arguments = (char **)malloc(sizeof(char *) * (COMMAND_MAX_ARGS+1));
    int argsNum = _parseCommandLine(cmd_line,arguments);
//////////////////////////////
    string originalCommand = arguments[0];
    bool alias_key_flag = false;
    if (aliasManager.isAlias(originalCommand)) {
        string resolvedCommand = aliasManager.resolveAlias(originalCommand);

        // Replace the command line with the resolved alias
        string updatedCmdLine = resolvedCommand;
        for (int i = 1; i < argsNum; ++i) {
            updatedCmdLine += " " + string(arguments[i]);
        }
        strcpy(cmd_line, updatedCmdLine.c_str());
        argsNum = _parseCommandLine(cmd_line, arguments);
        alias_key_flag = true;
    }
    ////////////////////////////
    command = string(arguments[0]);
    is_curr_cmd_done = false;
    is_background_command=false;
    is_foreground_command=false;
    if (!isCommandInternal(cmd_line)){
        if(_isBackgroundComamnd(cmd_line)) {
            is_background_command = true;
            is_foreground_command = false;
            is_fg_command = false;
        }
        else{
            is_foreground_command = true;
            is_background_command = false;
            is_fg_command = true;
        }
    }


    string cmd_s = string(cmd_line);


    if (strcmp("pwd",arguments[0]) == 0) {
        currCommand = new GetCurrDirCommand();
        currCommand->setCommand(command);
        currCommand->setArgument(arguments,argsNum);
        currCommand->setChpromptFlag(false);
        currCommand->setArgumentNum(argsNum);
        currCommand->setCmdLine(cmd_line);
        return currCommand;
    }

    else if (strcmp("fg",arguments[0])  == 0) {
        is_fg_command = true;
        if (argsNum > 2) {
            cerr << "smash error: fg: invalid arguments" << endl;
            freeArgs(arguments);
            return NULL;
        }
        if (argsNum == 1) {
            if (global_jobs_list->isEmpty()) {
                cerr << "smash error: fg: jobs list is empty" << endl;
                freeArgs(arguments);
                return NULL;
            }
            fg_job=global_jobs_list->maxJobId();
            currCommand = new ForegroundCommand(global_jobs_list->maxJobId());
            currCommand->setCommand(command);
            currCommand->setArgument(arguments,argsNum);
            currCommand->setChpromptFlag(false);
            currCommand->setArgumentNum(argsNum);
            currCommand->setCmdLine(cmd_line);
            return currCommand;
        }
        if (!isNumber(arguments[1])) {
            cerr << "smash error: fg: invalid arguments" << endl;
            freeArgs(arguments);
            return NULL;
        }
        bool is_with_minus = (*(arguments[1])) == '-';
        if(is_with_minus){
            cerr << "smash error: fg: invalid arguments" << endl;
            return NULL;
        }
        fg_job = charToInt(arguments[1], is_with_minus);
        currCommand = new ForegroundCommand(charToInt(arguments[1], is_with_minus));
        currCommand->setCommand(command);
        currCommand->setArgument(arguments,argsNum);
        currCommand->setChpromptFlag(false);
        currCommand->setArgumentNum(argsNum);
        currCommand->setCmdLine(cmd_line);
        return currCommand;
    }

    else if (strcmp("jobs",arguments[0]) == 0) {
        currCommand = new JobsCommand();
        currCommand->setCommand(command);
        currCommand->setArgument(arguments,argsNum);
        currCommand->setChpromptFlag(false);
        currCommand->setArgumentNum(argsNum);
        currCommand->setCmdLine(cmd_line);
        //freeArgs(arguments);
        return currCommand;
    }

    else if (strcmp("alias", arguments[0]) == 0) {
        if (argsNum == 1) { // No arguments provided, list all aliases
            aliasManager.listAliases();
            return nullptr;
        }

        // Concatenate all arguments to handle cases like alias ll = 'ls -l'
        std::string aliasDefinition;
        for (int i = 1; i < argsNum; ++i) {
            aliasDefinition += arguments[i];
            if (i < argsNum - 1) aliasDefinition += " ";
        }

        // Remove extra spaces around the equals sign (=)
        size_t eqPos = aliasDefinition.find('=');
        if (eqPos == std::string::npos || eqPos == 0 || eqPos == aliasDefinition.length() - 1) {
            std::cerr << "smash error: alias: invalid format" << std::endl;
            return nullptr;
        }

        std::string aliasName = aliasDefinition.substr(0, eqPos);
        std::string command = aliasDefinition.substr(eqPos + 1);

        // Trim spaces around aliasName and command
        aliasName = _trim(aliasName);
        command = _trim(command);

        // Check if aliasName is a reserved keyword
        if (aliasManager.isReservedKeyword(aliasName)) {
            std::cerr << "smash error: alias: " << aliasName << " already exists or is a reserved command" << std::endl;
            return nullptr;
        }

        // Remove surrounding quotes from the command if they exist
        if ((command.front() == '\'' && command.back() == '\'') ||
            (command.front() == '"' && command.back() == '"')) {
            command = command.substr(1, command.size() - 2);
        }

        // Add the alias to the alias manager
        aliasManager.addAlias(aliasName, command); // Store the alias
        return nullptr;
    }



    else if (strcmp("kill",arguments[0]) == 0) {
        if(argsNum > 3){
            cerr <<"smash error: kill: invalid arguments" << endl;
            freeArgs(arguments);
            return NULL;
        }
        if (argsNum != 3) {
            if (argsNum < 3) {
                cerr <<"smash error: kill: invalid arguments" << endl;
                freeArgs(arguments);
                return NULL;
            }
            else{
                bool is_with_minus2 = (*(arguments[2])) == '-';
                int jobId = charToInt(arguments[2], is_with_minus2);
                JobsList::JobEntry *tmp = global_jobs_list->getJobById(jobId);
                pid_t pid;
                if (tmp==NULL){
                    cerr << "smash error: kill: job-id " << jobId << " does not exist" << endl;
                    freeArgs(arguments);
                    return NULL;
                }
                cerr <<"smash error: kill: invalid arguments" << endl;
                freeArgs(arguments);
                return NULL;
            }
        }
        if((*arguments[1]) != '-' || (*arguments[2]) == '-' ||!isNumber((arguments[1]+1))){
            cerr <<"smash error: kill: invalid arguments" << endl;
            return NULL;
        }
        if ((!isNumber(arguments[2])) || (!isNumber(arguments[1] + 1)) ||
            (*(*(arguments + 1))) != '-') {
            if ((!isNumber(arguments[2])) || (!isNumber(arguments[1]))){
                cerr <<"smash error: kill: invalid arguments" << endl;
                freeArgs(arguments);
                return NULL;
            }
            bool is_with_minus2 = (*(arguments[2])) == '-';
            int jobId = charToInt(arguments[2], is_with_minus2);
            JobsList::JobEntry *tmp = global_jobs_list->getJobById(jobId);
            pid_t pid;
            if (tmp==NULL){
                cerr << "smash error: kill: job-id " << jobId << " does not exist" << endl;
                freeArgs(arguments);
                return NULL;
            }
            cerr <<"smash error: kill: invalid arguments" << endl;
            freeArgs(arguments);
            return NULL;
        }
        bool is_with_minus1 = (*(arguments[1] + 1)) == '-';
        bool is_with_minus2 = (*(arguments[2])) == '-';
        currCommand = new KillCommand(charToInt(arguments[1] + 1, is_with_minus1),
                                      charToInt(arguments[2], is_with_minus2));
        currCommand->setCommand(command);
        currCommand->setArgument(arguments,argsNum);
        currCommand->setChpromptFlag(false);
        currCommand->setArgumentNum(argsNum);
        currCommand->setCmdLine(cmd_line);
        return currCommand;
    }

    else if (strcmp("chprompt",arguments[0]) == 0) {
        currCommand = new ChangePrompt();
        currCommand->setCommand(command);
        currCommand->setArgument(arguments,argsNum);
        currCommand->setChpromptFlag(true);
        currCommand->setArgumentNum(argsNum);
        currCommand->setCmdLine(cmd_line);
        return currCommand;
    }

    else if (strcmp("showpid",arguments[0]) == 0) {
        currCommand = new ShowPidCommand();
        currCommand->setCommand(command);
        currCommand->setArgument(arguments,argsNum);
        currCommand->setChpromptFlag(false);
        currCommand->setArgumentNum(argsNum);
        currCommand->setCmdLine(cmd_line);
        return currCommand;
    }

    else if (strcmp("listdir",arguments[0]) == 0) { ///////////////////////////////////////////////////////////////// listdir
        currCommand = new ListDirCommand(cmd_line);
        currCommand->setCommand(command);
        currCommand->setArgument(arguments,argsNum);
        currCommand->setChpromptFlag(false);
        currCommand->setArgumentNum(argsNum);
        currCommand->setCmdLine(cmd_line);
        return currCommand;
    }

    else if (strcmp("whoami", arguments[0]) == 0) { ////////////////////////////////////////////////////////////////whoami
        currCommand = new WhoAmICommand();
        currCommand->setCommand(command);
        currCommand->setArgument(arguments, argsNum);
        currCommand->setChpromptFlag(false);
        currCommand->setArgumentNum(argsNum);
        currCommand->setCmdLine(cmd_line);
        return currCommand;
    }
    else if (strcmp("unsetenv", arguments[0]) == 0) {/////////////////unsetenve
    currCommand = new UnSetEnvCommand();
    currCommand->setCommand(command);
    currCommand->setArgument(arguments, argsNum);
    currCommand->setChpromptFlag(false);
    currCommand->setArgumentNum(argsNum);
    currCommand->setCmdLine(cmd_line);
    return currCommand;
    }
    else if (strcmp("du", arguments[0]) == 0) {
        currCommand = new DuCommand(cmd_line);
        currCommand->setCommand(command);
        currCommand->setArgument(arguments, argsNum);
        currCommand->setChpromptFlag(false);
        currCommand->setArgumentNum(argsNum);
        currCommand->setCmdLine(cmd_line);
        return currCommand;
    }
    else if (strcmp("netinfo", arguments[0]) == 0) {
        currCommand = new NetInfoCommand(cmd_line);
        currCommand->setCommand(command);
        currCommand->setArgument(arguments, argsNum);
        currCommand->setChpromptFlag(false);
        currCommand->setArgumentNum(argsNum);
        currCommand->setCmdLine(cmd_line);
        return currCommand;
    }


    else if (strcmp("quit",arguments[0]) == 0) {
        bool withKill=false;
        if(argsNum>1){
            for(int i=1; i<argsNum;i++) {
                if (strcmp(arguments[i], "kill") == 0) {
                    withKill = true;
                    break;
                }
            }
        }
        currCommand = new QuitCommand(withKill,&smash_pid);
        currCommand->setCommand(command);
        currCommand->setArgument(arguments,argsNum);
        currCommand->setChpromptFlag(false);
        currCommand->setArgumentNum(argsNum);
        currCommand->setCmdLine(cmd_line);
        return currCommand;
    }
    else if (strcmp("unalias", arguments[0]) == 0) {  // **Added for unalias**
        if (argsNum < 2) {  // No arguments provided
            cerr << "smash error: unalias: not enough arguments" << endl;
            freeArgs(arguments);
            return nullptr;
        }
        for (int i = 1; i < argsNum; ++i) {
            std::string aliasName = arguments[i];
            if (!aliasManager.isAlias(aliasName)) {  // Check if alias exists
                cerr << "smash error: unalias: " << aliasName << " alias does not exist" << endl;
                freeArgs(arguments);
                return nullptr;
            }
            aliasManager.removeAlias(aliasName);  // Remove alias
        }
        return nullptr;  // No command to execute further
    }

    else if (strcmp("cd",arguments[0]) == 0) {
        if (argsNum > 2) {
            cerr << "smash error: cd: too many arguments" << endl;
            delete[] arguments;
            return NULL;
        }
        if (argsNum == 1) {
            char *temp = prev_direc;
            prev_direc = (char *)malloc(strlen(curr_direc) + 1);
            if (!prev_direc) {
                cerr << "smash error: memory allocation failed" << endl;
                prev_direc = temp;
                freeArgs(arguments);
                return NULL;
            }
            strcpy(prev_direc, curr_direc);
            free(temp);
            freeArgs(arguments);
            return NULL;
        }
        currCommand = new ChangeDirCommand(arguments[1],&prev_direc,&curr_direc);
        currCommand->setCommand(command);
        currCommand->setArgument(arguments,argsNum);
        currCommand->setChpromptFlag(false);
        currCommand->setArgumentNum(argsNum);
        currCommand->setCmdLine(cmd_line);
        return  currCommand;
    }

    if (strcmp(arguments[0], "cp") == 0) {
        is_cp=true;
        strcpy(cp_cmd_line_for_ctlr_z,cmd_line);
        if(is_background_command){
            ///should we bring & when add to jobs
            strcpy(cp_cmd_line,cmd_line);
            _removeBackgroundSign(cmd_line);

        }


        argsNum = _parseCommandLine(cmd_line,arguments);

        is_current_internal = false;
        if (argsNum != 3) {
            cout << "smash error: cp: invalid arguments" << endl;
            return NULL;
        }
        currCommand = new CopyCommand(cmd_line,arguments[1], arguments[2], &current_pid,
                                      is_background_command);
        return currCommand;
    }
    else {
        is_current_internal=false;
        currCommand = new ExternalCommand(cmd_line, is_background_command, current_pid,originalCommand,alias_key_flag);
        currCommand->setCommand(command);
        currCommand->setArgument(arguments,argsNum);
        currCommand->setChpromptFlag(false);
        currCommand->setArgumentNum(argsNum);
        currCommand->setCmdLine(cmd_line);
        //freeArgs(arguments);
        return  currCommand;
    }
    return nullptr;
}

void SmallShell::executeCommand(char *cmd_line) {
    // TODO: Add your implementation here
    strcpy(cat_cmd_line,cmd_line);
    char cmd1[COMMAND_LINE_MAX_LENGTH];
    char cmd2[COMMAND_LINE_MAX_LENGTH];
    bool is_with_bg = _isBackgroundComamnd(cmd_line);
    int cmd_status = checkCommandStatus(cmd_line, cmd1, cmd2);
    is_current_internal = isCommandInternal(cmd_line);
    if (cmd_status == 0) {
        newCommand = CreateCommand(cmd_line);
        if (!newCommand)
            return;
        newCommand->execute();
    }
    strcpy(pipe_with_bg,cmd_line);
    _removeBackgroundSign(cmd_line);
    cmd_status = checkCommandStatus(cmd_line, cmd1, cmd2);
    global_jobs_list->removeFinishedJobs();
    if (cmd_status == 1) {
        /*int newFD = dup(1);
        if (close(1) == -1) {
            perror("smash error: close failed");
            return;
        }*/
        int newFD = dup(1);
        if (newFD == -1) {
             perror("smash error: dup failed");
             return;
        }
       if (close(1) == -1) {
             dup2(newFD, 1);
             close(newFD);
             return;
       }

        char **arguments;
        arguments = (char **) malloc(sizeof(char *) * COMMAND_MAX_ARGS);
        _parseCommandLine(cmd2, arguments);
        int file2 = open(arguments[0], O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (file2 < 0) {
            perror("smash error: open failed");
            freeArgs(arguments);
            dup2(newFD, 1);
            close(newFD);
            return;
        }
        freeArgs(arguments);
        int l = strlen(cmd1);
        if (is_with_bg) {
            cmd1[l] = '&';
            cmd1[l + 1] = 0;
        }
        newCommand = CreateCommand(cmd1);
        if (newCommand != NULL)
            newCommand->execute();
        fflush(stdout);
        close(file2);
        dup2(newFD, 1);
        close(newFD);
    }
    if (cmd_status == 2) {
        int newFd = dup(1);
        close(1);
        char **arguments;
        arguments = (char **) malloc(sizeof(char *) * COMMAND_MAX_ARGS);

        _parseCommandLine(cmd2, arguments);
        int file2 = open(arguments[0], O_WRONLY | O_CREAT | O_APPEND, 0666);
        freeArgs(arguments);
        if (file2 < 0) {
            perror("smash error: open failed");
            dup2(newFd, 1);
            close(newFd);
            return;
        }
        int l = strlen(cmd1);
        if (is_with_bg) {
            cmd1[l] = '&';
            cmd1[l + 1] = 0;
        }
        newCommand = CreateCommand(cmd1);
        if (newCommand != NULL)
            newCommand->execute();
        fflush(stdout);
        close(file2);
        dup2(newFd, 1);
        close(newFd);
    }
    int len = strlen(cmd1);
    cmd1[len] = '&';
    cmd1[len + 1] = 0;
    len = strlen(cmd2);
    cmd2[len] = '&';
    cmd2[len + 1] = 0;
    if (cmd_status == 3 || cmd_status == 4) {
        is_curr_cmd_done = false;
        is_current_internal = false;
        if (cmd_status == 3)
            output_channel = 1;
        else
            output_channel = 2;
        pid_t pid1 = fork();
        //pid1=0;
        if (pid1 < 0) {
            perror("smash error: fork failed");
            return;
        }
        if (pid1 == 0) {
            backDifSignal();
            setpgrp();
            if (pipe(fd) < 0) {
                perror("smash error: pipe failed");
                return;
            }
            if (isCommandInternal(cmd1)) {
                pid_t pid2 = fork();
                if (pid2 < 0) {
                    perror("smash error: fork failed");
                    return;
                }
                if (pid2 == 0) {
                    backDifSignal();
                    dup2(fd[1], output_channel);
                    close(fd[0]);
                    close(fd[1]);
                    is_son1 = true;
                    is_son2 = false;
                    newCommand = CreateCommand(cmd1);
                    if (!newCommand)
                        return;
                    newCommand->execute();
                    return;
                }
                if (pid2 > 0) {
                    pipe_1 = pid2;
                }
            } else {
                is_son1 = true;
                is_son2 = false;
                newCommand = CreateCommand(cmd1);
                if (!newCommand)
                    return;
                newCommand->execute();
            }
            if (isCommandInternal(cmd2)) {

                pid_t pid3 = fork();
                if (pid3 < 0) {
                    perror("smash error: fork failed");
                    return;
                }
                if (pid3 == 0) {
                    //cout << "HI"<< endl;
                    backDifSignal();
                    dup2(fd[0], 0);
                    close(fd[0]);
                    close(fd[1]);
                    is_son1 = false;
                    is_son2 = true;
                    newCommand = CreateCommand(cmd2);
                    if (!newCommand)
                        return;
                    newCommand->execute();
                    return;
                }
                if (pid3 > 0) {
                    pipe_2 = pid3;
                }
            } else {
                is_son1 = false;
                is_son2 = true;
                newCommand = CreateCommand(cmd2);
                if (!newCommand)
                    return;
                newCommand->execute();
            }
            close(fd[0]);
            close(fd[1]);
            int status;
            int wpid;
            while ((wpid = wait(&status)) != -1);
            return;
        }
        if (pid1 > 0) {
            *signal_pid = pid1;
            current_pid=pid1;
            if (!is_with_bg ) {
                waitpid(pid1, NULL, WUNTRACED);
            } else {
                global_jobs_list->addJob(currCommand, false,pipe_with_bg,current_pid);
            }
        }
    }

}
#endif
