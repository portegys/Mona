/*
 * This software is provided under the terms of the GNU General
 * Public License as published by the Free Software Foundation.
 *
 * Copyright (c) 2003-2005 Tom Portegys, All Rights Reserved.
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for NON-COMMERCIAL purposes and without
 * fee is hereby granted provided that this copyright notice
 * appears in all copies.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.
 */

/**
 * Evolve instinctive behavior for the "Monkey and Bananas" problem.
 *
 * Usage:
 * evolve_instincts
 *    -learning <instinct_only | experience_only |
 *       instinct_and_experience | instinct_and_retain_experience>
 *    -generations <evolution generations>
 *    [-input <evolution input file name> (for run continuation)]
 *    -output <evolution output file name>
 *    -scramble (scramble item locations)
 *    [-randomSeed <random seed>]
 *    [-logfile <log file name>]
 *    [-display <text | graphics>]
 */

#ifdef WIN32
#include <windows.h>
#endif
#ifdef UNIX
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#endif
#include "monkey_and_bananas.hpp"
#include "../common/log.hpp"

// Usage.
char *Usage[] =
{
    "Usage:",
    "evolve_instincts",
    "\t-learning <instinct_only | experience_only |",
    "\t\tinstinct_and_experience | instinct_and_retain_experience>",
    "\t-generations <evolution generations>",
    "\t[-input <evolution input file name> (for run continuation)]",
    "\t-output <evolution output file name>",
    "\t-scramble (scramble item locations)",
    "\t[-randomSeed <random seed>]",
    "\t[-logfile <log file name>]",
    "\t[-display <text | graphics>]",
    NULL
};

void logUsage()
{
    for (int i = 0; Usage[i] != NULL; i++)
    {
        sprintf(Log::messageBuf, Usage[i]);
        Log::logInformation();
    }
}


#ifdef WIN32
#ifdef _DEBUG
// For Windows memory checking, set CHECK_MEMORY = 1
#define CHECK_MEMORY 0
#if ( CHECK_MEMORY == 1 )
#include <crtdbg.h>
#endif
#endif
#endif

// Evolution parameters.
#define FIT_POPULATION_SIZE 10
#define NUM_MUTANTS 8
#define NUM_OFFSPRING 2
#define POPULATION_SIZE (FIT_POPULATION_SIZE + NUM_MUTANTS + NUM_OFFSPRING)
#define MUTATION_RATE 0.1
#ifdef ONE_BOX_WORLD
#define MONKEY_STEPS 50
#else
#define MONKEY_STEPS 200
#endif
#define MAXIMUM_MEDIATORS 50
#define EVOLVE_LOGGING LOG_TO_PRINT
#define DEFAULT_EVOLVE_LOG_FILE_NAME "evolve.log"

// Learning types.
enum
{
    // Learn only by instinct evolution.
    INSTINCT_ONLY,
    // Learn only by experience.
    EXPERIENCE_ONLY,
    // Learn by instinct evolution experience.
    INSTINCT_AND_EXPERIENCE,
    // Instinct learning and retained experiential learning.
    INSTINCT_AND_AND_RETAIN_EXPERIENCE
} LearningType;

#ifdef UNIX
// For remote control of display:

// Shared memory key.
#define SHMKEY 383

// Shared memory format.
struct SHMEM
{
    MonkeyAndBananas::DISPLAY_TYPE displayControl;
};
#endif

// Evolution generations.
int Generations;

// Population file names.
char *InputFileName;
char *OutputFileName;

// Scramble boxes and monkey locations.
bool Scramble;

// Random numbers.
RANDOM RandomSeed;
Random randomizer;

// Display mode.
MonkeyAndBananas::DISPLAY_TYPE Display;

// Population member.
class Member
{
    public:

        MonkeyInstincts *instincts;
        Mona *brain;
        double fitness;
        int age;

        // Constructors.
        Member(RANDOM randomSeed)
        {
            instincts = new MonkeyInstincts(randomSeed);
            assert(instincts != NULL);
            brain = NULL;
            fitness = 0.0;
            age = 0;
        }
        Member()
        {
            instincts = NULL;
            brain = NULL;
            fitness = 0.0;
            age = 0;
        }
        Member(MonkeyInstincts *instincts, double fitness, int age)
        {
            this->instincts = instincts;
            brain = NULL;
            this->fitness = fitness;
            this->age = age;
        }

        // Destructor.
        ~Member()
        {
            if (instincts != NULL) delete instincts;
            if (brain != NULL) delete brain;
        }

        // Evaluate.
        void evaluate()
        {
            int i,x,y,wx,wy,bx,mx,location,b0,b1,b2,steps;
            MonkeyAndBananas::DIRECTION direction;
            char **world;
            MonkeyAndBananas *monkey;

            // Determine monkey and box locations.
            for (i = 0; MonkeyAndBananas::DEFAULT_WORLD[i] != NULL; i++) {}
            wy = i;
            wx = strlen(MonkeyAndBananas::DEFAULT_WORLD[0]);
            world = new char*[i + 1];
            assert(world != NULL);
            for (i = 0; MonkeyAndBananas::DEFAULT_WORLD[i] != NULL; i++)
            {
                world[i] = new char[strlen(MonkeyAndBananas::DEFAULT_WORLD[i]) + 1];
                assert(world[i] != NULL);
                strcpy(world[i], MonkeyAndBananas::DEFAULT_WORLD[i]);
                assert(i == 0 || strlen(world[i - 1]) == strlen(world[i]));
            }
            world[i] = NULL;
            if (Scramble)
            {
                for (y = 0; y < wy; y++)
                {
                    for (x = 0; x < wx; x++)
                    {
                        if (world[y][x] == MonkeyAndBananas::MONKEY ||
                            world[y][x] == MonkeyAndBananas::BOX)
                        {
                            world[y][x] = MonkeyAndBananas::AIR;
                        }
                    }
                }
                for (bx = 0; bx < wx && world[wy - 1][bx] != MonkeyAndBananas::FLOOR; bx++) {}
                mx = bx + 1;
                bx += 4;
                b0 = randomizer.RAND_CHOICE(wx - bx) + bx;
                world[wy - 2][b0] = MonkeyAndBananas::BOX;
            #ifdef ONE_BOX_WORLD
                for (i = 0; i < 1000; i++)
                {
                    location = randomizer.RAND_CHOICE(wx - mx) + mx;
                    if (location == b0) continue;
                    break;
                }
                assert(i < 1000);
            #else
                for (i = 0; i < 1000; i++)
                {
                    b1 = randomizer.RAND_CHOICE(wx - bx) + bx;
                    if (b1 == b0) continue;
                    break;
                }
                assert(i < 1000);
                world[wy - 2][b1] = MonkeyAndBananas::BOX;
                for (i = 0; i < 1000; i++)
                {
                    b2 = randomizer.RAND_CHOICE(wx - bx) + bx;
                    if (b2 == b0) continue;
                    if (b2 == b1) continue;
                    break;
                }
                assert(i < 1000);
                world[wy - 2][b2] = MonkeyAndBananas::BOX;
                for (i = 0; i < 1000; i++)
                {
                    location = randomizer.RAND_CHOICE(wx - bx) + bx;
                    if (location == b0) continue;
                    if (location == b1) continue;
                    if (location ==  b2) continue;
                    break;
                }
                assert(i < 1000);
            #endif
                if (randomizer.RAND_BOOL())
                {
                    direction = MonkeyAndBananas::LOOK_LEFT;
                }
                else
                {
                    direction = MonkeyAndBananas::LOOK_RIGHT;
                }
            }
            else
            {
                location = MonkeyAndBananas::DEFAULT_MONKEY_LOCATION;
                direction = MonkeyAndBananas::DEFAULT_MONKEY_DIRECTION;
            }

            // Run a monkey with instincts.
            assert(instincts != NULL);
            if (LearningType == INSTINCT_AND_AND_RETAIN_EXPERIENCE)
            {
                if (brain == NULL)
                {
                    monkey = new MonkeyAndBananas(world,
                        location, direction, Display, instincts);
                }
                else
                {
                    brain->clearWorkingMemory();
                    monkey = new MonkeyAndBananas(world,
                        location, direction, Display, instincts, brain);
                }
            }
            else
            {
                monkey = new MonkeyAndBananas(world,
                    location, direction, Display, instincts);
            }
            assert(monkey != NULL);

            if (LearningType == INSTINCT_ONLY)
            {
                monkey->brain->MAX_MEDIATORS = instincts->instincts.size();
            }
            else
            {
                monkey->brain->MAX_MEDIATORS = MAXIMUM_MEDIATORS;
            }
            steps = monkey->run(MONKEY_STEPS);
            fitness = monkey->evaluate(steps);
            age++;
            if (LearningType == INSTINCT_AND_AND_RETAIN_EXPERIENCE)
            {
                brain = monkey->brain;
                monkey->brain = NULL;
            }
            else
            {
                instincts->clearMediators();
            }
            delete monkey;
            for (i = 0; world[i] != NULL; i++)
            {
                delete world[i];
            }
            delete world;
        }

        // Clone.
        Member *clone()
        {
            Member *member;

            member = new Member();
            assert(member != NULL);
            member->instincts = instincts->clone();
            member->brain = NULL;
            member->fitness = fitness;
            member->age = age;
            return member;
        }

        // Load.
        void load(FILE *fp)
        {
            instincts = new MonkeyInstincts(randomizer.RAND());
            assert(instincts != NULL);
            instincts->load(fp);
            FREAD_DOUBLE(&fitness, fp);
            FREAD_INT(&age, fp);
        }

        // Save.
        void save(FILE *fp)
        {
            instincts->save(fp);
            FWRITE_DOUBLE(&fitness, fp);
            FWRITE_INT(&age, fp);
        }

        // Load brain.
        void loadBrain(FILE *fp)
        {
            brain = Mona::load(fp);
        }

        // Save brain.
        void saveBrain(FILE *fp)
        {
            if (brain != NULL) brain->save(fp);
        }

        // Print.
        void print()
        {
            instincts->print();
            printf("fitness=%f age=%d\n", fitness, age);
        }
};

// Population.
Member *Population[POPULATION_SIZE];

// Start/end functions.
void logParameters();
void loadBodies(char *fileName);
void loadPopulation(char *fileName);
void savePopulation(char *fileName);
void terminate(int);

// Evolve functions.
void evolve(), evaluate(), prune(), mutate(), mate();

int
main(int argc, char *argv[])
{
    int i;
    bool learningOpt;

    // Logging.
    Log::LOGGING_FLAG = EVOLVE_LOGGING;
    Log::setLogFileName(DEFAULT_EVOLVE_LOG_FILE_NAME);

    // Parse arguments.
    learningOpt = false;
    LearningType = INSTINCT_ONLY;
    Generations = -1;
    InputFileName = OutputFileName = NULL;
    Scramble = false;
    RandomSeed = INVALID_RANDOM;
    Display = MonkeyAndBananas::NO_DISPLAY;

    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-learning") == 0)
        {
            learningOpt = true;
            i++;
            if (i >= argc)
            {
                logUsage();
                exit(1);
            }
            if (strcmp(argv[i], "instinct_only") == 0)
            {
                LearningType = INSTINCT_ONLY;
            } else if (strcmp(argv[i], "experience_only") == 0)
            {
                LearningType = EXPERIENCE_ONLY;
            } else if (strcmp(argv[i], "instinct_and_experience") == 0)
            {
                LearningType = INSTINCT_AND_EXPERIENCE;
            } else if (strcmp(argv[i], "instinct_and_retain_experience") == 0)
            {
                LearningType = INSTINCT_AND_AND_RETAIN_EXPERIENCE;
            }
            else
            {
                logUsage();
                exit(1);
            }
            continue;
        }

        if (strcmp(argv[i], "-generations") == 0)
        {
            i++;
            if (i >= argc)
            {
                logUsage();
                exit(1);
            }
            Generations = atoi(argv[i]);
            if (Generations < 0)
            {
                logUsage();
                exit(1);
            }
            continue;
        }

        if (strcmp(argv[i], "-input") == 0)
        {
            i++;
            if (i >= argc)
            {
                logUsage();
                exit(1);
            }
            InputFileName = argv[i];
            continue;
        }

        if (strcmp(argv[i], "-output") == 0)
        {
            i++;
            if (i >= argc)
            {
                logUsage();
                exit(1);
            }
            OutputFileName = argv[i];
            continue;
        }

        if (strcmp(argv[i], "-scramble") == 0)
        {
            Scramble = true;
            continue;
        }

        if (strcmp(argv[i], "-randomSeed") == 0)
        {
            i++;
            if (i >= argc)
            {
                logUsage();
                exit(1);
            }
            RandomSeed = atoi(argv[i]);
            continue;
        }

        if (strcmp(argv[i], "-logfile") == 0)
        {
            i++;
            if (i >= argc)
            {
                logUsage();
                exit(1);
            }
            Log::setLogFileName(argv[i]);
            continue;
        }

        if (strcmp(argv[i], "-display") == 0)
        {
            i++;
            if (i >= argc)
            {
                logUsage();
                exit(1);
            }
            if (strcmp(argv[i], "text") == 0)
            {
                Display = MonkeyAndBananas::TEXT_DISPLAY;
            } else if (strcmp(argv[i], "graphics") == 0)
            {
                Display = MonkeyAndBananas::GRAPHICS_DISPLAY;
            }
            else
            {
                logUsage();
                exit(1);
            }
            continue;
        }

        logUsage();
        exit(1);
    }

    if (!learningOpt)
    {
        logUsage();
        exit(1);
    }

    // If experience-only learning, prevent instincts.
    if (LearningType == EXPERIENCE_ONLY)
    {
        MonkeyInstincts::MAX_INSTINCTS = 0;
        MonkeyInstincts::MIN_INSTINCTS = 0;
    }

    if (OutputFileName == NULL)
    {
        logUsage();
        exit(1);
    }

    // Seed random numbers.
    if (RandomSeed == INVALID_RANDOM)
    {
        RandomSeed = (RANDOM)time(NULL);
    }
    randomizer.SRAND(RandomSeed);

    Log::logInformation("Initializing evolve:");
    switch(LearningType)
    {
        case INSTINCT_ONLY:
            sprintf(Log::messageBuf, "learning type=instinct_only");
            break;
        case EXPERIENCE_ONLY:
            sprintf(Log::messageBuf, "learning type=experience_only");
            break;
        case INSTINCT_AND_EXPERIENCE:
            sprintf(Log::messageBuf, "learning type=instinct_and_experience");
            break;
        case INSTINCT_AND_AND_RETAIN_EXPERIENCE:
            sprintf(Log::messageBuf, "learning type=instinct_and_retain_experience");
            break;
    }
    Log::logInformation();
    sprintf(Log::messageBuf, "generations=%d", Generations);
    Log::logInformation();
    if (InputFileName != NULL)
    {
        sprintf(Log::messageBuf, "input=%s", InputFileName);
        Log::logInformation();
    }
    sprintf(Log::messageBuf, "output=%s", OutputFileName);
    Log::logInformation();
    if (Scramble)
    {
        sprintf(Log::messageBuf, "scramble=true");
    }
    else
    {
        sprintf(Log::messageBuf, "scramble=false");
    }
    Log::logInformation();
    sprintf(Log::messageBuf, "random seed=%d", RandomSeed);
    Log::logInformation();

    // Log run parameters.
    logParameters();

    // Load population.
    if (InputFileName == NULL)
    {
        for (i = 0; i < POPULATION_SIZE; i++)
        {
            Population[i] = new Member(randomizer.RAND());
            assert(Population[i] != NULL);
        }
    }

    else
    {
        // Continue run.
        loadPopulation(InputFileName);
    }

    #ifdef UNIX
    // Setup for remote display control.
    key_t shmkey;
    int shmid;
    struct SHMEM *shmem;

    // Create the shared memory segment.
    shmkey = SHMKEY;
    if ((shmid = shmget(shmkey, sizeof(struct SHMEM), 0)) == -1)
    {
        // Shared memory does not exist - create it.
        if ((shmid = shmget(shmkey, sizeof(struct SHMEM), 0777|IPC_CREAT)) == -1)
        {
            sprintf(Log::messageBuf, "attach: shmget failed, errno=%d\n",errno);
            Log::logError();
        }
    }

    // Store the current display mode.
    shmem = (struct SHMEM *)shmat(shmid, 0, 0);
    shmem->displayControl = Display;
    #endif

    // Evolution loop.
    Log::logInformation("Begin evolve:");
    for (i = 0; i < Generations; i++)
    {
        #ifdef UNIX
        // Set display using remote control.
        if (shmid != -1) Display = shmem->displayControl;
        #endif

        sprintf(Log::messageBuf, "Generation=%d", i);
        Log::logInformation();
        evolve();

        // Save population.
        savePopulation(OutputFileName);
    }

    // Save population.
    savePopulation(OutputFileName);

    // Release memory.
    for (i = 0; i < POPULATION_SIZE; i++)
    {
        if (Population[i] != NULL)
        {
            delete Population[i];
            Population[i] = NULL;
        }
    }

    randomizer.RAND_CLEAR();

    #ifdef WIN32
    #if ( CHECK_MEMORY == 1 )
    // Check for memory leaks.
    HANDLE hFile = CreateFile(                    // Dump to temp log.
        TEMP_LOG_FILE_NAME,
        GENERIC_WRITE,
        FILE_SHARE_WRITE,
        NULL,
        OPEN_ALWAYS,
        0,
        NULL
        );
    if (hFile == INVALID_HANDLE_VALUE)
    {
        sprintf(Log::messageBuf, "Cannot open memory check temporary file %s",
            TEMP_LOG_FILE_NAME);
        Log::logError();
        exit(1);
    }

    Log::logInformation("\nMemory leak check output:");
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, hFile);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, hFile );
    if (!_CrtDumpMemoryLeaks())
    {
        Log::logInformation("No memory leaks");
        CloseHandle(hFile);
    }

    else
    {
        CloseHandle(hFile);
        Log::appendTempLog();
    }

    Log::removeTempLog();
    #endif
    #endif

    // Close log.
    Log::logInformation("End evolve");
    Log::close();

    return 0;
}


// Log run parameters.
void logParameters()
{
    Log::logInformation("Evolve Parameters:");
    sprintf(Log::messageBuf, "RandomSeed = %d", RandomSeed);
    Log::logInformation();
    sprintf(Log::messageBuf, "FIT_POPULATION_SIZE = %d", FIT_POPULATION_SIZE);
    Log::logInformation();
    sprintf(Log::messageBuf, "NUM_MUTANTS = %d", NUM_MUTANTS);
    Log::logInformation();
    sprintf(Log::messageBuf, "NUM_OFFSPRING = %d", NUM_OFFSPRING);
    Log::logInformation();
    sprintf(Log::messageBuf, "MUTATION_RATE = %f", MUTATION_RATE);
    Log::logInformation();
    sprintf(Log::messageBuf, "MONKEY_STEPS = %d", MONKEY_STEPS);
    Log::logInformation();
    sprintf(Log::messageBuf, "MAXIMUM_MEDIATORS = %d", MAXIMUM_MEDIATORS);
    Log::logInformation();

    Log::logInformation("Instinct Parameters:");
    sprintf(Log::messageBuf, "MAX_INSTINCTS = %d", MonkeyInstincts::MAX_INSTINCTS);
    Log::logInformation();
    sprintf(Log::messageBuf, "MIN_INSTINCTS = %d", MonkeyInstincts::MIN_INSTINCTS);
    Log::logInformation();
    sprintf(Log::messageBuf, "MAX_INSTINCT_EVENTS = %d", MonkeyInstincts::MAX_INSTINCT_EVENTS);
    Log::logInformation();
    sprintf(Log::messageBuf, "MIN_INSTINCT_EVENTS = %d", MonkeyInstincts::MIN_INSTINCT_EVENTS);
    Log::logInformation();
    sprintf(Log::messageBuf, "NUM_INSTINCT_NEEDS = %d", MonkeyInstincts::NUM_INSTINCT_NEEDS);
    Log::logInformation();
    sprintf(Log::messageBuf, "MAX_NEED_VALUE = %f", MonkeyInstincts::MAX_NEED_VALUE);
    Log::logInformation();
    sprintf(Log::messageBuf, "MIN_NEED_VALUE = %f", MonkeyInstincts::MIN_NEED_VALUE);
    Log::logInformation();
    sprintf(Log::messageBuf, "MAX_NEED_FREQ = %d", MonkeyInstincts::MAX_NEED_FREQ);
    Log::logInformation();
    sprintf(Log::messageBuf, "MIN_NEED_FREQ = %d", MonkeyInstincts::MIN_NEED_FREQ);
    Log::logInformation();
    sprintf(Log::messageBuf, "MAX_NEED_DURATION = %d", MonkeyInstincts::MAX_NEED_DURATION);
    Log::logInformation();
    sprintf(Log::messageBuf, "MIN_NEED_DURATION = %d", MonkeyInstincts::MIN_NEED_DURATION);
    Log::logInformation();
}


// Load evolution population.
void loadPopulation(char *fileName)
{
    FILE *fp;

    if ((fp = fopen(fileName, "r")) == NULL)
    {
        sprintf(Log::messageBuf, "Cannot load population file %s", fileName);
        Log::logError();
        exit(1);
    }
    for (int i = 0; i < POPULATION_SIZE; i++)
    {
        Population[i] = new Member();
        assert(Population[i] != NULL);
        Population[i]->load(fp);
    }
    if (LearningType == INSTINCT_AND_AND_RETAIN_EXPERIENCE)
    {
        for (int i = 0; i < POPULATION_SIZE; i++)
        {
            Population[i]->loadBrain(fp);
        }
    }
    fclose(fp);
}


// Save evolution population.
void savePopulation(char *fileName)
{
    FILE *fp;

    if ((fp = fopen(fileName, "w")) == NULL)
    {
        sprintf(Log::messageBuf, "Cannot save to population file %s", fileName);
        Log::logError();
        exit(1);
    }
    for (int i = 0; i < POPULATION_SIZE; i++)
    {
        Population[i]->save(fp);
    }
    if (LearningType == INSTINCT_AND_AND_RETAIN_EXPERIENCE)
    {
        for (int i = 0; i < POPULATION_SIZE; i++)
        {
            Population[i]->saveBrain(fp);
        }
    }
    fclose(fp);
}


// Evolution generation.
void evolve()
{
    // Evaluate member fitness.
    evaluate();

    // Prune unfit members.
    prune();

    // Create new members by mutation.
    mutate();

    // Create new members by mating.
    mate();
}


// Evaluate member fitnesses.
void evaluate()
{
    Log::logInformation("Evaluate:");

    for (int i = 0; i < POPULATION_SIZE; i++)
    {
        Population[i]->evaluate();
        sprintf(Log::messageBuf, "  Member=%d, Fitness=%f, Age=%d",
            i, Population[i]->fitness, Population[i]->age);
        Log::logInformation();
    }
}


// Prune unfit members.
void prune()
{
    double max;
    int i,j,m;
    Member *member;
    Member *fitPopulation[FIT_POPULATION_SIZE];

    Log::logInformation("Select:");
    for (i = 0; i < FIT_POPULATION_SIZE; i++)
    {
        m = -1;
        max = 0.0;
        for (j = 0; j < POPULATION_SIZE; j++)
        {
            member = Population[j];
            if (member == NULL) continue;
            if (m == -1 || member->fitness > max)
            {
                m = j;
                max = member->fitness;
            }
        }
        member = Population[m];
        Population[m] = NULL;
        fitPopulation[i] = member;
        sprintf(Log::messageBuf, "  Fitness=%f, Age=%d",
            member->fitness, member->age);
        Log::logInformation();
    }
    for (i = 0; i < POPULATION_SIZE; i++)
    {
        if (Population[i] != NULL)
        {
            delete Population[i];
            Population[i] = NULL;
        }
    }
    for (i = 0; i < FIT_POPULATION_SIZE; i++)
    {
        Population[i] = fitPopulation[i];
    }
}


// Mutate members.
void mutate()
{
    int i,j,k;
    Member *member,*mutant;
    vector<Instinct *> instinctsWork;

    Log::logInformation("Mutate:");
    for (i = 0; i < NUM_MUTANTS; i++)
    {
        // Select a fit member to mutate.
        j = randomizer.RAND_CHOICE(FIT_POPULATION_SIZE);
        member = Population[j];
        sprintf(Log::messageBuf, "  Member=%d", j);
        Log::logInformation();

        // Create mutant member.
        mutant = new Member(randomizer.RAND());
        assert(mutant != NULL);
        Population[FIT_POPULATION_SIZE + i] = mutant;

        // Mutate.
        instinctsWork.clear();
        for (j = 0; j < MonkeyInstincts::MAX_INSTINCTS; j++)
        {
            if (randomizer.RAND_CHANCE(MUTATION_RATE))
            {
                if (j < mutant->instincts->instincts.size())
                {
                    instinctsWork.push_back(mutant->instincts->instincts[j]->clone());
                }
            }
            else
            {
                if (j < member->instincts->instincts.size())
                {
                    instinctsWork.push_back(member->instincts->instincts[j]->clone());
                }
            }
        }
        for (j = 0, k = mutant->instincts->instincts.size(); j < k; j++)
        {
            delete mutant->instincts->instincts[j];
        }
        mutant->instincts->instincts.clear();
        for (j = 0, k = instinctsWork.size(); j < k; j++)
        {
            mutant->instincts->instincts.push_back(instinctsWork[j]);
        }
    }
}


// Produce offspring though matings.
void mate()
{
    int i,j,k;
    Member *member1,*member2,*offspring;
    vector<Instinct *> instinctsWork;

    Log::logInformation("Mate:");
    if (FIT_POPULATION_SIZE < 2) return;
    for (i = 0; i < NUM_OFFSPRING; i++)
    {
        // Select a pair of fit members to mate.
        j = randomizer.RAND_CHOICE(FIT_POPULATION_SIZE);
        member1 = Population[j];
        while ((k = randomizer.RAND_CHOICE(FIT_POPULATION_SIZE)) == j);
        member2 = Population[k];
        sprintf(Log::messageBuf, "  Members=%d,%d", j, k);
        Log::logInformation();

        // Create offspring.
        offspring = new Member(randomizer.RAND());
        assert(offspring != NULL);
        Population[FIT_POPULATION_SIZE + NUM_MUTANTS + i] = offspring;

        instinctsWork.clear();
        for (j = 0; j < MonkeyInstincts::MAX_INSTINCTS; j++)
        {
            if (randomizer.RAND_BOOL())
            {
                if (j < member1->instincts->instincts.size())
                {
                    instinctsWork.push_back(member1->instincts->instincts[j]->clone());
                }
            }
            else
            {
                if (j < member2->instincts->instincts.size())
                {
                    instinctsWork.push_back(member2->instincts->instincts[j]->clone());
                }
            }
        }
        for (j = 0, k = offspring->instincts->instincts.size(); j < k; j++)
        {
            delete offspring->instincts->instincts[j];
        }
        offspring->instincts->instincts.clear();
        for (j = 0, k = instinctsWork.size(); j < k; j++)
        {
            offspring->instincts->instincts.push_back(instinctsWork[j]);
        }
    }
}
