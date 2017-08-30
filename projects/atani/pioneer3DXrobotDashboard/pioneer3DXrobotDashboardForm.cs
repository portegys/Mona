//-----------------------------------------------------------------------
//  $File: pioneer3DXrobotDashboardForm.cs $ $Revision: 1 $
//-----------------------------------------------------------------------
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Security.Permissions;
using System.Windows.Forms;
using Microsoft.Dss.ServiceModel.Dssp;
using winforms = System.Windows.Forms;

namespace Robotics.Atani.Pioneer3DXrobotDashboard
{
   public partial class Pioneer3DXrobotDashboardForm : Form
   {
      Pioneer3DXrobotDashboardOperations _mainPort;

      ToolTip _toolTip;

      public Pioneer3DXrobotDashboardForm(Pioneer3DXrobotDashboardOperations mainPort)
      {
         _mainPort = mainPort;

         InitializeComponent();

         // Create tooltips.
         _toolTip = new ToolTip();
         _toolTip.AutoPopDelay = 5000;
         _toolTip.InitialDelay = 1000;
         _toolTip.ReshowDelay  = 500;
         _toolTip.ShowAlways   = true;
         _toolTip.SetToolTip(this.successNeedSlider, "Success need value");
         _toolTip.SetToolTip(this.rewardNeedSlider, "Reward need value");
         _toolTip.SetToolTip(this.resetButton, "Reset world");
         _toolTip.SetToolTip(this.driverSelect, "Driverselection");
         _toolTip.SetToolTip(this.btnLeft, "Manual move left");
         _toolTip.SetToolTip(this.btnForward, "Manual move forward");
         _toolTip.SetToolTip(this.btnRight, "Manual move right");
         _toolTip.SetToolTip(this.rewardSlider, "Reward value setting");
         _toolTip.SetToolTip(this.rewardActionSelect, "Select reward action");
         _toolTip.SetToolTip(this.rewardActionButton, "Execute reward action");
      }


      protected override void OnClosed(EventArgs e)
      {
         _mainPort.Post(new DsspDefaultDrop(DropRequestType.Instance));
         base.OnClosed(e);
      }


      // Range sensors value.
      public string LRFSensorsValue
      {
         get { return(lrfSensorsValue.Text); }
         set { lrfSensorsValue.Text = value; }
      }

      // Webcam sensors value.
      public string WebcamSensorsValue
      {
         get { return(webcamSensorsValue.Text); }
         set { webcamSensorsValue.Text = value; }
      }

      // Bumper sensor value.
      public string BumperSensorValue
      {
         get { return(bumperSensorValue.Text); }
         set { bumperSensorValue.Text = value; }
      }

      // Response value.
      public string ResponseValue
      {
         get { return(responseValue.Text); }
         set { responseValue.Text = value; }
      }

      // Success need value.
      public double SuccessNeedValue
      {
         get
         {
            return((double)successNeedSlider.Value / (double)successNeedSlider.Maximum);
         }
         set
         {
            successNeedSlider.Value = (int)(value * 100.0);
            successNeedValue.Text   = value + "";
         }
      }

      // Success need change.
      private void successNeedChange(object sender, System.EventArgs e)
      {
         double successNeed = (double)successNeedSlider.Value / (double)successNeedSlider.Maximum;
         double rewardNeed  = (double)rewardNeedSlider.Value / (double)rewardNeedSlider.Maximum;

         successNeedValue.Text = successNeed + "";
         _mainPort.Post(new NeedChange(new NeedChangeRequest(successNeed, rewardNeed)));
      }


      // Reward need value.
      public double RewardNeedValue
      {
         get
         {
            return((double)rewardNeedSlider.Value / (double)rewardNeedSlider.Maximum);
         }
         set
         {
            rewardNeedSlider.Value = (int)(value * 100.0);
            rewardNeedValue.Text   = value + "";
         }
      }

      // Reward need change.
      private void rewardNeedChange(object sender, System.EventArgs e)
      {
         double successNeed = (double)successNeedSlider.Value / (double)successNeedSlider.Maximum;
         double rewardNeed  = (double)rewardNeedSlider.Value / (double)rewardNeedSlider.Maximum;

         rewardNeedValue.Text = rewardNeed + "";
         _mainPort.Post(new NeedChange(new NeedChangeRequest(successNeed, rewardNeed)));
      }


      // Reset button.
      private void resetClick(object sender, EventArgs e)
      {
         ControlActionRequest controlActionRequest = new ControlActionRequest();

         controlActionRequest.ResetClick = true;
         _mainPort.Post(new ControlAction(controlActionRequest));
      }


      // Forget menu click.
      private void forgetClick(object sender, EventArgs e)
      {
         ControlActionRequest controlActionRequest = new ControlActionRequest();

         controlActionRequest.ForgetClick = true;
         _mainPort.Post(new ControlAction(controlActionRequest));
      }


      // Save menu click.
      private void saveClick(object sender, EventArgs e)
      {
         ControlActionRequest controlActionRequest = new ControlActionRequest();

         controlActionRequest.SaveClick = true;
         _mainPort.Post(new ControlAction(controlActionRequest));
      }


      // Print menu click.
      private void printClick(object sender, EventArgs e)
      {
         ControlActionRequest controlActionRequest = new ControlActionRequest();

         controlActionRequest.PrintBrain = true;
         _mainPort.Post(new ControlAction(controlActionRequest));
      }


      // Pause check box.
      public bool PauseCheck
      {
         get { return(pauseCheck.Checked); }
         set { pauseCheck.Checked = value; }
      }
      void pauseCheckChanged(object obj, EventArgs ea)
      {
         ControlActionRequest controlActionRequest = new ControlActionRequest();

         controlActionRequest.PauseCheck = pauseCheck.Checked;
         _mainPort.Post(new ControlAction(controlActionRequest));
      }


      // Step button.
      public string StepLabel
      {
         get { return(stepButton.Text); }
         set { stepButton.Text = value; }
      }
      private void stepClick(object sender, EventArgs e)
      {
         ControlActionRequest controlActionRequest = new ControlActionRequest();

         controlActionRequest.StepClick = true;
         _mainPort.Post(new ControlAction(controlActionRequest));
      }


      // Driver selection.
      public bool EnableDriverSelection
      {
         get { return(driverSelect.Enabled); }
         set { driverSelect.Enabled = value; }
      }
      public string DriverToolTip
      {
         get { return(_toolTip.GetToolTip(driverSelect)); }
         set { _toolTip.SetToolTip(this.driverSelect, value); }
      }
      public void setDriverSelectMenu(List<string> menu)
      {
         driverSelect.Text = menu[0];
         driverSelect.Items.Clear();
         foreach (string item in menu)
         {
            driverSelect.Items.Add(item);
         }
      }


      private void driverSelection(object sender, EventArgs e)
      {
         ControlActionRequest controlActionRequest = new ControlActionRequest();

         controlActionRequest.Driver = driverSelect.SelectedIndex;
         driverSelect.Enabled        = false;
         enableManualButtons(false);
         _mainPort.Post(new ControlAction(controlActionRequest));
      }


      // Move left button.
      private void moveLeftClick(object sender, EventArgs e)
      {
         ControlActionRequest controlActionRequest = new ControlActionRequest();

         controlActionRequest.MoveLeftClick = true;
         driverSelect.Enabled = false;
         enableManualButtons(false);
         _mainPort.Post(new ControlAction(controlActionRequest));
      }


      // Move forward button.
      private void moveForwardClick(object sender, EventArgs e)
      {
         ControlActionRequest controlActionRequest = new ControlActionRequest();

         controlActionRequest.MoveForwardClick = true;
         driverSelect.Enabled = false;
         enableManualButtons(false);
         _mainPort.Post(new ControlAction(controlActionRequest));
      }


      // Move right button.
      private void moveRightClick(object sender, EventArgs e)
      {
         ControlActionRequest controlActionRequest = new ControlActionRequest();

         controlActionRequest.MoveRightClick = true;
         driverSelect.Enabled = false;
         enableManualButtons(false);
         _mainPort.Post(new ControlAction(controlActionRequest));
      }


      // Enable/disable manual buttons.
      public void enableManualButtons(bool enable)
      {
         btnLeft.Enabled    = enable;
         btnForward.Enabled = enable;
         btnRight.Enabled   = enable;
      }


      // Reward change.
      private void rewardChange(object sender, EventArgs e)
      {
         double reward = (double)rewardSlider.Value / (double)rewardSlider.Maximum;

         rewardValue.Text = reward + "";
      }


      // Reward action button.
      private void rewardActionClick(object sender, EventArgs e)
      {
         double              reward = (double)rewardSlider.Value / (double)rewardSlider.Maximum;
         RewardActionRequest rewardActionRequest = new RewardActionRequest();

         rewardActionRequest.RewardAction = rewardActionSelect.SelectedIndex;
         rewardActionRequest.RewardValue  = reward;
         _mainPort.Post(new RewardAction(rewardActionRequest));
      }
   }
}
