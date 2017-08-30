//  Atani dashboard.

package atani.legoNXT;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;

// Atani dashboard.
public class AtaniDashboard extends JFrame
{
   // Title.
   public static final String TITLE = "Atani";

   // Components.
   SensorsResponsePanel sensorsResponse;
   NeedsPanel           needs;
   ControlsPanel        controls;
   RewardPanel          reward;
   boolean              quit = false;

   // Callback target.
   Atani atani;

   // Driver types.
   enum DRIVER_TYPE
   {
      ATANI(0),
      OVERRIDE(1),
      HIJACK(2);

      private int value;

      DRIVER_TYPE(int value)
      {
         this.value = value;
      }

      public int getValue()
      {
         return(value);
      }
   }

   // Reward actions.
   public enum REWARD_ACTION
   {
      CREATE(0),
      DISABLE(1),
      ENABLE_ALL(2),
      REMOVE(3),
      REMOVE_ALL(4);

      private int value;

      REWARD_ACTION(int value)
      {
         this.value = value;
      }

      public int getValue()
      {
         return(value);
      }
   }

   // Constructor.
   public AtaniDashboard(Atani atani)
   {
      this.atani = atani;

      setTitle(TITLE);
      addWindowListener(new WindowAdapter()
                        {
                           public void windowClosing(WindowEvent e) { quit = true; }
                        }
                        );
      JPanel basePanel = (JPanel)getContentPane();
      basePanel.setLayout(new BorderLayout());
      JPanel leftPanel = new JPanel();
      leftPanel.setLayout(new BorderLayout());
      basePanel.add(leftPanel, BorderLayout.WEST);
      JPanel rightPanel = new JPanel();
      rightPanel.setLayout(new BorderLayout());
      basePanel.add(rightPanel, BorderLayout.EAST);
      sensorsResponse = new SensorsResponsePanel();
      leftPanel.add(sensorsResponse, BorderLayout.NORTH);
      needs = new NeedsPanel();
      leftPanel.add(needs, BorderLayout.SOUTH);
      controls = new ControlsPanel();
      rightPanel.add(controls, BorderLayout.NORTH);
      reward = new RewardPanel();
      rightPanel.add(reward, BorderLayout.SOUTH);
      pack();
      setVisible(true);
   }


   // Get quit state.
   boolean getQuit()
   {
      return(quit);
   }


   // Set sensors display.
   void setSensors(String rangeSensorString,
                   String colorSensorString,
                   String treatSensorString)
   {
      sensorsResponse.rangeText.setText(rangeSensorString);
      sensorsResponse.colorText.setText(colorSensorString);
      sensorsResponse.treatText.setText(treatSensorString);
   }


   // Set response display.
   void setResponse(String responseString)
   {
      sensorsResponse.responseText.setText(responseString);
   }


   // Sensors/Response panel.
   class SensorsResponsePanel extends JPanel
   {
      // Components.
      JTextField rangeText;
      JTextField colorText;
      JTextField treatText;
      JTextField responseText;

      // Constructor.
      public SensorsResponsePanel()
      {
         setLayout(new BorderLayout());
         setBorder(BorderFactory.createTitledBorder(
                      BorderFactory.createLineBorder(Color.black),
                      "Sensors/Response"));
         JPanel sensorsPanel = new JPanel();
         sensorsPanel.setLayout(new BorderLayout());
         add(sensorsPanel, BorderLayout.NORTH);
         JPanel rangePanel = new JPanel();
         rangePanel.setLayout(new FlowLayout(FlowLayout.LEFT));
         sensorsPanel.add(rangePanel, BorderLayout.NORTH);
         rangePanel.add(new JLabel("Range:"));
         rangeText = new JTextField(10);
         rangeText.setEditable(false);
         rangePanel.add(rangeText);
         JPanel colorPanel = new JPanel();
         colorPanel.setLayout(new FlowLayout(FlowLayout.LEFT));
         sensorsPanel.add(colorPanel, BorderLayout.CENTER);
         colorPanel.add(new JLabel("Color:"));
         colorText = new JTextField(10);
         colorText.setEditable(false);
         colorPanel.add(colorText);
         JPanel treatPanel = new JPanel();
         treatPanel.setLayout(new FlowLayout(FlowLayout.LEFT));
         sensorsPanel.add(treatPanel, BorderLayout.SOUTH);
         treatPanel.add(new JLabel("Treat:"));
         treatText = new JTextField(10);
         treatText.setEditable(false);
         treatPanel.add(treatText);
         JPanel responsePanel = new JPanel();
         responsePanel.setLayout(new FlowLayout(FlowLayout.LEFT));
         add(responsePanel, BorderLayout.SOUTH);
         responsePanel.add(new JLabel("Response:"));
         responseText = new JTextField(10);
         responseText.setEditable(false);
         responsePanel.add(responseText);
      }
   }

   // Set needs.
   void setNeeds(double successNeed, double rewardNeed)
   {
      needs.successSlider.setValue((int)(successNeed * 100.0));
      needs.rewardSlider.setValue((int)(rewardNeed * 100.0));
   }


   // Get success need.
   double getSuccessNeed()
   {
      return((double)needs.successSlider.getValue() * 100.0);
   }


   // Get reward need.
   double getRewardNeed()
   {
      return((double)needs.rewardSlider.getValue() * 100.0);
   }


   // Needs panel.
   class NeedsPanel extends JPanel implements ChangeListener
   {
      // Components.
      JSlider successSlider;
      JLabel  successLabel;
      JSlider rewardSlider;
      JLabel  rewardLabel;

      // Constructor.
      public NeedsPanel()
      {
         setLayout(new BorderLayout());
         setBorder(BorderFactory.createTitledBorder(
                      BorderFactory.createLineBorder(Color.black), "Needs"));
         JPanel successPanel = new JPanel();
         successPanel.setLayout(new FlowLayout());
         add(successPanel, BorderLayout.NORTH);
         successPanel.add(new JLabel("Success:", Label.RIGHT));
         successSlider = new JSlider(JSlider.HORIZONTAL, 0, 100, 100);
         successSlider.setMajorTickSpacing(20);
         successSlider.setMinorTickSpacing(5);
         successSlider.setPaintTicks(true);
         successSlider.addChangeListener(this);
         successPanel.add(successSlider);
         successLabel = new JLabel("1", Label.LEFT);
         successPanel.add(successLabel);
         JPanel rewardPanel = new JPanel();
         rewardPanel.setLayout(new FlowLayout());
         add(rewardPanel, BorderLayout.SOUTH);
         rewardPanel.add(new JLabel("Reward:", Label.RIGHT));
         rewardSlider = new JSlider(JSlider.HORIZONTAL, 0, 100, 100);
         rewardSlider.setMajorTickSpacing(20);
         rewardSlider.setMinorTickSpacing(5);
         rewardSlider.setPaintTicks(true);
         rewardSlider.addChangeListener(this);
         rewardPanel.add(rewardSlider);
         rewardLabel = new JLabel("1", Label.LEFT);
         rewardPanel.add(rewardLabel);
      }


      // Slider listener.
      public void stateChanged(ChangeEvent evt)
      {
         if ((JSlider)evt.getSource() == successSlider)
         {
            double need = (double)successSlider.getValue() / 100.0;
            atani.setSuccessNeed(need);
            successLabel.setText(need + "");
            return;
         }
         if ((JSlider)evt.getSource() == rewardSlider)
         {
            double need = (double)rewardSlider.getValue() / 100.0;
            atani.setRewardNeed(need);
            rewardLabel.setText(need + "");
            return;
         }
      }
   }

   // Get stepping state.
   boolean getStepping()
   {
      return(controls.stepState);
   }


   // Set stepping state.
   void setStepping(boolean step)
   {
      controls.setStepping(step);
   }


   // Get pause state.
   boolean getPause()
   {
      return(controls.pauseCheckBox.isSelected());
   }


   // Set pause state.
   void setPause(boolean pause)
   {
      controls.pauseCheckBox.setSelected(pause);
   }


   // Get driver type.
   int getDriver()
   {
      return(controls.driverChoice.getSelectedIndex());
   }


   // Set driver type.
   void setDriver(int driver)
   {
      controls.driverChoice.select(driver);
   }


   // Get manual response.
   int getManualResponse()
   {
      return(controls.manualResponse);
   }


   // Set manual response.
   void setManualResponse(int response)
   {
      controls.manualResponse = response;
   }


   // Controls panel.
   class ControlsPanel extends JPanel implements ActionListener
   {
      // Components.
      JCheckBox pauseCheckBox;
      JButton   stepButton;
      boolean   stepState = false;
      Choice    driverChoice;
      JButton   moveLeftButton;
      JButton   moveForwardButton;
      JButton   moveRightButton;
      int       manualResponse = Atani.RESPONSE_TYPE.NULL_RESPONSE.getValue();

      // Constructor.
      public ControlsPanel()
      {
         setLayout(new BorderLayout());
         setBorder(BorderFactory.createTitledBorder(
                      BorderFactory.createLineBorder(Color.black), "Controls"));
         JPanel controlsPanel = new JPanel();
         controlsPanel.setLayout(new FlowLayout(FlowLayout.LEFT));
         add(controlsPanel, BorderLayout.NORTH);
         pauseCheckBox = new JCheckBox("Pause");
         pauseCheckBox.setSelected(true);
         controlsPanel.add(pauseCheckBox);
         stepButton = new JButton("Step");
         stepButton.addActionListener(this);
         controlsPanel.add(stepButton);
         JPanel manualPanel = new JPanel();
         manualPanel.setLayout(new BorderLayout());
         add(manualPanel, BorderLayout.SOUTH);
         JPanel driverPanel = new JPanel();
         driverPanel.setLayout(new FlowLayout(FlowLayout.LEFT));
         manualPanel.add(driverPanel, BorderLayout.NORTH);
         driverPanel.add(new JLabel("Driver:"));
         driverChoice = new Choice();
         driverPanel.add(driverChoice);
         driverChoice.add("atani");
         driverChoice.add("override");
         driverChoice.add("hijack");
         JPanel movePanel = new JPanel();
         movePanel.setLayout(new FlowLayout());
         manualPanel.add(movePanel, BorderLayout.CENTER);
         moveLeftButton = new JButton("Left");
         moveLeftButton.addActionListener(this);
         movePanel.add(moveLeftButton);
         moveForwardButton = new JButton("Forward");
         moveForwardButton.addActionListener(this);
         movePanel.add(moveForwardButton);
         moveRightButton = new JButton("Right");
         moveRightButton.addActionListener(this);
         movePanel.add(moveRightButton);
      }


      // Set stepping state.
      void setStepping(boolean step)
      {
         if (step)
         {
            stepButton.setText("Stepping");
         }
         else
         {
            stepButton.setText("Step");
         }
         stepState = step;
      }


      // Button listener.
      public void actionPerformed(ActionEvent evt)
      {
         if ((JButton)evt.getSource() == stepButton)
         {
            if (driverChoice.getSelectedIndex() == DRIVER_TYPE.ATANI.getValue())
            {
               setStepping(true);
               pauseCheckBox.setSelected(false);
            }
            return;
         }

         if ((JButton)evt.getSource() == moveLeftButton)
         {
            if (driverChoice.getSelectedIndex() != DRIVER_TYPE.ATANI.getValue())
            {
               manualResponse = Atani.RESPONSE_TYPE.LEFT.getValue();
               pauseCheckBox.setSelected(false);
            }
            return;
         }

         if ((JButton)evt.getSource() == moveForwardButton)
         {
            if (driverChoice.getSelectedIndex() != DRIVER_TYPE.ATANI.getValue())
            {
               manualResponse = Atani.RESPONSE_TYPE.FORWARD.getValue();
               pauseCheckBox.setSelected(false);
            }
            return;
         }

         if ((JButton)evt.getSource() == moveRightButton)
         {
            if (driverChoice.getSelectedIndex() != DRIVER_TYPE.ATANI.getValue())
            {
               manualResponse = Atani.RESPONSE_TYPE.RIGHT.getValue();
               pauseCheckBox.setSelected(false);
            }
            return;
         }
      }
   }

   // Get treat state.
   boolean getTreatState()
   {
      return(reward.treatCheckBox.isSelected());
   }


   // Set treat state.
   void setTreatState(boolean treat)
   {
      reward.treatCheckBox.setSelected(treat);
   }


   // Get reward need.
   double getReward()
   {
      return((double)reward.rewardSlider.getValue() * 100.0);
   }


   // Reward panel.
   class RewardPanel extends JPanel implements ActionListener, ChangeListener
   {
      // Components.
      Choice    actionChoice;
      Choice    sensorModeChoice;
      JButton   goButton;
      JCheckBox treatCheckBox;
      JSlider   rewardSlider;
      JLabel    rewardLabel;

      // Constructor.
      public RewardPanel()
      {
         setLayout(new BorderLayout());
         setBorder(BorderFactory.createTitledBorder(
                      BorderFactory.createLineBorder(Color.black), "Reward"));
         JPanel topPanel = new JPanel();
         topPanel.setLayout(new FlowLayout(FlowLayout.LEFT));
         add(topPanel, BorderLayout.NORTH);
         topPanel.add(new JLabel("Action:", Label.RIGHT));
         actionChoice = new Choice();
         topPanel.add(actionChoice);
         actionChoice.add("create");
         actionChoice.add("disable");
         actionChoice.add("enable all");
         actionChoice.add("remove");
         actionChoice.add("remove all");
         topPanel.add(new JLabel("Sensor mode:", Label.RIGHT));
         sensorModeChoice = new Choice();
         topPanel.add(sensorModeChoice);
         sensorModeChoice.add("base");
         sensorModeChoice.add("range");
         sensorModeChoice.add("color");
         sensorModeChoice.add("treat");
         goButton = new JButton("Go");
         goButton.addActionListener(this);
         topPanel.add(goButton);
         treatCheckBox = new JCheckBox("Treat");
         treatCheckBox.setSelected(false);
         topPanel.add(treatCheckBox);
         JPanel bottomPanel = new JPanel();
         bottomPanel.setLayout(new FlowLayout(FlowLayout.LEFT));
         add(bottomPanel, BorderLayout.SOUTH);
         bottomPanel.add(new JLabel("Reward:", Label.RIGHT));
         rewardSlider = new JSlider(JSlider.HORIZONTAL, 0, 100, 100);
         rewardSlider.setMajorTickSpacing(20);
         rewardSlider.setMinorTickSpacing(5);
         rewardSlider.setPaintTicks(true);
         rewardSlider.addChangeListener(this);
         bottomPanel.add(rewardSlider);
         rewardLabel = new JLabel("1", Label.LEFT);
         bottomPanel.add(rewardLabel);
      }


      // Button listener.
      public void actionPerformed(ActionEvent evt)
      {
         if ((JButton)evt.getSource() == goButton)
         {
            atani.rewardAction(actionChoice.getSelectedIndex(),
                               sensorModeChoice.getSelectedIndex(),
                               (double)rewardSlider.getValue() / 100.0);
            return;
         }
      }


      // Slider listener.
      public void stateChanged(ChangeEvent evt)
      {
         if ((JSlider)evt.getSource() == rewardSlider)
         {
            rewardLabel.setText(((double)rewardSlider.getValue() / 100.0) + "");
            return;
         }
      }
   }
}
