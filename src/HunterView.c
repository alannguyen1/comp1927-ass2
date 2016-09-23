// HunterView.c ... HunterView ADT implementation

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "HunterView.h"
#include "Map.h" //never hurts right... if you decide to use the Map ADT
     
struct hunterView {
    GameView g;
    LocationID DracTrail[TRAIL_SIZE]; //this is draculas trail, this is diff to actual trail
};
     

// Creates a new HunterView to summarise the current state of the game
HunterView newHunterView(char *pastPlays, PlayerMessage messages[])
{
    HunterView hunterView = malloc(sizeof(struct hunterView));
    hunterView->g = newGameView(pastPlays, messages); //we should have a dupe of gameview with all its shit filled correct/
    return hunterView;
}

// Frees all memory previously allocated for the HunterView toBeDeleted
void disposeHunterView(HunterView toBeDeleted)
{
    disposeGameView(toBeDeleted->g);
    free(toBeDeleted);
}


//// Functions to return simple information about the current state of the game

// Get the current round
Round giveMeTheRound(HunterView currentView)
{
    return getRound(currentView->g);
}

// Get the id of current player
PlayerID whoAmI(HunterView currentView)
{
    return getCurrentPlayer(currentView->g);
}

// Get the current score
int giveMeTheScore(HunterView currentView)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return getScore(currentView->g);
}

// Get the current health points for a given player
int howHealthyIs(HunterView currentView, PlayerID player)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return getHealth(currentView->g, player);
}

// Get the current location id of a given player
LocationID whereIs(HunterView currentView, PlayerID player)
{
    //talk to RICH ABOUT THIS BC WE DONT STORE DRACS TRUE LOC EVER, DONT WE NEED TO?
    LocationID Loc = getLocation(currentView->g, player);
    //currentView->g->players[player]->history[0];
    return Loc;
}

//// Functions that return information about the history of the game

// Fills the trail array with the location ids of the last 6 turns
void giveMeTheTrail(HunterView currentView, PlayerID player,
                            LocationID trail[TRAIL_SIZE])
{
    getHistory(currentView->g, player, trail);
}

//// Functions that query the map to find information about connectivity

// What are my possible next moves (locations)
LocationID *whereCanIgo(HunterView currentView, int *numLocations,
                        int road, int rail, int sea)
{
    //check first round where whole map is avail or go to conneectedlocs
    LocationID * Locs;
    if (giveMeTheRound(currentView) == 0) {//first round
        Locs = (LocationID *)(malloc(sizeof(LocationID)*NUM_MAP_LOCATIONS));
        *numLocations = NUM_MAP_LOCATIONS; //derefrence pointer and give location
        for (int i = 0; i< NUM_MAP_LOCATIONS; i++) {//whole map is good
            Locs[i] = i;
        }
    } else {
        Locs = connectedLocations(currentView->g, numLocations,getLocation(currentView->g, getCurrentPlayer(currentView->g)),getCurrentPlayer(currentView->g),getRound(currentView->g), road ,rail, sea);
    }
    return Locs;
}

// What are the specified player's next possible moves
LocationID *whereCanTheyGo(HunterView currentView, int *numLocations,
                           PlayerID player, int road, int rail, int sea)
{
    
    Round currRound = giveMeTheRound(currentView);
    LocationID currLoc = getLocation(currentView->g, player);
    LocationID * Locs;
    //first check if player is before you or after you and modify round if before
    if (player < whoAmI(currentView)) {
        currRound++;
    }
    //printf("player right now is: %d on round %d at %d\n", player, currRound, currLoc);
    if (currRound == 0 && player != whoAmI(currentView)){
        if (player == PLAYER_DRACULA) {
            Locs = (LocationID *)(malloc(sizeof(LocationID)*NUM_MAP_LOCATIONS-1));
            *numLocations = NUM_MAP_LOCATIONS-1;
            for (int i = 0; i< NUM_MAP_LOCATIONS; i++) {//all but hospital
                if (i != ST_JOSEPH_AND_ST_MARYS) {
                    Locs[i] = i;
                }
            }
        } else {
            Locs = (LocationID *)(malloc(sizeof(LocationID)*NUM_MAP_LOCATIONS));
            *numLocations = NUM_MAP_LOCATIONS; //derefrence pointer and give location
            for (int i = 0; i< NUM_MAP_LOCATIONS; i++) {//whole map is good
                Locs[i] = i;
            }
        }
    } else {
        if (player == PLAYER_DRACULA) {
            Locs = connectedLocations(currentView->g, numLocations,currLoc,player,currRound, road ,FALSE, sea);
        } else {
            Locs = connectedLocations(currentView->g, numLocations,currLoc,player,currRound, road ,rail, sea);
        }
    }
    return Locs;
}
