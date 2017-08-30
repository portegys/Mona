// For conditions of distribution and use, see copyright notice in mona.hpp

#ifdef WIN32
#include <windows.h>
#include "mona_cs_dll.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
   switch (reason)
   {
   case DLL_PROCESS_ATTACH:
   case DLL_THREAD_ATTACH:
   case DLL_THREAD_DETACH:
   case DLL_PROCESS_DETACH:
      break;
   }
   return(TRUE);
}


#ifdef _MANAGED
#pragma managed(pop)
#endif

extern "C" __declspec(dllexport) Mona * createMona(int numSensors,
                                                   int numResponses,
                                                   int numNeeds, int randomSeed)
{
   return(new Mona(numSensors, numResponses,
                   numNeeds, (RANDOM)randomSeed));
}

extern "C" __declspec(dllexport) void setSensorResolution(Mona *mona, float sensorResolution)
{
   if (mona != NULL)
   {
      mona->SENSOR_RESOLUTION = sensorResolution;
   }
}


extern "C" __declspec(dllexport) int addSensorMode(Mona *mona, int *sensorMask)
{
   if (mona != NULL)
   {
      vector<bool> mask;
      for (int i = 0; i < mona->numSensors; i++)
      {
         if (sensorMask[i] == 0)
         {
            mask.push_back(false);
         }
         else
         {
            mask.push_back(true);
         }
      }
      return(mona->addSensorMode(mask));
   }
   else
   {
      return(-1);
   }
}


extern "C" __declspec(dllexport) int addSensorModeWithResolution(Mona *mona,
                                                                 int *sensorMask, float sensorResolution)
{
   if (mona != NULL)
   {
      vector<bool> mask;
      for (int i = 0; i < mona->numSensors; i++)
      {
         if (sensorMask[i] == 0)
         {
            mask.push_back(false);
         }
         else
         {
            mask.push_back(true);
         }
      }
      return(mona->addSensorMode(mask, sensorResolution));
   }
   else
   {
      return(-1);
   }
}


extern "C" __declspec(dllexport) void disposeMona(Mona *mona)
{
   if (mona != NULL)
   {
      delete mona;
      mona = NULL;
   }
}


extern "C" __declspec(dllexport) int cycle(Mona *mona, float *sensors)
{
   int i;

   if (mona != NULL)
   {
      vector<float> s;
      for (i = 0; i < mona->numSensors; i++)
      {
         s.push_back(sensors[i]);
      }
      return(mona->cycle(s));
   }
   else
   {
      return(0);
   }
}


extern "C" __declspec(dllexport) int addResponse(Mona *mona)
{
   if (mona != NULL)
   {
      return(mona->addResponse());
   }
   else
   {
      return(-1);
   }
}


extern "C" __declspec(dllexport) double getResponsePotential(Mona *mona, int response)
{
   if (mona != NULL)
   {
      return(mona->getResponsePotential(response));
   }
   else
   {
      return(0.0);
   }
}


extern "C" __declspec(dllexport) void overrideResponse(Mona *mona, int response)
{
   if (mona != NULL)
   {
      mona->overrideResponse(response);
   }
}


extern "C" __declspec(dllexport) void clearResponseOverride(Mona *mona)
{
   if (mona != NULL)
   {
      mona->clearResponseOverride();
   }
}


extern "C" __declspec(dllexport) double getNeed(Mona *mona, int index)
{
   if (mona != NULL)
   {
      return(mona->getNeed(index));
   }
   else
   {
      return(0.0);
   }
}


extern "C" __declspec(dllexport) void setNeed(Mona *mona, int index, double value)
{
   if (mona != NULL)
   {
      mona->setNeed(index, value);
   }
}


extern __declspec(dllexport) void setPeriodicNeed(Mona *mona, int index,
                                                  int frequency, double periodicNeed)
{
   if (mona != NULL)
   {
      mona->setPeriodicNeed(index, frequency, periodicNeed);
   }
}


extern "C" __declspec(dllexport) int addGoalWithResponse(Mona *mona, int needIndex,
                                                         float *sensors, int sensorMode,
                                                         int response, double goalValue)
{
   if (mona != NULL)
   {
      vector<Mona::SENSOR> s;
      for (int i = 0; i < mona->numSensors; i++)
      {
         s.push_back(sensors[i]);
      }
      return(mona->addGoal(needIndex, s, sensorMode, response, goalValue));
   }
   else
   {
      return(-1);
   }
}


extern "C" __declspec(dllexport) int addGoal(Mona *mona, int needIndex,
                                             float *sensors, int sensorMode,
                                             double goalValue)
{
   if (mona != NULL)
   {
      vector<Mona::SENSOR> s;
      for (int i = 0; i < mona->numSensors; i++)
      {
         s.push_back(sensors[i]);
      }
      return(mona->addGoal(needIndex, s, sensorMode, goalValue));
   }
   else
   {
      return(-1);
   }
}


extern "C" __declspec(dllexport) int findGoalWithResponse(Mona *mona, int needIndex,
                                                          float *sensors, int sensorMode,
                                                          int response)
{
   int i;

   if (mona != NULL)
   {
      vector<Mona::SENSOR> s;
      for (i = 0; i < mona->numSensors; i++)
      {
         s.push_back(sensors[i]);
      }
      return(mona->findGoal(needIndex, s, sensorMode, response));
   }
   else
   {
      return(-1);
   }
}


extern "C" __declspec(dllexport) int findGoal(Mona *mona, int needIndex,
                                              float *sensors, int sensorMode)
{
   if (mona != NULL)
   {
      vector<Mona::SENSOR> s;
      for (int i = 0; i < mona->numSensors; i++)
      {
         s.push_back(sensors[i]);
      }
      return(mona->findGoal(needIndex, s, sensorMode));
   }
   else
   {
      return(-1);
   }
}


extern "C" __declspec(dllexport) int getNumGoals(Mona *mona, int needIndex)
{
   if (mona != NULL)
   {
      return(mona->getNumGoals(needIndex));
   }
   else
   {
      return(-1);
   }
}


extern "C" __declspec(dllexport) bool isGoalEnabled(Mona *mona,
                                                    int needIndex, int goalIndex)
{
   if (mona != NULL)
   {
      return(mona->isGoalEnabled(needIndex, goalIndex));
   }
   else
   {
      return(false);
   }
}


extern "C" __declspec(dllexport) bool enableGoal(Mona *mona,
                                                 int needIndex, int goalIndex)
{
   if (mona != NULL)
   {
      return(mona->enableGoal(needIndex, goalIndex));
   }
   else
   {
      return(false);
   }
}


extern "C" __declspec(dllexport) bool disableGoal(Mona *mona,
                                                  int needIndex, int goalIndex)
{
   if (mona != NULL)
   {
      return(mona->disableGoal(needIndex, goalIndex));
   }
   else
   {
      return(false);
   }
}


extern "C" __declspec(dllexport) bool removeGoal(Mona *mona,
                                                 int needIndex, int goalIndex)
{
   if (mona != NULL)
   {
      return(mona->removeGoal(needIndex, goalIndex));
   }
   else
   {
      return(false);
   }
}


extern "C" __declspec(dllexport) bool load(Mona *mona, char *filename)
{
   if (mona != NULL)
   {
      return(mona->load(filename));
   }
   else
   {
      return(false);
   }
}


extern "C" __declspec(dllexport) bool save(Mona *mona, char *filename)
{
   if (mona != NULL)
   {
      return(mona->save(filename));
   }
   else
   {
      return(false);
   }
}


extern "C" __declspec(dllexport) void clearWorkingMemory(Mona *mona)
{
   if (mona != NULL)
   {
      mona->clearWorkingMemory();
   }
}


extern "C" __declspec(dllexport) void clearLongTernMemory(Mona *mona)
{
   if (mona != NULL)
   {
      mona->clearLongTermMemory();
   }
}


extern "C" __declspec(dllexport) void clear(Mona *mona)
{
   if (mona != NULL)
   {
      mona->clear();
   }
}


extern "C" __declspec(dllexport) bool print(Mona *mona, char *filename)
{
   if (mona == NULL)
   {
      return(false);
   }
   if (filename == NULL)
   {
      mona->print();
   }
   else
   {
      FILE *fp = fopen(filename, "w");
      if (fp == NULL)
      {
         return(false);
      }
      mona->print(fp);
      fclose(fp);
   }
   return(true);
}


#endif
