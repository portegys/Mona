// Mona C# class.
// For conditions of distribution and use, see copyright notice in mona.hpp

using System;
using System.Runtime.InteropServices;

public class Mona : IDisposable
{
   // Constructors.
   public Mona(int numSensors, int numResponses,
               int numNeeds, int randomSeed)
   {
      mona = createMona(numSensors, numResponses,
                        numNeeds, randomSeed);
   }


   // Set sensor resolution.
   public void setSensorResolution(float sensorResolution)
   {
      setSensorResolution(mona, sensorResolution);
   }


   // Add sensor mode.
   public int addSensorMode(bool[] sensorMask)
   {
      int[] mask = new int[sensorMask.Length];
      for (int i = 0; i < mask.Length; i++)
      {
         if (sensorMask[i] == true)
         {
            mask[i] = 1;
         }
         else
         {
            mask[i] = 0;
         }
      }
      return(addSensorMode(mona, mask));
   }


   public int addSensorMode(bool[] sensorMask, float sensorResolution)
   {
      int[] mask = new int[sensorMask.Length];
      for (int i = 0; i < mask.Length; i++)
      {
         if (sensorMask[i] == true)
         {
            mask[i] = 1;
         }
         else
         {
            mask[i] = 0;
         }
      }
      return(addSensorMode(mona, mask, sensorResolution));
   }


   public void Dispose()
   {
      Dispose(true);
   }


   protected virtual void Dispose(bool disposing)
   {
      if (mona != IntPtr.Zero)
      {
         // Call the DLL export to dispose the object.
         disposeMona(mona);
         mona = IntPtr.Zero;
      }
      if (disposing)
      {
         // No need to call the finalizer since the
         // unmanaged memory has been released.
         GC.SuppressFinalize(this);
      }
   }


   // This finalizer is called when garbage collection occurs, but only if
   // the IDisposable.Dispose method was not already called.
   ~Mona()
   {
      Dispose(false);
   }


   // Sensory-response cycle.
   public int cycle(float[] sensors)
   {
      return(cycle(mona, sensors));
   }


   // Add response.
   public int addResponse()
   {
      return(addResponse(mona));
   }


   // Add response.
   public double getResponsePotential(int response)
   {
      return(getResponsePotential(mona, response));
   }


   // Override response.
   public void overrideResponse(int response)
   {
      overrideResponse(mona, response);
   }


   public void clearResponseOverride()
   {
      clearResponseOverride(mona);
   }


   // Get need.
   public double getNeed(int needIndex)
   {
      return(getNeed(mona, needIndex));
   }


   // Set need.
   public void setNeed(int needIndex, double value)
   {
      setNeed(mona, needIndex, value);
   }


   // Set periodic need.
   public void setPeriodicNeed(int needIndex, int frequency, double periodicNeed)
   {
      setPeriodicNeed(mona, needIndex, frequency, periodicNeed);
   }


   // Add goal.
   public int addGoal(int needIndex, float[] sensors,
                      int sensorMode, int response, double goalValue)
   {
      return(addGoalWithResponse(mona, needIndex, sensors, sensorMode,
                                 response, goalValue));
   }


   public int addGoal(int needIndex, float[] sensors,
                      int sensorMode, double goalValue)
   {
      return(addGoal(mona, needIndex, sensors, sensorMode, goalValue));
   }


   // Find goal.
   public int findGoal(int needIndex, float[] sensors,
                       int sensorMode, int response)
   {
      return(findGoalWithResponse(mona, needIndex, sensors, sensorMode, response));
   }


   public int findGoal(int needIndex, float[] sensors, int sensorMode)
   {
      return(findGoal(mona, needIndex, sensors, sensorMode));
   }


   // Get number of goals.
   public int getNumGoals(int needIndex)
   {
      return(getNumGoals(mona, needIndex));
   }


   // Is goal enabled?
   public bool isGoalEnabled(int needIndex, int goalIndex)
   {
      return(isGoalEnabled(mona, needIndex, goalIndex));
   }


   // Enable goal
   public bool enableGoal(int needIndex, int goalIndex)
   {
      return(enableGoal(mona, needIndex, goalIndex));
   }


   // Disable goal
   public bool disableGoal(int needIndex, int goalIndex)
   {
      return(disableGoal(mona, needIndex, goalIndex));
   }


   // Remove goal
   public bool removeGoal(int needIndex, int goalIndex)
   {
      return(removeGoal(mona, needIndex, goalIndex));
   }


   // Load.
   public bool load(string filename)
   {
      return(load(mona, filename));
   }


   // Save.
   public bool save(string filename)
   {
      return(save(mona, filename));
   }


   // Clear working memory.
   public void clearWorkingMemory()
   {
      clearWorkingMemory(mona);
   }


   // Clear.
   public void clear()
   {
      clear(mona);
   }


   // Print.
   public bool print()
   {
      return(print(mona, null));
   }


   public bool print(string filename)
   {
      return(print(mona, filename));
   }


   // External interfaces.
   [DllImport("mona_cs_dll.dll")]
   static private extern IntPtr createMona(int numSensors, int numResponses,
                                           int numNeeds, int randomSeed);

   [DllImport("mona_cs_dll.dll")]
   static private extern void setSensorResolution(IntPtr mona, float sensorResolution);

   [DllImport("mona_cs_dll.dll")]
   static private extern int addSensorMode(IntPtr mona, int[] sensorMask);

   [DllImport("mona_cs_dll.dll")]
   static private extern int addSensorMode(IntPtr mona, int[] sensorMask,
                                           float sensorResolution);

   [DllImport("mona_cs_dll.dll")]
   static private extern void disposeMona(IntPtr mona);

   [DllImport("mona_cs_dll.dll")]
   static private extern int cycle(IntPtr mona, float[] sensors);

   [DllImport("mona_cs_dll.dll")]
   static private extern int addResponse(IntPtr mona);

   [DllImport("mona_cs_dll.dll")]
   static private extern double getResponsePotential(IntPtr mona, int response);

   [DllImport("mona_cs_dll.dll")]
   static private extern void overrideResponse(IntPtr mona, int response);

   [DllImport("mona_cs_dll.dll")]
   static private extern void clearResponseOverride(IntPtr mona);

   [DllImport("mona_cs_dll.dll")]
   static private extern double getNeed(IntPtr mona, int index);

   [DllImport("mona_cs_dll.dll")]
   static private extern void setNeed(IntPtr mona, int index, double value);

   [DllImport("mona_cs_dll.dll")]
   static private extern void setPeriodicNeed(IntPtr mona, int needIndex,
                                              int frequency, double periodicNeed);

   [DllImport("mona_cs_dll.dll")]
   static private extern int addGoalWithResponse(IntPtr mona, int needIndex,
                                                 float[] sensors, int sensorMode,
                                                 int response, double goalValue);

   [DllImport("mona_cs_dll.dll")]
   static private extern int addGoal(IntPtr mona, int needIndex,
                                     float[] sensors, int sensorMode, double goalValue);

   [DllImport("mona_cs_dll.dll")]
   static private extern int findGoalWithResponse(IntPtr mona, int needIndex,
                                                  float[] sensors, int sensorMode, int response);

   [DllImport("mona_cs_dll.dll")]
   static private extern int findGoal(IntPtr mona, int needIndex,
                                      float[] sensors, int sensorMode);

   [DllImport("mona_cs_dll.dll")]
   static private extern int getNumGoals(IntPtr mona, int needIndex);

   [DllImport("mona_cs_dll.dll")]
   static private extern bool isGoalEnabled(IntPtr mona,
                                            int needIndex, int goalIndex);

   [DllImport("mona_cs_dll.dll")]
   static private extern bool enableGoal(IntPtr mona,
                                         int needIndex, int goalIndex);

   [DllImport("mona_cs_dll.dll")]
   static private extern bool disableGoal(IntPtr mona,
                                          int needIndex, int goalIndex);

   [DllImport("mona_cs_dll.dll")]
   static private extern bool removeGoal(IntPtr mona,
                                         int needIndex, int goalIndex);

   [DllImport("mona_cs_dll.dll", CharSet = CharSet.Ansi)]
   static private extern bool load(IntPtr mona, string filename);

   [DllImport("mona_cs_dll.dll", CharSet = CharSet.Ansi)]
   static private extern bool save(IntPtr mona, string filename);

   [DllImport("mona_cs_dll.dll")]
   static private extern void clearWorkingMemory(IntPtr mona);

   [DllImport("mona_cs_dll.dll")]
   static private extern void clear(IntPtr mona);

   [DllImport("mona_cs_dll.dll")]
   static private extern bool print(IntPtr mona, string filename);

   // Mona.
   private IntPtr mona;
}
