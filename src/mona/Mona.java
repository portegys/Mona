// Mona java class using JNI.
// For conditions of distribution and use, see copyright notice in mona.hpp
package mona;

import java.util.*;
import java.io.*;
import java.lang.reflect.*;

public class Mona
{
   static
   {
      try
      {
         System.loadLibrary("mona_jni");
      }
      catch (UnsatisfiedLinkError e)
      {
         try
         {
            setLibraryPath(".");
         }
         catch (Exception e2)
         {
            System.err.println("Cannot set system property 'java.library.path'");
         }
         try
         {
            System.loadLibrary("mona_jni");
         }
         catch (UnsatisfiedLinkError e2)
         {
            System.err.println("mona_jni native library not found in 'java.library.path': " +
                               System.getProperty("java.library.path"));
            throw e2;
         }
         catch (Exception e2)
         {
            System.err.println("Failure loading mona_jni native library from 'java.library.path': " +
                               System.getProperty("java.library.path"));
            throw e2;
         }
      }
      catch (Exception e)
      {
         System.err.println("Failure loading mona_jni native library from 'java.library.path': " +
                            System.getProperty("java.library.path"));
         throw e;
      }
   }

   public static void setLibraryPath(String path) throws Exception
   {
      System.setProperty("java.library.path", path);
      final Field sysPathsField = ClassLoader.class .getDeclaredField("sys_paths");
      sysPathsField.setAccessible(true);
      sysPathsField.set(null, null);
   }


   // Constructors.
   public Mona(int numSensors, int numResponses, int numNeeds, int randomSeed)
   {
      monaReference = createMona(numSensors, numResponses, numNeeds, randomSeed);
   }


   // Construct with parameters.
   // Required parameters:
   // "NUM_SENSORS" -> Number of sensors (String).
   // "NUM_RESPONSES" -> Number of responses (String).
   // "NUM_NEEDS" -> Number of needs (String).
   // All parameters values are type String
   // except the repeatable "SENSOR_MODE" parameter,
   // which is type String[] and has the format:
   // <sensor mask values ("0"|"1")><resolution>.
   public Mona(Vector<String> parameterKeys, Vector<Object> parameterValues)
   {
      String[] keys   = new String[parameterKeys.size()];
      Object[] values = new Object[parameterValues.size()];
      for (int i = 0; i < parameterKeys.size(); i++)
      {
         keys[i]   = parameterKeys.get(i);
         values[i] = parameterValues.get(i);
      }
      monaReference = createMonaWithParameters(keys, values);
   }


   // Set sensor resolution.
   public void setSensorResolution(float sensorResolution)
   {
      setSensorResolution(monaReference, sensorResolution);
   }


   // Add sensor mode.
   public int addSensorMode(boolean[] sensorMask)
   {
      return(addSensorMode(monaReference, sensorMask));
   }


   public int addSensorMode(boolean[] sensorMask, float sensorResolution)
   {
      return(addSensorMode(monaReference, sensorMask, sensorResolution));
   }


   // Set number of effect event intervals for mediator level.
   public void setEffectEventIntervals(int level, int intervals)
   {
      setEffectEventIntervals(monaReference, level, intervals);
   }


   // Set effect event interval values.
   public void setEffectEventInterval(int level, int interval, int value, float weight)
   {
      setEffectEventInterval(monaReference, level, interval, value, weight);
   }


   // Delete object.
   public void dispose()
   {
      dispose(monaReference);
   }


   // Sensory-response cycle.
   public int cycle(float[] sensors)
   {
      return(cycle(monaReference, sensors));
   }


   // Add response.
   public int addResponse()
   {
      return(addResponse(monaReference));
   }


   // Get response potential.
   public double getResponsePotential(int response)
   {
      return(getResponsePotential(monaReference, response));
   }


   // Override response.
   public void overrideResponse(int response)
   {
      overrideResponse(monaReference, response);
   }


   public void clearResponseOverride()
   {
      clearResponseOverride(monaReference);
   }


   // Get need.
   public double getNeed(int needIndex)
   {
      return(getNeed(monaReference, needIndex));
   }


   // Set need.
   public void setNeed(int needIndex, double value)
   {
      setNeed(monaReference, needIndex, value);
   }


   // Set periodic need.
   public void setPeriodicNeed(int needIndex, int frequency, double periodicNeed)
   {
      setPeriodicNeed(monaReference, needIndex, frequency, periodicNeed);
   }


   // Add goal.
   public int addGoal(int needIndex, float[] sensors,
                      int sensorMode, int response, double goalValue)
   {
      return(addGoal(monaReference, needIndex, sensors, sensorMode, response, goalValue));
   }


   public int addGoal(int needIndex, float[] sensors,
                      int sensorMode, double goalValue)
   {
      return(addGoal(monaReference, needIndex, sensors, sensorMode, goalValue));
   }


   // Find goal.
   public int findGoal(int needIndex, float[] sensors,
                       int sensorMode, int response)
   {
      return(findGoal(monaReference, needIndex, sensors, sensorMode, response));
   }


   public int findGoal(int needIndex, float[] sensors, int sensorMode)
   {
      return(findGoal(monaReference, needIndex, sensors, sensorMode));
   }


   // Get number of goals.
   public int getNumGoals(int needIndex)
   {
      return(getNumGoals(monaReference, needIndex));
   }


   // Is goal enabled?
   public boolean isGoalEnabled(int needIndex, int goalIndex)
   {
      return(isGoalEnabled(monaReference, needIndex, goalIndex));
   }


   // Enable goal.
   public boolean enableGoal(int needIndex, int goalIndex)
   {
      return(enableGoal(monaReference, needIndex, goalIndex));
   }


   // Disable goal.
   public boolean disableGoal(int needIndex, int goalIndex)
   {
      return(disableGoal(monaReference, needIndex, goalIndex));
   }


   // Remove goal.
   public boolean removeGoal(int needIndex, int goalIndex)
   {
      return(removeGoal(monaReference, needIndex, goalIndex));
   }


   // Load.
   public boolean load(String filename)
   {
      return(load(monaReference, filename));
   }


   public boolean load(NativeFileDescriptor fd)
   {
      return(load(monaReference, fd));
   }


   // Save.
   public boolean save(String filename)
   {
      return(save(monaReference, filename));
   }


   public boolean save(NativeFileDescriptor fd)
   {
      return(save(monaReference, fd));
   }


   // Clear working memory.
   public void clearWorkingMemory()
   {
      clearWorkingMemory(monaReference);
   }


   // Clear long term memory.
   public void clearLongTermMemory()
   {
      clearLongTermMemory(monaReference);
   }


   // Clear.
   public void clear()
   {
      clear(monaReference);
   }


   // Print.
   public boolean print()
   {
      return(print_jni(monaReference, "stdout"));
   }


   public boolean print(String filename)
   {
      return(print_jni(monaReference, filename));
   }


   private native int createMona(int numSensors, int numResponses,
                                 int numNeeds, int randomSeed);

   private native int createMonaWithParameters(String[] parameterKeys,
                                               Object[] parameterValues);

   private native int setSensorResolution(int reference, float sensorResolution);

   private native int addSensorMode(int reference, boolean[] sensorMask);

   private native int addSensorMode(int reference, boolean[] sensorMask,
                                    float sensorResolution);

   private native void setEffectEventIntervals(int reference, int level,
                                               int intervals);

   private native void setEffectEventInterval(int reference, int level,
                                              int interval, int value, float weight);

   private native void dispose(int reference);

   private native int cycle(int reference, float[] sensors);

   private native int addResponse(int reference);

   private native double getResponsePotential(int reference, int response);

   private native void overrideResponse(int reference, int response);

   private native void clearResponseOverride(int reference);

   private native double getNeed(int reference, int needIndex);

   private native void setNeed(int reference, int needIndex, double value);

   private native void setPeriodicNeed(int reference, int needIndex,
                                       int frequency, double periodicNeed);

   private native int addGoal(int reference, int needIndex, float[] sensors,
                              int sensorMode, int response, double goalValue);

   private native int addGoal(int reference, int needIndex, float[] sensors,
                              int sensorMode, double goalValue);

   private native int findGoal(int reference, int needIndex, float[] sensors,
                               int sensorMode, int response);

   private native int findGoal(int reference, int needIndex, float[] sensors,
                               int sensorMode);

   private native int getNumGoals(int reference, int needIndex);

   private native boolean isGoalEnabled(int reference, int needIndex, int goalIndex);

   private native boolean enableGoal(int reference, int needIndex, int goalIndex);

   private native boolean disableGoal(int reference, int needIndex, int goalIndex);

   private native boolean removeGoal(int reference, int needIndex, int goalIndex);

   private native boolean load(int reference, String filename);

   private native boolean load(int reference, NativeFileDescriptor fd);

   private native boolean save(int reference, String filename);

   private native boolean save(int reference, NativeFileDescriptor fd);

   private native void clearWorkingMemory(int reference);

   private native void clearLongTermMemory(int reference);

   private native void clear(int reference);

   private native boolean print_jni(int reference, String filename);

   // Reference to native mona object.
   private int monaReference;
}
