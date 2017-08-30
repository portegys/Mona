/*
 *
 * Game of Life application.
 *
 * Usage:
 *
 * java GameOfLifeApplication [-gridSize <width> <height>]
 *      [-screenSize <width> <height>]
 *
 */

import java.awt.*;
import java.awt.event.*;
import java.io.*;
import javax.swing.*;
import javax.swing.event.*;


// Game of Life application.
public class GameOfLifeApplication implements Runnable
{
   // Update frequency (ms).
   static final int       MIN_DELAY           = 5;
   static final int       MAX_DELAY           = 500;
   static final int       MAX_SLEEP           = 100;

   // Game of Life automaton.
   GameOfLife             gameOfLife;
   static final Dimension DEFAULT_GRID_SIZE = new Dimension(50, 50);
   Dimension              gridSize          = DEFAULT_GRID_SIZE;
   int    delay = MAX_DELAY;
   Thread updateThread;


   // Display.
   static final Dimension DEFAULT_SCREEN_SIZE = new Dimension(600, 700);   
   JFrame           screen;
   Dimension        screenSize = DEFAULT_SCREEN_SIZE;
   Dimension        displaySize;
   GameOfLifeCanvas gameOfLifeCanvas;

   // Controls.
   Controls controls;

   // Constructor.
   public GameOfLifeApplication(Dimension gridSize, Dimension screenSize)
   {
      this.gridSize   = new Dimension(gridSize);
      this.screenSize = new Dimension(screenSize);

      // Set up screen.
      screen = new JFrame("Game of Life");
      screen.addWindowListener(new WindowAdapter()
                               {
                                  public void windowClosing(WindowEvent e)
                                  {
                                     System.exit(0);
                                  }
                               }
                               );
      screen.setSize(screenSize);
      screen.getContentPane().setLayout(new BorderLayout());

      // Create automaton.
      gameOfLife = new GameOfLife(gridSize);

      // Create display.
      displaySize = new Dimension((int)((double)screenSize.width * .99),
                                  (int)((double)screenSize.height * .75));
      gameOfLifeCanvas = new GameOfLifeCanvas(gameOfLife, displaySize);
      screen.getContentPane().add(gameOfLifeCanvas, BorderLayout.NORTH);

      // Create controls.
      controls = new Controls();
      screen.getContentPane().add(controls);

      // Make screen visible.
      screen.setVisible(true);

      // Start update thread.
      updateThread = new Thread(this);
      updateThread.start();
   }


   // Run.
   public void run()
   {
      int timer = 0;

      // Lower thread's priority.
      if (Thread.currentThread() == updateThread)
      {
         Thread.currentThread().setPriority(Thread.MIN_PRIORITY);
      }

      // Update loop.
      while ((Thread.currentThread() == updateThread) &&
             !updateThread.isInterrupted())
      {
         if ((delay < MAX_DELAY) && (timer >= delay))
         {
            gameOfLife.step();
            timer = 0;
         }

         gameOfLifeCanvas.display();

         try
         {
            if (delay < MAX_SLEEP)
            {
               Thread.sleep(delay);

               if (timer < MAX_DELAY)
               {
                  timer += delay;
               }
            }
            else
            {
               Thread.sleep(MAX_SLEEP);

               if (timer < MAX_DELAY)
               {
                  timer += MAX_SLEEP;
               }
            }
         }
         catch (InterruptedException e) {
            break;
         }
      }
   }


   // Main.
   public static void main(String[] args)
   {
      Dimension gridSize   = new Dimension(DEFAULT_GRID_SIZE);
      Dimension screenSize = new Dimension(DEFAULT_SCREEN_SIZE);

      // Get sizes.
      for (int i = 0; i < args.length; )
      {
         if (args[i].equals("-gridSize"))
         {
            i++;
            gridSize.width = gridSize.height = -1;

            if (i < args.length)
            {
               gridSize.width = Integer.parseInt(args[i]);
               i++;
            }

            if (i < args.length)
            {
               gridSize.height = Integer.parseInt(args[i]);
               i++;
            }

            if ((gridSize.width <= 0) || (gridSize.height <= 0))
            {
               System.err.println("Invalid grid size");
               System.exit(1);
            }
         }
         else if (args[i].equals("-screenSize"))
         {
            i++;
            screenSize.width = screenSize.height = -1;

            if (i < args.length)
            {
               screenSize.width = Integer.parseInt(args[i]);
               i++;
            }

            if (i < args.length)
            {
               screenSize.height = Integer.parseInt(args[i]);
               i++;
            }

            if ((screenSize.width <= 0) || (screenSize.height <= 0))
            {
               System.err.println("Invalid screen size");
               System.exit(1);
            }
         }
         else
         {
            System.err.println(
               "java GameOfLifeApplication [-gridSize <width> <height>] [-screenSize <width> <height>]");
            System.exit(1);
         }
      }

      // Create the application.
      new GameOfLifeApplication(gridSize, screenSize);
   }


   // Control panel.
   class Controls extends JPanel implements ActionListener, ChangeListener
   {
      // Components.
      JSlider    speedSlider;
      JButton    stepButton;
      JButton    clearButton;
      JButton    checkpointButton;
      JButton    restoreButton;
      JButton    loadButton;
      JButton    saveButton;
      JTextField inputText;
      JTextField outputText;

      // Constructor.
      Controls()
      {
         setLayout(new GridLayout(4, 1));
         setBorder(BorderFactory.createRaisedBevelBorder());

         JPanel panel = new JPanel();
         panel.add(new JLabel("Speed:   Fast", Label.RIGHT));
         speedSlider = new JSlider(JSlider.HORIZONTAL, MIN_DELAY, MAX_DELAY,
                                   MAX_DELAY);
         speedSlider.addChangeListener(this);
         panel.add(speedSlider);
         panel.add(new JLabel("Stop", Label.LEFT));
         stepButton = new JButton("Step");
         stepButton.addActionListener(this);
         panel.add(stepButton);
         add(panel);
         panel       = new JPanel();
         clearButton = new JButton("Clear");
         clearButton.addActionListener(this);
         panel.add(clearButton);
         checkpointButton = new JButton("Checkpoint");
         checkpointButton.addActionListener(this);
         panel.add(checkpointButton);
         restoreButton = new JButton("Restore");
         restoreButton.addActionListener(this);
         panel.add(restoreButton);
         add(panel);
         panel = new JPanel();
         panel.add(new JLabel("File: "));
         inputText = new JTextField("", 25);
         panel.add(inputText);
         loadButton = new JButton("Load");
         loadButton.addActionListener(this);
         panel.add(loadButton);
         saveButton = new JButton("Save");
         saveButton.addActionListener(this);
         panel.add(saveButton);
         add(panel);
         panel = new JPanel();
         panel.add(new JLabel("Status: "));
         outputText = new JTextField("", 40);
         outputText.setEditable(false);
         panel.add(outputText);
         add(panel);
      }


      // Speed slider listener.
      public void stateChanged(ChangeEvent evt)
      {
         outputText.setText("");
         delay = speedSlider.getValue();
      }


      // Input text listener.
      public void actionPerformed(ActionEvent evt)
      {
         outputText.setText("");

         // Step?
         if (evt.getSource() == (Object)stepButton)
         {
            speedSlider.setValue(MAX_DELAY);
            gameOfLife.step();
            gameOfLifeCanvas.display();

            return;
         }

         // Clear?
         if (evt.getSource() == (Object)clearButton)
         {
            gameOfLife.clear();

            return;
         }

         // Checkpoint?
         if (evt.getSource() == (Object)checkpointButton)
         {
            gameOfLife.checkpoint();

            return;
         }

         // Restore?
         if (evt.getSource() == (Object)restoreButton)
         {
            gameOfLife.restore();

            return;
         }

         // Load/save.
         String fileName = inputText.getText().trim();

         if (fileName.equals(""))
         {
            return;
         }

         // Load?
         if (evt.getSource() == (Object)loadButton)
         {
            try {
               gameOfLife.load(fileName);
               outputText.setText(fileName + " loaded");
            }
            catch (IOException e) {
               outputText.setText(e.getMessage());
            }
         }

         // Save?
         if (evt.getSource() == (Object)saveButton)
         {
            try {
               gameOfLife.save(fileName);
               outputText.setText(fileName + " saved");
            }
            catch (IOException e) {
               outputText.setText(e.getMessage());
            }
         }
      }
   }
}
