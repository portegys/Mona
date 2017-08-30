// For conditions of distribution and use, see copyright notice in muzz.hpp

/*
 *
 * The Muzz World.
 *
 * Muzz creatures interact with their block world.
 * Each muzz has a mona neural network for a brain.
 *
 */

#ifndef MUZZ_WORLD
#define MUZZ_WORLD

// Version (SCCS "what" format).
#define MUZZ_WORLD_VERSION    "@(#)Muzz world version 1.1"
extern const char *MuzzWorldVersion;

#include "TmazeTerrain.hpp"
#include "muzz.hpp"
#include "mushroom.hpp"
#include "pool.hpp"

// Print version.
void printVersion(FILE *out = stdout);

// Muzzes.
#define DEFAULT_NUM_MUZZES    1
extern int            NUM_MUZZES;
extern vector<Muzz *> Muzzes;

// Block terrain.
// See blockTerrain.hpp for terrain generation parameters.
#define DEFAULT_TERRAIN_DIMENSION    4
extern int          TerrainDimension;
extern BlockTerrain *Terrain;

// Mushrooms.
#define DEFAULT_NUM_MUSHROOMS    1
extern int                NUM_MUSHROOMS;
extern vector<Mushroom *> Mushrooms;

// Water pools.
#define DEFAULT_NUM_POOLS    1
extern int            NUM_POOLS;
extern vector<Pool *> Pools;

// Sense block id information.
// This provides additional sensory information on blocks.
#define SENSE_BLOCK_ID    0

// Initialize, run, and terminate.
void initMuzzWorld();
void runMuzzWorld();
void termMuzzWorld();

// Reset muzz world.
void resetMuzzWorld();

// Place muzz in world without changing its initial stored placement.
void placeMuzz(int muzzIndex, int placeX, int placeZ,
               BlockTerrain::Block::DIRECTION placeDir);

// Run cycles.
extern int Cycles;
extern int CycleCounter;

// Reset training.
void resetTraining();

// Number of training trials.
extern int NumTrainingTrials;

// Return runMuzzWorld when muzz has gotten food and water.
extern bool RunMuzzWorldUntilGoalsGotten;

// Forced response training.
extern bool ForcedResponseTrain;

// Random numbers.
extern RANDOM RandomSeed;
extern Random *Randomizer;
extern RANDOM ObjectSeed;
extern RANDOM TerrainSeed;

// Files.
extern char *SaveFile;
extern char *LoadFile;

// Show graphics.
extern bool Graphics;

// Smooth movements.
extern bool SmoothMove;

// Pause cycles.
extern bool Pause;
#endif
