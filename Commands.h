#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_


#include <vector>
#include <string>
#include <list>
#include <signal.h>
#include <map>
#include <cstring>

#define COMMAND_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define COMMAND_LINE_MAX_LENGTH (100)
/// ***************

///// list

template<class T>
class ListNode {
public:
    T data;
    ListNode *next;
    ListNode *prev;

    ListNode();

    ListNode(T data, ListNode *next, ListNode *prev);

    ~ListNode() = default;


    ListNode(const ListNode &oldNode)= default;

    ListNode &operator=(const ListNode &oldNode) = default;

    bool operator==(const ListNode &rhs) const;

    bool operator!=(const ListNode &rhs) const;
};


template<class T>
ListNode<T>::ListNode():
        next(nullptr), prev(nullptr){}

template<class T>
ListNode<T>::ListNode(T data, ListNode *next, ListNode *prev):
        data(data), next(next), prev(prev) {}



template<class T>
bool ListNode<T>::operator==(const ListNode &rhs) const {
    return data == rhs.data &&
           next == rhs.next &&
           prev == rhs.prev;
}

template<class T>
bool ListNode<T>::operator!=(const ListNode &rhs) const {
    return !(rhs == *this);
}
// list//////////////////////////////////////////////////////////////////////

template<class T>
class List {

public:
    ListNode<T> *head;
    static ListNode<T> * getHead(List<T>* &l){return l->head;}
    ListNode<T> *tail;

    List();

    List &operator=(const List &oldList) ;

    ~List();

    List(const List &oldList) ;

    /*Clears the list (Deletes all nodes between head and tail).*/
    void clearList();


    void pushBefore(ListNode<T> * prev ,ListNode<T> *next ,T data);

    /*Adds a new element 'data' at the beginning of the list.*/
    void pushFirst(T data);

    /*Adds a new element 'data' at the end of the list.*/
    void pushLast(T data);


    /*Removes a node in the list that ptr is pointing to.*/
    void remove(ListNode<T> *ptr);

    /*Returns a pointer to the first element in the list.*/
    ListNode<T> *getFirst() const;

    /*Returns a pointer to the last element in the list.*/
    ListNode<T> *getLast() const;

    /*Returns true in case the list is empty, and false otherwise*/
    bool empty() const;
    class Iterator;

    // Root of LinkedList wrapped in Iterator type
    Iterator begin()
    {
        return Iterator(head);
    }

    // End of LInkedList wrapped in Iterator type
    Iterator end()
    {
        return Iterator(nullptr);
    }

    // Iterator class can be used to
    // sequentially access nodes of linked list
    class Iterator
    {
    public:
        Iterator() noexcept :
                m_pCurrentNode (getHead(this)) {
        }

        Iterator(const ListNode<T>* pNode) noexcept :
                m_pCurrentNode (pNode) { }

        Iterator& operator=(ListNode<T>* pNode)
        {
            this->m_pCurrentNode = pNode;
            return *this;
        }

        // Prefix ++ overload
        Iterator& operator++()
        {
            if (m_pCurrentNode)
                m_pCurrentNode = m_pCurrentNode->next;
            return *this;
        }

        // Postfix ++ overload
        Iterator operator++(int)
        {
            Iterator iterator = *this;
            ++*this;
            return iterator;
        }

        bool operator!=(const Iterator& iterator)
        {
            return m_pCurrentNode != iterator.m_pCurrentNode;
        }

        T operator*()const
        {
            return m_pCurrentNode->data;
        }

    private:
        const ListNode<T>* m_pCurrentNode;
    };

};

template <class T>
List<T>::List(const List<T> &oldList){
    this->head=new ListNode<T>;
    this->tail=new ListNode<T>;
    head->next=tail;
    tail->prev=head;
    if(oldList.empty())
        return ;
    for(ListNode<T> *iter=oldList.getFirst();iter!=oldList.tail;iter=iter->next){
        this->pushLast(iter->data);
    }
}

template <class T>
List<T>& List<T>::operator=(const List<T> &oldList){
    if(this==&oldList)
        return *this;
    this->clearList();
    if(oldList.empty())
        return *this;
    for(ListNode<T> *iter=oldList.getFirst();iter!=oldList.tail;iter=iter->next){
        this->pushLast(iter->data);
    }
    return *this;
}

template <class T>
List<T>::~List(){
    this->clearList();
    delete this->head;
    delete this->tail;
}

template<class T>
List<T>::List() {
    ListNode<T> *newNode1 = new ListNode<T>();
    head = newNode1;
    ListNode<T> *newNode2 = new ListNode<T>();
    tail = newNode2;
    head->next = tail;
    tail->prev = head;
}


template<class T>
void List<T>::clearList() {
    while (head->next != tail) {
        ListNode<T> *tmp = head->next;
        head->next = tmp->next;
        tmp->next->prev=head;
        delete tmp;
    }
}

template<class T>
ListNode<T> *List<T>::getFirst() const{
    if(this->empty())
        return nullptr;
    return head->next;
}

template<class T>
ListNode<T> *List<T>::getLast() const{
    if(this->empty())
        return nullptr;
    return tail->prev;
}
template<class T>
void List<T>::pushBefore(ListNode<T> * prev ,ListNode<T> *next ,T data) {
    ListNode<T> *newNode = new ListNode<T>(data ,next ,prev);
    next->prev =newNode;
    prev->next =newNode;
}

template<class T>
void List<T>::pushFirst(T data) {
    ListNode<T> *newNode = new ListNode<T>(data, head->next, head);
    (head->next)->prev = newNode;
    head->next = newNode;
}

template<class T>
void List<T>::pushLast(T data) {
    ListNode<T> *newNode = new ListNode<T>(data, tail, tail->prev);
    (tail->prev)->next = newNode;
    tail->prev = newNode;
}

template<class T>
void List<T>::remove(ListNode<T> *ptr) {
    (ptr->prev)->next = ptr->next;
    (ptr->next)->prev = ptr->prev;
    delete ptr;
}

template<class T>
bool List<T>::empty() const{
    return (head->next == tail);
}
/////////////////////////////////////////////////////////////////////////////
class Command {
    // TODO: Add your data members
    std::string command;
    const char* cmd_line;
    char** argument;
    bool chpromptFlag;
    int argumentNum;
public:

    Command(const char *cmd_line):cmd_line(cmd_line){}

    Command(){
        argument = (char**)malloc(sizeof(char*)* COMMAND_MAX_ARGS);
    }
    virtual ~Command() = default;

    virtual void execute() = 0;

    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed

    const std::string &getCommand() const {
        return command;
    }
    void setCommand(const std::string &command) {
        Command::command = command;
    }
    char **getArgument() const {
        return argument;
    }
    void setArgument(char **argument, int len) {
        for (int i = 0; i < len; ++i) {
            Command::argument[i] = argument[i];
        }
    }
    bool isChpromptFlag() const {
        return chpromptFlag;
    }
    void setChpromptFlag(bool chpromptFlag) {
        Command::chpromptFlag = chpromptFlag;
    }
    int getArgumentNum() const {
        return argumentNum;
    }

    void setArgumentNum(int argumentNum) {
        Command::argumentNum = argumentNum;
    }

    const char *getCmdLine() const {
        return cmd_line;
    }

    void setCmdLine(const char *cmdLine) {
        cmd_line = cmdLine;
    }
};

/// *******************
class BuiltInCommand : public Command {
public:
    //BuiltInCommand(const char *cmd_line);
    BuiltInCommand();

    virtual ~BuiltInCommand() {
    }
};

class ExternalCommand : public Command {
public:
    char *cmd_line;
    bool is_back_ground;
    pid_t curr_pid;
    std::string alias_key;
    bool alias_key_flag;
public:
    ExternalCommand( char *cmd_line, bool is_back_ground, pid_t curr_pid,std::string alias_key, bool alias_key_flag)
            : is_back_ground(is_back_ground), curr_pid(curr_pid), alias_key(alias_key), alias_key_flag(alias_key_flag) {
        this->cmd_line= cmd_line;
    };

    virtual ~ExternalCommand() {};

    void execute() override;
};

/// *************
class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line);

    virtual ~PipeCommand() {
    }

    void execute() override;
};
/// ************************
class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char *cmd_line);

    virtual ~RedirectionCommand() {
    }

    void execute() override;
};
/// cd command
class ChangeDirCommand : public BuiltInCommand {
    // TODO: Add your data members public:
public:
    char* newPath;
    char** lastDir;
    char** currDir;
    //ChangeDirCommand(const char *cmd_line, char **plastPwd);
    ChangeDirCommand(char* newPath, char** lastDir, char** currDir):
            lastDir(lastDir), currDir(currDir){
        this->newPath = newPath;
    };

    virtual ~ChangeDirCommand() {
    }

    void execute() override;
};
/// pwd command
class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(char *cmd_line);
    GetCurrDirCommand();
    virtual ~GetCurrDirCommand() {
    }

    void execute() override;
};
/// showpid command
class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char *cmd_line);

    ShowPidCommand();

    virtual ~ShowPidCommand() {
    }

    void execute() override;
};

class JobsList;


/// quite command
class QuitCommand : public BuiltInCommand {
    // TODO: Add your data members public:
public:
    bool withKill;
    pid_t * pid;
    // QuitCommand(const char *cmd_line, JobsList *jobs);
    QuitCommand(bool withKill,pid_t* pid) : withKill(withKill),pid(pid) {};

    virtual ~QuitCommand() {
    }

    void execute() override;
};
/// for chprompt command
class ChangePrompt : public BuiltInCommand {
public:
    ChangePrompt();
    virtual ~ChangePrompt() ;
    void execute() override;
};
//////////////////////////////////////////////////////////////////////////////////////
class UnSetEnvCommand : public BuiltInCommand {
public:
    UnSetEnvCommand() {}
    virtual ~UnSetEnvCommand() {}
    void execute() override;
};


/////////////////////////////////////////////////////////////////////////////////////
class JobsList {
public:
    class JobEntry {
        // TODO: Add your data members
    public:
        int job_id;
        Command *command;
        pid_t process_id;
        bool isStopped;
        char cmd_line[COMMAND_LINE_MAX_LENGTH];
        JobEntry();

        JobEntry(int job_id, Command *command, pid_t process_id,
                 bool isStopped,char cmd_line[COMMAND_LINE_MAX_LENGTH]) :
                job_id(job_id), command(command), process_id(process_id),
                isStopped(isStopped) {
            strcpy(this->cmd_line,cmd_line);
        }
    };
    List<JobEntry> job_list;
    // TODO: Add your data members
public:
    JobsList():job_list(List<JobEntry>()){}

    ~JobsList();

    void addJob(Command *cmd, bool isStopped = false);

    void addJob(Command* cmd, bool isStopped,char* cmd_line,pid_t pid );

    void printJobsList();

    void killAllJobs();

    void removeFinishedJobs();

    JobEntry *getJobById(int jobId);

    void removeJobById(int jobId);

    JobEntry *getLastJob(int *lastJobId);

    JobEntry *getLastStoppedJob(int *jobId);

    int maxJobId();

    bool isEmpty();

    JobEntry *getLastStoppedJob();


    // TODO: Add extra methods or modify exisitng ones as needed
};
/// job command
class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    //JobsCommand(const char *cmd_line, JobsList *jobs);
    JobsCommand();
    virtual ~JobsCommand() {
    }

    void execute() override;
};
/// kill command
class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
    int sigNum;
    int jobId;
public:
    //KillCommand(const char *cmd_line, JobsList *jobs);
    KillCommand(int sigNum, int jobId) : sigNum(sigNum), jobId(jobId) {};
    virtual ~KillCommand() {
    }

    void execute() override;
};
/// fg command
class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
    int jobId;
public:
    //ForegroundCommand(const char *cmd_line, JobsList *jobs);
    explicit ForegroundCommand(int jobId) : jobId(jobId) {};
    virtual ~ForegroundCommand() {
    }

    void execute() override;
};

class ListDirCommand : public Command {
    std::string dir_path;
public:
    ListDirCommand(const char *cmd_line);

    virtual ~ListDirCommand() {
    }

    void execute() override;
private:
    void listDirectory(const std::string& path, int depth);
};

class WhoAmICommand : public Command {
public:
   // WhoAmICommand(const char *cmd_line);

    virtual ~WhoAmICommand() {
    }
    WhoAmICommand();

    void execute() override;
};
/*
class NetInfo : public Command {
    // TODO: Add your data members
public:
    NetInfo(const char *cmd_line);

    virtual ~NetInfo() {
    }

    void execute() override;
};*/
class DuCommand : public BuiltInCommand {
    std::string path;
public:
    DuCommand(const char* cmd_line);
    virtual ~DuCommand() {}
    void execute() override;
};

class NetInfoCommand : public BuiltInCommand {
    std::string interface;
public:
    NetInfoCommand(const char* cmd_line);
    virtual ~NetInfoCommand() {}
    void execute() override;
};


///alias Command
class aliasCommand : public BuiltInCommand {
    std::string  key;
    std::string name;
public:
    aliasCommand(const char *cmd_line);
    aliasCommand(std::string key, std::string name): key(key),name(name){}

    virtual ~aliasCommand() {
    }

    void execute() override;
};
/// unaliasCommand
class unaliasCommand : public BuiltInCommand {
    std::string  key;
    std::string name;
public:
    unaliasCommand(const char *cmd_line);
    unaliasCommand(std::string key, std::string name): key(key),name(name){}
    virtual ~unaliasCommand() {
    }

    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
    // TODO: Add your data members
    int jobId;
    char cmd_line[COMMAND_LINE_MAX_LENGTH];
public:
    BackgroundCommand(int jobId) : jobId(jobId) {};

    virtual ~BackgroundCommand() {}

    void execute() override;
};

class CopyCommand : public Command {
    char orig_file[COMMAND_LINE_MAX_LENGTH];
    char file_path[COMMAND_LINE_MAX_LENGTH];
    pid_t *curr_pid;
    bool is_back_ground;
    char cmd_line[COMMAND_LINE_MAX_LENGTH];
public:
    CopyCommand(const char cmd_line[COMMAND_LINE_MAX_LENGTH], char *orig_file,
                char *file_path, pid_t *curr_pid, bool is_back_ground)
            : curr_pid(curr_pid), is_back_ground(is_back_ground) {
        strcpy(this->cmd_line, cmd_line);
        strcpy(this->orig_file, orig_file);
        strcpy(this->file_path, file_path);
    };

    virtual ~CopyCommand() {}

    void execute() override;
};

/////////////////////////////////////////// alias
class AliasManager {
private:
    std::map<std::string, std::string> aliasMap; // Map alias names to commands.

public:
    bool isReservedKeyword(const std::string& name) const;
    void addAlias(const std::string& name, const std::string& command);
    void removeAlias(const std::string& name);
    void listAliases() const;
    bool isAlias(const std::string& name) const;
    std::string resolveAlias(const std::string& name) const;
};

class SmallShell {
private:
    // TODO: Add your data members
    AliasManager aliasManager; ////////alias
    SmallShell();

public:
    Command *CreateCommand(char *cmd_line);

    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    void handleAliasCommand(const std::string& cmdLine); ////////alias
    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

    ~SmallShell();

    void executeCommand( char *cmd_line);


    // TODO: add extra methods as needed
};


#endif //SMASH_COMMAND_H_
