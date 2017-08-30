//-----------------------------------------------------------------------
//  Atani neural network controlled robot.
//
//  $File: ataniTypes.cs $ $Revision: 1 $
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.ComponentModel;
using Microsoft.Ccr.Core;
using Microsoft.Dss.ServiceModel.Dssp;
using Microsoft.Dss.Core.Attributes;
using Microsoft.Dss.Core.DsspHttp;
using W3C.Soap;
using dssp = Microsoft.Dss.ServiceModel.Dssp;
using pioneer3DXrobotDashboard = Robotics.Atani.Pioneer3DXrobotDashboard.Proxy;

namespace Robotics.Atani
{
   // DSS atani contract.
   static class Contract
   {
      // DSS atani namespace.
      public const string Identifier = "http://schemas.tempuri.org/2008/09/atani.html";
   }

   // Operations.
   public class AtaniOperations : PortSet<
                                     DsspDefaultLookup,
                                     DsspDefaultDrop,
                                     Get,
                                     pioneer3DXrobotDashboard.NeedChange,
                                     pioneer3DXrobotDashboard.ControlAction,
                                     pioneer3DXrobotDashboard.RewardAction,
                                     Pioneer3DXinvokeBrain,
                                     HeartbeatUpdate>
   {
   }

   // DSS Get definition.
   [Description("Gets the current state of the service.")]
   public class Get : Get<dssp.GetRequestType, PortSet<AtaniState, W3C.Soap.Fault>>
   {
      // Default DSS Get constructor.
      public Get()
      {
      }


      // DSS Get constructor.
      public Get(dssp.GetRequestType body)
         : base(body)
      {
      }
   }

   // Pioneer 3DX robot brain invocation.
   [Description("Invoke the Pioneer3DX robot brain.")]
   public class Pioneer3DXinvokeBrain : Submit<Pioneer3DXinvokeBrainRequest, PortSet<DefaultSubmitResponseType, Fault>>
   {
      public Pioneer3DXinvokeBrain()
         : base(new Pioneer3DXinvokeBrainRequest())
      { }
   }

   [DataContract]
   public class Pioneer3DXinvokeBrainRequest { }

   // Heartbeat timer definition.
   [Description("Heartbeat timer.")]
   public class HeartbeatUpdate : Update<HeartbeatUpdateRequest, PortSet<DefaultUpdateResponseType, Fault>>
   {
      public HeartbeatUpdate(HeartbeatUpdateRequest body)
         : base(body)
      { }

      public HeartbeatUpdate() { }
   }

   [DataContract]
   public class HeartbeatUpdateRequest
   {
      private DateTime _timeStamp;

      [DataMember]
      public DateTime TimeStamp
      {
         get { return(_timeStamp); }
         set { _timeStamp = value; }
      }

      public HeartbeatUpdateRequest(DateTime timeStamp)
      {
         TimeStamp = timeStamp;
      }


      public HeartbeatUpdateRequest() { }
   }
}
