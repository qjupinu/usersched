# User Weighted Round-Robin Scheduler
### User-space simulation using fork(), sleep() and POSIX signals

## Overview

This project implements a **User Weighted Round-Robin (UWRR) scheduler simulation** in **user space**, written in **C**, using standard POSIX mechanisms available on Linux.

The goal is to demonstrate how **user-level weights** influence CPU allocation, while preserving the fairness properties of a round-robin scheduler.  
Instead of modifying the Linux kernel, the scheduler operates entirely in user space and uses **real OS processes** to simulate execution and preemption.

The project is intended for educational purposes, as part of an **Operating Systems** course.

---

## Implemented Requirement

> Implement a user weighted round-robin scheduler: the algorithm iterates over the list of users with runnable processes in round-robin order. Once a user is selected, one of its processes is allowed to execute for a finite amount of time. Each user has an associated weight that dictates the allowed execution time.

---

## Key Idea

- **Scheduling unit:** user (not process)
- Each user has:
  - a unique ID (`uid`)
  - a weight (`weight`)
  - one associated process (child process)
- The scheduler:
  1. iterates over users in round-robin order
  2. resumes the user’s process
  3. allows it to run for `weight × TIME_SLICE` seconds
  4. preempts it and moves to the next user

This ensures:
- proportional CPU allocation based on user weight
- no starvation (round-robin fairness)

---

## Architecture

### Scheduler (parent process)

- creates one child process per user
- controls execution using signals
- enforces time quantums
- collects CPU usage statistics

### Worker processes (child processes)

- represent user workloads
- run an infinite loop with `sleep(1)`
- can be stopped and resumed by the scheduler

---

## Preemption Mechanism

Preemption is simulated using **POSIX signals**:

- `SIGCONT` → resume execution
- `SIGSTOP` → suspend execution

This closely mimics how a real scheduler performs context switching, but without kernel-level code.

---

## Implementation Details

- **Language:** C
- **Platform:** Linux (tested on Ubuntu 24.04)
- **APIs used:**
  - `fork()`
  - `sleep()`
  - `kill()`
  - `waitpid()`
  - POSIX signals
- ANSI color codes are used to improve output readability

---

## Compilation and Execution

```bash
gcc usersched.c -o run
./run
