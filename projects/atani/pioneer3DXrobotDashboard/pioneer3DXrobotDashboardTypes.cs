//-----------------------------------------------------------------------
//  $File: pioneer3DXrobotDashboardTypes.cs $ $Revision: 1 $
//-----------------------------------------------------------------------
using Microsoft.Ccr.Core;
using Microsoft.Dss.Core.Attributes;
using Microsoft.Dss.ServiceModel.Dssp;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using W3C.Soap;

namespace Robotics.Atani.Pioneer3DXrobotDashboard
{
   /// <summary>
   /// Pioneer3DXrobotDashboard Contract
   /// </summary>
   public static class Contract
   {
      /// The Unique Contract Identifier for the Pioneer3DXrobotDashboard service
      public const String Identifier = "http://schemas.tempuri.org/2009/03/pioneer3dxrobotdashboard.html";
   }

   [DataContract]
   public class Pioneer3DXrobotDashboardState
   {
   }

   [ServicePort]
   public class Pioneer3DXrobotDashboardOperations : PortSet<
                                                        DsspDefaultLookup,
                                                        DsspDefaultDrop,
                                                        Get,
                                                        Replace,
                                                        Subscribe,
                                                        Sensors,
                                                        Response,
                                                        NeedChange,
                                                        NeedSet,
                                                        ControlAction,
                                                        ControlSet,
                                                        RewardAction>
   {
   }

   [DisplayName("Get")]
   [Description("Gets the current state of the dashboard.")]
   public class Get : Get<GetRequestType, PortSet<Pioneer3DXrobotDashboardState, Fault>>
   {
   }

   [DisplayName("DashboardStateChange")]
   [Description("Indicates when the dashboard state changes.")]
   public class Replace : Replace<Pioneer3DXrobotDashboardState, PortSet<DefaultReplaceResponseType, Fault>>
   {
   }

   [DisplayName("Subscription")]
   [Description("Subscription request.")]
   public class Subscribe : Subscribe<SubscribeRequestType, PortSet<SubscribeResponseType, Fault>>
   {
   }

   [DisplayName("Sensors")]
   [Description("Robot sensors display.")]
   public class Sensors : Update<SensorsRequest, PortSet<DefaultUpdateResponseType, Fault>>
   {
      public Sensors(SensorsRequest body)
         : base(body)
      { }

      public Sensors()
      { }
   }

   [DataContract]
   public class SensorsRequest
   {
      private string _lrfSensors;
      private string _webcamSensors;
      private string _bumperSensor;

      [DataMember]
      public string LRFSensors
      {
         get { return(_lrfSensors); }
         set { _lrfSensors = value; }
      }

      [DataMember]
      public string WebcamSensors
      {
         get { return(_webcamSensors); }
         set { _webcamSensors = value; }
      }

      [DataMember]
      public string BumperSensor
      {
         get { return(_bumperSensor); }
         set { _bumperSensor = value; }
      }

      public SensorsRequest(string lrfSensors, string webcamSensors,
                            string bumperSensor)
      {
         _lrfSensors    = lrfSensors;
         _webcamSensors = webcamSensors;
         _bumperSensor  = bumperSensor;
      }


      public SensorsRequest() { }
   }

   [DisplayName("Response")]
   [Description("Robot response display.")]
   public class Response : Update<ResponseRequest, PortSet<DefaultUpdateResponseType, Fault>>
   {
      public Response(ResponseRequest body)
         : base(body)
      { }

      public Response()
      { }
   }

   [DataContract]
   public class ResponseRequest
   {
      private string _response;

      [DataMember]
      public string Response
      {
         get { return(_response); }
         set { _response = value; }
      }

      public ResponseRequest(string response)
      {
         _response = response;
      }


      public ResponseRequest() { }
   }

   [DisplayName("NeedChange")]
   [Description("Need value change.")]
   public class NeedChange : Update<NeedChangeRequest, PortSet<DefaultUpdateResponseType, Fault>>
   {
      public NeedChange(NeedChangeRequest body)
         : base(body)
      { }

      public NeedChange()
      { }
   }

   [DataContract]
   public class NeedChangeRequest
   {
      private double _successNeed;
      private double _rewardNeed;

      [DataMember]
      public double SuccessNeed
      {
         get { return(_successNeed); }
         set { _successNeed = value; }
      }

      [DataMember]
      public double RewardNeed
      {
         get { return(_rewardNeed); }
         set { _rewardNeed = value; }
      }

      public NeedChangeRequest(double successNeed, double rewardNeed)
      {
         _successNeed = successNeed;
         _rewardNeed  = rewardNeed;
      }


      public NeedChangeRequest() { }
   }

   [DisplayName("NeedSet")]
   [Description("Need value setting.")]
   public class NeedSet : Update<NeedSetRequest, PortSet<DefaultUpdateResponseType, Fault>>
   {
      public NeedSet(NeedSetRequest body)
         : base(body)
      { }

      public NeedSet()
      { }
   }

   [DataContract]
   public class NeedSetRequest
   {
      private double _successNeed;
      private double _rewardNeed;

      [DataMember]
      public double SuccessNeed
      {
         get { return(_successNeed); }
         set { _successNeed = value; }
      }

      [DataMember]
      public double RewardNeed
      {
         get { return(_rewardNeed); }
         set { _rewardNeed = value; }
      }

      public NeedSetRequest(double successNeed, double rewardNeed)
      {
         _successNeed = successNeed;
         _rewardNeed  = rewardNeed;
      }


      public NeedSetRequest()
      {
         _successNeed = 0.0;
         _rewardNeed  = 0.0;
      }
   }

   [DisplayName("ControlAction")]
   [Description("Control action.")]
   public class ControlAction : Update<ControlActionRequest, PortSet<DefaultUpdateResponseType, Fault>>
   {
      public ControlAction(ControlActionRequest body)
         : base(body)
      { }

      public ControlAction()
      { }
   }

   [DataContract]
   public class ControlActionRequest
   {
      private bool _resetClick;
      private bool _forgetClick;
      private bool _saveClick;
      private bool _pauseCheck;
      private bool _pauseCheckValid;
      private bool _stepClick;
      private int  _driver;
      private bool _moveLeftClick;
      private bool _moveForwardClick;
      private bool _moveRightClick;
      private bool _printBrain;

      [DataMember]
      public bool ResetClick
      {
         get { return(_resetClick); }
         set { _resetClick = value; }
      }

      [DataMember]
      public bool ForgetClick
      {
         get { return(_forgetClick); }
         set { _forgetClick = value; }
      }

      [DataMember]
      public bool SaveClick
      {
         get { return(_saveClick); }
         set { _saveClick = value; }
      }
      [DataMember]
      public bool PauseCheck
      {
         get { return(_pauseCheck); }
         set
         {
            _pauseCheck      = value;
            _pauseCheckValid = true;
         }
      }

      [DataMember]
      public bool PauseCheckValid
      {
         get { return(_pauseCheckValid); }
         set { _pauseCheckValid = value; }
      }

      [DataMember]
      public bool StepClick
      {
         get { return(_stepClick); }
         set { _stepClick = value; }
      }

      [DataMember]
      public int Driver
      {
         get { return(_driver); }
         set { _driver = value; }
      }

      [DataMember]
      public bool MoveLeftClick
      {
         get { return(_moveLeftClick); }
         set { _moveLeftClick = value; }
      }

      [DataMember]
      public bool MoveForwardClick
      {
         get { return(_moveForwardClick); }
         set { _moveForwardClick = value; }
      }

      [DataMember]
      public bool MoveRightClick
      {
         get { return(_moveRightClick); }
         set { _moveRightClick = value; }
      }

      [DataMember]
      public bool PrintBrain
      {
         get { return(_printBrain); }
         set { _printBrain = value; }
      }

      public ControlActionRequest(
         bool resetClick,
         bool forgetClick,
         bool saveClick,
         bool pauseCheck,
         bool stepClick,
         int  driver,
         bool moveLeftClick,
         bool moveForwardClick,
         bool moveRightClick,
         bool printBrain)
      {
         _resetClick       = resetClick;
         _forgetClick      = forgetClick;
         _saveClick        = saveClick;
         _pauseCheck       = pauseCheck;
         _pauseCheck       = true;
         _stepClick        = stepClick;
         _driver           = driver;
         _moveLeftClick    = moveLeftClick;
         _moveForwardClick = moveForwardClick;
         _moveRightClick   = moveRightClick;
         _printBrain       = printBrain;
      }


      public ControlActionRequest()
      {
         _resetClick       = false;
         _forgetClick      = false;
         _saveClick        = false;
         _pauseCheck       = false;
         _pauseCheckValid  = false;
         _stepClick        = false;
         _driver           = -1;
         _moveLeftClick    = false;
         _moveForwardClick = false;
         _moveRightClick   = false;
         _printBrain       = false;
      }
   }

   [DisplayName("ControlSet")]
   [Description("Control setting.")]
   public class ControlSet : Update<ControlSetRequest, PortSet<DefaultUpdateResponseType, Fault>>
   {
      public ControlSet(ControlSetRequest body)
         : base(body)
      { }

      public ControlSet()
      { }
   }

   [DataContract]
   public class ControlSetRequest
   {
      private bool         _pauseCheck;
      private bool         _pauseCheckValid;
      private string       _stepLabel;
      private bool         _stepLabelValid;
      private bool         _enableDriverSelection;
      private bool         _enableManualButtons;
      private List<string> _driverSelectMenu;
      private string       _driverToolTip;

      [DataMember]
      public bool PauseCheck
      {
         get { return(_pauseCheck); }
         set
         {
            _pauseCheck      = value;
            _pauseCheckValid = true;
         }
      }

      [DataMember]
      public bool PauseCheckValid
      {
         get { return(_pauseCheckValid); }
         set { _pauseCheckValid = value; }
      }

      [DataMember]
      public string StepLabel
      {
         get { return(_stepLabel); }
         set
         {
            _stepLabel      = value;
            _stepLabelValid = true;
         }
      }

      [DataMember]
      public bool StepLabelValid
      {
         get { return(_stepLabelValid); }
         set { _stepLabelValid = value; }
      }

      [DataMember]
      public bool EnableDriverSelection
      {
         get { return(_enableDriverSelection); }
         set { _enableDriverSelection = value; }
      }

      [DataMember]
      public bool EnableManualButtons
      {
         get { return(_enableManualButtons); }
         set { _enableManualButtons = value; }
      }

      [DataMember]
      public List<string> DriverSelectMenu
      {
         get { return(_driverSelectMenu); }
         set { _driverSelectMenu = value; }
      }

      [DataMember]
      public string DriverToolTip
      {
         get { return(_driverToolTip); }
         set { _driverToolTip = value; }
      }

      public ControlSetRequest(bool pauseCheck,
                               string stepLabel, bool enableDriverSelection,
                               bool enableManualButtons, List<string> driverSelectMenu,
                               string driverToolTip)
      {
         _pauseCheck            = pauseCheck;
         _pauseCheckValid       = true;
         _stepLabel             = stepLabel;
         _stepLabelValid        = true;
         _enableDriverSelection = enableDriverSelection;
         _enableManualButtons   = enableManualButtons;
         _driverSelectMenu      = driverSelectMenu;
         _driverToolTip         = driverToolTip;
      }


      public ControlSetRequest()
      {
         _pauseCheckValid       = false;
         _stepLabelValid        = false;
         _enableDriverSelection = false;
         _enableManualButtons   = false;
         _driverSelectMenu      = null;
         _driverToolTip         = null;
      }
   }

   [DisplayName("RewardAction")]
   [Description("Reward action.")]
   public class RewardAction : Update<RewardActionRequest, PortSet<DefaultUpdateResponseType, Fault>>
   {
      public RewardAction(RewardActionRequest body)
         : base(body)
      { }

      public RewardAction()
      { }
   }

   [DataContract]
   public class RewardActionRequest
   {
      private int    _rewardAction;
      private double _rewardValue;

      [DataMember]
      public int RewardAction
      {
         get { return(_rewardAction); }
         set { _rewardAction = value; }
      }

      [DataMember]
      public double RewardValue
      {
         get { return(_rewardValue); }
         set { _rewardValue = value; }
      }

      public RewardActionRequest(int rewardAction, double rewardValue)
      {
         _rewardAction = rewardAction;
         _rewardValue  = rewardValue;
      }


      public RewardActionRequest() { }
   }
}
