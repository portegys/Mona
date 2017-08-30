//  Mox dashboard.

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;

// Mox dashboard.
public class MoxDashboard extends JFrame
{
   // Components.
   SensorsResponsePanel sensorsResponse;
   NeedsPanel           needs;
   DriverPanel          driver;

   // Callback targets.
   Mox               mox;
   MoxWorldDashboard worldDashboard;

   // Constructor.
   public MoxDashboard(Mox mox, MoxWorldDashboard worldDashboard)
   {
      this.mox            = mox;
      this.worldDashboard = worldDashboard;

      setTitle("Mox " + mox.id);
      addWindowListener(new WindowAdapter()
                        {
                           public void windowClosing(WindowEvent e) { closeDashboard(); }
                        }
                        );
      JPanel basePanel = (JPanel)getContentPane();
      basePanel.setLayout(new BorderLayout());
      sensorsResponse = new SensorsResponsePanel();
      basePanel.add(sensorsResponse, BorderLayout.NORTH);
      needs = new NeedsPanel();
      basePanel.add(needs, BorderLayout.CENTER);
      driver = new DriverPanel();
      basePanel.add(driver, BorderLayout.SOUTH);
      pack();
      setVisible(false);
      update();
   }


   // Update dashboard.
   void update()
   {
      setSensors(mox.sensors[0] + "", mox.sensors[1] + "/" + mox.sensors[2]);
      if (mox.response == Mox.RESPONSE_TYPE.WAIT.getValue())
      {
         setResponse("wait");
      }
      else if (mox.response == Mox.RESPONSE_TYPE.FORWARD.getValue())
      {
         setResponse("move forward");
      }
      else if (mox.response == Mox.RESPONSE_TYPE.RIGHT.getValue())
      {
         setResponse("turn right");
      }
      else if (mox.response == Mox.RESPONSE_TYPE.LEFT.getValue())
      {
         setResponse("turn left");
      }
      else
      {
         setResponse("");
      }
      for (int i = 0; i < mox.needValues.length; i++)
      {
         setNeed(i, mox.getNeed(i));
      }
      setDriverChoice(mox.driverType);
   }


   // Close the dashboard.
   void closeDashboard()
   {
      setVisible(false);
      worldDashboard.closeMoxDashboard();
   }


   // Set sensors display.
   void setSensors(String rangeSensorString,
                   String colorSensorString)
   {
      sensorsResponse.rangeText.setText(rangeSensorString);
      sensorsResponse.colorText.setText(colorSensorString);
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
         JPanel responsePanel = new JPanel();
         responsePanel.setLayout(new FlowLayout(FlowLayout.LEFT));
         add(responsePanel, BorderLayout.SOUTH);
         responsePanel.add(new JLabel("Response:"));
         responseText = new JTextField(10);
         responseText.setEditable(false);
         responsePanel.add(responseText);
      }
   }

   // Get need value.
   double getNeed(int needNum)
   {
      return(mox.getNeed(needNum));
   }


   // Set need value.
   void setNeed(int needNum, double needValue)
   {
      mox.setNeed(needNum, needValue);
      int value = (int)((needValue / mox.needValues[needNum]) * 100.0);
      needs.needSliders[needNum].setValue(value);
      needs.needValueLabels[needNum].setText(needValue + "");
   }


   // Needs panel.
   class NeedsPanel extends JPanel implements ChangeListener
   {
      // Components.
      JSlider[] needSliders;
      JLabel[]  needValueLabels;

      // Constructor.
      public NeedsPanel()
      {
         setLayout(new BorderLayout());
         setBorder(BorderFactory.createTitledBorder(
                      BorderFactory.createLineBorder(Color.black), "Needs"));
         if (mox.needValues == null)
         {
            return;
         }
         needSliders     = new JSlider[mox.needValues.length];
         needValueLabels = new JLabel[mox.needValues.length];
         JPanel needPanel;
         for (int i = 0; i < mox.needValues.length; i++)
         {
            needPanel = new JPanel();
            needPanel.setLayout(new FlowLayout());
            switch (i)
            {
            case 0:
               add(needPanel, BorderLayout.NORTH);
               break;

            case 1:
               add(needPanel, BorderLayout.CENTER);
               break;

            default:
               add(needPanel, BorderLayout.SOUTH);
               break;
            }
            needPanel.add(new JLabel(mox.needNames[i], Label.RIGHT));
            needSliders[i] = new JSlider(JSlider.HORIZONTAL, 0, 100, 100);
            needSliders[i].setMajorTickSpacing(20);
            needSliders[i].setMinorTickSpacing(5);
            needSliders[i].setPaintTicks(true);
            needSliders[i].addChangeListener(this);
            needPanel.add(needSliders[i]);
            needValueLabels[i] = new JLabel(getNeed(i) + "", Label.LEFT);
            needPanel.add(needValueLabels[i]);
            needSliders[i].setValue((int)((getNeed(i) / mox.needValues[i]) * 100.0));
         }
      }


      // Slider listener.
      public void stateChanged(ChangeEvent evt)
      {
         for (int i = 0; i < needSliders.length; i++)
         {
            if ((JSlider)evt.getSource() == needSliders[i])
            {
               double needValue = mox.needValues[i] *
                                  ((double)needSliders[i].getValue() / 100.0);
               mox.setNeed(i, needValue);
               needValueLabels[i].setText(needValue + "");
               break;
            }
         }
      }
   }


   // Get driver choice.
   int getDriverChoice()
   {
      return(driver.driverChoice.getSelectedIndex());
   }


   // Set driver choice.
   void setDriverChoice(int driverChoice)
   {
      driver.driverChoice.select(driverChoice);
   }


   // Driver panel.
   class DriverPanel extends JPanel implements ItemListener, ActionListener
   {
      // Components.
      Choice  driverChoice;
      JButton turnLeftButton;
      JButton moveForwardButton;
      JButton turnRightButton;

      // Constructor.
      public DriverPanel()
      {
         setLayout(new BorderLayout());
         setBorder(BorderFactory.createTitledBorder(
                      BorderFactory.createLineBorder(Color.black), "Driver"));
         JPanel driverPanel = new JPanel();
         driverPanel.setLayout(new FlowLayout(FlowLayout.LEFT));
         add(driverPanel, BorderLayout.NORTH);
         driverPanel.add(new JLabel("Driver:"));
         driverChoice = new Choice();
         driverPanel.add(driverChoice);
         driverChoice.add("mox");
         driverChoice.add("override");
         driverChoice.add("hijack");
         driverChoice.addItemListener(this);
         JPanel movePanel = new JPanel();
         movePanel.setLayout(new FlowLayout());
         add(movePanel, BorderLayout.SOUTH);
         turnLeftButton = new JButton("Left");
         turnLeftButton.addActionListener(this);
         movePanel.add(turnLeftButton);
         moveForwardButton = new JButton("Forward");
         moveForwardButton.addActionListener(this);
         movePanel.add(moveForwardButton);
         turnRightButton = new JButton("Right");
         turnRightButton.addActionListener(this);
         movePanel.add(turnRightButton);
      }


      // Choice listener.
      public void itemStateChanged(ItemEvent e)
      {
         mox.driverType = driverChoice.getSelectedIndex();
      }


      // Button listener.
      public void actionPerformed(ActionEvent evt)
      {
         if ((JButton)evt.getSource() == turnLeftButton)
         {
            mox.driverResponse = Mox.RESPONSE_TYPE.LEFT.getValue();
            return;
         }

         if ((JButton)evt.getSource() == moveForwardButton)
         {
            mox.driverResponse = Mox.RESPONSE_TYPE.FORWARD.getValue();
            return;
         }

         if ((JButton)evt.getSource() == turnRightButton)
         {
            mox.driverResponse = Mox.RESPONSE_TYPE.RIGHT.getValue();
            return;
         }
      }
   }
}
