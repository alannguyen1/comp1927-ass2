// GameView.c ... GameView ADT implementation

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "Map.h"

typedef struct _player *Player;

struct gameView {
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    Player players[NUM_PLAYERS];
    Map map;
    int score;
    Round currentRound;
    PlayerID currentPlayer;
    LocationID immatureVampire;
    LocationID Traps[NUM_MAP_LOCATIONS];
}; //hey baby


typedef struct _player {
    int health;
    LocationID current;
    LocationID history[TRAIL_SIZE];
} *Player;

// Extra Functions
static void setInitialState(GameView newGameView);
static void analyseMove(char move[], GameView g);
static void actionT(PlayerID player, GameView g);
static void actionD(PlayerID player, GameView g);
static void actionV(PlayerID player, GameView g);
static int calculateArrayLength(char* pastPlays);
static void addToTrail(PlayerID player, GameView g, LocationID currLocation);
static void checkDracSea(GameView g);

// Drac Actions
static void dracT(GameView g);
static void dracIV(GameView g);
static void dracM(GameView g);
static void dracMV(GameView g);

// Creates a new GameView to summarise the current state of the game
GameView newGameView(char *pastPlays, PlayerMessage messages[])
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    GameView g = malloc(sizeof(struct gameView));
    assert(g != NULL);
    int numPlays = calculateArrayLength(pastPlays)/8;
    setInitialState(g);
    g->currentRound = numPlays / NUM_PLAYERS;
    g->currentPlayer = numPlays % NUM_PLAYERS;
    int currPlay = 0;
    int strIndex = 0;

    while (currPlay < numPlays) {
        // pass in GST.....
        char move[8];
        memcpy(move, &pastPlays[strIndex], 7);
        move[8] = '\0';
        //where all the magic happens
        analyseMove(move, g);
        strIndex += 8;
        currPlay += 1;
    }

    return g;
}

static int calculateArrayLength(char* pastPlays)
{
    int length = 0;
    for (int i=0; pastPlays[i] != '\0'; i++) {
        length++;
    }
    return (length+1);
}

static void analyseMove(char move[], GameView g)
{   // when you initialise an array you have to 
    // write the array size in the square brackets
    // you need an extra byte for a null terminator
    // otherwise there will be buffer overflow
    // Khaled said go back to COMP1917 
    PlayerID player = move[0];
    char location[3];
    memcpy(location, &move[1], 2);
    location[2] = '\0';
    LocationID currLocation = abbrevToID(location);
    char actions[5];
    memcpy(actions, &move[3], 4);
    actions[4] = '\0';
    //Do stuff if it's a dracula
    if (player == 'D') {
        // move phase
        if (strcmp("C?",location) == 0) {
            g->players[PLAYER_DRACULA]->current = CITY_UNKNOWN;
            addToTrail(PLAYER_DRACULA, g, CITY_UNKNOWN);
        } else if (strcmp("S?", location) == 0) {
            g->players[PLAYER_DRACULA]->current = SEA_UNKNOWN;
            addToTrail(PLAYER_DRACULA, g, SEA_UNKNOWN);
        } else if (strcmp("HI", location) == 0) {
            g->players[PLAYER_DRACULA]->current = HIDE;
            addToTrail(PLAYER_DRACULA, g, HIDE);
        } else if (strcmp("D1", location) == 0) {
            g->players[PLAYER_DRACULA]->current = DOUBLE_BACK_1;
            addToTrail(PLAYER_DRACULA, g, DOUBLE_BACK_1);
        } else if (strcmp("D2", location) == 0) {
            g->players[PLAYER_DRACULA]->current = DOUBLE_BACK_2;
            addToTrail(PLAYER_DRACULA, g, DOUBLE_BACK_2);
        } else if (strcmp("D3", location) == 0) {
            g->players[PLAYER_DRACULA]->current = DOUBLE_BACK_3;
            addToTrail(PLAYER_DRACULA, g, DOUBLE_BACK_3);
        } else if (strcmp("D4", location) == 0) {
            g->players[PLAYER_DRACULA]->current = DOUBLE_BACK_4;
            addToTrail(PLAYER_DRACULA, g, DOUBLE_BACK_4);
        } else if (strcmp("D5", location) == 0) {
            g->players[PLAYER_DRACULA]->current = DOUBLE_BACK_5;
            addToTrail(PLAYER_DRACULA, g, DOUBLE_BACK_5);
        } else if (strcmp("TP", location) == 0) {
            g->players[PLAYER_DRACULA]->current = CASTLE_DRACULA;
            addToTrail(PLAYER_DRACULA, g, CASTLE_DRACULA);
            g->players[PLAYER_DRACULA]->health += LIFE_GAIN_CASTLE_DRACULA;
        } else if (strcmp("..", location) == 0){
            // do nothing
        } else {
            g->players[PLAYER_DRACULA]->current = abbrevToID(location);
            addToTrail(PLAYER_DRACULA, g, abbrevToID(location));
        }
        checkDracSea(g);
        // Encounter
        char encounter[3];
        memcpy(encounter, &actions[0], 2);
        encounter[2] = '\0';
        for (int i = 0; i < 2; i++) {
            switch(encounter[i]) {
                case 'T': dracT(g); break;
                case 'V': dracIV(g); break;
                case '.': break;
            }
        }

        //Action
        char action[3];
        memcpy(action, &actions[2], 2);
        action[2] = '\0';
        for (int i = 0; i < 2; i++) {
            switch(action[i]) {
                case 'M': dracM(g); break;
                case 'V': dracMV(g); break;
                case '.': break;
            }
        }
    }
    
    // Do stuff if its a hunter
    if (player != 'D') {//this is not dracula, aka hunter
        int currHunter;
        switch (player) { //gain hunter value
                case 'G': currHunter = PLAYER_LORD_GODALMING; break;
                case 'S': currHunter = PLAYER_DR_SEWARD; break;
                case 'H': currHunter = PLAYER_VAN_HELSING; break;
                case 'M': currHunter = PLAYER_MINA_HARKER; break;
            }
        //make a case if hunter has previously died, if so, reset health cuz he is good to go
        if (g->players[currHunter]->current == ST_JOSEPH_AND_ST_MARYS && g->players[currHunter]->health <= 0) {
            g->players[currHunter]->health = GAME_START_HUNTER_LIFE_POINTS;

        }
        // Move the player
        g->players[currHunter]->current = currLocation;
        addToTrail(currHunter, g, currLocation);

        // Make the action?
        for (int i=0; i < 4; i++) {
            if (g->players[currHunter]->health > 0 && g->players[PLAYER_DRACULA]->health > 0) {
                // both alive
                switch(actions[i]) {
                    case 'T': actionT(currHunter, g); break;
                    case 'D': actionD(currHunter, g); break;
                    case 'V': actionV(currHunter, g); break;
                    case '.': break;//do nothing;
                }
            } else if (g->players[currHunter]->health <= 0 && g->players[PLAYER_DRACULA]->health > 0) {
                // player is dead dracula is alive
                g->players[currHunter]->health = 0;
                g->players[currHunter]->current = ST_JOSEPH_AND_ST_MARYS;
                g->score -= SCORE_LOSS_HUNTER_HOSPITAL;
                addToTrail(currHunter, g, currLocation);
                break;
            } else if (g->players[PLAYER_DRACULA]->health <= 0) {
                // player is alive and dracula is dead
                // player is dead and dracula is dead
                // endGame(g GameView); //implement this??
            }
        }
    }
}

static void dracT(GameView g)
{
    LocationID curr = g->players[PLAYER_DRACULA]->current;
    g->Traps[curr] += 1;
}

static void dracIV(GameView g)
{
    LocationID curr = g->players[PLAYER_DRACULA]->current;
    g->immatureVampire = curr;
}

static void dracM(GameView g)
{
    LocationID trapLoc = g->players[PLAYER_DRACULA]->history[5];
    assert(g->Traps[trapLoc] > 0);
    g->Traps[trapLoc] -= 1;
}

static void dracMV(GameView g)
{
    g->score -= SCORE_LOSS_VAMPIRE_MATURES;
    g->immatureVampire = UNKNOWN_LOCATION;
}

static void actionT(PlayerID player, GameView g)
{
    g->players[player]->health = g->players[player]->health - LIFE_LOSS_TRAP_ENCOUNTER;
}

static void checkDracSea(GameView g)
{
    Player Dracula = g->players[PLAYER_DRACULA];
    if (Dracula->current == SEA_UNKNOWN) {
        Dracula->health -= LIFE_LOSS_SEA;
        return;
    }
    if (Dracula->current == DOUBLE_BACK_1) {
        if (Dracula->history[1] == SEA_UNKNOWN) {
            Dracula->health -= LIFE_LOSS_SEA;
            return;
        }
        if (validPlace(Dracula->history[1])) {
            if (idToType(Dracula->history[1]) == SEA) {
                Dracula->health -= LIFE_LOSS_SEA;
                return;
            }
        }
    }
    if (Dracula->current == DOUBLE_BACK_2) {
        if (Dracula->history[2] == SEA_UNKNOWN) {
            Dracula->health -= LIFE_LOSS_SEA;
            return;
        }
        if (validPlace(Dracula->history[2])) {
            if (idToType(Dracula->history[2]) == SEA) {
                Dracula->health -= LIFE_LOSS_SEA;
                return;
            }
        }
    }
    if (Dracula->current == DOUBLE_BACK_3) {
        if (Dracula->history[3] == SEA_UNKNOWN) {
            Dracula->health -= LIFE_LOSS_SEA;
            return;
        }
        if (validPlace(Dracula->history[3])) {
            if (idToType(Dracula->history[3]) == SEA) {
                Dracula->health -= LIFE_LOSS_SEA;
                return;
            }
        }
    }
    if (Dracula->current == DOUBLE_BACK_4) {
        if (Dracula->history[4] == SEA_UNKNOWN) {
            Dracula->health -= LIFE_LOSS_SEA;
            return;
        }
        if (validPlace(Dracula->history[4])) {
            if (idToType(Dracula->history[4]) == SEA) {
                Dracula->health -= LIFE_LOSS_SEA;
                return;
            }
        }
    }
    if (Dracula->current == DOUBLE_BACK_5) {
        if (Dracula->history[5] == SEA_UNKNOWN) {
            Dracula->health -= LIFE_LOSS_SEA;
            return;
        }
        if (validPlace(Dracula->history[5])) {
            if (idToType(Dracula->history[5]) == SEA) {
                Dracula->health -= LIFE_LOSS_SEA;
                return;
            }
        }
    }
    if (validPlace(Dracula->current)) {
        if (idToType(Dracula->current) == SEA) {
            Dracula->health -= LIFE_LOSS_SEA;
            return;
        }
    }
}

static void actionD(PlayerID player, GameView g)
{
    g->players[player]->health = g->players[player]->health - LIFE_LOSS_DRACULA_ENCOUNTER;
    g->players[PLAYER_DRACULA]->health = g->players[PLAYER_DRACULA]->health - LIFE_LOSS_HUNTER_ENCOUNTER;
    g->players[PLAYER_DRACULA]->current = g->players[player]->current;
    g->players[PLAYER_DRACULA]->history[0] = g->players[player]->current;
}

static void actionV(PlayerID player, GameView g)
{
    //immature vampire at this location is killed
    g->immatureVampire = NOWHERE;
    
    //for dracula if (currentRound % 18 = 0 && g->immatureVampire != NOWHERE) {
    //  g->score -= 13;
    //  g->immatureVampire = NOWHERE;
    //} //also make a case 
    
}

static void addToTrail(PlayerID player, GameView g, LocationID currLocation)
{
    for (int i = TRAIL_SIZE-1; i > 0; i--) {
        g->players[player]->history[i] = g->players[player]->history[i-1];
    }
    g->players[player]->history[0] = currLocation;
}


     
static void setInitialState(GameView g)
{
    g->map = newMap();
    g->score = GAME_START_SCORE;
    g->immatureVampire = NOWHERE;

    g->currentRound = 0;
    g->currentPlayer = PLAYER_LORD_GODALMING;

    g->players[PLAYER_LORD_GODALMING] = malloc(sizeof(struct _player));
    g->players[PLAYER_DR_SEWARD] = malloc(sizeof(struct _player));
    g->players[PLAYER_VAN_HELSING] = malloc(sizeof(struct _player));
    g->players[PLAYER_MINA_HARKER] = malloc(sizeof(struct _player));
    g->players[PLAYER_DRACULA] = malloc(sizeof(struct _player));
    

    g->players[PLAYER_LORD_GODALMING]->health = GAME_START_HUNTER_LIFE_POINTS;
    g->players[PLAYER_LORD_GODALMING]->current = NOWHERE;
    memset ( g->players[PLAYER_LORD_GODALMING]->history, NOWHERE, sizeof ( g->players[PLAYER_LORD_GODALMING]->history));

    g->players[PLAYER_DR_SEWARD]->health = GAME_START_HUNTER_LIFE_POINTS;
    g->players[PLAYER_DR_SEWARD]->current = NOWHERE;
    memset ( g->players[PLAYER_DR_SEWARD]->history, NOWHERE, sizeof ( g->players[PLAYER_DR_SEWARD]->history));

    g->players[PLAYER_VAN_HELSING]->health = GAME_START_HUNTER_LIFE_POINTS;
    g->players[PLAYER_VAN_HELSING]->current = NOWHERE;
    memset ( g->players[PLAYER_VAN_HELSING]->history, NOWHERE, sizeof ( g->players[PLAYER_VAN_HELSING]->history));
    
    g->players[PLAYER_MINA_HARKER]->health = GAME_START_HUNTER_LIFE_POINTS;
    g->players[PLAYER_MINA_HARKER]->current = NOWHERE;
    memset ( g->players[PLAYER_MINA_HARKER]->history, NOWHERE, sizeof ( g->players[PLAYER_MINA_HARKER]->history));

    g->players[PLAYER_DRACULA]->health = GAME_START_BLOOD_POINTS;
    g->players[PLAYER_DRACULA]->current = NOWHERE;
    memset ( g->players[PLAYER_DRACULA]->history, NOWHERE, sizeof ( g->players[PLAYER_DRACULA]->history));

}
// Frees all memory previously allocated for the GameView toBeDeleted
void disposeGameView(GameView toBeDeleted)
{
    //COMPLETE THIS IMPLEMENTATION
    assert(toBeDeleted != NULL);
    disposeMap(toBeDeleted->map);
    for (int i = 0; i < NUM_PLAYERS; i++) {
        free(toBeDeleted->players[i]);
    }
    free ( toBeDeleted );
}


//// Functions to return simple information about the current state of the game

// Get the current round
Round getRound(GameView currentView)
{
    return currentView->currentRound;
}

// Get the id of current player - ie whose turn is it?
PlayerID getCurrentPlayer(GameView currentView)
{
    return currentView->currentPlayer;
}

// Get the current score
int getScore(GameView currentView)
{
    return currentView->score;
}

// Get the current health points for a given player
int getHealth(GameView currentView, PlayerID player)
{
    return currentView->players[player]->health;
}

// Get the current location id of a given player
LocationID getLocation(GameView currentView, PlayerID player)
{
    return currentView->players[player]->current;
}

//// Functions that return information about the history of the game

// Fills the trail array with the location ids of the last 6 turns
void getHistory(GameView currentView, PlayerID player,
                            LocationID trail[TRAIL_SIZE])
{
    for (int i=0; i < TRAIL_SIZE; i++) {
        trail[i] = currentView->players[player]->history[i];
    }
}

//// Functions that query the map to find information about connectivity

// Returns an array of LocationIDs for all directly connected locations

LocationID *connectedLocations(GameView currentView, int *numLocations,
                               LocationID from, PlayerID player, Round round,
                               int road, int rail, int sea)
{
    Map map = currentView->map;
    int railDist = (round + player) % 4;

    if (player == PLAYER_DRACULA) {
        railDist = FALSE;
    }
    
    int reachableRoad[NUM_MAP_LOCATIONS] = {FALSE};
    int reachableSea[NUM_MAP_LOCATIONS] = {FALSE};
    int reachableRailOne[NUM_MAP_LOCATIONS] = {FALSE};
    int reachableRailTwo[NUM_MAP_LOCATIONS] = {FALSE};
    int reachableRailThree[NUM_MAP_LOCATIONS] = {FALSE};
    
    //connections via ROAD and SEA and RAIL (one step ONLY)
    for (int to = 0; to < NUM_MAP_LOCATIONS; to++) {
        TransportID type[] = {0,0,0};
        connections(map, from, to, type);
        for (int i = 0; i < 3; i++) {
            if (type[i] == 1 && road == TRUE) {
                reachableRoad[to] = TRUE;
            }
            if (type[i] == 2 && railDist >= 1) {
                reachableRailOne[to] = TRUE;
            }
            if (type[i] == 3 && sea == TRUE) {
                reachableSea[to] = TRUE;
            }
        }
    }
    
    if (railDist == 2) {
        for (int start = 0; start < NUM_MAP_LOCATIONS; start++) {
            if (reachableRailOne[start] == TRUE) {
                for (int to = 0; to <= NUM_MAP_LOCATIONS; to++) {
                    TransportID type[] = {0,0,0};
                    connections(map, start, to, type);
                    for (int i = 0; i < 3; i++) {
                        if (type[i] == 2 && railDist == 2) {
                            reachableRailTwo[to] = TRUE;
                        }
                    }
                }
            }
        }
    }
    
    if (railDist == 3) {
        for (int start = 0; start < NUM_MAP_LOCATIONS; start++) {
            if (reachableRailTwo[start] == TRUE) {
                for (int to = 0; to <= NUM_MAP_LOCATIONS; to++) {
                    TransportID type[] = {0,0,0};
                    connections(map, start, to, type);
                    for (int i = 0; i < 3; i++) {
                        if (type[i] == 2 && railDist == 3) {
                            reachableRailThree[to] = TRUE;
                        }
                    }
                }
            }
        }
    }
    
    int finalReach[NUM_MAP_LOCATIONS] = {FALSE};
    
    for (int j = 0; j < NUM_MAP_LOCATIONS; j++) {
        if (reachableRoad[j] == TRUE) {
            finalReach[j] = TRUE;
        } if (reachableSea[j] == TRUE) {
            finalReach[j] = TRUE;
        } if (reachableRailOne[j] == TRUE) {
            finalReach[j] = TRUE;
        } if (reachableRailTwo[j] == TRUE) {
            finalReach[j] = TRUE;
        } if (reachableRailThree[j] == TRUE) {
            finalReach[j] = TRUE;
        }
    }
    //known cases
    finalReach[from] = TRUE;
    if (player == PLAYER_DRACULA) {
        finalReach[ST_JOSEPH_AND_ST_MARYS] = FALSE;
    }
    
    int retLen = 0;
    for (int k = 0; k < NUM_MAP_LOCATIONS; k++) {
        if (finalReach[k] == TRUE) {
            retLen++;
        }
    }
    LocationID * connectedLocs = (LocationID *)malloc(retLen*sizeof(LocationID));
    int k = 0;
    for (int locations = 0; locations < NUM_MAP_LOCATIONS; locations++) {
        if (finalReach[locations] == TRUE) {
            connectedLocs[k] = locations;
            k++;
        }
    }
    *numLocations = retLen;
    //printf("retLen is %d and k is %d", retLen, k);
    return connectedLocs;
}
