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

// Creates a new GameView to summarise the current state of the game
GameView newGameView(char *pastPlays, PlayerMessage messages[])
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    GameView g = malloc(sizeof(struct gameView));
    assert(g != NULL);
    int numPlays = calculateArrayLength(pastPlays)/8;
    printf("numPlays is %d\n", numPlays);
    setInitialState(g);
    g->currentRound = numPlays / NUM_PLAYERS;
    int currPlay = 0;
    int strIndex = 0;

    while (currPlay < numPlays) {
        // pass in GST.....
        char move[8];
        memcpy(move, &pastPlays[strIndex], 7);
        move[8] = '\0';
        printf("move is %s\n", move);
        printf("currPlay is %d\n", currPlay);
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
{
    PlayerID player = move[0];
    char location[2];
    memcpy(location, &move[1], 2);
    location[2] = '\0';
    LocationID currLocation = abbrevToID(location);
    char actions[4];
    memcpy(actions, &move[3], 4);
    actions[4] = '\0';
    printf("curr Player is %c\n", player);
    printf("location is %s\n", location);
    printf("action is %s\n", actions);
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
        } else if (strcmp("..", location) == 0){

        } else {
            g->players[PLAYER_DRACULA]->current = abbrevToID(location);
            addToTrail(PLAYER_DRACULA, g, abbrevToID(location));
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
        if (g->players[currHunter]->current == ST_JOSEPH_AND_ST_MARYS && g->players[currHunter]->health == 0) {
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

static void actionT(PlayerID player, GameView g)
{
    g->players[player]->health = g->players[player]->health - LIFE_LOSS_TRAP_ENCOUNTER;
}

static void actionD(PlayerID player, GameView g)
{
    g->players[player]->health = g->players[player]->health - LIFE_LOSS_DRACULA_ENCOUNTER;
    g->players[PLAYER_DRACULA]->health = g->players[PLAYER_DRACULA]->health - LIFE_LOSS_HUNTER_ENCOUNTER;
    printf("%d\n", g->players[player]->current);
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
    for (int i = 0; i < TRAIL_SIZE; i++) {
        free(toBeDeleted->players[i]);
    }
    free( toBeDeleted );
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
    //TODO: Implement function to return array of locationIDs
    int railDist = (currentRound + player) % 4;
    int rail = TRUE;

    if (player == PLAYER_DRACULA) {
        rail = FALSE;
    }

    reachable[NUM_MAP_LOCATIONS] = {FALSE};
    reachableOne[NUM_MAP_LOCATIONS] = {FALSE};
    reachableTwo[NUM_MAP_LOCATIONS] = {FALSE};
    reachableThree[NUM_MAP_LOCATIONS] = {FALSE};

    for (i = 0; i <= NUM_MAP_LOCATIONS; i++) {

    }

    reachablep[from] = TRUE; //because current location is always true af




    return NULL;
}