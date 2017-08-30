//-----------------------------------------------------------------------
//  $File: pioneer3DXrobotDashboard.Designer.cs $ $Revision: 1 $
//-----------------------------------------------------------------------
namespace Robotics.Atani.Pioneer3DXrobotDashboard
{
   partial class Pioneer3DXrobotDashboardForm
   {
      /// <summary>
      /// Required designer variable.
      /// </summary>
      private System.ComponentModel.IContainer components = null;

      /// <summary>
      /// Clean up any resources being used.
      /// </summary>
      /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
      protected override void Dispose(bool disposing)
      {
         if (disposing && (components != null))
         {
            components.Dispose();
         }
         base.Dispose(disposing);
      }


      #region Windows Form Designer generated code

      /// <summary>
      /// Required method for Designer support - do not modify
      /// the contents of this method with the code editor.
      /// </summary>
      private void InitializeComponent()
      {
         this.components = new System.ComponentModel.Container();
         System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Pioneer3DXrobotDashboardForm));
         this.sensorsResponseBox = new System.Windows.Forms.GroupBox();
         this.lrfSensorsValue    = new System.Windows.Forms.Label();
         this.lrfSensorsLabel    = new System.Windows.Forms.Label();
         this.webcamSensorsValue = new System.Windows.Forms.Label();
         this.webcamSensorsLabel = new System.Windows.Forms.Label();
         this.bumperSensorValue  = new System.Windows.Forms.Label();
         this.bumperSensorLabel  = new System.Windows.Forms.Label();
         this.responseValue      = new System.Windows.Forms.Label();
         this.responseLabel      = new System.Windows.Forms.Label();
         this.needsBox           = new System.Windows.Forms.GroupBox();
         this.successNeedSlider  = new System.Windows.Forms.TrackBar();
         this.successNeedLabel   = new System.Windows.Forms.Label();
         this.successNeedValue   = new System.Windows.Forms.Label();
         this.rewardNeedSlider   = new System.Windows.Forms.TrackBar();
         this.rewardNeedLabel    = new System.Windows.Forms.Label();
         this.rewardNeedValue    = new System.Windows.Forms.Label();
         this.controlBox         = new System.Windows.Forms.GroupBox();
         this.resetButton        = new System.Windows.Forms.Button();
         this.driverMoveLabel    = new System.Windows.Forms.Label();
         this.driverSelectLabel  = new System.Windows.Forms.Label();
         this.pauseCheck         = new System.Windows.Forms.CheckBox();
         this.stepButton         = new System.Windows.Forms.Button();
         this.driverSelect       = new System.Windows.Forms.ComboBox();
         this.btnLeft            = new System.Windows.Forms.Button();
         this.btnForward         = new System.Windows.Forms.Button();
         this.btnRight           = new System.Windows.Forms.Button();
         this.rewardBox          = new System.Windows.Forms.GroupBox();
         this.rewardValue        = new System.Windows.Forms.Label();
         this.rewardSlider       = new System.Windows.Forms.TrackBar();
         this.rewardLabel        = new System.Windows.Forms.Label();
         this.rewardActionLabel  = new System.Windows.Forms.Label();
         this.rewardActionButton = new System.Windows.Forms.Button();
         this.rewardActionSelect = new System.Windows.Forms.ComboBox();
         this.mainMenu           = new System.Windows.Forms.MainMenu(this.components);
         this.fileMenuItem       = new System.Windows.Forms.MenuItem();
         this.clearMenuItem      = new System.Windows.Forms.MenuItem();
         this.saveMenuItem       = new System.Windows.Forms.MenuItem();
         this.printMenuItem      = new System.Windows.Forms.MenuItem();
         this.sensorsResponseBox.SuspendLayout();
         this.needsBox.SuspendLayout();
         ((System.ComponentModel.ISupportInitialize)(this.successNeedSlider)).BeginInit();
         ((System.ComponentModel.ISupportInitialize)(this.rewardNeedSlider)).BeginInit();
         this.controlBox.SuspendLayout();
         this.rewardBox.SuspendLayout();
         ((System.ComponentModel.ISupportInitialize)(this.rewardSlider)).BeginInit();
         this.SuspendLayout();
         //
         // sensorsResponseBox
         //
         this.sensorsResponseBox.Controls.Add(this.lrfSensorsValue);
         this.sensorsResponseBox.Controls.Add(this.lrfSensorsLabel);
         this.sensorsResponseBox.Controls.Add(this.webcamSensorsValue);
         this.sensorsResponseBox.Controls.Add(this.webcamSensorsLabel);
         this.sensorsResponseBox.Controls.Add(this.bumperSensorValue);
         this.sensorsResponseBox.Controls.Add(this.bumperSensorLabel);
         this.sensorsResponseBox.Controls.Add(this.responseValue);
         this.sensorsResponseBox.Controls.Add(this.responseLabel);
         this.sensorsResponseBox.Location = new System.Drawing.Point(12, 12);
         this.sensorsResponseBox.Name     = "sensorsResponseBox";
         this.sensorsResponseBox.Size     = new System.Drawing.Size(297, 160);
         this.sensorsResponseBox.TabIndex = 0;
         this.sensorsResponseBox.TabStop  = false;
         this.sensorsResponseBox.Text     = "Sensors/Response";
         //
         // lrfSensorsValue
         //
         this.lrfSensorsValue.AutoSize    = true;
         this.lrfSensorsValue.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
         this.lrfSensorsValue.Location    = new System.Drawing.Point(104, 35);
         this.lrfSensorsValue.MinimumSize = new System.Drawing.Size(80, 15);
         this.lrfSensorsValue.Name        = "lrfSensorsValue";
         this.lrfSensorsValue.Size        = new System.Drawing.Size(80, 15);
         this.lrfSensorsValue.TabIndex    = 1;
         //
         // lrfSensorsLabel
         //
         this.lrfSensorsLabel.AutoSize = true;
         this.lrfSensorsLabel.Location = new System.Drawing.Point(6, 35);
         this.lrfSensorsLabel.Name     = "lrfSensorsLabel";
         this.lrfSensorsLabel.Size     = new System.Drawing.Size(69, 13);
         this.lrfSensorsLabel.TabIndex = 0;
         this.lrfSensorsLabel.Text     = "LRF sensors:";
         //
         // webcamSensorsValue
         //
         this.webcamSensorsValue.AutoSize    = true;
         this.webcamSensorsValue.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
         this.webcamSensorsValue.Location    = new System.Drawing.Point(104, 60);
         this.webcamSensorsValue.MinimumSize = new System.Drawing.Size(80, 15);
         this.webcamSensorsValue.Name        = "webcamSensorsValue";
         this.webcamSensorsValue.Size        = new System.Drawing.Size(80, 15);
         this.webcamSensorsValue.TabIndex    = 1;
         //
         // webcamSensorsLabel
         //
         this.webcamSensorsLabel.AutoSize = true;
         this.webcamSensorsLabel.Location = new System.Drawing.Point(6, 60);
         this.webcamSensorsLabel.Name     = "webcamSensorsLabel";
         this.webcamSensorsLabel.Size     = new System.Drawing.Size(92, 13);
         this.webcamSensorsLabel.TabIndex = 0;
         this.webcamSensorsLabel.Text     = "Webcam sensors:";
         //
         // bumperSensorValue
         //
         this.bumperSensorValue.AutoSize    = true;
         this.bumperSensorValue.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
         this.bumperSensorValue.Location    = new System.Drawing.Point(104, 83);
         this.bumperSensorValue.MinimumSize = new System.Drawing.Size(80, 15);
         this.bumperSensorValue.Name        = "bumperSensorValue";
         this.bumperSensorValue.Size        = new System.Drawing.Size(80, 15);
         this.bumperSensorValue.TabIndex    = 1;
         //
         // bumperSensorLabel
         //
         this.bumperSensorLabel.AutoSize = true;
         this.bumperSensorLabel.Location = new System.Drawing.Point(7, 83);
         this.bumperSensorLabel.Name     = "bumperSensorLabel";
         this.bumperSensorLabel.Size     = new System.Drawing.Size(80, 13);
         this.bumperSensorLabel.TabIndex = 0;
         this.bumperSensorLabel.Text     = "Bumper sensor:";
         //
         // responseValue
         //
         this.responseValue.AutoSize    = true;
         this.responseValue.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
         this.responseValue.Location    = new System.Drawing.Point(104, 125);
         this.responseValue.MinimumSize = new System.Drawing.Size(70, 15);
         this.responseValue.Name        = "responseValue";
         this.responseValue.Size        = new System.Drawing.Size(70, 15);
         this.responseValue.TabIndex    = 3;
         //
         // responseLabel
         //
         this.responseLabel.AutoSize = true;
         this.responseLabel.Location = new System.Drawing.Point(7, 127);
         this.responseLabel.Name     = "responseLabel";
         this.responseLabel.Size     = new System.Drawing.Size(58, 13);
         this.responseLabel.TabIndex = 2;
         this.responseLabel.Text     = "Response:";
         //
         // needsBox
         //
         this.needsBox.Controls.Add(this.successNeedSlider);
         this.needsBox.Controls.Add(this.successNeedLabel);
         this.needsBox.Controls.Add(this.successNeedValue);
         this.needsBox.Controls.Add(this.rewardNeedSlider);
         this.needsBox.Controls.Add(this.rewardNeedLabel);
         this.needsBox.Controls.Add(this.rewardNeedValue);
         this.needsBox.Location = new System.Drawing.Point(12, 178);
         this.needsBox.Name     = "needsBox";
         this.needsBox.Size     = new System.Drawing.Size(297, 119);
         this.needsBox.TabIndex = 1;
         this.needsBox.TabStop  = false;
         this.needsBox.Text     = "Needs";
         //
         // successNeedSlider
         //
         this.successNeedSlider.LargeChange   = 10;
         this.successNeedSlider.Location      = new System.Drawing.Point(58, 19);
         this.successNeedSlider.Maximum       = 100;
         this.successNeedSlider.Name          = "successNeedSlider";
         this.successNeedSlider.Size          = new System.Drawing.Size(194, 45);
         this.successNeedSlider.TabIndex      = 0;
         this.successNeedSlider.TickFrequency = 5;
         this.successNeedSlider.Scroll       += new System.EventHandler(this.successNeedChange);
         //
         // successNeedLabel
         //
         this.successNeedLabel.AutoSize = true;
         this.successNeedLabel.Location = new System.Drawing.Point(9, 33);
         this.successNeedLabel.Name     = "successNeedLabel";
         this.successNeedLabel.Size     = new System.Drawing.Size(51, 13);
         this.successNeedLabel.TabIndex = 2;
         this.successNeedLabel.Text     = "Success:";
         //
         // successNeedValue
         //
         this.successNeedValue.AutoSize = true;
         this.successNeedValue.Location = new System.Drawing.Point(258, 33);
         this.successNeedValue.Name     = "successNeedValue";
         this.successNeedValue.Size     = new System.Drawing.Size(22, 13);
         this.successNeedValue.TabIndex = 2;
         this.successNeedValue.Text     = "0.0";
         //
         // rewardNeedSlider
         //
         this.rewardNeedSlider.LargeChange   = 10;
         this.rewardNeedSlider.Location      = new System.Drawing.Point(58, 68);
         this.rewardNeedSlider.Maximum       = 100;
         this.rewardNeedSlider.Name          = "rewardNeedSlider";
         this.rewardNeedSlider.Size          = new System.Drawing.Size(194, 45);
         this.rewardNeedSlider.TabIndex      = 1;
         this.rewardNeedSlider.TickFrequency = 5;
         this.rewardNeedSlider.Scroll       += new System.EventHandler(this.rewardNeedChange);
         //
         // rewardNeedLabel
         //
         this.rewardNeedLabel.AutoSize = true;
         this.rewardNeedLabel.Location = new System.Drawing.Point(13, 79);
         this.rewardNeedLabel.Name     = "rewardNeedLabel";
         this.rewardNeedLabel.Size     = new System.Drawing.Size(47, 13);
         this.rewardNeedLabel.TabIndex = 2;
         this.rewardNeedLabel.Text     = "Reward:";
         //
         // rewardNeedValue
         //
         this.rewardNeedValue.AutoSize = true;
         this.rewardNeedValue.Location = new System.Drawing.Point(258, 79);
         this.rewardNeedValue.Name     = "rewardNeedValue";
         this.rewardNeedValue.Size     = new System.Drawing.Size(22, 13);
         this.rewardNeedValue.TabIndex = 2;
         this.rewardNeedValue.Text     = "0.0";
         //
         // controlBox
         //
         this.controlBox.Controls.Add(this.resetButton);
         this.controlBox.Controls.Add(this.driverMoveLabel);
         this.controlBox.Controls.Add(this.driverSelectLabel);
         this.controlBox.Controls.Add(this.pauseCheck);
         this.controlBox.Controls.Add(this.stepButton);
         this.controlBox.Controls.Add(this.driverSelect);
         this.controlBox.Controls.Add(this.btnLeft);
         this.controlBox.Controls.Add(this.btnForward);
         this.controlBox.Controls.Add(this.btnRight);
         this.controlBox.Location = new System.Drawing.Point(315, 12);
         this.controlBox.Name     = "controlBox";
         this.controlBox.Size     = new System.Drawing.Size(297, 160);
         this.controlBox.TabIndex = 1;
         this.controlBox.TabStop  = false;
         this.controlBox.Text     = "Controls";
         //
         // resetButton
         //
         this.resetButton.Location = new System.Drawing.Point(216, 25);
         this.resetButton.Name     = "resetButton";
         this.resetButton.Size     = new System.Drawing.Size(75, 23);
         this.resetButton.TabIndex = 11;
         this.resetButton.Tag      = "";
         this.resetButton.Text     = "Reset";
         this.resetButton.UseVisualStyleBackColor = true;
         this.resetButton.Click += new System.EventHandler(this.resetClick);
         //
         // driverMoveLabel
         //
         this.driverMoveLabel.AutoSize = true;
         this.driverMoveLabel.Location = new System.Drawing.Point(10, 102);
         this.driverMoveLabel.Name     = "driverMoveLabel";
         this.driverMoveLabel.Size     = new System.Drawing.Size(37, 13);
         this.driverMoveLabel.TabIndex = 8;
         this.driverMoveLabel.Text     = "Move:";
         //
         // driverSelectLabel
         //
         this.driverSelectLabel.AutoSize = true;
         this.driverSelectLabel.Location = new System.Drawing.Point(10, 63);
         this.driverSelectLabel.Name     = "driverSelectLabel";
         this.driverSelectLabel.Size     = new System.Drawing.Size(38, 13);
         this.driverSelectLabel.TabIndex = 4;
         this.driverSelectLabel.Text     = "Driver:";
         //
         // pauseCheck
         //
         this.pauseCheck.AutoSize                = true;
         this.pauseCheck.Checked                 = true;
         this.pauseCheck.CheckState              = System.Windows.Forms.CheckState.Checked;
         this.pauseCheck.Location                = new System.Drawing.Point(54, 29);
         this.pauseCheck.Name                    = "pauseCheck";
         this.pauseCheck.RightToLeft             = System.Windows.Forms.RightToLeft.Yes;
         this.pauseCheck.Size                    = new System.Drawing.Size(56, 17);
         this.pauseCheck.TabIndex                = 0;
         this.pauseCheck.Text                    = "Pause";
         this.pauseCheck.UseVisualStyleBackColor = true;
         this.pauseCheck.CheckedChanged         += new System.EventHandler(this.pauseCheckChanged);
         //
         // stepButton
         //
         this.stepButton.Location = new System.Drawing.Point(135, 25);
         this.stepButton.Name     = "stepButton";
         this.stepButton.Size     = new System.Drawing.Size(75, 23);
         this.stepButton.TabIndex = 1;
         this.stepButton.Text     = "Step";
         this.stepButton.UseVisualStyleBackColor = true;
         this.stepButton.Click += new System.EventHandler(this.stepClick);
         //
         // driverSelect
         //
         this.driverSelect.Location              = new System.Drawing.Point(54, 60);
         this.driverSelect.Name                  = "driverSelect";
         this.driverSelect.Size                  = new System.Drawing.Size(75, 21);
         this.driverSelect.TabIndex              = 2;
         this.driverSelect.SelectedIndexChanged += new System.EventHandler(this.driverSelection);
         //
         // btnLeft
         //
         this.btnLeft.Enabled  = false;
         this.btnLeft.Font     = new System.Drawing.Font("Marlett", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(2)));
         this.btnLeft.Location = new System.Drawing.Point(54, 95);
         this.btnLeft.Name     = "btnLeft";
         this.btnLeft.Size     = new System.Drawing.Size(75, 23);
         this.btnLeft.TabIndex = 1;
         this.btnLeft.Text     = "3";
         this.btnLeft.UseVisualStyleBackColor = true;
         this.btnLeft.Click += new System.EventHandler(this.moveLeftClick);
         //
         // btnForward
         //
         this.btnForward.Enabled  = false;
         this.btnForward.Font     = new System.Drawing.Font("Marlett", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(2)));
         this.btnForward.Location = new System.Drawing.Point(135, 95);
         this.btnForward.Name     = "btnForward";
         this.btnForward.Size     = new System.Drawing.Size(75, 23);
         this.btnForward.TabIndex = 2;
         this.btnForward.Text     = "5";
         this.btnForward.UseVisualStyleBackColor = true;
         this.btnForward.Click += new System.EventHandler(this.moveForwardClick);
         //
         // btnRight
         //
         this.btnRight.Enabled  = false;
         this.btnRight.Font     = new System.Drawing.Font("Marlett", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(2)));
         this.btnRight.Location = new System.Drawing.Point(216, 95);
         this.btnRight.Name     = "btnRight";
         this.btnRight.Size     = new System.Drawing.Size(75, 23);
         this.btnRight.TabIndex = 3;
         this.btnRight.Text     = "4";
         this.btnRight.UseVisualStyleBackColor = true;
         this.btnRight.Click += new System.EventHandler(this.moveRightClick);
         //
         // rewardBox
         //
         this.rewardBox.Controls.Add(this.rewardValue);
         this.rewardBox.Controls.Add(this.rewardSlider);
         this.rewardBox.Controls.Add(this.rewardLabel);
         this.rewardBox.Controls.Add(this.rewardActionLabel);
         this.rewardBox.Controls.Add(this.rewardActionButton);
         this.rewardBox.Controls.Add(this.rewardActionSelect);
         this.rewardBox.Location = new System.Drawing.Point(315, 178);
         this.rewardBox.Name     = "rewardBox";
         this.rewardBox.Size     = new System.Drawing.Size(297, 119);
         this.rewardBox.TabIndex = 2;
         this.rewardBox.TabStop  = false;
         this.rewardBox.Text     = "Reward";
         //
         // rewardValue
         //
         this.rewardValue.AutoSize = true;
         this.rewardValue.Location = new System.Drawing.Point(261, 79);
         this.rewardValue.Name     = "rewardValue";
         this.rewardValue.Size     = new System.Drawing.Size(22, 13);
         this.rewardValue.TabIndex = 10;
         this.rewardValue.Text     = "0.0";
         //
         // rewardSlider
         //
         this.rewardSlider.LargeChange   = 10;
         this.rewardSlider.Location      = new System.Drawing.Point(61, 69);
         this.rewardSlider.Maximum       = 100;
         this.rewardSlider.Name          = "rewardSlider";
         this.rewardSlider.Size          = new System.Drawing.Size(194, 45);
         this.rewardSlider.TabIndex      = 9;
         this.rewardSlider.TickFrequency = 5;
         this.rewardSlider.Scroll       += new System.EventHandler(this.rewardChange);
         //
         // rewardLabel
         //
         this.rewardLabel.AutoSize = true;
         this.rewardLabel.Location = new System.Drawing.Point(10, 79);
         this.rewardLabel.Name     = "rewardLabel";
         this.rewardLabel.Size     = new System.Drawing.Size(47, 13);
         this.rewardLabel.TabIndex = 8;
         this.rewardLabel.Text     = "Reward:";
         //
         // rewardActionLabel
         //
         this.rewardActionLabel.AutoSize = true;
         this.rewardActionLabel.Location = new System.Drawing.Point(10, 34);
         this.rewardActionLabel.Name     = "rewardActionLabel";
         this.rewardActionLabel.Size     = new System.Drawing.Size(40, 13);
         this.rewardActionLabel.TabIndex = 4;
         this.rewardActionLabel.Text     = "Action:";
         //
         // rewardActionButton
         //
         this.rewardActionButton.Location = new System.Drawing.Point(135, 30);
         this.rewardActionButton.Name     = "rewardActionButton";
         this.rewardActionButton.Size     = new System.Drawing.Size(75, 23);
         this.rewardActionButton.TabIndex = 1;
         this.rewardActionButton.Text     = "Go";
         this.rewardActionButton.UseVisualStyleBackColor = true;
         this.rewardActionButton.Click += new System.EventHandler(this.rewardActionClick);
         //
         // rewardActionSelect
         //
         this.rewardActionSelect.Items.AddRange(new object[] {
                                                   "create",
                                                   "disable",
                                                   "enable all",
                                                   "remove",
                                                   "remove all"
                                                }
                                                );
         this.rewardActionSelect.Location = new System.Drawing.Point(54, 30);
         this.rewardActionSelect.Name     = "rewardActionSelect";
         this.rewardActionSelect.Size     = new System.Drawing.Size(75, 21);
         this.rewardActionSelect.TabIndex = 2;
         this.rewardActionSelect.Text     = "create";
         //
         // mainMenu
         //
         this.mainMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                             this.fileMenuItem
                                          }
                                          );
         //
         // fileMenuItem
         //
         this.fileMenuItem.Index = 0;
         this.fileMenuItem.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                 this.clearMenuItem,
                                                 this.saveMenuItem,
                                                 this.printMenuItem
                                              }
                                              );
         this.fileMenuItem.Text = "File";
         //
         // clearMenuItem
         //
         this.clearMenuItem.Index  = 0;
         this.clearMenuItem.Text   = "Clear";
         this.clearMenuItem.Click += new System.EventHandler(this.forgetClick);
         //
         // saveMenuItem
         //
         this.saveMenuItem.Index  = 1;
         this.saveMenuItem.Text   = "Save";
         this.saveMenuItem.Click += new System.EventHandler(this.saveClick);
         //
         // printMenuItem
         //
         this.printMenuItem.Index  = 2;
         this.printMenuItem.Text   = "Print";
         this.printMenuItem.Click += new System.EventHandler(this.printClick);
         //
         // Pioneer3DXrobotDashboardForm
         //
         this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
         this.AutoScaleMode       = System.Windows.Forms.AutoScaleMode.Font;
         this.ClientSize          = new System.Drawing.Size(623, 304);
         this.Controls.Add(this.rewardBox);
         this.Controls.Add(this.sensorsResponseBox);
         this.Controls.Add(this.needsBox);
         this.Controls.Add(this.controlBox);
         this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
         this.Icon            = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
         this.KeyPreview      = true;
         this.Menu            = this.mainMenu;
         this.Name            = "Pioneer3DXrobotDashboardForm";
         this.Text            = "Pioneer3DX Robot Dashboard";
         this.sensorsResponseBox.ResumeLayout(false);
         this.sensorsResponseBox.PerformLayout();
         this.needsBox.ResumeLayout(false);
         this.needsBox.PerformLayout();
         ((System.ComponentModel.ISupportInitialize)(this.successNeedSlider)).EndInit();
         ((System.ComponentModel.ISupportInitialize)(this.rewardNeedSlider)).EndInit();
         this.controlBox.ResumeLayout(false);
         this.controlBox.PerformLayout();
         this.rewardBox.ResumeLayout(false);
         this.rewardBox.PerformLayout();
         ((System.ComponentModel.ISupportInitialize)(this.rewardSlider)).EndInit();
         this.ResumeLayout(false);
      }


      #endregion

      private System.Windows.Forms.GroupBox sensorsResponseBox;
      private System.Windows.Forms.Label    lrfSensorsValue;
      private System.Windows.Forms.Label    lrfSensorsLabel;
      private System.Windows.Forms.Label    webcamSensorsValue;
      private System.Windows.Forms.Label    webcamSensorsLabel;
      private System.Windows.Forms.Label    bumperSensorValue;
      private System.Windows.Forms.Label    bumperSensorLabel;
      private System.Windows.Forms.Label    responseLabel;
      private System.Windows.Forms.Label    responseValue;
      private System.Windows.Forms.GroupBox needsBox;
      private System.Windows.Forms.TrackBar successNeedSlider;
      private System.Windows.Forms.Label    successNeedLabel;
      private System.Windows.Forms.Label    successNeedValue;
      private System.Windows.Forms.TrackBar rewardNeedSlider;
      private System.Windows.Forms.Label    rewardNeedLabel;
      private System.Windows.Forms.Label    rewardNeedValue;
      private System.Windows.Forms.GroupBox controlBox;
      private System.Windows.Forms.CheckBox pauseCheck;
      private System.Windows.Forms.Button   stepButton;
      private System.Windows.Forms.ComboBox driverSelect;
      private System.Windows.Forms.Button   btnLeft;
      private System.Windows.Forms.Button   btnForward;
      private System.Windows.Forms.Button   btnRight;
      private System.Windows.Forms.Label    driverSelectLabel;
      private System.Windows.Forms.Label    driverMoveLabel;
      private System.Windows.Forms.GroupBox rewardBox;
      private System.Windows.Forms.Label    rewardLabel;
      private System.Windows.Forms.Label    rewardActionLabel;
      private System.Windows.Forms.Button   rewardActionButton;
      private System.Windows.Forms.ComboBox rewardActionSelect;
      private System.Windows.Forms.Label    rewardValue;
      private System.Windows.Forms.TrackBar rewardSlider;
      private System.Windows.Forms.Button   resetButton;
      private System.Windows.Forms.MainMenu mainMenu;
      private System.Windows.Forms.MenuItem fileMenuItem;
      private System.Windows.Forms.MenuItem clearMenuItem;
      private System.Windows.Forms.MenuItem saveMenuItem;
      private System.Windows.Forms.MenuItem printMenuItem;
   }
}
