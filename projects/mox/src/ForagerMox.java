/*
 * Forager mox.
 */

import java.util.*;
import java.awt.Color;
import mona.Mona;

public class ForagerMox extends Mox
{
   // Color.
   public static final Color FORAGER_COLOR       = Color.BLACK;
   public static final int   FORAGER_COLOR_VALUE = 9;

   // Maximum sensor range (-1 == infinite).
   public static float MAX_SENSOR_RANGE = -1.0f;

   // Need types.
   public enum NEED_TYPE
   {
      BLUE_FOOD(0),
      NUM_NEEDS(1);

      private int value;

      NEED_TYPE(int value)
      {
         this.value = value;
      }

      public int getValue()
      {
         return(value);
      }
   }

   // Need and goal values.
   public static              final double BLUE_FOOD_NEED_VALUE = 1.0;
   public static final double BLUE_FOOD_GOAL_VALUE = 1.0;
   public static final String BLUE_FOOD_NEED_NAME  = "blue food";

   // Constructors.
   public ForagerMox()
   {
      super();
      setProperties(-1, SPECIES.FORAGER.getValue(),
                    0, 0, DIRECTION.NORTH.getValue(), true);

      // Create mona.
      createMona(DEFAULT_RANDOM_SEED);

      // Configure forager-specific needs and goals.
      configureNeedsAndGoals();
   }


   public ForagerMox(int id, int x, int y, int direction, int randomSeed)
   {
      mutex = new Object();
      setProperties(id, SPECIES.FORAGER.getValue(),
                    x, y, direction, true);

      // Create mona.
      createMona(randomSeed);

      // Configure forager-specific needs and goals.
      configureNeedsAndGoals();
   }


   // Construct with mona parameters.
   public ForagerMox(int id, int x, int y, int direction,
                     Vector<String> monaParmKeys, Vector<Object> monaParmVals)
   {
      mutex = new Object();
      setProperties(id, SPECIES.FORAGER.getValue(),
                    x, y, direction, true);

      // Create mona.
      createMona(monaParmKeys, monaParmVals);

      // Configure forager-specific needs and goals.
      configureNeedsAndGoals();
   }


   // Create mona.
   void createMona(int randomSeed)
   {
      synchronized (mutex)
      {
         mona = new Mona(SENSOR_CONFIG.NUM_SENSORS.getValue(),
                         RESPONSE_TYPE.NUM_RESPONSES.getValue(),
                         NEED_TYPE.NUM_NEEDS.getValue(), randomSeed);
         boolean[] sensorMask = new boolean[SENSOR_CONFIG.NUM_SENSORS.getValue()];
         for (int i = 0; i < SENSOR_CONFIG.NUM_SENSORS.getValue(); i++)
         {
            sensorMask[i] = true;
         }
         mona.addSensorMode(sensorMask, SENSOR_RESOLUTION);
         for (int i = 0; i < SENSOR_CONFIG.NUM_SENSORS.getValue(); i++)
         {
            if (i < SENSOR_CONFIG.NUM_RANGE_SENSORS.getValue())
            {
               sensorMask[i] = true;
            }
            else
            {
               sensorMask[i] = false;
            }
         }
         mona.addSensorMode(sensorMask, SENSOR_RESOLUTION);
         for (int i = 0; i < SENSOR_CONFIG.NUM_SENSORS.getValue(); i++)
         {
            if (i < SENSOR_CONFIG.NUM_RANGE_SENSORS.getValue())
            {
               sensorMask[i] = false;
            }
            else
            {
               sensorMask[i] = true;
            }
         }
         mona.addSensorMode(sensorMask, SENSOR_RESOLUTION);
      }
   }


   // Configure forager-specific needs and goals.
   void configureNeedsAndGoals()
   {
      synchronized (mutex)
      {
         needValues = new double[NEED_TYPE.NUM_NEEDS.getValue()];
         needValues[NEED_TYPE.BLUE_FOOD.getValue()] = BLUE_FOOD_NEED_VALUE;
         goalValues = new double[NEED_TYPE.NUM_NEEDS.getValue()];
         goalValues[NEED_TYPE.BLUE_FOOD.getValue()] = BLUE_FOOD_GOAL_VALUE;
         needNames = new String[NEED_TYPE.NUM_NEEDS.getValue()];
         needNames[NEED_TYPE.BLUE_FOOD.getValue()] = BLUE_FOOD_NEED_NAME;
         mona.setNeed(NEED_TYPE.BLUE_FOOD.getValue(), BLUE_FOOD_NEED_VALUE);
         float[] sensors = new float[SENSOR_CONFIG.NUM_SENSORS.getValue()];
         sensors[SENSOR_CONFIG.RANGE_SENSOR_INDEX.getValue()]     = 0;
         sensors[SENSOR_CONFIG.COLOR_HUE_SENSOR_INDEX.getValue()] =
            GameOfLife.BLUE_CELL_COLOR_VALUE;
         sensors[SENSOR_CONFIG.COLOR_INTENSITY_SENSOR_INDEX.getValue()] = 1.0f;
         mona.addGoal(NEED_TYPE.BLUE_FOOD.getValue(), sensors,
                      SENSOR_CONFIG.BASIC_SENSOR_MODE.getValue(),
                      RESPONSE_TYPE.NULL_RESPONSE.getValue(),
                      BLUE_FOOD_GOAL_VALUE);
      }
   }


   // Reset state.
   void reset()
   {
      super.reset();
      synchronized (mutex)
      {
         mona.setNeed(NEED_TYPE.BLUE_FOOD.getValue(), BLUE_FOOD_NEED_VALUE);
      }
   }
}
