#include "kernel/types.h"
#include "user/user.h"
#include "kernel/stat.h"

#define TEAMS 3
#define RUNNERS_PER_TEAM 5
#define TARGET_SCORE 30

/*
    * This user program simulates a relay race
    flow:
    run_race(int fav)
    - create lock with favoritism = fav
    - in a loop, create new proccess and assign them to a team
    - each procces will be assigned to a team via setgid()
    - maintain a shared score via the scores array, which get init via the race_init syscall
    - each process will increment the score of its team by calling the race_inc_score syscall
    - each process that increments the score will print the process id, team id and updated score
    - the race ends when one of the teams reaches the target score
*/

void
run_race(int fav){
    int lock_id;
    int total_runners = TEAMS * RUNNERS_PER_TEAM;

    if (race_init(TEAMS, TARGET_SCORE) < 0) {
        printf("Failed to initialize race\n");
        exit(1);
    };

    if ((lock_id = israeli_create(fav)) < 0) {
        printf("Failed to create Israeli lock\n");
        exit(1);
    }

    for (int i = 0; i < total_runners; i++) {
        int pid = fork();
        if (pid < 0) {
            printf("Failed to fork process\n");
            exit(1);
        } else if (pid == 0) {
            int team_id = i % TEAMS;
            setgid(team_id);
            while (race_get_winner() == -1) {
                if (israeli_acquire(lock_id) < 0) {
                    printf("Process %d failed to acquire lock\n", getpid());
                    exit(1);
                }

                if (race_get_winner() == -1) {
                    int score = race_inc_score(team_id);
                    printf("Runner %d from Team %d holds the baton\n", getpid(), team_id);
                    printf("Team %d updated score after increment: %d\n", team_id, score);
                }

                israeli_release(lock_id);
            }
            exit(0);
        }
    }

    for (int i = 0; i < total_runners; i++) {
        wait(0);
    }

    int winning_team = race_get_winner();

    printf("\nFinal scores for favoritism = %d:\n", fav);
    for (int i = 0; i < TEAMS; i++) {
        printf("Team %d: %d\n", i, race_get_score(i));
    }

    printf("--------------------------\n");
    printf("Winning team: Team %d\n", winning_team);
    printf("---------------------------\n\n");
    
    if (israeli_destroy(lock_id) < 0) {
        printf("Failed to destroy Israeli lock\n");
        exit(1);
    }
}

int
main(void) {
    run_race(0); // No favorit
    run_race(50); // Moderate favoritism
    run_race(100); // Strong favoritism
    exit(0);
}


