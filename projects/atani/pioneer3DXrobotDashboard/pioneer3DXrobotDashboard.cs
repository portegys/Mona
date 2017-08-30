//-----------------------------------------------------------------------
//  $File: pioneer3DXrobotDashboard.cs $ $Revision: 1 $
//-----------------------------------------------------------------------
using Microsoft.Ccr.Core;
using Microsoft.Dss.Core;
using Microsoft.Dss.Core.Attributes;
using Microsoft.Dss.ServiceModel.Dssp;
using Microsoft.Dss.ServiceModel.DsspServiceBase;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Security.Permissions;
using xml  = System.Xml;
using sm   = Microsoft.Dss.Services.SubscriptionManager;
using soap = W3C.Soap;
using dssp = Microsoft.Dss.ServiceModel.Dssp;
using Microsoft.Ccr.Adapters.WinForms;
using W3C.Soap;


namespace Robotics.Atani.Pioneer3DXrobotDashboard
{
   [DisplayName("Pioneer3DX Robot Dashboard")]
   [Description("Pioneer3DX Robot Dashboard Service")]
   [Contract(Contract.Identifier)]
   public class Pioneer3DXrobotDashboard : DsspServiceBase
   {
      // Dashboard form.
      private Pioneer3DXrobotDashboardForm _dashboardForm = null;

      // State.
      private Pioneer3DXrobotDashboardState _state = new Pioneer3DXrobotDashboardState();

      // Service port.
      [ServicePort("/pioneer3dxrobotdashboard", AllowMultipleInstances = true)]
      private Pioneer3DXrobotDashboardOperations _mainPort = new Pioneer3DXrobotDashboardOperations();

      // Subscription manager.
      [Partner("SubMgr", Contract = sm.Contract.Identifier, CreationPolicy = PartnerCreationPolicy.CreateAlways, Optional = false)]
      sm.SubscriptionManagerPort _submgr = new sm.SubscriptionManagerPort();

      /// <summary>
      /// Default Service Constructor
      /// </summary>
      public Pioneer3DXrobotDashboard(DsspServiceCreationPort creationPort) :
         base(creationPort)
      {
      }


      /// <summary>
      /// Service Start
      /// </summary>
      protected override void Start()
      {
         base.Start();

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
                        Arbiter.Receive<Sensors>(true, _mainPort, sensorsHandler),
                        Arbiter.Receive<Response>(true, _mainPort, responseHandler),
                        Arbiter.Receive<NeedChange>(true, _mainPort, needChangeHandler),
                        Arbiter.Receive<NeedSet>(true, _mainPort, needSetHandler),
                        Arbiter.Receive<ControlAction>(true, _mainPort, controlActionHandler),
                        Arbiter.Receive<ControlSet>(true, _mainPort, controlSetHandler),
                        Arbiter.Receive<RewardAction>(true, _mainPort, rewardActionHandler)
                     ),
                     new ConcurrentReceiverGroup
                     (
                        Arbiter.Receive<DsspDefaultLookup>(true, _mainPort, DefaultLookupHandler)
                     )
                     ));

         // Create form.
         createDashboardForm();
      }


      // Create the dashboard form.
      void createDashboardForm()
      {
         Fault fault = null;

         RunForm runForm = new RunForm(startDashboardForm);

         WinFormsServicePort.Post(runForm);

         Arbiter.Choice(
            runForm.pResult,
            delegate(SuccessResult success) { },
            delegate(Exception e)
            {
               fault = Fault.FromException(e);
            }
            );

         if (fault != null)
         {
            LogError("Cannot create Pioneer3DX robot dashboard", fault);
         }
      }


      // Start the dashboard form.
      System.Windows.Forms.Form startDashboardForm()
      {
         _dashboardForm = new Pioneer3DXrobotDashboardForm(_mainPort);

         invoke(delegate()
                {
                   _dashboardForm.Text = "Pioneer3DX Robot Dashboard";
                }
                );
         return(_dashboardForm);
      }


      void invoke(System.Windows.Forms.MethodInvoker mi)
      {
         WinFormsServicePort.Post(new FormInvoke(mi));
      }


      /// <summary>
      /// Get Handler
      /// </summary>
      /// <param name="get"></param>
      /// <returns></returns>
      [ServiceHandler(ServiceHandlerBehavior.Concurrent)]
      public virtual IEnumerator<ITask> GetHandler(Get get)
      {
         get.ResponsePort.Post(_state);
         yield break;
      }


      /// <summary>
      /// Replace Handler
      /// </summary>
      /// <param name="replace"></param>
      /// <returns></returns>
      [ServiceHandler(ServiceHandlerBehavior.Exclusive)]
      public virtual IEnumerator<ITask> ReplaceHandler(Replace replace)
      {
         replace.ResponsePort.Post(DefaultReplaceResponseType.Instance);
         SendNotification<Replace>(_submgr, _state);
         yield break;
      }


      // Drop handler shuts down service.
      [ServiceHandler(ServiceHandlerBehavior.Teardown)]
      public void DropHandler(DsspDefaultDrop drop)
      {
         base.DefaultDropHandler(drop);
      }


      // Handle sensors/response update.
      [ServiceHandler(ServiceHandlerBehavior.Exclusive)]
      public void sensorsHandler(Sensors sensors)
      {
         FormInvoke setSensors = new FormInvoke(
            delegate()
            {
               _dashboardForm.LRFSensorsValue = sensors.Body.LRFSensors;
               _dashboardForm.WebcamSensorsValue = sensors.Body.WebcamSensors;
               _dashboardForm.BumperSensorValue = sensors.Body.BumperSensor;
            }
            );
         updateDashboard(setSensors);
      }


      // Handle sensors/response update.
      [ServiceHandler(ServiceHandlerBehavior.Exclusive)]
      public void responseHandler(Response response)
      {
         FormInvoke setResponse = new FormInvoke(
            delegate()
            {
               _dashboardForm.ResponseValue = response.Body.Response;
            }
            );
         updateDashboard(setResponse);
      }


      // Handle need change.
      [ServiceHandler(ServiceHandlerBehavior.Exclusive)]
      public void needChangeHandler(NeedChange needChange)
      {
         needChange.ResponsePort.Post(DefaultUpdateResponseType.Instance);
         SendNotification(_submgr, needChange);
      }


      // Handle need setting.
      [ServiceHandler(ServiceHandlerBehavior.Exclusive)]
      public void needSetHandler(NeedSet needSet)
      {
         FormInvoke setNeed = new FormInvoke(
            delegate()
            {
               _dashboardForm.SuccessNeedValue = needSet.Body.SuccessNeed;
               _dashboardForm.RewardNeedValue = needSet.Body.RewardNeed;
            }
            );
         updateDashboard(setNeed);
      }


      // Handle control action.
      [ServiceHandler(ServiceHandlerBehavior.Exclusive)]
      public void controlActionHandler(ControlAction controlAction)
      {
         controlAction.ResponsePort.Post(DefaultUpdateResponseType.Instance);
         SendNotification(_submgr, controlAction);
      }


      // Handle control setting.
      [ServiceHandler(ServiceHandlerBehavior.Exclusive)]
      public void controlSetHandler(ControlSet controlSet)
      {
         FormInvoke setControls;

         if (controlSet.Body.PauseCheckValid)
         {
            setControls = new FormInvoke(
               delegate()
               {
                  _dashboardForm.PauseCheck = controlSet.Body.PauseCheck;
               }
               );
            updateDashboard(setControls);
         }

         if (controlSet.Body.StepLabelValid)
         {
            setControls = new FormInvoke(
               delegate()
               {
                  _dashboardForm.StepLabel = controlSet.Body.StepLabel;
               }
               );
            updateDashboard(setControls);
         }

         if (controlSet.Body.EnableDriverSelection)
         {
            setControls = new FormInvoke(
               delegate()
               {
                  _dashboardForm.EnableDriverSelection = true;
               }
               );
            updateDashboard(setControls);
         }

         if (controlSet.Body.EnableManualButtons)
         {
            setControls = new FormInvoke(
               delegate()
               {
                  _dashboardForm.enableManualButtons(true);
               }
               );
            updateDashboard(setControls);
         }

         if (controlSet.Body.DriverSelectMenu != null)
         {
            setControls = new FormInvoke(
               delegate()
               {
                  _dashboardForm.setDriverSelectMenu(controlSet.Body.DriverSelectMenu);
               }
               );
            updateDashboard(setControls);
         }

         if (controlSet.Body.DriverToolTip != null)
         {
            setControls = new FormInvoke(
               delegate()
               {
                  _dashboardForm.DriverToolTip = controlSet.Body.DriverToolTip;
               }
               );
            updateDashboard(setControls);
         }
      }


      // Handle reward action.
      [ServiceHandler(ServiceHandlerBehavior.Exclusive)]
      public void rewardActionHandler(RewardAction rewardAction)
      {
         rewardAction.ResponsePort.Post(DefaultUpdateResponseType.Instance);
         SendNotification(_submgr, rewardAction);
      }


      [ServiceHandler(ServiceHandlerBehavior.Concurrent)]
      public virtual IEnumerator<ITask> SubscribeHandler(Subscribe subscribe)
      {
         SubscribeHelper(_submgr, subscribe.Body, subscribe.ResponsePort);
         yield break;
      }


      // Update dashboard.
      void updateDashboard(FormInvoke update)
      {
         if (_dashboardForm == null) { return; }

         WinFormsServicePort.Post(update);

         Fault fault = null;
         Arbiter.Choice(
            update.ResultPort,
            delegate(EmptyValue success) { },
            delegate(Exception e)
            {
               fault = Fault.FromException(e);
            }
            );
         if (fault != null)
         {
            LogError("Cannot update Pioneer3DX robot dashboard", fault);
         }
      }
   }
}
