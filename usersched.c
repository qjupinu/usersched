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

typedef struct {
    int uid;
    int weight;
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
void uwrr_scheduler(User *users, int n, int rounds) {
    printf(BWHT"\n=== Starting round-robin user weighted scheduler  ===\n"WHT);

    for (int r = 0; r < rounds; r++) {
        printf(BMAG"\n--- Round #%d ---\n"WHT, r + 1);

        for (int i = 0; i < n; i++) {
            int quantum = users[i].weight * TIME_SLICE;

            printf("Scheduler: START user %d for %d seconds\n",
                   users[i].uid, quantum);

            kill(users[i].pid, SIGCONT);
            sleep(quantum);
            kill(users[i].pid, SIGSTOP);

            users[i].cpu_time += quantum;
            printf("Scheduler: STOP user %d\n", users[i].uid);
        }
    }
}

int main() {
    system("clear");
    User users[] = {
        {1000, 1, 0, 0},
        {2000, 3, 0, 0},
        {3000, 5, 0, 0}
    };

    int n = sizeof(users) / sizeof(users[0]);

    // create child procs
    for (int i = 0; i < n; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            raise(SIGSTOP);  // wait for scheduler
            child_work(users[i].uid);
            exit(0);
        } else {
            users[i].pid = pid;
        }
    }

    uwrr_scheduler(users, n, 3);

    // kill all children
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
