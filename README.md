*This project has been created as part of the 42 curriculum by ael-bakk.*

# Codexion

## Description

Codexion is a resource management simulation. Each "coder" represents a thread competing for limited "dongles" and attempting to complete a cycle of compile, debug, and refactor operations without burning out. The monitor thread provides supervision for timeouts (burnout detection) and fairness enforcement.

## Instructions

### Compilation

To compile the project, simply run:

```sh
make
```

This produces the `codexion` executable.

### Usage

Run Codexion with parameters:

```sh
./codexion <n_coders> <time_to_burnout> <t_compile> <t_debug> <t_refactor> <must_compile> <arrival_window> <scheduler>
```

- `n_coders`: Number of coder threads to create.
- `time_to_burnout`: Time (ms) before a coder "burns out" w/o compiling.
- `t_compile`, `t_debug`, `t_refactor`: Time (ms) each step takes.
- `must_compile`: Number of required cycles per coder (0 = infinite, exits when burnout).
- `arrival_window`: Max delay for coder thread creation (ms).
- `scheduler`: "fifo" or another supported scheduler.

Example:

```sh
./codexion 5 1000 20 10 10 3 50 fifo
```

### Installation

No special installation is required. Just clone the repo and use the make command.

## Resources

- [pthread(7) Manual](https://man7.org/linux/man-pages/man7/pthreads.7.html)
- [42 Codam Philosophy Resources](https://harm-smits.github.io/42docs/projects/philosophers)
- [Deadlock & Coffman's Conditions (Wikipedia)](https://en.wikipedia.org/wiki/Coffman_conditions)
- [GeeksforGeeks: Race Conditions and Critical Sections](https://www.geeksforgeeks.org/race-condition-in-process-control-block/)

**AI Use:**  
ChatGPT/Copilot were consulted for:
- Reviewing partial-init resource leak patterns
- Creating edge-case and Valgrind test scaffolds

---

## Blocking cases handled

- **Deadlock Prevention:**  
  Ensures no circular wait among coders by controlling the order of dongle acquisition and prompt release immediately after compilation.

- **Starvation Prevention:**  
  The scheduler (FIFO or another, as provided) ensures coders access shared dongles fairly, so no thread is perpetually denied progress.

- **Cooldown Handling:**  
  Proper delays are imposed (via `ms_sleep`) between compiled phases. No thread is scheduled for resource access sooner than appropriate.

- **Precise Burnout Detection:**  
  The monitor thread tracks the last successful compilation per coder and logs or terminates a coder exactly when the allowed time is exceeded.

- **Log Serialization:**  
  All output to the log (e.g., "has taken a dongle", "is compiling", etc.) is protected by a mutex to avoid garbled/interleaved lines.

---

## Thread synchronization mechanisms

- **`pthread_mutex_t`:**  
  Used for:
  - Each coder's state (`state_mtx`) to protect `compile_count`, `last_compile_start_ms`.
  - Each dongle (resource) for atomic availability checks and locking.
  - Log serialization: all log output passes through a mutex for thread safety.

- **`pthread_cond_t`:**  
  Scheduler uses condition variables for complex waiting and signaling among coders for fair queueing and wakeup.

- **Custom event implementation:**  
  The scheduler and main control logic sometimes employ custom event/wait and signaling solutions (see `scheduler.c`).

**Prevention of race conditions:**

Example:

```c
pthread_mutex_lock(&sim->log_mtx);
log_event(sim, id, "is compiling");
pthread_mutex_unlock(&sim->log_mtx);
```
- Ensures all log lines are complete and non-interleaved.

**Thread-safe communication:**
- Coder threads coordinate via the scheduler and resource mutexes/condition variables.
- State checks for burnout and quota are always mutex protected, preventing unsafe reads.

---