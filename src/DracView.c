// DracView.c ... DracView ADT implementation

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "DracView.h"
// #include "Map.h" ... if you decide to use the Map ADT
typedef struct _player *Player;

struct dracView {
    GameView g;
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    Player players[NUM_PLAYERS];
    LocationID immatureVampire;
    LocationID Traps[NUM_MAP_LOCATIONS];
};

typedef struct _player {
    int connections[NUM_MAP_LOCATIONS];
    int numLocations;
} *Player;

static void analyseTraps(DracView dracView, char *pastPlays);
     

// Creates a new DracView to summarise the current state of the game
DracView newDracView(char *pastPlays, PlayerMessage messages[])
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    DracView dracView = malloc(sizeof(struct dracView));
    dracView->g = newGameView(pastPlays, messages);
    memset(dracView->Traps, 0, NUM_MAP_LOCATIONS* sizeof(LocationID));

    for (int i = 0; i < NUM_PLAYERS; i++) {
        dracView->players[i] = malloc(sizeof(struct _player));
        int size, *edges;
        if (getLocation(dracView->g, i) == UNKNOWN_LOCATION) break;
        memset(dracView->players[i]->connections, 0, sizeof(dracView->players[i]->connections));
        edges = connectedLocations(dracView->g, &size, getLocation(dracView->g, i), i, getRound(dracView->g), TRUE, TRUE, TRUE);
        for (int j = 0; j < size; j++) {
            dracView->players[i]->connections[edges[j]] = 1;
        }
        free(edges);
    }
    analyseTraps(dracView, pastPlays);
    return dracView;
}

static void analyseTraps(DracView dracView, char *pastPlays)
{
    int length = 0;
    for (int i=0; pastPlays[i] != '\0'; i++) {
        length++;
    }
    length = length+1;
    int numPlays = length / 8;

    int currPlay = 0;

    while (currPlay < numPlays) {
        if ((currPlay+1) % 5 == 0) {
            assert(pastPlays[currPlay*8] == 'D');

            char action[2];
            memcpy(action, &pastPlays[currPlay*8+5], 1);
            action[1] = '\0';

            char location[3];
            memcpy(location, &pastPlays[currPlay*8+1], 2);
            location[2] = '\0';

            char encounter[3];
            memcpy(encounter, &pastPlays[currPlay*8+3], 2);
            encounter[2] = '\0';

            for (int i = 0; i < 2; i++) {
                switch(encounter[i]) {
                    case 'T': dracView->Traps[abbrevToID(location)] += 1; break;
                    case 'V': dracView->immatureVampire = abbrevToID(location); break;
                }
            }
            if (action[0] == 'M') {
                LocationID history[TRAIL_SIZE];
                getHistory(dracView->g, PLAYER_DRACULA, history);
                LocationID trapLoc = history[5];
                dracView->Traps[trapLoc] -= 1;
            }
            if (action[0] == 'V') {
                dracView->immatureVampire = NOWHERE;
            }
        }
        currPlay += 1;
    }
}
     
     
// Frees all memory previously allocated for the DracView toBeDeleted
void disposeDracView(DracView toBeDeleted)
{
    //COMPLETE THIS IMPLEMENTATION
    disposeGameView(toBeDeleted->g);
    for (int i = 0; i < NUM_PLAYERS; i++) {
        free ( toBeDeleted->players[i] );
    }
    free( toBeDeleted );
}


//// Functions to return simple information about the current state of the game

// Get the current round
Round giveMeTheRound(DracView currentView)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return getRound(currentView->g);
}

// Get the current score
int giveMeTheScore(DracView currentView)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return getScore(currentView->g);
}

// Get the current health points for a given player
int howHealthyIs(DracView currentView, PlayerID player)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return getHealth(currentView->g, player);
}

// Get the current location id of a given player
LocationID whereIs(DracView currentView, PlayerID player)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return getLocation(currentView->g, player);
}

// Get the most recent move of a given player
void lastMove(DracView currentView, PlayerID player,
                 LocationID *start, LocationID *end)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    LocationID history[TRAIL_SIZE];
    getHistory(currentView->g, player, history);
    *start = history[1];
    *end = history[0];
}

// Find out what minions are placed at the specified location
void whatsThere(DracView currentView, LocationID where,
                         int *numTraps, int *numVamps)
{
    *numTraps = currentView->Traps[where];
    if (where == currentView->immatureVampire) {
        *numVamps = 1;
    } else {
        *numVamps = 0;
    }
}

//// Functions that return information about the history of the game

// Fills the trail array with the location ids of the last 6 turns
void giveMeTheTrail(DracView currentView, PlayerID player,
                            LocationID trail[TRAIL_SIZE])
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    LocationID history[TRAIL_SIZE];
    getHistory(currentView->g, player, history);
    for (int i = 0; i < TRAIL_SIZE; i++) {
        trail[i] = history[i];
    }
}

//// Functions that query the map to find information about connectivity

// What are my (Dracula's) possible next moves (locations)
LocationID *whereCanIgo(DracView currentView, int *numLocations, int road, int sea)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    LocationID *moves = (LocationID *)malloc(currentView->players[PLAYER_DRACULA]->numLocations*sizeof(LocationID));
    int k = 0;
    for (int i = 0; i < NUM_MAP_LOCATIONS; i++) {
        if (currentView->players[PLAYER_DRACULA]->connections[i] == 1 ) {
            if (road == TRUE && idToType(i) == ROAD) {
                moves[k] = i;
                k++;
            }
            if (sea == TRUE && idToType(i) == SEA) {
                moves[k] = i;
                k++;
            }
        }
    }
    *numLocations = k;

    return moves;
}

// What are the specified player's next possible moves
LocationID *whereCanTheyGo(DracView currentView, int *numLocations,
                           PlayerID player, int road, int rail, int sea)
{
    Round currRound = giveMeTheRound(currentView);
    LocationID currLoc = getLocation(currentView->g, player);
    LocationID * Locs;
    //first check if player is before you or after you and modify round if before
    currRound++;
    //printf("player right now is: %d on round %d at %d\n", player, currRound, currLoc);
    if (currRound == 0){
            Locs = (LocationID *)(malloc(sizeof(LocationID)*NUM_MAP_LOCATIONS));
            *numLocations = NUM_MAP_LOCATIONS; //derefrence pointer and give location
            for (int i = 0; i< NUM_MAP_LOCATIONS; i++) {//whole map is good
                Locs[i] = i;
            }
    } else {
        Locs = connectedLocations(currentView->g, numLocations,currLoc,player,currRound, road ,rail, sea);
    }
    return Locs;
}
