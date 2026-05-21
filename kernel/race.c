#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

#define MAX_TEAMS 16

static struct spinlock race_lock;
static int scores[MAX_TEAMS];
static int num_teams;
static int target_score;
static int winner;

void
race_init_kernel(void) {
    initlock(&race_lock, "race_lock");
    num_teams = 0;
    target_score = 0;
    winner = -1;

    for (int i = 0; i < MAX_TEAMS; i++) {
        scores[i] = 0;
    }
}

int
race_init(int teams, int target) {
    if (teams <= 0 || teams > MAX_TEAMS || target <= 0) {
        return -1; // Invalid parameters
    }

    acquire(&race_lock);
    num_teams = teams;
    target_score = target;
    winner = -1;

    for (int i = 0; i < num_teams; i++) {
        scores[i] = 0;
    }
    release(&race_lock);
    return 0; // Success
}

int
race_inc_score(int team_id) {
    int score;

    if (team_id < 0 || team_id >= MAX_TEAMS) {
    return -1; // Invalid team ID
    }

    acquire(&race_lock);
    
    if (team_id >= num_teams) {
        release(&race_lock);
        return -1; // Team ID exceeds number of teams
    }

    if (winner != -1) {
        score = scores[team_id];
        release(&race_lock);
        return score; // Race already finished, return score of winning team
    }
    scores[team_id]++;
    score = scores[team_id];
    if (scores[team_id] >= target_score) {
        winner = team_id;
    }
    release(&race_lock);
    return score; // Return current score of the team
}

int
race_get_score(int team_id) {
    int score;

    if (team_id < 0 || team_id >= MAX_TEAMS) {
        return -1; // Invalid team ID
    }

    acquire(&race_lock);
    
    if (team_id >= num_teams) {
        release(&race_lock);
        return -1; // Team ID exceeds number of teams
    }

    score = scores[team_id];
    release(&race_lock);
    return score; // Return current score of the team
}

int
race_get_winner(void) {
    int winning_team_index = -1;
    acquire(&race_lock);
    
    winning_team_index = winner;
    
    release(&race_lock);
    return winning_team_index; // Return the ID of the winning team, or -1 if no winner yet
}


