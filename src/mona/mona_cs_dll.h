// Mona DLL exported interface for use with external mona object.

#ifdef WIN32
#ifndef __MONA_CS__
#define __MONA_CS__

#include "mona.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

extern __declspec(dllexport) Mona * createMona(
   int numSensors, int numResponses, int numNeeds, int randomSeed);

extern __declspec(dllexport) void setSensorResolution(Mona *mona, float sensorResolution);

extern __declspec(dllexport) int addSensorMode(Mona *mona, int *sensorMask);

extern __declspec(dllexport) int addSensorModeWithResolution(Mona *mona, int *sensorMask,
                                                             float sensorResolution);

extern __declspec(dllexport) void disposeMona(Mona *mona);

extern __declspec(dllexport) int cycle(Mona *mona, float *sensors);

extern __declspec(dllexport) int addResponse(Mona *mona);

extern __declspec(dllexport) double getResponsePotential(Mona *mona, int response);

extern __declspec(dllexport) void overrideResponse(Mona *mona, int response);

extern __declspec(dllexport) void clearResponseOverride(Mona *mona);

extern __declspec(dllexport) double getNeed(Mona *mona, int index);

extern __declspec(dllexport) void setNeed(Mona *mona, int index, double value);

extern __declspec(dllexport) void setPeriodicNeed(Mona *mona, int index,
                                                  int frequency, double periodicNeed);

extern __declspec(dllexport) int addGoalWithResponse(Mona *mona, int needIndex,
                                                     float *sensors, int sensorMode,
                                                     int response, double goalValue);

extern __declspec(dllexport) int addGoal(Mona *mona, int needIndex,
                                         float *sensors, int sensorMode,
                                         double goalValue);

extern __declspec(dllexport) int findGoalWithResponse(Mona *mona, int needIndex,
                                                      float *sensors, int sensorMode, int response);

extern __declspec(dllexport) int findGoal(Mona *mona, int needIndex,
                                          float *sensors, int sensorMode);

extern __declspec(dllexport) int getNumGoals(Mona *mona, int needIndex);

extern __declspec(dllexport) bool isGoalEnabled(Mona *mona,
                                                int needIndex, int goalIndex);

extern __declspec(dllexport) bool enableGoal(Mona *mona,
                                             int needIndex, int goalIndex);

extern __declspec(dllexport) bool disableGoal(Mona *mona,
                                              int needIndex, int goalIndex);

extern __declspec(dllexport) bool removeGoal(Mona *mona,
                                             int needIndex, int goalIndex);

extern __declspec(dllexport) bool load(Mona *mona, char *filename);

extern __declspec(dllexport) bool save(Mona *mona, char *filename);

extern __declspec(dllexport) void clearWorkingMemory(Mona *mona);

extern __declspec(dllexport) void clearLongTermMemory(Mona *mona);

extern __declspec(dllexport) void clear(Mona *mona);

extern __declspec(dllexport) bool print(Mona *mona, char *filename);

#ifdef __cplusplus
}
#endif
#endif
#endif
