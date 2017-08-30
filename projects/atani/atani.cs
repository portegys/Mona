//-----------------------------------------------------------------------
//  Atani neural network controlled robot.
//
//  $File: atani.cs $ $Revision: 1 $
//
//  An atani is a robotic entity controlled by a mona neural network.
//-----------------------------------------------------------------------

#region Namespaces
using Microsoft.Ccr.Core;
using Microsoft.Ccr.Adapters.WinForms;
using Microsoft.Dss.Core;
using Microsoft.Dss.Core.Attributes;
using Microsoft.Dss.ServiceModel.Dssp;
using Microsoft.Dss.ServiceModel.DsspServiceBase;
using System;
using System.IO;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using System.Threading;
using Microsoft.Robotics.Simulation;
using Microsoft.Robotics.Simulation.Engine;
using Microsoft.Robotics.Simulation.Physics;
using Microsoft.Robotics.PhysicalModel;
using W3C.Soap;
using dssp        = Microsoft.Dss.ServiceModel.Dssp;
using engineproxy = Microsoft.Robotics.Simulation.Engine.Proxy;
using simdrive    = Microsoft.Robotics.Services.Simulation.Drive.Proxy;
using drive       = Microsoft.Robotics.Services.Drive.Proxy;
using simlrf      = Microsoft.Robotics.Services.Simulation.Sensors.LaserRangeFinder.Proxy;
using sicklrf     = Microsoft.Robotics.Services.Sensors.SickLRF.Proxy;
using simbumper   = Microsoft.Robotics.Services.Simulation.Sensors.Bumper.Proxy;
using bumper      = Microsoft.Robotics.Services.ContactSensor.Proxy;
using simwebcam   = Microsoft.Robotics.Services.Simulation.Sensors.SimulatedWebcam.Proxy;
using webcam      = Microsoft.Robotics.Services.WebCam.Proxy;
using pioneer3DXrobotDashboard = Robotics.Atani.Pioneer3DXrobotDashboard.Proxy;
using xna = Microsoft.Xna.Framework;
#endregion

namespace Robotics.Atani
{
   [DisplayName("Atani")]
   [Description("Atani Service")]
   [Contract(Contract.Identifier)]
   public class Atani : DsspServiceBase
   {
      #region Parameters and constants

      // Interval between heartbeat timer notifications.
      public const int HEARTBEAT_INTERVAL = 100;   // msec
      #endregion

      #region Inititalization

      // Config folder.
      const string ConfigFolder = "/apps/atani/config/";

      // Config URI.
      const string InitialStateUri = ServicePaths.MountPoint + @ConfigFolder + "atani.config.xml";

      // Using the InitialStatePartner attribute allows settings
      // to be read from a config file.
      [ServiceState]
      [InitialStatePartner(Optional = true, ServiceUri = InitialStateUri)]
      AtaniState _state = null;

      // Partner attribute will cause simulation engine service to start.
      [Partner("Engine",
               Contract = engineproxy.Contract.Identifier,
               CreationPolicy = PartnerCreationPolicy.UseExistingOrCreate)]
      engineproxy.SimulationEnginePort _engineServicePort =
         new engineproxy.SimulationEnginePort();

      // Main service port.
      [ServicePort("/atani", AllowMultipleInstances = false)]
      AtaniOperations _mainPort = new AtaniOperations();

      // Pioneer3DX robot service creation ports.
      DsspResponsePort<CreateResponse> _pioneer3DXmotorCreatePort  = null;
      DsspResponsePort<CreateResponse> _pioneer3DXlrfCreatePort    = null;
      DsspResponsePort<CreateResponse> _pioneer3DXwebcamCreatePort = null;
      DsspResponsePort<CreateResponse> _pioneer3DXbumperCreatePort = null;

      // Pioneer3DX partner ports.
      drive.DriveOperations               _pioneer3DXdrivePort     = null;
      drive.DriveOperations               _pioneer3DXdriveNotify   = new drive.DriveOperations();
      Port<Shutdown>                      _pioneer3DXmotorShutdown = new Port<Shutdown>();
      sicklrf.SickLRFOperations           _pioneer3DXlrfPort       = null;
      sicklrf.SickLRFOperations           _pioneer3DXlrfNotify     = new sicklrf.SickLRFOperations();
      Port<Shutdown>                      _pioneer3DXlrfShutdown   = new Port<Shutdown>();
      webcam.WebCamOperations             _pioneer3DXwebcamPort    = null;
      webcam.WebCamOperations             _pioneer3DXwebcamNotify  = new webcam.WebCamOperations();
      bumper.ContactSensorArrayOperations _pioneer3DXbumperPort    = null;
      bumper.ContactSensorArrayOperations _pioneer3DXbumperNotify  = new bumper.ContactSensorArrayOperations();

      // Pioneer3DX dashboard service.
      DsspResponsePort<CreateResponse> _pioneer3DXrobotDashboardCreatePort = null;
      pioneer3DXrobotDashboard.Pioneer3DXrobotDashboardOperations _pioneer3DXrobotDashboardPort   = null;
      pioneer3DXrobotDashboard.Pioneer3DXrobotDashboardOperations _pioneer3DXrobotDashboardNotify =
         new pioneer3DXrobotDashboard.Pioneer3DXrobotDashboardOperations();

      // Worlds.
      BlockWorld _blockWorld;
      TmazeWorld _TmazeWorld;

      // Pioneer3DX T-maze guide.
      TmazeGuide _pioneer3DXtmazeGuide;

      // Constructor.
      public Atani(DsspServiceCreationPort creationPort) : base(creationPort) { }

      // Start service.
      protected override void Start()
      {
         if (_state == null)
         {
            _state = new AtaniState();
            base.SaveState(_state);
         }
         _state.init();
         base.Start();

         // Create entities.
         createEntities();

         // Subscribe to the Pioneer3DX robot services when created.
         Activate(Arbiter.ReceiveWithIterator<CreateResponse>(true, _pioneer3DXmotorCreatePort, subscribePioneer3DXmotor));
         Activate(Arbiter.ReceiveWithIterator<CreateResponse>(true, _pioneer3DXlrfCreatePort, subscribePioneer3DXlrf));
         Activate(Arbiter.ReceiveWithIterator<CreateResponse>(true, _pioneer3DXbumperCreatePort, subscribePioneer3DXbumper));
         Activate(Arbiter.ReceiveWithIterator<CreateResponse>(true, _pioneer3DXwebcamCreatePort, subscribePioneer3DXwebcam));
         Activate(Arbiter.ReceiveWithIterator<CreateResponse>(true, _pioneer3DXrobotDashboardCreatePort, subscribePioneer3DXrobotDashboard));

         // Handlers that need write or exclusive access to state go under
         // the exclusive group. Handlers that need read or shared access, and can be
         // concurrent to other readers, go to the concurrent group.
         Activate(Arbiter.Interleave(
                     new TeardownReceiverGroup
                     (
                        Arbiter.Receive<DsspDefaultDrop>(false, _mainPort, DropHandler)
                     ),
                     new ExclusiveReceiverGroup
                     (
                        Arbiter.Receive<pioneer3DXrobotDashboard.NeedChange>(true, _pioneer3DXrobotDashboardNotify, pioneer3DXrobotDashboardNeedChangeHandler),
                        Arbiter.Receive<pioneer3DXrobotDashboard.ControlAction>(true, _pioneer3DXrobotDashboardNotify, pioneer3DXrobotDashboardControlActionHandler),
                        Arbiter.Receive<pioneer3DXrobotDashboard.RewardAction>(true, _pioneer3DXrobotDashboardNotify, pioneer3DXrobotDashboardRewardActionHandler)
                     ),
                     new ConcurrentReceiverGroup
                     (
                        Arbiter.Receive<DsspDefaultLookup>(true, _mainPort, DefaultLookupHandler),
                        Arbiter.Receive<Get>(true, _mainPort, GetHandler)
                     )
                     ));

         // These are non-persistent activations to prevent event overloading;
         // they are re-activated by their handlers after consuming the event queue.
         Activate(Arbiter.Receive<drive.Update>(false, _pioneer3DXdriveNotify, pioneer3DXdriveUpdateNotificationHandler));
         Activate(Arbiter.Receive<sicklrf.Replace>(false, _pioneer3DXlrfNotify, pioneer3DXlrfReplaceNotificationHandler));
         Activate(Arbiter.Receive<sicklrf.Reset>(false, _pioneer3DXlrfNotify, pioneer3DXlrfResetNotificationHandler));
         Activate(Arbiter.Receive<webcam.Replace>(false, _pioneer3DXwebcamNotify, pioneer3DXwebcamReplaceNotificationHandler));
         Activate(Arbiter.ReceiveWithIterator<webcam.UpdateFrame>(false, _pioneer3DXwebcamNotify, pioneer3DXwebcamUpdateFrameNotificationHandler));
         Activate(Arbiter.Receive<bumper.Replace>(false, _pioneer3DXbumperNotify, pioneer3DXbumperReplaceNotificationHandler));
         Activate(Arbiter.Receive<bumper.Update>(false, _pioneer3DXbumperNotify, pioneer3DXbumperUpdateNotificationHandler));
         Activate(Arbiter.Receive<Pioneer3DXinvokeBrain>(false, _mainPort, pioneer3DXbrainInvocationHandler));

         // Heartbeat timer.
         Activate(Arbiter.Receive<HeartbeatUpdate>(true, _mainPort, heartbeatUpdateHandler));

         // Start heartbeat timer.
         _mainPort.Post(new HeartbeatUpdate(new HeartbeatUpdateRequest(DateTime.Now)));
      }


      #endregion

      #region Entities

      // Create entities.
      private void createEntities()
      {
         Vector3 pioneer3DXRobotPosition = new Vector3(1, 0.1f, 0);
         Vector3 legoNxtRobotPosition    = new Vector3(2, 0.1f, 0);
         Vector3 IRobotRobotPosition     = new Vector3(3, 0.1f, 0);

         // Create world.
         switch (_state._worldType)
         {
         case AtaniState.WORLD_TYPE.BLOCK_WORLD:
            _blockWorld = new BlockWorld();
            _blockWorld.create();
            break;

         case AtaniState.WORLD_TYPE.TMAZE_WORLD:
            _TmazeWorld = new TmazeWorld(_state.TMAZE_WIDTH, _state.TMAZE_HEIGHT, _state._random);
            _TmazeWorld.create();
            if (_TmazeWorld.MazePath.Count > 2)
            {
               pioneer3DXRobotPosition =
                  new Vector3(
                     _TmazeWorld.MazePath[0].X * _TmazeWorld.BlockSize,
                     0.1f * _TmazeWorld.BlockHeight,
                     _TmazeWorld.MazePath[0].Y * _TmazeWorld.BlockSize);
               _pioneer3DXtmazeGuide =
                  new TmazeGuide(ref _TmazeWorld.MazePath,
                                 _TmazeWorld.MazePath[0], TmazeWorld.DIRECTION.NORTH);

               legoNxtRobotPosition =
                  new Vector3(
                     _TmazeWorld.MazePath[1].X * _TmazeWorld.BlockSize,
                     0.1f * _TmazeWorld.BlockHeight,
                     _TmazeWorld.MazePath[1].Y * _TmazeWorld.BlockSize);

               IRobotRobotPosition =
                  new Vector3(
                     _TmazeWorld.MazePath[2].X * _TmazeWorld.BlockSize,
                     0.1f * _TmazeWorld.BlockHeight,
                     _TmazeWorld.MazePath[2].Y * _TmazeWorld.BlockSize);
            }
            break;
         }

         // Create robots.
         if (_state.PIONEER3DX_ROBOT)
         {
            createPioneer3DXRobot(pioneer3DXRobotPosition);
            createPioneer3DXrobotDashboard();
         }
         if (_state.LEGO_NXT_ROBOT)
         {
            createLegoNxtRobot(legoNxtRobotPosition);
         }
         if (_state.IROBOT)
         {
            createIRobotRobot(IRobotRobotPosition);
         }
      }


      #endregion

      #region Robots

      #region Pioneer3DX

      // Create the Pioneer3DX robot.
      void createPioneer3DXRobot(Vector3 position)
      {
         _state._pioneer3DXrobotState._initialPosition = position;
         _state._pioneer3DXrobotState._robotBaseEntity =
            createPioneer3DXmotorBase();

         // Create the laser rangefinder service.
         LaserRangeFinderEntity lrf = createPioneer3DXlaserRangeFinder();
         _state._pioneer3DXrobotState._robotBaseEntity.InsertEntity(lrf);

         // Create the robot camera service.
         CameraEntity camera = createPioneer3DXcamera();
         _state._pioneer3DXrobotState._robotBaseEntity.InsertEntity(camera);
         CameraSprite camSprite = new CameraSprite(0.5f, 0.5f, SpritePivotType.Center, 0, new Vector3(0, 1, 0));
         camSprite.State.Name = "camSprite";
         camSprite.Flags     |= VisualEntityProperties.DisableBackfaceCulling;
         camera.InsertEntity(camSprite);

         // Create bumper array service.
         BumperArrayEntity bumperArray = createPioneer3DXbumperArray();
         _state._pioneer3DXrobotState._robotBaseEntity.InsertEntity(bumperArray);

         // Insert the motor base into the simulation.
         SimulationEngine.GlobalInstancePort.Insert(_state._pioneer3DXrobotState._robotBaseEntity);
      }


      // Create the Pioneer3DX motor base.
      private Pioneer3DX createPioneer3DXmotorBase()
      {
         // Use supplied entity that creates a motor base
         // with two active wheels and one caster.
         Pioneer3DX robotBaseEntity =
            new Pioneer3DX(_state._pioneer3DXrobotState._initialPosition);
         Vector3 rotation = new Vector3(0.0f, 0.0f, 0.0f);

         robotBaseEntity.RotationAngles = rotation;

         // Specify mesh.
         robotBaseEntity.State.Assets.Mesh = "Pioneer3dx.bos";
         robotBaseEntity.WheelMesh         = "PioneerWheel.bos";

         // specify color if no mesh is specified.
         robotBaseEntity.ChassisShape.State.DiffuseColor = new Vector4(0.8f, 0.25f, 0.25f, 1.0f);

         // The name must match manifest.
         robotBaseEntity.State.Name = "P3DXMotorBase";

         // Start simulated arcos motor service.
         _pioneer3DXmotorCreatePort =
            simdrive.Contract.CreateService(ConstructorPort,
                                            Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
                                               "http://localhost/" + robotBaseEntity.State.Name)
                                            );
         if (_pioneer3DXmotorCreatePort == null)
         {
            LogError("Cannot create P3DXMotorBase service");
         }
         return(robotBaseEntity);
      }


      // Subscribe to the Pioneer3DX motor service and enable it.
      IEnumerator<ITask> subscribePioneer3DXmotor(CreateResponse createResponse)
      {
         string service = createResponse.Service + "/drive";

         _pioneer3DXdrivePort = ServiceForwarder<drive.DriveOperations>(service);

         drive.ReliableSubscribe subscribe = new drive.ReliableSubscribe(
            new ReliableSubscribeRequestType(10)
            );
         subscribe.NotificationPort         = _pioneer3DXdriveNotify;
         subscribe.NotificationShutdownPort = _pioneer3DXmotorShutdown;
         _pioneer3DXdrivePort.Post(subscribe);

         bool subOK = false;
         yield return(Arbiter.Choice(
                         subscribe.ResponsePort,
                         delegate(SubscribeResponseType subscribeResponse)
                         {
                            subOK = true;
                         },
                         delegate(Fault fault)
                         {
                            LogError("Cannot subscribe to P3DXMotorBase service", fault);
                         }
                         ));

         if (!subOK) { yield break; }

         // Enable the motor.
         drive.EnableDriveRequest request = new drive.EnableDriveRequest(true);
         yield return(Arbiter.Choice(
                         _pioneer3DXdrivePort.EnableDrive(request),
                         delegate(DefaultUpdateResponseType response) { },
                         delegate(Fault fault)
                         {
                            LogError("Cannot enable P3DXMotorBase motor", fault);
                         }
                         ));

         yield break;
      }


      // Motor notification handler.
      void pioneer3DXdriveUpdateNotificationHandler(drive.Update update)
      {
         // Get most recent update, discarding older ones.
         drive.Update testUpdate;
         while ((testUpdate = _pioneer3DXdriveNotify.Test<drive.Update>()) != null)
         {
            update = testUpdate;
         }

         drive.DriveDifferentialTwoWheelState s = update.Body;
         float w = s.LeftWheel.MotorState.Pose.Orientation.W;
         _state._pioneer3DXrobotState._driveState = s;
         _state._pioneer3DXrobotState._velocity   =
            (velocityFromWheel(s.LeftWheel) + velocityFromWheel(s.RightWheel)) / 2;

         // Update the pose.
         // NOTE: This only gets updated when something
         // changes, not continuously, so it is usually out of date.
         _state._pioneer3DXrobotState._x = (s.LeftWheel.MotorState.Pose.Position.X +
                                            s.RightWheel.MotorState.Pose.Position.X) / 2;
         _state._pioneer3DXrobotState._y = (s.LeftWheel.MotorState.Pose.Position.Z +
                                            s.RightWheel.MotorState.Pose.Position.Z) / 2;
         _state._pioneer3DXrobotState._orientation = (float)(Math.Acos(w) * 2 * 180 / Math.PI);
         if (s.LeftWheel.MotorState.Pose.Orientation.Y < 0)
         {
            _state._pioneer3DXrobotState._orientation = -_state._pioneer3DXrobotState._orientation;
         }

         // Re-activate handler.
         Activate(Arbiter.Receive<drive.Update>(false, _pioneer3DXdriveNotify, pioneer3DXdriveUpdateNotificationHandler));
      }


      // Computes the wheel velocity in mm/s.
      int velocityFromWheel(Microsoft.Robotics.Services.Motor.Proxy.WheeledMotorState wheel)
      {
         if (wheel == null)
         {
            return(0);
         }
         return((int)(1000 * wheel.WheelSpeed));   // meters to millimeters
      }


      // Sets the forward velocity of the drive.
      PortSet<DefaultUpdateResponseType, Fault> pioneer3DXmoveForward(double speed)
      {
         drive.SetDriveSpeedRequest request = new drive.SetDriveSpeedRequest();
         request.LeftWheelSpeed  = speed;
         request.RightWheelSpeed = speed;
         return(_pioneer3DXdrivePort.SetDriveSpeed(request));
      }


      // Moves the drive forward for the specified distance.
      PortSet<DefaultUpdateResponseType, Fault> pioneer3DXdriveDistance(double distance, double speed)
      {
         drive.DriveDistanceRequest request = new drive.DriveDistanceRequest(distance, speed);
         return(_pioneer3DXdrivePort.DriveDistance(request));
      }


      // Sets the velocity of the drive to 0.
      PortSet<DefaultUpdateResponseType, Fault> pioneer3DXstopMoving()
      {
         return(pioneer3DXmoveForward(0.0));
      }


      // Turns the drive relative to its current heading.
      PortSet<DefaultUpdateResponseType, Fault> pioneer3DXturn(double angle, double speed)
      {
         drive.RotateDegreesRequest request = new drive.RotateDegreesRequest(angle, speed);
         return(_pioneer3DXdrivePort.RotateDegrees(request));
      }


      // Sets the turning velocity to 0.
      PortSet<DefaultUpdateResponseType, Fault> pioneer3DXstopTurning()
      {
         return(pioneer3DXturn(0.0, 0.0));
      }


      // Enables the drive.
      PortSet<DefaultUpdateResponseType, Fault> pioneer3DXenableMotor()
      {
         return(pioneer3DXenableMotor(true));
      }


      // Disables the drive.
      PortSet<DefaultUpdateResponseType, Fault> pioneer3DXdisableMotor()
      {
         return(pioneer3DXenableMotor(false));
      }


      // Sets the drive enabled state.
      PortSet<DefaultUpdateResponseType, Fault> pioneer3DXenableMotor(bool enable)
      {
         drive.EnableDriveRequest request = new drive.EnableDriveRequest(enable);
         return(_pioneer3DXdrivePort.EnableDrive(request));
      }


      // Create the Pioneer3DX laser rangefinder.
      LaserRangeFinderEntity createPioneer3DXlaserRangeFinder()
      {
         // Create a Laser Range Finder Entity.
         // Place it 30cm above base CenterofMass.
         LaserRangeFinderEntity lrf = new LaserRangeFinderEntity(
            new Pose(new Vector3(0, 0.30f, 0)));

         lrf.State.Name = "P3DXLaserRangeFinder";
         lrf.LaserBox.State.DiffuseColor = new Vector4(0.25f, 0.25f, 0.8f, 1.0f);

         // Create LaserRangeFinder simulated service.
         _pioneer3DXlrfCreatePort =
            simlrf.Contract.CreateService(
               ConstructorPort,
               Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
                  "http://localhost/" + lrf.State.Name));
         if (_pioneer3DXlrfCreatePort == null)
         {
            LogError("Cannot create P3DXLaserRangeFinder service");
         }
         return(lrf);
      }


      // Subscribe to the laser rangefinder service.
      IEnumerator<ITask> subscribePioneer3DXlrf(CreateResponse createResponse)
      {
         string service = createResponse.Service + "/sicklrf";

         _pioneer3DXlrfPort = ServiceForwarder<sicklrf.SickLRFOperations>(service);

         sicklrf.ReliableSubscribe subscribe = new sicklrf.ReliableSubscribe(
            new ReliableSubscribeRequestType(5)
            );
         subscribe.NotificationPort         = _pioneer3DXlrfNotify;
         subscribe.NotificationShutdownPort = _pioneer3DXlrfShutdown;

         _pioneer3DXlrfPort.Post(subscribe);

         yield return(Arbiter.Choice(
                         subscribe.ResponsePort,
                         delegate(SubscribeResponseType response)
                         {
                         },
                         delegate(Fault fault)
                         {
                            LogError("Cannot subscribe to P3DXLaserRangeFinder service", fault);
                         }
                         ));

         yield break;
      }


      // Laser rangefinder replace notification handler.
      void pioneer3DXlrfReplaceNotificationHandler(sicklrf.Replace replace)
      {
         // Get most recent reading, discarding older ones.
         _state._pioneer3DXrobotState._lrfData = replace.Body;
         sicklrf.Replace testReplace;
         while ((testReplace = _pioneer3DXlrfNotify.Test<sicklrf.Replace>()) != null)
         {
            _state._pioneer3DXrobotState._lrfData = testReplace.Body;
         }

         if ((_state._pioneer3DXrobotState._lrfData == null) ||
             (_state._pioneer3DXrobotState._lrfData.DistanceMeasurements == null) ||
             (_state._pioneer3DXrobotState._lrfData.DistanceMeasurements.Length == 0))
         {
            _state._pioneer3DXrobotState._lrfData = null;
         }
         else
         {
            // Let brain process LRF data.
            _mainPort.Post(new Pioneer3DXinvokeBrain());
         }

         // Re-activate handler.
         Activate(Arbiter.Receive<sicklrf.Replace>(false, _pioneer3DXlrfNotify, pioneer3DXlrfReplaceNotificationHandler));
      }


      // Laser rangefinder reset notification handler.
      void pioneer3DXlrfResetNotificationHandler(sicklrf.Reset reset)
      {
         // Consume events and re-activate handler.
         while (_pioneer3DXlrfNotify.Test<sicklrf.Reset>() != null) { }
         Activate(Arbiter.Receive<sicklrf.Reset>(false, _pioneer3DXlrfNotify, pioneer3DXlrfResetNotificationHandler));
      }


      // Create the Pioneer3DX camera.
      CameraEntity createPioneer3DXcamera()
      {
         // Low resolution, wide field of view.
         CameraEntity cam = new CameraEntity(320, 240, CameraEntity.CameraModelType.AttachedChild);

         cam.State.Name = "robocam";

         // Position on top of the robot.
         cam.State.Pose.Position = new Vector3(0.0f, 0.5f, 0.0f);

         // Camera renders in an offline buffer at each frame
         // required for service.
         cam.IsRealTimeCamera = true;

         // Start simulated webcam service.
         _pioneer3DXwebcamCreatePort =
            simwebcam.Contract.CreateService(
               ConstructorPort,
               Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
                  "http://localhost/" + cam.State.Name)
               );
         if (_pioneer3DXwebcamCreatePort == null)
         {
            LogError("Cannot create Pioneer3DX webcam service");
         }
         return(cam);
      }


      // Subscribe to the Pioneer3DX webcam service.
      IEnumerator<ITask> subscribePioneer3DXwebcam(CreateResponse createResponse)
      {
         string service = createResponse.Service + "/webcamservice";

         _pioneer3DXwebcamPort = ServiceForwarder<webcam.WebCamOperations>(service);

         webcam.Subscribe subscribe = new webcam.Subscribe(
            new SubscribeRequestType()
            );
         subscribe.NotificationPort = _pioneer3DXwebcamNotify;

         _pioneer3DXwebcamPort.Post(subscribe);

         yield return(Arbiter.Choice(
                         subscribe.ResponsePort,
                         delegate(SubscribeResponseType response)
                         {
                         },
                         delegate(Fault fault)
                         {
                            LogError("Cannot subscribe to Pioneer3DX webcam service", fault);
                         }
                         ));

         yield break;
      }


      // Pioneer3DX webcam replace notification handler.
      void pioneer3DXwebcamReplaceNotificationHandler(webcam.Replace replace)
      {
         // Consume events and re-activate handler.
         while (_pioneer3DXwebcamNotify.Test<webcam.Replace>() != null) { }
         Activate(Arbiter.Receive<webcam.Replace>(false, _pioneer3DXwebcamNotify, pioneer3DXwebcamReplaceNotificationHandler));
      }


      // Pioneer3DX webcam frame update notification handler.
      IEnumerator<ITask> pioneer3DXwebcamUpdateFrameNotificationHandler(webcam.UpdateFrame update)
      {
         // Get most recent update, discarding older ones.
         webcam.UpdateFrame testUpdate;
         while ((testUpdate = _pioneer3DXwebcamNotify.Test<webcam.UpdateFrame>()) != null) { }
         {
            update = testUpdate;
         }

         // Get frame.
         _state._pioneer3DXrobotState._webcamFrame = null;
         Fault fault = null;
         yield return(Arbiter.Choice(
                         _pioneer3DXwebcamPort.QueryFrame(new webcam.QueryFrameRequest()),
                         delegate(webcam.QueryFrameResponse frame)
                         {
                            _state._pioneer3DXrobotState._webcamFrame = frame;
                         },
                         delegate(Fault f)
                         {
                            fault = f;
                         }
                         ));

         if (fault != null)
         {
            LogError("Failed to get frame from camera", fault);
            yield break;
         }

         // Let brain process webcam data.
         _mainPort.Post(new Pioneer3DXinvokeBrain());

         // Re-activate handler.
         Activate(Arbiter.ReceiveWithIterator<webcam.UpdateFrame>(false, _pioneer3DXwebcamNotify, pioneer3DXwebcamUpdateFrameNotificationHandler));
         yield break;
      }


      // Create the Pioneer3DX bumper.
      BumperArrayEntity createPioneer3DXbumperArray()
      {
         // Create a bumper array entity with two bumpers.
         BoxShape frontBumper = new BoxShape(
            new BoxShapeProperties("front",
                                   0.001f,
                                   new Pose(new Vector3(0, 0.05f, -0.25f)),
                                   new Vector3(0.40f, 0.03f, 0.03f)
                                   )
            );

         frontBumper.State.DiffuseColor = new Vector4(0.1f, 0.1f, 0.1f, 1.0f);

         BoxShape rearBumper = new BoxShape(
            new BoxShapeProperties("rear",
                                   0.001f,
                                   new Pose(new Vector3(0, 0.05f, 0.25f)),
                                   new Vector3(0.40f, 0.03f, 0.03f)
                                   )
            );
         rearBumper.State.DiffuseColor = new Vector4(0.1f, 0.1f, 0.1f, 1.0f);

         // The physics engine will issue contact notifications only
         // if enabled per shape.
         frontBumper.State.EnableContactNotifications = true;
         rearBumper.State.EnableContactNotifications  = true;

         // Put some force filtering so only significant bumps notify us.
         // frontBumper.State.ContactFilter = new ContactNotificationFilter(1,1);
         // rearBumper.State.ContactFilter = new ContactNotificationFilter(1, 1);
         BumperArrayEntity
            bumperArray = new BumperArrayEntity(frontBumper, rearBumper);

         // Entity name must match manifest partner name.
         bumperArray.State.Name = "P3DXBumpers";

         // Start simulated bumper service.
         _pioneer3DXbumperCreatePort =
            simbumper.Contract.CreateService(
               ConstructorPort,
               Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
                  "http://localhost/" + bumperArray.State.Name));
         if (_pioneer3DXbumperCreatePort == null)
         {
            LogError("Cannot create P3DXBumpers service");
         }
         return(bumperArray);
      }


      // Subscribe to the Pioneer3DX bumper service.
      IEnumerator<ITask> subscribePioneer3DXbumper(CreateResponse createResponse)
      {
         string service = createResponse.Service + "/contactsensor";

         _pioneer3DXbumperPort = ServiceForwarder<bumper.ContactSensorArrayOperations>(service);

         bumper.ReliableSubscribe subscribe = new bumper.ReliableSubscribe(
            new ReliableSubscribeRequestType(5)
            );
         subscribe.NotificationPort = _pioneer3DXbumperNotify;

         _pioneer3DXbumperPort.Post(subscribe);

         yield return(Arbiter.Choice(
                         subscribe.ResponsePort,
                         delegate(SubscribeResponseType response)
                         {
                         },
                         delegate(Fault fault)
                         {
                            LogError("Cannot subscribe to P3DXBumpers service", fault);
                         }
                         ));

         yield break;
      }


      // Pioneer3DX bumper replace notification handler.
      void pioneer3DXbumperReplaceNotificationHandler(bumper.Replace replace)
      {
         // Get most recent reading, discarding older ones.
         bumper.Replace testReplace;
         while ((testReplace = _pioneer3DXbumperNotify.Test<bumper.Replace>()) != null)
         {
            replace = testReplace;
         }
         _state._pioneer3DXrobotState._bumperHit =
            pioneer3DXbumpersPressed(replace.Body);

         // Let brain process bumper data.
         _mainPort.Post(new Pioneer3DXinvokeBrain());

         // Re-activate handler.
         Activate(Arbiter.Receive<bumper.Replace>(false, _pioneer3DXbumperNotify, pioneer3DXbumperReplaceNotificationHandler));
      }


      // Pioneer3DX bumper update notification handler.
      void pioneer3DXbumperUpdateNotificationHandler(bumper.Update update)
      {
         // Get most recent reading, discarding older ones.
         bumper.Update testUpdate;
         while ((testUpdate = _pioneer3DXbumperNotify.Test<bumper.Update>()) != null)
         {
            update = testUpdate;
         }
         _state._pioneer3DXrobotState._bumperHit = update.Body.Pressed;

         // Let brain process bumper data.
         _mainPort.Post(new Pioneer3DXinvokeBrain());

         // Re-activate handler.
         Activate(Arbiter.Receive<bumper.Update>(false, _pioneer3DXbumperNotify, pioneer3DXbumperUpdateNotificationHandler));
      }


      // Checks whether at least one of the contact sensors is pressed.
      bool pioneer3DXbumpersPressed(bumper.ContactSensorArrayState bumpers)
      {
         if (bumpers.Sensors == null)
         {
            return(false);
         }
         foreach (bumper.ContactSensor s in bumpers.Sensors)
         {
            if (s.Pressed)
            {
               return(true);
            }
         }
         return(false);
      }


      // Prepare sensors and invoke brain for response.
      void pioneer3DXbrainInvocationHandler(Pioneer3DXinvokeBrain invoke)
      {
         // Consume events.
         while (_mainPort.Test<Pioneer3DXinvokeBrain>() != null) { }

         // Ready to invoke?
         // 1. Sensor data must be available.
         // 2. Previous movement completed.
         sicklrf.State             lrfData     = _state._pioneer3DXrobotState._lrfData;
         webcam.QueryFrameResponse webcamFrame = _state._pioneer3DXrobotState._webcamFrame;
         if (_state._pioneer3DXrobotState._moving || (lrfData == null) || (webcamFrame == null))
         {
            // Re-activate handler.
            Activate(Arbiter.Receive<Pioneer3DXinvokeBrain>(false, _mainPort, pioneer3DXbrainInvocationHandler));
            return;
         }

         // Reset stepping button label.
         pioneer3DXsetDashboardStepLabel("Step");

         // Prepare sensory input to brain.
         _state._pioneer3DXrobotState.lrfPrepareSensors(lrfData);
         _state._pioneer3DXrobotState.webcamPrepareSensors(webcamFrame);
         _state._pioneer3DXrobotState.bumperPrepareSensors();

         // Automatically creep ahead?
         if (_state._pioneer3DXrobotState._creep)
         {
            double distance = _state._pioneer3DXrobotState.getCreepDistance();
            if (distance > 0.0)
            {
               _state._pioneer3DXrobotState._moving = true;
               SpawnIterator(distance, pioneer3DXdrive);
               Activate(Arbiter.Receive<Pioneer3DXinvokeBrain>(false, _mainPort, pioneer3DXbrainInvocationHandler));
               return;
            }
         }

         // Automatically align?
         if (_state._pioneer3DXrobotState._align)
         {
            double angle = _state._pioneer3DXrobotState.getAlignment(lrfData);
            if (angle != 0.0)
            {
               _state._pioneer3DXrobotState._moving = true;
               SpawnIterator(angle, pioneer3DXturn);
               Activate(Arbiter.Receive<Pioneer3DXinvokeBrain>(false, _mainPort, pioneer3DXbrainInvocationHandler));
               return;
            }
         }

         // Display sensors on dashboard.
         pioneer3DXdisplaySensors();

         // Paused?
         if (_state._pioneer3DXrobotState._pause)
         {
            pioneer3DXdisplayNeeds();
            Activate(Arbiter.Receive<Pioneer3DXinvokeBrain>(false, _mainPort, pioneer3DXbrainInvocationHandler));
            return;
         }

         // Check driver.
         switch (_state._pioneer3DXrobotState._driver)
         {
         case Pioneer3DXrobotState.DRIVER_TYPE.ATANI:

            // If stepping set pause.
            if (_state._pioneer3DXrobotState._step)
            {
               pioneer3DXsetStepping(false);
               pioneer3DXsetDashboardStepLabel("Stepping");
               pioneer3DXsetPause(true);
            }

            // Get response.
            _state._pioneer3DXrobotState.cycleBrain();

            break;

         case Pioneer3DXrobotState.DRIVER_TYPE.OVERRIDE:
         case Pioneer3DXrobotState.DRIVER_TYPE.HIJACK:

            if (_state._pioneer3DXrobotState._step)
            {
               pioneer3DXsetStepping(false);
            }

            if ((_state._pioneer3DXrobotState._manualResponse !=
                 Pioneer3DXrobotState.RESPONSE_TYPE.NULL_RESPONSE) &&
                (_state._pioneer3DXrobotState._manualResponse !=
                 Pioneer3DXrobotState.RESPONSE_TYPE.PENDING_RESPONSE))
            {
               if (_state._pioneer3DXrobotState._driver ==
                   Pioneer3DXrobotState.DRIVER_TYPE.OVERRIDE)
               {
                  // Override response.
                  _state._pioneer3DXrobotState.overrideResponse(
                     _state._pioneer3DXrobotState._manualResponse);
                  _state._pioneer3DXrobotState.cycleBrain();
               }
               else
               {
                  // Hijack response.
                  _state._pioneer3DXrobotState._response =
                     _state._pioneer3DXrobotState._manualResponse;
               }
               _state._pioneer3DXrobotState._manualResponse =
                  Pioneer3DXrobotState.RESPONSE_TYPE.PENDING_RESPONSE;
            }
            else
            {
               // Release driver for next action?
               if (_state._pioneer3DXrobotState._manualResponse ==
                   Pioneer3DXrobotState.RESPONSE_TYPE.PENDING_RESPONSE)
               {
                  _state._pioneer3DXrobotState._manualResponse =
                     Pioneer3DXrobotState.RESPONSE_TYPE.NULL_RESPONSE;
                  pioneer3DXrobotDashboard.ControlSetRequest controlSetRequest =
                     new pioneer3DXrobotDashboard.ControlSetRequest();
                  controlSetRequest.EnableDriverSelection = true;
                  if ((_state._pioneer3DXrobotState._driver ==
                       Pioneer3DXrobotState.DRIVER_TYPE.OVERRIDE) ||
                      (_state._pioneer3DXrobotState._driver ==
                       Pioneer3DXrobotState.DRIVER_TYPE.HIJACK))
                  {
                     controlSetRequest.EnableManualButtons = true;
                  }
                  _pioneer3DXrobotDashboardPort.Post(new pioneer3DXrobotDashboard.ControlSet(controlSetRequest));
               }

               pioneer3DXdisplayNeeds();
               Activate(Arbiter.Receive<Pioneer3DXinvokeBrain>(false, _mainPort, pioneer3DXbrainInvocationHandler));
               return;
            }

            break;

         case Pioneer3DXrobotState.DRIVER_TYPE.GOTO_GOAL:
            if (_state._pioneer3DXrobotState._step)
            {
               pioneer3DXsetStepping(false);
            }

            // Override response with movement leading to goal.
            switch (_pioneer3DXtmazeGuide.getMovement())
            {
            case TmazeGuide.MOVEMENT_TYPE.FORWARD:
               _state._pioneer3DXrobotState.overrideResponse(
                  Pioneer3DXrobotState.RESPONSE_TYPE.FORWARD);
               break;

            case TmazeGuide.MOVEMENT_TYPE.LEFT:
               _state._pioneer3DXrobotState.overrideResponse(
                  Pioneer3DXrobotState.RESPONSE_TYPE.LEFT);
               break;

            case TmazeGuide.MOVEMENT_TYPE.RIGHT:
               _state._pioneer3DXrobotState.overrideResponse(
                  Pioneer3DXrobotState.RESPONSE_TYPE.RIGHT);
               break;

            case TmazeGuide.MOVEMENT_TYPE.NOOP:
               _state._pioneer3DXrobotState.overrideResponse(
                  Pioneer3DXrobotState.RESPONSE_TYPE.WAIT);
               break;
            }
            _state._pioneer3DXrobotState.cycleBrain();

            break;
         }

         // Display response on dashboard.
         pioneer3DXdisplayResponse();

         // Display need values on dashboard.
         pioneer3DXdisplayNeeds();

         // Save sensors for reward purposes.
         _state._pioneer3DXrobotState.saveSensors();

         // Process response.
         switch (_state._pioneer3DXrobotState._response)
         {
         case Pioneer3DXrobotState.RESPONSE_TYPE.FORWARD:
            double distance = _state._pioneer3DXrobotState.getDriveDistance();
            if (distance > 0.0)
            {
               _state._pioneer3DXrobotState._moving     = true;
               _state._pioneer3DXrobotState._creep      = true;
               _state._pioneer3DXrobotState._creepRange =
                  _state._pioneer3DXrobotState._lrfForwardDistance / 1000.0;
               _state._pioneer3DXrobotState._creepDistance = distance;
               _state._pioneer3DXrobotState._align         = true;
               if (_state._worldType == AtaniState.WORLD_TYPE.TMAZE_WORLD)
               {
                  _pioneer3DXtmazeGuide.forward();
               }
               SpawnIterator(distance, pioneer3DXdrive);
            }
            break;

         case Pioneer3DXrobotState.RESPONSE_TYPE.RIGHT:
            _state._pioneer3DXrobotState._moving = true;
            _state._pioneer3DXrobotState._align  = true;
            if (_state._worldType == AtaniState.WORLD_TYPE.TMAZE_WORLD)
            {
               _pioneer3DXtmazeGuide.right();
            }
            SpawnIterator(-90.0, pioneer3DXturn);
            break;

         case Pioneer3DXrobotState.RESPONSE_TYPE.LEFT:
            _state._pioneer3DXrobotState._moving = true;
            _state._pioneer3DXrobotState._align  = true;
            if (_state._worldType == AtaniState.WORLD_TYPE.TMAZE_WORLD)
            {
               _pioneer3DXtmazeGuide.left();
            }
            SpawnIterator(90.0, pioneer3DXturn);
            break;

         case Pioneer3DXrobotState.RESPONSE_TYPE.WAIT:
            break;
         }

         // Re-activate handler.
         Activate(Arbiter.Receive<Pioneer3DXinvokeBrain>(false, _mainPort, pioneer3DXbrainInvocationHandler));
      }


      // Drive.
      IEnumerator<ITask> pioneer3DXdrive(double distance)
      {
         // Turn on motor?
         if ((_state._pioneer3DXrobotState._driveState == null) ||
             !_state._pioneer3DXrobotState._driveState.IsEnabled)
         {
            yield return(Arbiter.Choice
                         (
                            pioneer3DXenableMotor(),
                            delegate(DefaultUpdateResponseType response)
                            {
                            },
                            delegate(Fault fault)
                            {
                               LogError("Cannot enable Pioneer3DX motor", fault);
                            }
                         ));
         }

         // Move forward.
         yield return(Arbiter.Choice
                      (
                         pioneer3DXdriveDistance(distance, _state._pioneer3DXrobotState._driveSpeed),
                         delegate(DefaultUpdateResponseType response)
                         {
                         },
                         delegate(Fault fault)
                         {
                            LogError("Cannot drive Pioneer3DX motor");
                         }
                      ));

         // Indicate completion of movement after waiting
         // for sensors to stabilize.
         Thread.Sleep(1000);
         _state._pioneer3DXrobotState._moving = false;
         yield break;
      }


      // Turn.
      IEnumerator<ITask> pioneer3DXturn(double angle)
      {
         // Turn on motor?
         if ((_state._pioneer3DXrobotState._driveState == null) ||
             !_state._pioneer3DXrobotState._driveState.IsEnabled)
         {
            yield return(Arbiter.Choice
                         (
                            pioneer3DXenableMotor(),
                            delegate(DefaultUpdateResponseType response)
                            {
                            },
                            delegate(Fault fault)
                            {
                               LogError("Cannot enable Pioneer3DX motor", fault);
                            }
                         ));
         }

         // Turn angle degrees.
         yield return(Arbiter.Choice
                      (
                         pioneer3DXturn(angle, _state._pioneer3DXrobotState._turnSpeed),
                         delegate(DefaultUpdateResponseType response)
                         {
                         },
                         delegate(Fault fault)
                         {
                            LogError("Cannot turn Pioneer3DX");
                         }
                      ));

         // Indicate completion of movement after waiting
         // for sensors to stabilize.
         Thread.Sleep(1000);
         _state._pioneer3DXrobotState._moving = false;
         yield break;
      }


      // Create the Pioneer3DX robot dashboard service.
      void createPioneer3DXrobotDashboard()
      {
         _pioneer3DXrobotDashboardCreatePort =
            pioneer3DXrobotDashboard.Contract.CreateService(ConstructorPort,
                                                            Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
                                                               "http://localhost/pioneer3dxrobotdashboard")
                                                            );
         if (_pioneer3DXrobotDashboardCreatePort == null)
         {
            LogError("Cannot create Pioneer3DX dashboard service");
         }
      }


      // Subscribe to the Pioneer3DX dashboard.
      IEnumerator<ITask> subscribePioneer3DXrobotDashboard(CreateResponse createResponse)
      {
         string service = createResponse.Service;

         _pioneer3DXrobotDashboardPort =
            ServiceForwarder<pioneer3DXrobotDashboard.Pioneer3DXrobotDashboardOperations>(service);

         pioneer3DXrobotDashboard.Subscribe subscribe =
            new pioneer3DXrobotDashboard.Subscribe(
               new SubscribeRequestType()
               );
         subscribe.NotificationPort = _pioneer3DXrobotDashboardNotify;
         _pioneer3DXrobotDashboardPort.Post(subscribe);

         yield return(Arbiter.Choice(
                         subscribe.ResponsePort,
                         delegate(SubscribeResponseType subscribeResponse)
                         {
                         },
                         delegate(Fault fault)
                         {
                            LogError("Cannot subscribe to Pioneer3DX dashboard service", fault);
                         }
                         ));

         // Populate the driver selection menu.
         Thread.Sleep(2000);
         pioneer3DXrobotDashboard.ControlSetRequest controlSetRequest =
            new pioneer3DXrobotDashboard.ControlSetRequest();
         List<string> menu = new List<string>();
         if (_state._worldType == AtaniState.WORLD_TYPE.TMAZE_WORLD)
         {
            menu.AddRange(new string[] {
                             "atani",
                             "override",
                             "hijack",
                             "goto goal"
                          }
                          );
            controlSetRequest.DriverToolTip =
               "atani=robot, override=manual with learning, hijack=manual without learning\ngoto goal with learning";
         }
         else
         {
            menu.AddRange(new string[] {
                             "atani",
                             "override",
                             "hijack"
                          }
                          );
            controlSetRequest.DriverToolTip =
               "atani=robot, override=manual with learning, hijack=manual without learning";
         }
         controlSetRequest.DriverSelectMenu = menu;
         _pioneer3DXrobotDashboardPort.Post(new pioneer3DXrobotDashboard.ControlSet(controlSetRequest));

         yield break;
      }


      // Handle need change.
      [ServiceHandler(ServiceHandlerBehavior.Exclusive)]
      public void pioneer3DXrobotDashboardNeedChangeHandler(pioneer3DXrobotDashboard.NeedChange needChange)
      {
         _state._pioneer3DXrobotState.setSuccessNeed(needChange.Body.SuccessNeed);
         _state._pioneer3DXrobotState.setRewardNeed(needChange.Body.RewardNeed);
      }


      // Handle dashboard control actions.
      [ServiceHandler(ServiceHandlerBehavior.Exclusive)]
      public void pioneer3DXrobotDashboardControlActionHandler(pioneer3DXrobotDashboard.ControlAction controlAction)
      {
         if (controlAction.Body.PauseCheckValid)
         {
            bool change = false;
            if (_state._pioneer3DXrobotState._pause != controlAction.Body.PauseCheck)
            {
               _state._pioneer3DXrobotState._pause = controlAction.Body.PauseCheck;
               change = true;
            }

            // Turn off stepping.
            if (change)
            {
               pioneer3DXsetStepping(false);
               pioneer3DXsetDashboardStepLabel("Step");
            }
            return;
         }

         if (controlAction.Body.StepClick)
         {
            pioneer3DXsetStepping(!_state._pioneer3DXrobotState._step);
            pioneer3DXsetPause(false);
            return;
         }

         if (controlAction.Body.ResetClick)
         {
            // Reset position.
            xna.Vector3 position = new xna.Vector3();
            position.X = _state._pioneer3DXrobotState._initialPosition.X;
            position.Y = _state._pioneer3DXrobotState._initialPosition.Y;
            position.Z = _state._pioneer3DXrobotState._initialPosition.Z;
            _state._pioneer3DXrobotState._robotBaseEntity.Position = position;
            Vector3 rotation = new Vector3(0.0f, 0.0f, 0.0f);
            _state._pioneer3DXrobotState._robotBaseEntity.RotationAngles = rotation;
            _state._pioneer3DXrobotState.setSuccessNeed(Pioneer3DXrobotState.SUCCESS_NEED_VALUE);
            _state._pioneer3DXrobotState.setRewardNeed(Pioneer3DXrobotState.REWARD_NEED_VALUE);
            if (_state._worldType == AtaniState.WORLD_TYPE.TMAZE_WORLD)
            {
               _pioneer3DXtmazeGuide.reset(_TmazeWorld.MazePath[0], TmazeWorld.DIRECTION.NORTH);
            }
            return;
         }

         if (controlAction.Body.ForgetClick)
         {
            // Clear and save brain.
            double successNeed = _state._pioneer3DXrobotState.getSuccessNeed();
            double rewardNeed  = _state._pioneer3DXrobotState.getRewardNeed();
            _state._pioneer3DXrobotState.createBrain(_state._random.Next());
            _state._pioneer3DXrobotState.saveBrain();
            return;
         }

         if (controlAction.Body.SaveClick)
         {
            _state._pioneer3DXrobotState.saveBrain();
            return;
         }

         if (controlAction.Body.Driver != -1)
         {
            _state._pioneer3DXrobotState._manualResponse =
               Pioneer3DXrobotState.RESPONSE_TYPE.NULL_RESPONSE;
            _state._pioneer3DXrobotState._driver =
               (Pioneer3DXrobotState.DRIVER_TYPE)controlAction.Body.Driver;
            pioneer3DXrobotDashboard.ControlSetRequest controlSetRequest =
               new pioneer3DXrobotDashboard.ControlSetRequest();
            controlSetRequest.EnableDriverSelection = true;
            if ((_state._pioneer3DXrobotState._driver ==
                 Pioneer3DXrobotState.DRIVER_TYPE.OVERRIDE) ||
                (_state._pioneer3DXrobotState._driver ==
                 Pioneer3DXrobotState.DRIVER_TYPE.HIJACK))
            {
               controlSetRequest.EnableManualButtons = true;
            }
            _pioneer3DXrobotDashboardPort.Post(new pioneer3DXrobotDashboard.ControlSet(controlSetRequest));
            return;
         }

         if (controlAction.Body.MoveLeftClick)
         {
            _state._pioneer3DXrobotState._manualResponse =
               Pioneer3DXrobotState.RESPONSE_TYPE.LEFT;
            return;
         }

         if (controlAction.Body.MoveForwardClick)
         {
            _state._pioneer3DXrobotState._manualResponse =
               Pioneer3DXrobotState.RESPONSE_TYPE.FORWARD;
            return;
         }

         if (controlAction.Body.MoveRightClick)
         {
            _state._pioneer3DXrobotState._manualResponse =
               Pioneer3DXrobotState.RESPONSE_TYPE.RIGHT;
            return;
         }

         if (controlAction.Body.PrintBrain)
         {
            _state._pioneer3DXrobotState.printBrain();
            return;
         }
      }


      // Set pause state.
      void pioneer3DXsetPause(bool isChecked)
      {
         _state._pioneer3DXrobotState._pause = isChecked;
         pioneer3DXsetDashboardPauseChecked(isChecked);
      }


      // Set dashboard pause checked state.
      void pioneer3DXsetDashboardPauseChecked(bool isChecked)
      {
         if (_pioneer3DXrobotDashboardPort == null) { return; }

         pioneer3DXrobotDashboard.ControlSetRequest controlSetRequest =
            new pioneer3DXrobotDashboard.ControlSetRequest();
         controlSetRequest.PauseCheck      = isChecked;
         controlSetRequest.PauseCheckValid = true;
         _pioneer3DXrobotDashboardPort.Post(new pioneer3DXrobotDashboard.ControlSet(controlSetRequest));
      }


      // Set stepping state.
      void pioneer3DXsetStepping(bool state)
      {
         _state._pioneer3DXrobotState._step = state;
      }


      // Set dashboard step button label.
      void pioneer3DXsetDashboardStepLabel(string label)
      {
         if (_pioneer3DXrobotDashboardPort == null) { return; }

         pioneer3DXrobotDashboard.ControlSetRequest controlSetRequest =
            new pioneer3DXrobotDashboard.ControlSetRequest();
         controlSetRequest.StepLabel      = label;
         controlSetRequest.StepLabelValid = true;
         _pioneer3DXrobotDashboardPort.Post(new pioneer3DXrobotDashboard.ControlSet(controlSetRequest));
      }


      // Display sensors on dashboard.
      void pioneer3DXdisplaySensors()
      {
         if (_pioneer3DXrobotDashboardPort == null) { return; }

         int    i;
         string lrfSensorString = "";
         for (i = 0; i < (int)Pioneer3DXrobotState.SENSOR_CONFIG.NUM_LRF_SENSORS; i++)
         {
            lrfSensorString = lrfSensorString +
                              _state._pioneer3DXrobotState._sensors[(int)Pioneer3DXrobotState.SENSOR_CONFIG.LRF_SENSOR_INDEX + i] + " ";
         }
         string webcamSensorString = "";
         for (i = 0; i < (int)Pioneer3DXrobotState.SENSOR_CONFIG.NUM_WEBCAM_SENSORS; i++)
         {
            webcamSensorString = webcamSensorString +
                                 _state._pioneer3DXrobotState._sensors[(int)Pioneer3DXrobotState.SENSOR_CONFIG.WEBCAM_SENSOR_INDEX + i] + " ";
         }
         string bumperSensorString = "";
         for (i = 0; i < (int)Pioneer3DXrobotState.SENSOR_CONFIG.NUM_BUMPER_SENSORS; i++)
         {
            bumperSensorString = bumperSensorString +
                                 _state._pioneer3DXrobotState._sensors[(int)Pioneer3DXrobotState.SENSOR_CONFIG.BUMPER_SENSOR_INDEX + i] + " ";
         }

         pioneer3DXrobotDashboard.Sensors sensors = new pioneer3DXrobotDashboard.Sensors();
         sensors.Body.LRFSensors    = lrfSensorString;
         sensors.Body.WebcamSensors = webcamSensorString;
         sensors.Body.BumperSensor  = bumperSensorString;
         _pioneer3DXrobotDashboardPort.Post(sensors);
      }


      // Display response on dashboard.
      void pioneer3DXdisplayResponse()
      {
         if (_pioneer3DXrobotDashboardPort == null) { return; }

         string responseValue = "";
         switch (_state._pioneer3DXrobotState._response)
         {
         case Pioneer3DXrobotState.RESPONSE_TYPE.WAIT:
            responseValue = "wait";
            break;

         case Pioneer3DXrobotState.RESPONSE_TYPE.FORWARD:
            responseValue = "forward";
            break;

         case Pioneer3DXrobotState.RESPONSE_TYPE.RIGHT:
            responseValue = "right";
            break;

         case Pioneer3DXrobotState.RESPONSE_TYPE.LEFT:
            responseValue = "left";
            break;
         }

         pioneer3DXrobotDashboard.Response response = new pioneer3DXrobotDashboard.Response();
         response.Body.Response = responseValue;
         _pioneer3DXrobotDashboardPort.Post(response);
      }


      // Display needs on dashboard.
      void pioneer3DXdisplayNeeds()
      {
         if (_pioneer3DXrobotDashboardPort == null) { return; }

         pioneer3DXrobotDashboard.NeedSet needSet =
            new pioneer3DXrobotDashboard.NeedSet();
         needSet.Body.SuccessNeed =
            _state._pioneer3DXrobotState.getSuccessNeed();
         needSet.Body.RewardNeed =
            _state._pioneer3DXrobotState.getRewardNeed();
         _pioneer3DXrobotDashboardPort.Post(needSet);
      }


      // Handle dashboard reward action.
      [ServiceHandler(ServiceHandlerBehavior.Exclusive)]
      public void pioneer3DXrobotDashboardRewardActionHandler(pioneer3DXrobotDashboard.RewardAction rewardAction)
      {
         _state._pioneer3DXrobotState.rewardAction(rewardAction.Body.RewardAction, rewardAction.Body.RewardValue);
      }


      #endregion

      #region Lego NXT
      void createLegoNxtRobot(Vector3 position)
      {
         LegoNXTTribot robotBaseEntity = createLegoNxtMotorBase(ref position);

         EntityNameSprite nameSprite = new EntityNameSprite(
            0.25f, 0.0625f, 256, 64, SpritePivotType.Center, new Vector3(0, 0.25f, 0), "Courier", 14);

         nameSprite.State.Name = "nameSprite";

         robotBaseEntity.InsertEntity(nameSprite);

         // Create bumper array entity and start simulated bumper service.
         BumperArrayEntity bumperArray = createLegoNxtBumper();
         robotBaseEntity.InsertEntity(bumperArray);

         // Insert the motor base and its two children to the simulation.
         SimulationEngine.GlobalInstancePort.Insert(robotBaseEntity);
      }


      LegoNXTTribot createLegoNxtMotorBase(ref Vector3 position)
      {
         // Use supplied entity that creates a motor base
         // with 2 active wheels and one caster.
         LegoNXTTribot robotBaseEntity = new LegoNXTTribot(position);

         // Specify mesh.
         robotBaseEntity.State.Assets.Mesh = "LegoNXTTribot.bos";
         robotBaseEntity.WheelMesh         = "LegoNXTTribotWheel.bos";

         // The name below must match manifest.
         robotBaseEntity.State.Name = "LegoNXTMotorBase";

         // Start simulated arcos motor service.
         CreateService(
            simdrive.Contract.Identifier,
            Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
               "http://localhost/" + robotBaseEntity.State.Name)
            );
         return(robotBaseEntity);
      }


      BumperArrayEntity createLegoNxtBumper()
      {
         // Create a little bumper shape that models the NXT bumper.
         BoxShape frontBumper = new BoxShape(
            new BoxShapeProperties(
               "front", 0.001f,                               //mass
               new Pose(new Vector3(0, 0.063f, -0.09f)),      //position
               new Vector3(0.023f, 0.023f, 0.045f)));

         // The physics engine will issue contact notifications only
         // if enabled per shape.
         frontBumper.State.EnableContactNotifications = true;

         BumperArrayEntity
            bumperArray = new BumperArrayEntity(frontBumper);

         // Entity name must match manifest partner name.
         bumperArray.State.Name = "LegoNXTBumpers";

         // Start simulated bumper service.
         CreateService(
            simbumper.Contract.Identifier,
            Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
               "http://localhost/" + bumperArray.State.Name));
         return(bumperArray);
      }


      #endregion

      #region IRobot Create
      void createIRobotRobot(Vector3 position)
      {
         IRobotCreate robotBaseEntity = createIRobotMotorBase(ref position);

         // Create left and right bumper array entities and start simulated bumper service.
         BumperArrayEntity bumperArray = createIRobotBumper(true);

         robotBaseEntity.InsertEntity(bumperArray);
         bumperArray = createIRobotBumper(false);
         robotBaseEntity.InsertEntity(bumperArray);

         // Insert the motor base and its children to the simulation.
         SimulationEngine.GlobalInstancePort.Insert(robotBaseEntity);
      }


      IRobotCreate createIRobotMotorBase(ref Vector3 position)
      {
         // Use supplied entity that creates a motor base
         // with 2 active wheels and one caster.
         IRobotCreate robotBaseEntity = new IRobotCreate(position);

         // Specify mesh.
         robotBaseEntity.State.Assets.Mesh = "IRobot-Create.bos";

         // The name below must match manifest.
         robotBaseEntity.State.Name = "IRobotCreateMotorBase";

         // Start simulated arcos motor service.
         CreateService(
            simdrive.Contract.Identifier,
            Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
               "http://localhost/" + robotBaseEntity.State.Name)
            );
         return(robotBaseEntity);
      }


      BumperArrayEntity createIRobotBumper(bool isLeft)
      {
         string  Name;
         Vector3 Offset;
         float   Rotation;

         if (isLeft)
         {
            Name     = "Left";
            Offset   = new Vector3(-0.07f, 0.055f, -0.14f);
            Rotation = 1.75f;
         }
         else
         {
            Name     = "Right";
            Offset   = new Vector3(0.07f, 0.055f, -0.14f);
            Rotation = -1.75f;
         }

         // Create a little bumper shape that models the iRobot bumper.
         BoxShape Bumper = new BoxShape(
            new BoxShapeProperties(
               Name, 0.001f,                                                                    //mass
               new Pose(Offset,                                                                 // position
                        Quaternion.FromAxisAngle(0, 1, 0, (float)(Rotation * Math.PI / 2.0f))), // rotation
               new Vector3(0.15f, 0.06f, 0.01f)));                                              // dimensions

         // The physics engine will issue contact notifications only
         // if enabled per shape.
         Bumper.State.EnableContactNotifications = true;

         BumperArrayEntity
            bumperArray = new BumperArrayEntity(Bumper);

         // Entity name must match manifest partner name.
         bumperArray.State.Name = "IRobotCreateBumper" + Name;

         // Start simulated bumper service.
         CreateService(
            simbumper.Contract.Identifier,
            Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
               "http://localhost/" + bumperArray.State.Name));

         return(bumperArray);
      }


      #endregion
      #endregion

      #region Heartbeat timer
      // Handle heartbeat timer.
      public void heartbeatUpdateHandler(HeartbeatUpdate update)
      {
         Activate(
            Arbiter.Receive(
               false,
               TimeoutPort(HEARTBEAT_INTERVAL),
               delegate(DateTime ts)
               {
                  _mainPort.Post(new HeartbeatUpdate(new HeartbeatUpdateRequest(ts)));
               }
               )
            );

         update.ResponsePort.Post(DefaultUpdateResponseType.Instance);
      }


      #endregion

      #region DSSP handlers

      // Get handler returns state.
      [ServiceHandler(ServiceHandlerBehavior.Concurrent)]
      public void GetHandler(Get get)
      {
         get.ResponsePort.Post(_state);
      }


      // Drop handler shuts down service.
      [ServiceHandler(ServiceHandlerBehavior.Teardown)]
      public void DropHandler(DsspDefaultDrop drop)
      {
         PerformShutdown(ref _pioneer3DXlrfShutdown);
         PerformShutdown(ref _pioneer3DXmotorShutdown);
         base.DefaultDropHandler(drop);
      }


      void PerformShutdown(ref Port<Shutdown> port)
      {
         if (port == null) { return; }
         Shutdown shutdown = new Shutdown();
         port.Post(shutdown);
         port = null;
      }


      #endregion
   }
}
