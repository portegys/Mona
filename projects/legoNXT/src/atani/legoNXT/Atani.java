//  Atani: A Lego NXT robot controlled by a mona neural network.

package atani.legoNXT;

import lejos.nxt.*;
import lejos.nxt.ColorSensor.*;
import lejos.robotics.*;
import lejos.robotics.navigation.*;
import java.util.*;
import javax.swing.*;
import mona.Mona;

// Atani.
public class Atani
{
   // Mona neural network.
   Mona _mona;

   // Sensor configuration.
   public enum SENSOR_CONFIG
   {
      NUM_RANGE_SENSORS(1),
      RANGE_SENSOR_INDEX(0),
      NUM_COLOR_SENSORS(1),
      COLOR_SENSOR_INDEX(1),
      NUM_TREAT_SENSORS(1),
      TREAT_SENSOR_INDEX(2),
      NUM_SENSORS(3),
      BASE_SENSOR_MODE(0),
      RANGE_SENSOR_MODE(1),
      COLOR_SENSOR_MODE(2),
      TREAT_SENSOR_MODE(3),
      NUM_SENSOR_MODES(4);

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

   // Mona sensors and response.
   public final float SENSOR_RESOLUTION = 0.02f;
   float[] _sensors;
   int _response;

   // Need types.
   public enum NEED_TYPE
   {
      SUCCESS(0),
      REWARD(1),
      NUM_NEEDS(2);

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

   // Need values.
   public              final double SUCCESS_NEED_VALUE = 1.0;
   public final double REWARD_NEED_VALUE = 1.0;

   // Treat goal value.
   public final double TREAT_GOAL_VALUE = 1.0;

   // Sensor devices.
   public final SensorPort RANGE_SENSOR_PORT = SensorPort.S3;
   public final SensorPort COLOR_SENSOR_PORT = SensorPort.S2;
   public final int        MAX_RANGE         = 255;
   UltrasonicSensor        _rangeSensor;
   ColorSensor             _colorSensor;

   // Color names (see Colors.Color values).
   public enum COLOR_NAME
   {
      BLACK("Black"),
      BLUE("Blue"),
      GREEN("Green"),
      NONE("None"),
      RED("Red"),
      WHITE("White"),
      YELLOW("Yellow");

      private String value;

      COLOR_NAME(String value)
      {
         this.value = value;
      }

      public String getValue()
      {
         return(value);
      }
   }

   // Sensor data.
   int               _range;
   ColorSensor.Color _color;
   COLOR_NAME        _colorName;
   boolean           _treat;

   // Drive parameters.
   // Distances are in cm, angles in degrees.
   public final float MIN_DRIVE_DISTANCE          = 1.0f;
   public final float MAX_DRIVE_DISTANCE          = 13.5f;
   public final float OBSTACLE_AVOIDANCE_DISTANCE = 1.0f;
   public final int   MIN_TURN_ANGLE = 5;
   public final int   TURN_ANGLE     = 180;
   public final float WHEEL_DIAMETER = 2.25f;
   public final float TRACK_WIDTH    = 5.5f;

   public final long CYCLE_DELAY = 3000L;

   // Drive pilot.
   DifferentialPilot _pilot;

   // Dashboard.
   AtaniDashboard _dashboard;

   // Random numbers.
   public final int RANDOM_SEED = 2;
   Random           _random;

   // Mona mutex.
   private Object _mutex;

   // Constructor.
   public Atani()
   {
      init();
   }


   // Initialize.
   void init()
   {
      // Random numbers.
      _random = new Random(RANDOM_SEED);

      // Mona mutex.
      _mutex = new Object();

      // Clear.
      clear();

      // Create mona.
      createMona(_random.nextInt());

      // Create sensors.
      _rangeSensor = new UltrasonicSensor(RANGE_SENSOR_PORT);
      _colorSensor = new ColorSensor(COLOR_SENSOR_PORT);

      // Create drive pilot.
      _pilot = new DifferentialPilot(WHEEL_DIAMETER, TRACK_WIDTH, Motor.A, Motor.C);

      // Create dashboard.
      _dashboard = new AtaniDashboard(this);
   }


   // Clear state.
   void clear()
   {
      if (_mona != null)
      {
         _mona.dispose();
         _mona = null;
      }
      _sensors = new float[SENSOR_CONFIG.NUM_SENSORS.getValue()];

      for (int i = 0; i < SENSOR_CONFIG.NUM_SENSORS.getValue(); i++)
      {
         _sensors[i] = 0.0f;
      }
      _response    = RESPONSE_TYPE.NULL_RESPONSE.getValue();
      _rangeSensor = null;
      _colorSensor = null;
      _range       = 0;
      _color       = null;
      _colorName   = COLOR_NAME.BLACK;
      _treat       = false;
      _pilot       = null;
      _dashboard   = null;
   }


   // Clear mona.
   void clearMona()
   {
      synchronized (_mutex)
      {
         _mona.clear();
      }
   }


   // Create mona.
   void createMona(int randomSeed)
   {
      _mona = new Mona(SENSOR_CONFIG.NUM_SENSORS.getValue(),
                       RESPONSE_TYPE.NUM_RESPONSES.getValue(),
                       NEED_TYPE.NUM_NEEDS.getValue(), randomSeed);

      boolean[] sensorMask = new boolean[SENSOR_CONFIG.NUM_SENSORS.getValue()];
      for (int i = 0; i < SENSOR_CONFIG.NUM_SENSORS.getValue(); i++)
      {
         sensorMask[i] = true;
      }
      _mona.addSensorMode(sensorMask, SENSOR_RESOLUTION);
      for (int i = 0; i < SENSOR_CONFIG.NUM_SENSORS.getValue(); i++)
      {
         if (i < SENSOR_CONFIG.COLOR_SENSOR_INDEX.getValue())
         {
            sensorMask[i] = true;
         }
         else
         {
            sensorMask[i] = false;
         }
      }
      _mona.addSensorMode(sensorMask, SENSOR_RESOLUTION);
      for (int i = 0; i < SENSOR_CONFIG.NUM_SENSORS.getValue(); i++)
      {
         if (i < SENSOR_CONFIG.COLOR_SENSOR_INDEX.getValue())
         {
            sensorMask[i] = false;
         }
         else if (i < SENSOR_CONFIG.TREAT_SENSOR_INDEX.getValue())
         {
            sensorMask[i] = true;
         }
         else
         {
            sensorMask[i] = false;
         }
      }
      _mona.addSensorMode(sensorMask, SENSOR_RESOLUTION);
      for (int i = 0; i < SENSOR_CONFIG.NUM_SENSORS.getValue(); i++)
      {
         if (i < SENSOR_CONFIG.TREAT_SENSOR_INDEX.getValue())
         {
            sensorMask[i] = false;
         }
         else
         {
            sensorMask[i] = true;
         }
      }
      _mona.addSensorMode(sensorMask, SENSOR_RESOLUTION);

      // Add needs.
      setSuccessNeed(SUCCESS_NEED_VALUE);
      setRewardNeed(REWARD_NEED_VALUE);

      // Add goal for treat.
      addTreatGoal();
   }


   // Add goal for treat.
   void addTreatGoal()
   {
      float[] sensors = new float[SENSOR_CONFIG.NUM_SENSORS.getValue()];

      sensors[SENSOR_CONFIG.RANGE_SENSOR_INDEX.getValue()] = 0.0f;
      sensors[SENSOR_CONFIG.COLOR_SENSOR_INDEX.getValue()] = 0.0f;
      sensors[SENSOR_CONFIG.TREAT_SENSOR_INDEX.getValue()] = 1.0f;
      _mona.addGoal(NEED_TYPE.REWARD.getValue(), sensors,
                    SENSOR_CONFIG.TREAT_SENSOR_MODE.getValue(),
                    RESPONSE_TYPE.NULL_RESPONSE.getValue(),
                    TREAT_GOAL_VALUE);
   }


   // Load.
   public boolean load(String filename)
   {
      boolean ret = true;

      synchronized (_mutex)
      {
         ret = _mona.load(filename);
      }
      return(ret);
   }


   // Save.
   public boolean save(String filename)
   {
      boolean ret = true;

      synchronized (_mutex)
      {
         ret = _mona.save(filename);
      }
      return(ret);
   }


   // Run.
   public void run()
   {
      displayNeeds();

      while (!_dashboard.getQuit())
      {
         cycle();
      }
   }


   // Print mona to file (XML).
   public boolean printMona(String filename)
   {
      boolean ret = true;

      synchronized (_mutex)
      {
         ret = _mona.print(filename);
      }
      return(ret);
   }


   // Read sensors and invoke mona for response.
   void cycle()
   {
      // Allow sensors to stabilize.
      try {
         Thread.sleep(CYCLE_DELAY);
      }
      catch (InterruptedException e) {}

      // Read sensor data.
      readRangeSensor();
      readColorSensor();
      readTreatSensor();

      // Display sensors on dashboard.
      displaySensors();

      // Paused?
      if (_dashboard.getPause())
      {
         return;
      }

      // Check driver.
      int driver = _dashboard.getDriver();

      if (driver == AtaniDashboard.DRIVER_TYPE.ATANI.getValue())
      {
         // Get response.
         cycleMona();
      }
      else
      {
         if (_dashboard.getManualResponse() != RESPONSE_TYPE.NULL_RESPONSE.getValue())
         {
            if (driver == AtaniDashboard.DRIVER_TYPE.OVERRIDE.getValue())
            {
               // Override response.
               overrideResponse(_dashboard.getManualResponse());
               cycleMona();
            }
            else
            {
               // Hijack response.
               _response = _dashboard.getManualResponse();
            }
         }
         else
         {
            return;
         }
      }

      // Display response on dashboard.
      displayResponse();

      // Display need values on dashboard.
      displayNeeds();

      // Process response.
      if (_response == RESPONSE_TYPE.FORWARD.getValue())
      {
         float distance = getDriveDistance(_range);

         if (distance != 0.0f)
         {
            drive(distance);
         }
      }
      else if (_response == RESPONSE_TYPE.RIGHT.getValue())
      {
         turn(TURN_ANGLE);
      }
      else if (_response == RESPONSE_TYPE.LEFT.getValue())
      {
         turn(-TURN_ANGLE);
      }

      // Turn off treat.
      if (_treat)
      {
         _dashboard.setTreatState(false);
         setRewardNeed(REWARD_NEED_VALUE);
      }

      // Pause for next step?
      // Must step in manual mode to allow user response selection.
      if (_dashboard.getStepping() ||
          (_dashboard.getDriver() != AtaniDashboard.DRIVER_TYPE.ATANI.getValue()))
      {
         _dashboard.setStepping(false);
         _dashboard.setPause(true);
      }
   }


   // Read range sensor.
   void readRangeSensor()
   {
      _range = _rangeSensor.getDistance();
      _sensors[SENSOR_CONFIG.RANGE_SENSOR_INDEX.getValue()] =
         Math.min((float)_range / (float)MAX_RANGE, 1.0f);
   }


   // Read color sensor.
   void readColorSensor()
   {
      _color = _colorSensor.getColor();
      switch (_color.getColor())
      {
      case ColorSensor.Color.BLACK:
         _sensors[SENSOR_CONFIG.COLOR_SENSOR_INDEX.getValue()] = 0.0f;
         _colorName = COLOR_NAME.BLACK;
         break;

      case ColorSensor.Color.BLUE:
         _sensors[SENSOR_CONFIG.COLOR_SENSOR_INDEX.getValue()] = 1.0f;
         _colorName = COLOR_NAME.BLUE;
         break;

      case ColorSensor.Color.GREEN:
         _sensors[SENSOR_CONFIG.COLOR_SENSOR_INDEX.getValue()] = 2.0f;
         _colorName = COLOR_NAME.GREEN;
         break;

      case ColorSensor.Color.NONE:
         _sensors[SENSOR_CONFIG.COLOR_SENSOR_INDEX.getValue()] = 3.0f;
         _colorName = COLOR_NAME.NONE;
         break;

      case ColorSensor.Color.RED:
         _sensors[SENSOR_CONFIG.COLOR_SENSOR_INDEX.getValue()] = 4.0f;
         _colorName = COLOR_NAME.RED;
         break;

      case ColorSensor.Color.WHITE:
         _sensors[SENSOR_CONFIG.COLOR_SENSOR_INDEX.getValue()] = 5.0f;
         _colorName = COLOR_NAME.WHITE;
         break;

      case ColorSensor.Color.YELLOW:
         _sensors[SENSOR_CONFIG.COLOR_SENSOR_INDEX.getValue()] = 6.0f;
         _colorName = COLOR_NAME.YELLOW;
         break;
      }
   }


   // Read treat sensor from dashboard.
   void readTreatSensor()
   {
      _treat = _dashboard.getTreatState();

      if (_treat)
      {
         _sensors[SENSOR_CONFIG.TREAT_SENSOR_INDEX.getValue()] = 1.0f;
      }
      else
      {
         _sensors[SENSOR_CONFIG.TREAT_SENSOR_INDEX.getValue()] = 0.0f;
      }
   }


   // Sensor/response cycle.
   void cycleMona()
   {
      synchronized (_mutex)
      {
         _response = _mona.cycle(_sensors);
      }
   }


   // Override response.
   void overrideResponse(int response)
   {
      synchronized (_mutex)
      {
         _mona.overrideResponse(response);
      }
   }


   // Get driving distance given a range.
   float getDriveDistance(float range)
   {
      float distance = range;

      if (range < MAX_RANGE)
      {
         distance -= OBSTACLE_AVOIDANCE_DISTANCE;
      }

      if (distance < MIN_DRIVE_DISTANCE)
      {
         distance = 0.0f;
      }

      if (distance > MAX_DRIVE_DISTANCE)
      {
         distance = MAX_DRIVE_DISTANCE;
      }

      return(distance);
   }


   // Drive given distance.
   void drive(float distance)
   {
      _pilot.travel(distance);
   }


   // Turn given angle.
   void turn(int angle)
   {
      _pilot.rotate(angle);
   }


   // Get/set needs.
   double getSuccessNeed()
   {
      double ret = 0.0;

      synchronized (_mutex)
      {
         ret = _mona.getNeed(NEED_TYPE.SUCCESS.getValue());
      }
      return(ret);
   }


   void setSuccessNeed(double value)
   {
      synchronized (_mutex)
      {
         _mona.setNeed(NEED_TYPE.SUCCESS.getValue(), value);
      }
   }


   double getRewardNeed()
   {
      double ret = 0.0;

      synchronized (_mutex)
      {
         ret = _mona.getNeed(NEED_TYPE.REWARD.getValue());
      }
      return(ret);
   }


   void setRewardNeed(double value)
   {
      synchronized (_mutex)
      {
         _mona.setNeed(NEED_TYPE.REWARD.getValue(), value);
      }
   }


   // Reward action.
   void rewardAction(int action, int sensorMode, double reward)
   {
      int goalIndex;

      synchronized (_mutex)
      {
         if (action == AtaniDashboard.REWARD_ACTION.CREATE.getValue())
         {
            _mona.addGoal(NEED_TYPE.REWARD.getValue(), _sensors,
                          sensorMode, _response, reward);
         }
         else if (action == AtaniDashboard.REWARD_ACTION.DISABLE.getValue())
         {
            goalIndex = _mona.findGoal(NEED_TYPE.REWARD.getValue(),
                                       _sensors, sensorMode, _response);
            if (goalIndex != -1)
            {
               _mona.disableGoal(NEED_TYPE.REWARD.getValue(), goalIndex);
            }
         }
         else if (reward == AtaniDashboard.REWARD_ACTION.ENABLE_ALL.getValue())
         {
            int i = _mona.getNumGoals(NEED_TYPE.REWARD.getValue());

            for (goalIndex = 0; goalIndex < i; goalIndex++)
            {
               _mona.enableGoal(NEED_TYPE.REWARD.getValue(), goalIndex);
            }
         }
         else if (action == AtaniDashboard.REWARD_ACTION.REMOVE.getValue())
         {
            goalIndex = _mona.findGoal(NEED_TYPE.REWARD.getValue(),
                                       _sensors, sensorMode, _response);

            if (goalIndex != -1)
            {
               _mona.removeGoal(NEED_TYPE.REWARD.getValue(), goalIndex);
            }
         }
         else if (action == AtaniDashboard.REWARD_ACTION.REMOVE_ALL.getValue())
         {
            while (_mona.getNumGoals(NEED_TYPE.REWARD.getValue()) > 0)
            {
               _mona.removeGoal(NEED_TYPE.REWARD.getValue(), 0);
            }
         }
      }
   }


   // Display sensors on dashboard.
   void displaySensors()
   {
      String rangeSensorString = _range + "";
      String colorSensorString = _colorName.getValue();
      String treatSensorString;

      if (_treat)
      {
         treatSensorString = "true";
      }
      else
      {
         treatSensorString = "false";
      }

      _dashboard.setSensors(rangeSensorString, colorSensorString,
                            treatSensorString);
   }


   // Display response on dashboard.
   void displayResponse()
   {
      String responseString = "";

      if (_response == RESPONSE_TYPE.WAIT.getValue())
      {
         responseString = "wait";
      }
      else if (_response == RESPONSE_TYPE.FORWARD.getValue())
      {
         responseString = "forward";
      }
      else if (_response == RESPONSE_TYPE.RIGHT.getValue())
      {
         responseString = "right";
      }
      else if (_response == RESPONSE_TYPE.LEFT.getValue())
      {
         responseString = "left";
      }

      _dashboard.setResponse(responseString);
   }


   // Display needs on dashboard.
   void displayNeeds()
   {
      _dashboard.setNeeds(getSuccessNeed(), getRewardNeed());
   }


   // Main.
   public static void main(String[] args)
   {
      // Get args.
      String loadfile = null;
      String savefile = null;

      for (int i = 0; i < args.length; i++)
      {
         if (args[i].equals("-load"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid load option");
               System.exit(1);
            }
            if (loadfile == null)
            {
               loadfile = args[i];
            }
            else
            {
               System.err.println("Duplicate load option");
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-save"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid save option");
               System.exit(1);
            }
            if (savefile == null)
            {
               savefile = args[i];
            }
            else
            {
               System.err.println("Duplicate save option");
               System.exit(1);
            }
            continue;
         }
         System.err.println("Usage [-load <load file name>] [-save <save file name>]");
         System.exit(1);
      }

      // Set look and feel.
      try {
         UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
      }
      catch (Exception e)
      {
         System.err.println("Warning: cannot set look and feel");
      }

      // Create atani.
      Atani atani = new Atani();

      // Load?
      if (loadfile != null)
      {
         atani.load(loadfile);
      }

      // Run.
      atani.run();

      // Save?
      if (savefile != null)
      {
         atani.save(savefile);
      }
      atani.clear();
   }
}
