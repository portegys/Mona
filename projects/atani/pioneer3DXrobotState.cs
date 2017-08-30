//-----------------------------------------------------------------------
//  Atani neural network controlled robot.
//
//  $File: pioneer3DXrobotState.cs $ $Revision: 1 $
//-----------------------------------------------------------------------

using Microsoft.Dss.Core;
using Microsoft.Dss.Core.Attributes;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using Microsoft.Robotics.Simulation;
using Microsoft.Robotics.Simulation.Engine;
using Microsoft.Robotics.Simulation.Physics;
using Microsoft.Robotics.PhysicalModel;
using drive   = Microsoft.Robotics.Services.Drive.Proxy;
using sicklrf = Microsoft.Robotics.Services.Sensors.SickLRF.Proxy;
using bumper  = Microsoft.Robotics.Services.ContactSensor.Proxy;
using webcam  = Microsoft.Robotics.Services.WebCam.Proxy;

namespace Robotics.Atani
{
   // Pioneer3DX robot state.
   public class Pioneer3DXrobotState
   {
      // Sensor configuration.
      public enum SENSOR_CONFIG
      {
         NUM_LRF_SENSORS     = 3,
         LRF_SENSOR_INDEX    = 0,
         NUM_WEBCAM_SENSORS  = 3,
         WEBCAM_SENSOR_INDEX = 3,
         NUM_BUMPER_SENSORS  = 1,
         BUMPER_SENSOR_INDEX = 6,
         NUM_SENSORS         = 7,
         BASE_SENSOR_MODE    = 0,
         LRF_SENSOR_MODE     = 1,
         WEBCAM_SENSOR_MODE  = 2,
         BUMPER_SENSOR_MODE  = 3,
         NUM_SENSOR_MODES    = 4
      };

      // Sensor resolution.
      public const float SENSOR_RESOLUTION = 0.02f;

      // Need types.
      public enum NEED_TYPE
      {
         SUCCESS   = 0,
         REWARD    = 1,
         NUM_NEEDS = 2
      };

      // Need values.
      public const double SUCCESS_NEED_VALUE = 1.0;
      public const double REWARD_NEED_VALUE  = 1.0;

      // Reward actions.
      public enum REWARD_ACTION
      {
         CREATE     = 0,
         DISABLE    = 1,
         ENABLE_ALL = 2,
         REMOVE     = 3,
         REMOVE_ALL
      };

      // Response types.
      // NULL_RESPONSE must be consistent with mona.cpp value.
      public enum RESPONSE_TYPE
      {
         WAIT             = 0,
         FORWARD          = 1,
         RIGHT            = 2,
         LEFT             = 3,
         PENDING_RESPONSE = (-1),
         NULL_RESPONSE    = (0x7fffffff),
         NUM_RESPONSES    = 4
      };

      // Laser rangefinder max value (mm).
      public const int LRF_MAX_RANGE = 8192;

      // Movement default parameters.
      public const double DEFAULT_MIN_DRIVE_DISTANCE   = 0.01;    // meters
      public const double DEFAULT_MAX_DRIVE_DISTANCE   = 1.0;     // meters
      public const double DEFAULT_MIN_TURN_ANGLE       = 5.0;     // degrees
      public const double DEFAULT_DRIVE_SPEED          = 0.2;     // meters/sec
      public const double DEFAULT_TURN_SPEED           = 0.2;     // meters/sec
      public const double DEFAULT_MAX_CREEP_DISTANCE   = 0.1;     // meters
      public const double DEFAULT_MAX_ALIGN_RANGE      = 1.0;     // meters
      public const double DEFAULT_MAX_ALIGN_TURN_ANGLE = 10.0;    // degrees

      // Robot structure.
      public Pioneer3DX _robotBaseEntity;

      // Initial position.
      public Vector3 _initialPosition;

      // Dimensions (meters)
      public Vector3 _dimensions;

      // Environment sensor data.
      public sicklrf.State             _lrfData;
      public int                       _lrfForwardDistance;
      public int                       _lrfLeftDistance;
      public int                       _lrfRightDistance;
      public webcam.QueryFrameResponse _webcamFrame;
      public bool                      _bumperHit;

      // Drive.
      public double _minDriveDistance;
      public double _maxDriveDistance;
      public double _driveSpeed;
      public double _minTurnAngle;
      public double _turnSpeed;
      public int    _velocity;
      public drive.DriveDifferentialTwoWheelState _driveState;

      // This is pose information extracted from the wheels of the
      // differential drive. It only works for the simulated drive.
      public float _x, _y, _orientation;

      // Is robot moving?
      public bool _moving;

      // Movement adjustments.
      public bool   _creep;
      public double _creepRange;
      public double _creepDistance;
      public double _maxCreepDistance;
      public bool   _align;
      public double _maxAlignRange;
      public double _maxAlignTurnAngle;

      // Brain sensors and response.
      public float[]       _sensors;
      public float[]       _lastSensors;
      public RESPONSE_TYPE _response;

      // Mona neural network brain.
      public Mona _brain;

      // Brain save file.
      const string BRAIN_SAVE_FILE = "apps/atani/config/atani.pioneer3DXrobot.mona";

      // Brain print file (XML).
      const string BRAIN_PRINT_FILE = "apps/atani/config/atani.pioneer3DXrobot.xml";

      // Pause mode.
      public bool _pause;

      // Stepping mode.
      public bool _step;

      // Driver type.
      public enum DRIVER_TYPE
      {
         ATANI     = 0,
         OVERRIDE  = 1,
         HIJACK    = 2,
         GOTO_GOAL = 3
      };

      // Driver.
      public DRIVER_TYPE   _driver;
      public RESPONSE_TYPE _manualResponse;

      // Constructor.
      public Pioneer3DXrobotState(Random random)
      {
         clear();

         // Create brain.
         createBrain(random.Next());

         // Try to load previously saved brain.
         if (loadBrain())
         {
            setSuccessNeed(SUCCESS_NEED_VALUE);
            setRewardNeed(REWARD_NEED_VALUE);
         }
         else
         {
            saveBrain();
         }
      }


      // Clear state.
      public void clear()
      {
         _robotBaseEntity    = null;
         _initialPosition    = new Vector3();
         _dimensions         = new Vector3();
         _lrfData            = null;
         _lrfForwardDistance = 0;
         _lrfLeftDistance    = 0;
         _lrfRightDistance   = 0;
         _webcamFrame        = null;
         _bumperHit          = false;
         _minDriveDistance   = DEFAULT_MIN_DRIVE_DISTANCE;
         _maxDriveDistance   = DEFAULT_MAX_DRIVE_DISTANCE;
         _driveSpeed         = DEFAULT_DRIVE_SPEED;
         _minTurnAngle       = DEFAULT_MIN_TURN_ANGLE;
         _turnSpeed          = DEFAULT_TURN_SPEED;
         _velocity           = 0;
         _driveState         = null;
         _x                 = _y = _orientation = 0.0f;
         _moving            = false;
         _creep             = false;
         _creepRange        = 0.0;
         _creepDistance     = 0.0;
         _maxCreepDistance  = DEFAULT_MAX_CREEP_DISTANCE;
         _align             = false;
         _maxAlignRange     = DEFAULT_MAX_ALIGN_RANGE;
         _maxAlignTurnAngle = DEFAULT_MAX_ALIGN_TURN_ANGLE;
         _sensors           = null;
         _sensors           = new float[(int)SENSOR_CONFIG.NUM_SENSORS];
         _lastSensors       = new float[(int)SENSOR_CONFIG.NUM_SENSORS];
         for (int i = 0; i < (int)SENSOR_CONFIG.NUM_SENSORS; i++)
         {
            _sensors[i] = _lastSensors[i] = 0.0f;
         }
         _response       = RESPONSE_TYPE.NULL_RESPONSE;
         _brain          = null;
         _pause          = true;
         _step           = false;
         _driver         = DRIVER_TYPE.ATANI;
         _manualResponse = RESPONSE_TYPE.NULL_RESPONSE;
      }


      // Create brain.
      public void createBrain(int randomSeed)
      {
         _brain = new Mona((int)SENSOR_CONFIG.NUM_SENSORS,
                           (int)RESPONSE_TYPE.NUM_RESPONSES,
                           (int)NEED_TYPE.NUM_NEEDS, randomSeed);

         bool[] sensorMask = new bool[(int)SENSOR_CONFIG.NUM_SENSORS];
         for (int i = 0; i < (int)SENSOR_CONFIG.NUM_SENSORS; i++)
         {
            sensorMask[i] = true;
         }
         _brain.addSensorMode(sensorMask, SENSOR_RESOLUTION);
         for (int i = 0; i < (int)SENSOR_CONFIG.NUM_SENSORS; i++)
         {
            if (i < (int)SENSOR_CONFIG.WEBCAM_SENSOR_INDEX)
            {
               sensorMask[i] = true;
            }
            else
            {
               sensorMask[i] = false;
            }
         }
         _brain.addSensorMode(sensorMask, SENSOR_RESOLUTION);
         for (int i = 0; i < (int)SENSOR_CONFIG.NUM_SENSORS; i++)
         {
            if (i < (int)SENSOR_CONFIG.WEBCAM_SENSOR_INDEX)
            {
               sensorMask[i] = false;
            }
            else if (i < (int)SENSOR_CONFIG.BUMPER_SENSOR_INDEX)
            {
               sensorMask[i] = true;
            }
            else
            {
               sensorMask[i] = false;
            }
         }
         _brain.addSensorMode(sensorMask, SENSOR_RESOLUTION);
         for (int i = 0; i < (int)SENSOR_CONFIG.NUM_SENSORS; i++)
         {
            if (i < (int)SENSOR_CONFIG.BUMPER_SENSOR_INDEX)
            {
               sensorMask[i] = false;
            }
            else
            {
               sensorMask[i] = true;
            }
         }
         _brain.addSensorMode(sensorMask, SENSOR_RESOLUTION);

         // Add needs.
         setSuccessNeed(SUCCESS_NEED_VALUE);
         setRewardNeed(REWARD_NEED_VALUE);

         // Add goal for success.
         float[] sensors = new float[(int)SENSOR_CONFIG.NUM_SENSORS];
         for (int i = 0; i < (int)SENSOR_CONFIG.NUM_SENSORS; i++)
         {
            sensors[i] = 0.0f;
         }
         _brain.addGoal((int)NEED_TYPE.SUCCESS,
                        sensors, (int)SENSOR_CONFIG.WEBCAM_SENSOR_MODE,
                        (int)RESPONSE_TYPE.NULL_RESPONSE, SUCCESS_NEED_VALUE);
      }


      // Save previous sensors for reward purposes.
      public void saveSensors()
      {
         for (int i = 0; i < (int)SENSOR_CONFIG.NUM_SENSORS; i++)
         {
            _lastSensors[i] = _sensors[i];
         }
      }


      // Prepare sensors from LRF data.
      public void lrfPrepareSensors(sicklrf.State lrfData)
      {
         _lrfForwardDistance = LRF_MAX_RANGE;
         _lrfLeftDistance    = LRF_MAX_RANGE;
         _lrfRightDistance   = LRF_MAX_RANGE;

         if ((lrfData == null) || (lrfData.DistanceMeasurements == null)) { return; }

         // Get distances to nearest obstacles.
         lrfGetDistances(lrfData);

         // Set sensor values to distances.
         _sensors[(int)SENSOR_CONFIG.LRF_SENSOR_INDEX] =
            Math.Min((float)_lrfLeftDistance / (float)LRF_MAX_RANGE, 1.0f);
         _sensors[(int)SENSOR_CONFIG.LRF_SENSOR_INDEX + 1] =
            Math.Min((float)_lrfForwardDistance / (float)LRF_MAX_RANGE, 1.0f);
         _sensors[(int)SENSOR_CONFIG.LRF_SENSOR_INDEX + 2] =
            Math.Min((float)_lrfRightDistance / (float)LRF_MAX_RANGE, 1.0f);
      }


      // Get distances to nearest obstacles
      // in the forward, left, and right directions.
      void lrfGetDistances(sicklrf.State lrfData)
      {
         lrfGetDistances(lrfData.DistanceMeasurements,
                         lrfData.AngularRange, ref _lrfLeftDistance,
                         ref _lrfForwardDistance, ref _lrfRightDistance);
      }


      // Get distances to nearest obstacles
      // in the forward, left, and right directions.
      void lrfGetDistances(int[] distanceMeasurements,
                           int angularRange, ref int leftDistance,
                           ref int forwardDistance, ref int rightDistance)
      {
         // Get the half-width of the robot in mm.
         if (_dimensions.X == 0.0f)
         {
            // Assumes dimensions at index of 1 in the shapes list.
            List<Shape> shapes = _robotBaseEntity.State.PhysicsPrimitives;
            if (shapes.Count < 1) { return; }
            _dimensions.X = shapes[1].State.Dimensions.X;
            _dimensions.Y = shapes[1].State.Dimensions.Y;
            _dimensions.Z = shapes[1].State.Dimensions.Z;
         }
         float baseWidth = _dimensions.X * 500.0f;

         int   numMeasures = distanceMeasurements.Length;
         float angleDelta  = 0.0f;
         if (numMeasures > 1)
         {
            angleDelta = (float)angularRange / (float)(numMeasures - 1);
         }
         float rangeLow  = (float)-angularRange / 2.0f;
         float rangeHigh = (float)angularRange / 2.0f;
         float angle, x, y;
         int   range;

         // Compute distances.
         for (int i = 0; i < numMeasures; i++)
         {
            angle = rangeLow + (angleDelta * (float)i);
            angle = angle * (float)Math.PI / 180.0f;
            range = distanceMeasurements[i];
            if (range == 0) { range = LRF_MAX_RANGE; }
            x = (float)range * (float)Math.Sin(angle);
            y = (float)range * (float)Math.Cos(angle);

            // Forward distance.
            if (Math.Abs(x) <= baseWidth)
            {
               if ((int)Math.Abs(y) < forwardDistance)
               {
                  forwardDistance = (int)Math.Abs(y);
               }
            }

            // Left distance.
            if ((angle > 0.0f) && (Math.Abs(y) <= baseWidth))
            {
               if ((int)Math.Abs(x) < leftDistance)
               {
                  leftDistance = (int)Math.Abs(x);
               }
            }

            // Right distance.
            if ((angle < 0.0f) && (Math.Abs(y) <= baseWidth))
            {
               if ((int)Math.Abs(x) < rightDistance)
               {
                  rightDistance = (int)Math.Abs(x);
               }
            }
         }
      }


      // Prepare sensors from webcam data.
      public void webcamPrepareSensors(webcam.QueryFrameResponse webcamFrame)
      {
         if ((webcamFrame == null) || (webcamFrame.Format != Guid.Empty)) { return; }

         // Get RGB average.
         int    i, x, y, n;
         int[]  rgb = new int[3];
         Bitmap bmp = makeBitmap(
            webcamFrame.Size.Width,
            webcamFrame.Size.Height,
            webcamFrame.Frame);
         BitmapData bmd = bmp.LockBits(
            new Rectangle(0, 0, bmp.Width, bmp.Height),
            ImageLockMode.ReadOnly,
            PixelFormat.Format24bppRgb
            );
         for (y = 0; y < bmd.Height; y++)
         {
            for (x = 0; x < bmd.Width; x++)
            {
               for (i = 0; i < 3; i++)
               {
                  rgb[i] += Marshal.ReadByte(bmd.Scan0, (bmd.Stride * y) + (3 * x) + i);
               }
            }
         }
         n = bmd.Height * bmd.Width;
         for (i = 0; i < 3; i++)
         {
            rgb[i] /= n;
         }
         bmp.UnlockBits(bmd);

         // Set sensory values.
         for (i = 0; i < (int)SENSOR_CONFIG.NUM_WEBCAM_SENSORS; i++)
         {
            _sensors[(int)SENSOR_CONFIG.WEBCAM_SENSOR_INDEX + i] = (float)rgb[i] / 255.0f;
         }
      }


      Bitmap makeBitmap(int width, int height, byte[] imageData)
      {
         Bitmap bmp = new Bitmap(width, height, PixelFormat.Format24bppRgb);

         BitmapData data = bmp.LockBits(
            new Rectangle(0, 0, bmp.Width, bmp.Height),
            ImageLockMode.WriteOnly,
            PixelFormat.Format24bppRgb
            );

         Marshal.Copy(imageData, 0, data.Scan0, imageData.Length);

         bmp.UnlockBits(data);

         return(bmp);
      }


      // Prepare sensors from bumper data.
      public void bumperPrepareSensors()
      {
         if (_bumperHit)
         {
            _sensors[(int)SENSOR_CONFIG.BUMPER_SENSOR_INDEX] = 1.0f;
         }
         else
         {
            _sensors[(int)SENSOR_CONFIG.BUMPER_SENSOR_INDEX] = 0.0f;
         }
      }


      // Get forward driving distance in meters using LRF forward range.
      public double getDriveDistance()
      {
         return(getDriveDistance((double)_lrfForwardDistance / 1000.0));
      }


      // Get forward driving distance in meters given a forward range.
      public double getDriveDistance(double range)
      {
         double distance = 0.0;

         if (_dimensions.Z > 0.0f)
         {
            distance = range - _dimensions.Z;
            if (distance <= _dimensions.Z)
            {
               distance = 0.0;
            }
            if (distance > _maxDriveDistance)
            {
               distance = _maxDriveDistance;

               // Approach obstacle that is close ahead.
               if (distance < range)
               {
                  double d = getDriveDistance(range - distance);
                  if (d < (_maxDriveDistance * 0.5))
                  {
                     distance += d;
                  }
               }
            }
         }
         if (distance < _minDriveDistance) { distance = 0.0; }
         return(distance);
      }


      // Get creep ahead distance when drive distance is too short.
      public double getCreepDistance()
      {
         // One-time creep.
         _creep = false;

         double currentRange   = (double)_lrfForwardDistance / 1000.0;
         double drivenDistance = _creepRange - currentRange;
         double distance       = _creepDistance - drivenDistance;
         if (Math.Abs(distance) < _minDriveDistance) { distance = 0.0; }
         if (Math.Abs(distance) > _maxCreepDistance) { distance = _maxCreepDistance; }
         return(distance);
      }


      // Get turn angle that aligns with nearby wall.
      public double getAlignment(sicklrf.State lrfData)
      {
         int    i, numMeasures;
         double angleDelta, angle;
         double a, b, a2, b2, c, C, B, s;

         // One-time alignment.
         _align = false;

         // Get number of measures and angle delta.
         if ((numMeasures = lrfData.DistanceMeasurements.Length) < 2) { return(0.0); }
         angleDelta = (double)lrfData.AngularRange / (double)(numMeasures - 1);

         // Get and validate triangle SAS (aCb) measurements for side
         // closer to wall. Angle C approximates 45 degrees.
         a  = (double)lrfData.DistanceMeasurements[0] / 1000.0;
         i  = (int)((double)numMeasures * 0.25);
         b  = (double)lrfData.DistanceMeasurements[i] / 1000.0;
         C  = angleDelta * (double)i;
         a2 = (double)lrfData.DistanceMeasurements[numMeasures - 1] / 1000.0;
         i  = (int)((double)numMeasures * 0.75);
         b2 = (double)lrfData.DistanceMeasurements[i] / 1000.0;
         s  = -1.0;
         if ((a <= _maxAlignRange) && (b <= _maxAlignRange))
         {
            if ((a2 <= _maxAlignRange) && (b2 <= _maxAlignRange))
            {
               if (a > a2)
               {
                  a = a2;
                  b = b2;
                  s = 1.0;
               }
            }
         }
         else
         {
            if ((a2 <= _maxAlignRange) && (b2 <= _maxAlignRange))
            {
               a = a2;
               b = b2;
               s = 1.0;
            }
            else
            {
               return(0.0);
            }
         }

         // Calculate remaining side and angle.
         c = Math.Sqrt((b * b) + (a * a) -
                       (2.0 * b * a * Math.Cos(C * Math.PI / 180.0)));
         B = Math.Acos(((a * a) + (c * c) - (b * b)) / (2.0 * a * c));
         B = Math.Abs(B * 180.0 / Math.PI);

         // Determine turn angle to align with wall.
         angle = (B - 90.0) * s;
         if (Math.Abs(angle) > _maxAlignTurnAngle)
         {
            if (angle < 0.0)
            {
               angle = -_maxAlignTurnAngle;
            }
            else
            {
               angle = _maxAlignTurnAngle;
            }
         }
         if (Math.Abs(angle) < _minTurnAngle) { angle = 0.0; }
         return(angle);
      }


      // Sensor/response cycle.
      public void cycleBrain()
      {
         _response = (RESPONSE_TYPE)_brain.cycle(_sensors);
      }


      // Override response.
      public void overrideResponse(RESPONSE_TYPE response)
      {
         _brain.overrideResponse((int)response);
      }


      // Get/set needs.
      public double getSuccessNeed()
      {
         return(_brain.getNeed((int)NEED_TYPE.SUCCESS));
      }


      public void setSuccessNeed(double value)
      {
         _brain.setNeed((int)NEED_TYPE.SUCCESS, value);
      }


      public double getRewardNeed()
      {
         return(_brain.getNeed((int)NEED_TYPE.REWARD));
      }


      public void setRewardNeed(double value)
      {
         _brain.setNeed((int)NEED_TYPE.REWARD, value);
      }


      // Reward action.
      public void rewardAction(int action, double reward)
      {
         int goalIndex;

         switch ((REWARD_ACTION)action)
         {
         case REWARD_ACTION.CREATE:
            _brain.addGoal((int)NEED_TYPE.REWARD,
                           _lastSensors, 0, (int)_response, reward);
            break;

         case REWARD_ACTION.DISABLE:
            goalIndex = _brain.findGoal((int)NEED_TYPE.REWARD,
                                        _lastSensors, 0, (int)_response);
            if (goalIndex != -1)
            {
               _brain.disableGoal((int)NEED_TYPE.REWARD, goalIndex);
            }
            break;

         case REWARD_ACTION.ENABLE_ALL:
            int i = _brain.getNumGoals((int)NEED_TYPE.REWARD);
            for (goalIndex = 0; goalIndex < i; goalIndex++)
            {
               _brain.enableGoal((int)NEED_TYPE.REWARD, goalIndex);
            }
            break;

         case REWARD_ACTION.REMOVE:
            goalIndex = _brain.findGoal((int)NEED_TYPE.REWARD,
                                        _lastSensors, 0, (int)_response);
            if (goalIndex != -1)
            {
               _brain.removeGoal((int)NEED_TYPE.REWARD, goalIndex);
            }
            break;

         case REWARD_ACTION.REMOVE_ALL:
            while (_brain.getNumGoals((int)NEED_TYPE.REWARD) > 0)
            {
               _brain.removeGoal((int)NEED_TYPE.REWARD, 0);
            }
            break;
         }
      }


      // Load brain.
      public bool loadBrain()
      {
         return(_brain.load(BRAIN_SAVE_FILE));
      }


      // Save brain.
      public bool saveBrain()
      {
         return(_brain.save(BRAIN_SAVE_FILE));
      }


      // Clear brain.
      public void clearBrain()
      {
         _brain.clear();
      }


      // Print brain to file (XML).
      public bool printBrain()
      {
         return(_brain.print(BRAIN_PRINT_FILE));
      }
   }
}
