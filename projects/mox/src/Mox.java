/*
 * Mox: mona learning automaton.
 */

import java.util.*;
import java.io.*;
import java.nio.channels.FileChannel;
import mona.Mona;
import mona.NativeFileDescriptor;

// Mox.
public abstract class Mox
{
   // Properties.
   public int     id;
   public int     species;
   public int     x, y;
   public int     direction;
   public int     x2, y2;
   public int     direction2;
   public boolean isAlive;
   public int     driverType;
   public int     driverResponse;

   // Neural network.
   public Mona mona;
   float[] sensors;
   int response;

   // Mox species.
   public enum SPECIES
   {
      FORAGER(0),
      PREDATOR(1);

      private int value;

      SPECIES(int value)
      {
         this.value = value;
      }

      public int getValue()
      {
         return(value);
      }


      public void setValue(int value)
      {
         this.value = value;
      }
   }

   // Direction.
   public enum DIRECTION
   {
      NORTH(0),
      EAST(1),
      SOUTH(2),
      WEST(3),
      NUM_DIRECTIONS(4);

      private int value;

      DIRECTION(int value)
      {
         this.value = value;
      }

      public int getValue()
      {
         return(value);
      }


      public void setValue(int value)
      {
         this.value = value;
      }
   }

   // Default random seed.
   public static final int DEFAULT_RANDOM_SEED = 4517;

   // Sensor configuration.
   public enum SENSOR_CONFIG
   {
      NUM_RANGE_SENSORS(1),
      RANGE_SENSOR_INDEX(0),
      NUM_COLOR_SENSORS(2),
      COLOR_HUE_SENSOR_INDEX(1),
      COLOR_INTENSITY_SENSOR_INDEX(2),
      NUM_SENSORS(3),
      BASIC_SENSOR_MODE(0),
      RANGE_SENSOR_MODE(1),
      COLOR_SENSOR_MODE(2),
      NUM_SPECIFIC_SENSOR_MODES(2);

      private int value;

      SENSOR_CONFIG(int value)
      {
         this.value = value;
      }

      public int getValue()
      {
         return(value);
      }
   }

   // Response types.
   // NULL_RESPONSE must be consistent with mona.cpp value.
   public enum RESPONSE_TYPE
   {
      WAIT(0),
      FORWARD(1),
      RIGHT(2),
      LEFT(3),
      NUM_RESPONSES(4),
      NULL_RESPONSE(0x7fffffff);

      private int value;

      RESPONSE_TYPE(int value)
      {
         this.value = value;
      }

      public int getValue()
      {
         return(value);
      }
   }

   // Mona sensor resolution.
   public static final float SENSOR_RESOLUTION = 0.0f;

   // Needs.
   public double[] needValues;
   public double[] goalValues;
   public          String[] needNames;

   // Driver type.
   public enum DRIVER_TYPE
   {
      MOX(0),
      OVERRIDE(1),
      HIJACK(2);

      private int value;

      DRIVER_TYPE(int value)
      {
         this.value = value;
      }

      public int getValue()
      {
         return(value);
      }
   }

   // Mutex.
   protected Object mutex;

   // Cycle time accumulator.
   boolean accumulateCycleTime;
   long    cycleTimeAccumulator;

   // Constructor.
   public Mox()
   {
      mutex   = new Object();
      sensors = new float[SENSOR_CONFIG.NUM_SENSORS.getValue()];
      mona    = null;
      clear();
   }


   // Set properties.
   void setProperties(int id, int species, int x, int y,
                      int direction, boolean isAlive)
   {
      clear();
      this.id      = id;
      this.species = species;
      setSpacialProperties(x, y, direction);
      this.isAlive = isAlive;
   }


   // Set spacial properties.
   void setSpacialProperties(int x, int y, int direction)
   {
      this.x         = x2 = x;
      this.y         = y2 = y;
      this.direction = direction2 = direction;
   }


   // Reset state.
   void reset()
   {
      x              = x2;
      y              = y2;
      direction      = direction2;
      isAlive        = true;
      driverType     = DRIVER_TYPE.MOX.getValue();
      driverResponse = RESPONSE_TYPE.NULL_RESPONSE.getValue();
      for (int i = 0; i < SENSOR_CONFIG.NUM_SENSORS.getValue(); i++)
      {
         sensors[i] = 0.0f;
      }
      response = RESPONSE_TYPE.NULL_RESPONSE.getValue();

      synchronized (mutex)
      {
         mona.clearWorkingMemory();
      }
   }


   // Clear state.
   void clear()
   {
      id             = -1;
      x              = y = x2 = y2 = 0;
      direction      = direction2 = DIRECTION.NORTH.getValue();
      isAlive        = true;
      driverType     = DRIVER_TYPE.MOX.getValue();
      driverResponse = RESPONSE_TYPE.NULL_RESPONSE.getValue();
      for (int i = 0; i < SENSOR_CONFIG.NUM_SENSORS.getValue(); i++)
      {
         sensors[i] = 0.0f;
      }
      response             = RESPONSE_TYPE.NULL_RESPONSE.getValue();
      accumulateCycleTime  = false;
      cycleTimeAccumulator = 0;

      synchronized (mutex)
      {
         if (mona != null)
         {
            mona.dispose();
            mona = null;
         }
      }
   }


   // Create mona.
   abstract void createMona(int randomSeed);

   // Create mona with parameters.
   void createMona(Vector<String> parmKeys, Vector<Object> parmVals)
   {
      synchronized (mutex)
      {
         mona = new Mona(parmKeys, parmVals);
      }
   }


   // Add goal.
   public void addGoal(int needIndex, float[] sensors,
                       int sensorMode, int response, double goalValue)
   {
      mona.addGoal(needIndex, sensors, sensorMode, response, goalValue);
   }


   public void addGoal(int needIndex, float[] sensors,
                       int sensorMode, int response, double goalValue,
                       int frequency, double periodicNeed)
   {
      mona.addGoal(needIndex, sensors, sensorMode, response, goalValue);
      mona.setPeriodicNeed(needIndex, frequency, periodicNeed);
   }


   // Load mox from file.
   public void load(String filename) throws IOException
   {
      FileInputStream      input;
      NativeFileDescriptor fd;

      // Open the file.
      try {
         input = new FileInputStream(new File(filename));
         fd    = new NativeFileDescriptor(filename, "r");
         fd.open();
      }
      catch (Exception e) {
         throw new IOException("Cannot open input file " + filename +
                               ":" + e.getMessage());
      }

      // Load the file.
      load(input, fd);

      input.close();
      fd.close();
   }


   // Load mox.
   public void load(FileInputStream input, NativeFileDescriptor fd) throws IOException
   {
      // Load the properties.
      // DataInputStream is for unbuffered input.
      DataInputStream reader = new DataInputStream(input);

      id         = Utility.loadInt(reader);
      species    = Utility.loadInt(reader);
      x          = Utility.loadInt(reader);
      y          = Utility.loadInt(reader);
      direction  = Utility.loadInt(reader);
      x2         = Utility.loadInt(reader);
      y2         = Utility.loadInt(reader);
      direction2 = Utility.loadInt(reader);
      if (Utility.loadInt(reader) == 1)
      {
         isAlive = true;
      }
      else
      {
         isAlive = false;
      }

      // Load mona.
      FileChannel channel = input.getChannel();
      fd.seek(channel.position());
      synchronized (mutex)
      {
         mona.load(fd);
      }
      channel.position(fd.tell() + 1);
   }


   // Save mox to file.
   public void save(String filename) throws IOException
   {
      FileOutputStream     output;
      NativeFileDescriptor fd;

      try
      {
         output = new FileOutputStream(new File(filename));
         fd     = new NativeFileDescriptor(filename, "w");
         fd.open();
      }
      catch (Exception e) {
         throw new IOException("Cannot open output file " + filename +
                               ":" + e.getMessage());
      }

      // Save to the file.
      save(output, fd);

      output.close();
      fd.close();
   }


   // Save mox.
   public void save(FileOutputStream output, NativeFileDescriptor fd) throws IOException
   {
      // Save the properties.
      PrintWriter writer = new PrintWriter(new OutputStreamWriter(output));

      Utility.saveInt(writer, id);
      Utility.saveInt(writer, species);
      Utility.saveInt(writer, x);
      Utility.saveInt(writer, y);
      Utility.saveInt(writer, direction);
      Utility.saveInt(writer, x2);
      Utility.saveInt(writer, y2);
      Utility.saveInt(writer, direction2);
      if (isAlive)
      {
         Utility.saveInt(writer, 1);
      }
      else
      {
         Utility.saveInt(writer, 0);
      }
      writer.flush();

      // Save mona.
      FileChannel channel = output.getChannel();
      fd.seek(channel.position());
      synchronized (mutex)
      {
         mona.save(fd);
      }
      channel.position(fd.tell());
   }


   // Sensor/response cycle.
   public int cycle(float[] sensors)
   {
      long startTime, stopTime;

      this.sensors = sensors;
      if (driverType == DRIVER_TYPE.HIJACK.getValue())
      {
         response = driverResponse;
      }
      else
      {
         synchronized (mutex)
         {
            if (driverType == DRIVER_TYPE.OVERRIDE.getValue())
            {
               mona.overrideResponse(driverResponse);
            }
            if (accumulateCycleTime)
            {
               startTime             = ProcessInformation.getProcessCPUTime();
               response              = mona.cycle(sensors);
               stopTime              = ProcessInformation.getProcessCPUTime();
               cycleTimeAccumulator += stopTime - startTime;
            }
            else
            {
               response = mona.cycle(sensors);
            }
         }
      }
      return(response);
   }


   // Override response.
   public void overrideResponse(int response)
   {
      synchronized (mutex)
      {
         mona.overrideResponse(response);
      }
   }


   // Clear response override.
   public void clearResponseOverride()
   {
      synchronized (mutex)
      {
         mona.clearResponseOverride();
      }
   }


   // Get/set needs.
   public double getNeed(int needType)
   {
      double ret = 0.0;

      synchronized (mutex)
      {
         ret = mona.getNeed(needType);
      }
      return(ret);
   }


   public void setNeed(int needType, double value)
   {
      synchronized (mutex)
      {
         mona.setNeed(needType, value);
      }
   }


   // Print mona to file (XML).
   public boolean printMona(String filename)
   {
      boolean ret = true;

      synchronized (mutex)
      {
         ret = mona.print(filename);
      }
      return(ret);
   }


   // Cycle time accumulation.
   public void startCycleTimeAccumulation()
   {
      synchronized (mutex)
      {
         accumulateCycleTime  = true;
         cycleTimeAccumulator = 0;
      }
   }


   public void stopCycleTimeAccumulation()
   {
      synchronized (mutex)
      {
         accumulateCycleTime = false;
      }
   }


   public long getCycleTimeAccumulator()
   {
      long t;

      synchronized (mutex)
      {
         t = cycleTimeAccumulator;
      }
      return(t);
   }


   // Get maximum cycle time according to current running conditions.
   public static long getMaxCycleTime()
   {
      long i, j;
      long startTime = ProcessInformation.getProcessCPUTime();

      for (i = j = 0; i < 100000000; i++)
      {
         j = (i + 1) / 2;
      }
      long stopTime = ProcessInformation.getProcessCPUTime();
      return((stopTime - startTime) * 10);
   }
}
