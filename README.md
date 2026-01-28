# SmashShell – Custom Linux Shell

A custom Linux shell implemented in **C/C++** as part of the **Operating Systems** course at the **Technion – Israel Institute of Technology**.

This project focuses on low-level process management, signal handling, and job control in a Unix/Linux environment.

---

## Project Overview

SmashShell is a simplified Linux shell that supports executing external commands, built-in commands, and managing foreground and background jobs.  
The shell demonstrates practical use of **POSIX system calls** and **signal handling**.

---

## Features

- Command parsing and execution
- Foreground and background process management
- Job control (tracking running and stopped jobs)
- Signal handling:
  - `Ctrl+C` (SIGINT)
  - `Ctrl+Z` (SIGTSTP)
- Built-in shell commands
- Error handling and robust process cleanup

---

## Technologies Used

- **C / C++**
- **Linux (POSIX API)**
- `fork`, `execvp`, `waitpid`
- Signals (`SIGINT`, `SIGTSTP`)
- Bash environment
- Debugging with **GDB**

---

## Project Structure

