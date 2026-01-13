#define WHT "\e[0;37m"
#define BWHT "\e[1;37m"
#define CYN "\e[0;36m"
#define YEL "\e[0;33m"
#define BCYN "\e[1;36m"
#define BYEL "\e[1;33m"
#define BMAG "\e[1;35m"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define TIME_SLICE 1

typedef enum {
    COARSE,
    FINE
} sched_mode_t;

/*
set SCHED_MODE to:
    COARSE = coarse-grained; standard linear version
    FINE = fine-grained, interleaving
*/
#define SCHED_MODE COARSE

typedef struct {
    int uid;
    int weight;
    int original_weight;
    pid_t pid;
    int cpu_time;
} User;


// child process func
void child_work(int uid) {
    printf("Child process started for user %d (pid=%d)\n",
           uid, getpid());

    while (1) {
        printf("User %d runs...\n", uid);
        sleep(1);  // activity simulation
    }
}

// scheduler
void uwrr_scheduler(User *users, int n, int rounds, sched_mode_t mode) {
    printf(BWHT"\n=== Starting User Weighted Round-Robin Scheduler ===\n"WHT);
    printf("Mode: %s\n\n", mode == COARSE ? BYEL"COARSE-GRAINED" : "FINE-GRAINED"WHT);

    for (int r = 0; r < rounds; r++) {
        printf(BMAG"\n--- Round #%d ---\n"WHT, r + 1);

        // reset weights at start of each round
        for (int i = 0; i < n; i++) {
            users[i].weight = users[i].original_weight;
        }

        if (mode == COARSE) {
            // ===== COARSE-GRAINED =====
            for (int i = 0; i < n; i++) {
                int quantum = users[i].weight * TIME_SLICE;

                if (quantum <= 0)
                    continue;

                printf("Scheduler: START user %d for %d seconds\n",
                       users[i].uid, quantum);

                kill(users[i].pid, SIGCONT);
                sleep(quantum);
                kill(users[i].pid, SIGSTOP);

                users[i].cpu_time += quantum;
                printf("Scheduler: STOP user %d\n", users[i].uid);
            }
        } else {
            // ===== FINE-GRAINED (INTERLEAVING) =====
            int work_left;

            do {
                work_left = 0;

                for (int i = 0; i < n; i++) {
                    if (users[i].weight > 0) {
                        work_left = 1;

                        printf("Scheduler: START user %d for 1 second\n",
                               users[i].uid);

                        kill(users[i].pid, SIGCONT);
                        sleep(1);
                        kill(users[i].pid, SIGSTOP);

                        users[i].cpu_time += 1;
                        users[i].weight--;
                    }
                }
            } while (work_left);
        }
    }
}



int main() {
    system("clear");

    User users[] = {
        {1000, 1, 0, 0, 0},
        {2000, 5, 0, 0, 0},
        {3000, 3, 0, 0, 0}
    };

    int n = sizeof(users) / sizeof(users[0]);

    // IMPORTANT: initialize original_weight
    for (int i = 0; i < n; i++) {
        users[i].original_weight = users[i].weight;
    }

    // create child procs
    for (int i = 0; i < n; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            raise(SIGSTOP);
            child_work(users[i].uid);
            exit(0);
        } else {
            users[i].pid = pid;
        }
    }

    uwrr_scheduler(users, n, 3, SCHED_MODE);

    // cleanup
    for (int i = 0; i < n; i++) {
        kill(users[i].pid, SIGKILL);
        waitpid(users[i].pid, NULL, 0);
    }

    printf("\n=== CPU Stats ===\n");
    for (int i = 0; i < n; i++) {
        printf("User %d -> CPU total: %d seconds\n",
               users[i].uid, users[i].cpu_time);
    }

    return 0;
}

