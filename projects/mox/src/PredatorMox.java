/*
 * Predator mox.
 */

import java.util.*;
import java.awt.Color;
import mona.Mona;

public class PredatorMox extends Mox
{
   // Color.
   public static final Color PREDATOR_COLOR       = Color.RED;
   public static final int   PREDATOR_COLOR_VALUE = 10;

   // Maximum sensor range (-1 == infinite).
   public static float MAX_SENSOR_RANGE = -1.0f;

   // Need types.
   public enum NEED_TYPE
   {
      MOX_FOOD(0),
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

   // Need and goal value.
   public static              final double MOX_FOOD_NEED_VALUE = 1.0;
   public static final double MOX_FOOD_GOAL_VALUE = 1.0;
   public static final String MOX_FOOD_NEED_NAME  = "Mox food";

   // Constructors.
   public PredatorMox()
   {
      super();
      setProperties(-1, SPECIES.PREDATOR.getValue(),
                    0, 0, DIRECTION.NORTH.getValue(), true);

      // Create mona.
      createMona(DEFAULT_RANDOM_SEED);

      // Configure predator-specific needs and goals.
      configureNeedsAndGoals();
   }


   public PredatorMox(int id, int x, int y, int direction, int randomSeed)
   {
      mutex = new Object();
      setProperties(id, SPECIES.PREDATOR.getValue(),
                    x, y, direction, true);

      // Create mona.
      createMona(randomSeed);

      // Configure predator-specific needs and goals.
      configureNeedsAndGoals();
   }


   // Construct with mona parameters.
   public PredatorMox(int id, int x, int y, int direction,
                      Vector<String> monaParmKeys, Vector<Object> monaParmVals)
   {
      mutex = new Object();
      setProperties(id, SPECIES.PREDATOR.getValue(),
                    x, y, direction, true);

      // Create mona.
      createMona(monaParmKeys, monaParmVals);

      // Configure predator-specific needs and goals.
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


   // Configure predator-specific needs and goals.
   void configureNeedsAndGoals()
   {
      synchronized (mutex)
      {
         needValues = new double[NEED_TYPE.NUM_NEEDS.getValue()];
         needValues[NEED_TYPE.MOX_FOOD.getValue()] = MOX_FOOD_NEED_VALUE;
         goalValues = new double[NEED_TYPE.NUM_NEEDS.getValue()];
         goalValues[NEED_TYPE.MOX_FOOD.getValue()] = MOX_FOOD_GOAL_VALUE;
         needNames = new String[NEED_TYPE.NUM_NEEDS.getValue()];
         needNames[NEED_TYPE.MOX_FOOD.getValue()] = MOX_FOOD_NEED_NAME;
         mona.setNeed(NEED_TYPE.MOX_FOOD.getValue(), MOX_FOOD_NEED_VALUE);
         float[] sensors = new float[SENSOR_CONFIG.NUM_SENSORS.getValue()];
         sensors[SENSOR_CONFIG.RANGE_SENSOR_INDEX.getValue()]     = 0.0f;
         sensors[SENSOR_CONFIG.COLOR_HUE_SENSOR_INDEX.getValue()] =
            (float)ForagerMox.FORAGER_COLOR_VALUE;
         sensors[SENSOR_CONFIG.COLOR_INTENSITY_SENSOR_INDEX.getValue()] = 1.0f;
         mona.addGoal(NEED_TYPE.MOX_FOOD.getValue(), sensors,
                      SENSOR_CONFIG.BASIC_SENSOR_MODE.getValue(),
                      RESPONSE_TYPE.NULL_RESPONSE.getValue(),
                      MOX_FOOD_GOAL_VALUE);
      }
   }


   // Reset state.
   void reset()
   {
      super.reset();
      mona.setNeed(NEED_TYPE.MOX_FOOD.getValue(), MOX_FOOD_NEED_VALUE);
   }
}
