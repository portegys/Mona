/**
 * Mox world dashboard.
 */

import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;


// Mox dashboard.
public class MoxWorldDashboard extends JFrame
{
   // Moxen.
   ArrayList<Mox> moxen;

   // Moxen dashboards.
   ArrayList<MoxDashboard> moxenDashboards;
   int currentMox;

   // Game of Life.
   GameOfLife gameOfLife;

   // Title.
   static final String TITLE = "Mox world";

   // Dimensions.
   static final Dimension DASHBOARD_SIZE = new Dimension(600, 700);

   // Mox display.
   MoxDisplay display;

   // Controls.
   MoxControls controls;

   // Step frequency (ms).
   static final int MIN_STEP_DELAY = 0;
   static final int MAX_STEP_DELAY = 1000;
   int              stepDelay      = MAX_STEP_DELAY;

   // Quit.
   boolean quit;

   // Constructors.
   public MoxWorldDashboard(GameOfLife gameOfLife, ArrayList<Mox> moxen)
   {
      this.gameOfLife = gameOfLife;
      setMoxen(moxen);
      currentMox = -1;
      init();
   }


   public MoxWorldDashboard(GameOfLife gameOfLife)
   {
      this.gameOfLife = gameOfLife;
      moxen           = new ArrayList<Mox>();
      setMoxen(moxen);
      currentMox = -1;
      init();
   }


   // Initialize.
   void init()
   {
      // Set up dashboard.
      setTitle(TITLE);
      addWindowListener(new WindowAdapter()
                        {
                           public void windowClosing(WindowEvent e)
                           {
                              reset();
                              quit = true;
                           }
                        }
                        );
      setBounds(0, 0, DASHBOARD_SIZE.width, DASHBOARD_SIZE.height);
      JPanel basePanel = (JPanel)getContentPane();
      basePanel.setLayout(new BorderLayout());

      // Create display.
      Dimension displaySize = new Dimension((int)((double)DASHBOARD_SIZE.width * .99),
                                            (int)((double)DASHBOARD_SIZE.height * .85));
      display = new MoxDisplay(displaySize);
      basePanel.add(display, BorderLayout.NORTH);

      // Create controls.
      controls = new MoxControls();
      basePanel.add(controls, BorderLayout.SOUTH);

      // Make dashboard visible.
      setVisible(true);
   }


   // Set moxen.
   void setMoxen(ArrayList<Mox> moxen)
   {
      // Reset previous dashboards.
      reset();

      // Create moxen dashboards.
      this.moxen      = moxen;
      moxenDashboards = new ArrayList<MoxDashboard>();
      MoxDashboard moxDashboard;
      for (int i = 0; i < moxen.size(); i++)
      {
         moxDashboard = new MoxDashboard(moxen.get(i), this);
         moxenDashboards.add(i, moxDashboard);
      }
   }


   // Reset.
   void reset()
   {
      currentMox = -1;
      if (moxenDashboards != null)
      {
         for (int i = 0; i < moxenDashboards.size(); i++)
         {
            moxenDashboards.get(i).setVisible(false);
         }
      }
   }


   // Update dashboard.
   public void update(int step, int steps)
   {
      controls.updateStepCounter(step, steps);
      update();
   }


   private int timer = 0;
   public void update()
   {
      if (quit) { return; }

      // Update moxen dashboards.
      for (int i = 0; i < moxenDashboards.size(); i++)
      {
         moxenDashboards.get(i).update();
      }

      // Update display.
      display.update();

      // Timer loop: count down delay by 1ms.
      for (timer = stepDelay; timer > 0 && !quit; )
      {
         try
         {
            Thread.sleep(1);
         }
         catch (InterruptedException e) {
            break;
         }

         display.update();

         if (stepDelay < MAX_STEP_DELAY)
         {
            timer--;
         }
      }
   }


   // Set step delay.
   void setStepDelay(int delay)
   {
      stepDelay = timer = delay;
   }


   // Step.
   void step()
   {
      timer = 0;
   }


   // Close current mox dashboard.
   public void closeMoxDashboard()
   {
      if (currentMox != -1)
      {
         closeMoxDashboard(currentMox);
      }
   }


   // Close given mox dashboard.
   public void closeMoxDashboard(int moxIndex)
   {
      if (moxIndex == currentMox)
      {
         moxenDashboards.get(currentMox).setVisible(false);
         currentMox = -1;
      }
   }


   // Set message
   void setMessage(String message)
   {
      if (message == null)
      {
         controls.messageText.setText("");
      }
      else
      {
         controls.messageText.setText(message);
      }
   }


   // Mox display.
   public class MoxDisplay extends Canvas
   {
      // Buffered display.
      private Dimension canvasSize;
      private Graphics  graphics;
      private Image     image;
      private Graphics  imageGraphics;

      // Last cell visited by mouse.
      private int lastX = -1;

      // Last cell visited by mouse.
      private int lastY = -1;

      // Constructor.
      public MoxDisplay(Dimension canvasSize)
      {
         // Configure canvas.
         this.canvasSize = canvasSize;
         setBounds(0, 0, canvasSize.width, canvasSize.height);
         addMouseListener(new CanvasMouseListener());
         addMouseMotionListener(new CanvasMouseMotionListener());
      }


      // Update display.
      void update()
      {
         int   x;
         int   y;
         int   x2;
         int   y2;
         float cellWidth;
         float cellHeight;

         if (quit)
         {
            return;
         }

         if (graphics == null)
         {
            graphics      = getGraphics();
            image         = createImage(canvasSize.width, canvasSize.height);
            imageGraphics = image.getGraphics();
         }

         if (graphics == null)
         {
            return;
         }

         // Clear display.
         imageGraphics.setColor(Color.white);
         imageGraphics.fillRect(0, 0, canvasSize.width, canvasSize.height);

         // Draw grid.
         synchronized (gameOfLife.lock)
         {
            cellWidth  = (float)canvasSize.width / (float)gameOfLife.size.width;
            cellHeight = (float)canvasSize.height / (float)gameOfLife.size.height;
            imageGraphics.setColor(Color.black);
            y2 = canvasSize.height;

            for (x = 1, x2 = (int)cellWidth; x < gameOfLife.size.width;
                 x++, x2 = (int)(cellWidth * (double)x))
            {
               imageGraphics.drawLine(x2, 0, x2, y2);
            }

            x2 = canvasSize.width;

            for (y = 1, y2 = (int)cellHeight; y < gameOfLife.size.height;
                 y++, y2 = (int)(cellHeight * (double)y))
            {
               imageGraphics.drawLine(0, y2, x2, y2);
            }

            // Draw cells.
            for (x = x2 = 0; x < gameOfLife.size.width;
                 x++, x2 = (int)(cellWidth * (double)x))
            {
               for (y = 0, y2 = canvasSize.height - (int)cellHeight;
                    y < gameOfLife.size.height;
                    y++, y2 = (int)(cellHeight *
                                    (double)(gameOfLife.size.height - (y + 1))))
               {
                  if (gameOfLife.cells[x][y] > 0)
                  {
                     switch (gameOfLife.cells[x][y])
                     {
                     case GameOfLife.GREEN_CELL_COLOR_VALUE:
                        imageGraphics.setColor(Color.green);
                        break;

                     case GameOfLife.BLUE_CELL_COLOR_VALUE:
                        imageGraphics.setColor(Color.blue);
                        break;

                     default:
                        imageGraphics.setColor(Color.black);
                        break;
                     }
                     imageGraphics.fillRect(x2, y2, (int)cellWidth + 1,
                                            (int)cellHeight + 1);
                  }
               }
            }
         }

         imageGraphics.setColor(Color.black);

         // Draw moxen.
         Mox mox;
         int[] vx = new int[3];
         int[] vy = new int[3];
         for (int i = 0; i < moxen.size(); i++)
         {
            mox = (Mox)moxen.get(i);
            if (!mox.isAlive)
            {
               continue;
            }
            x2 = (int)(cellWidth * (double)mox.x);
            y2 = (int)(cellHeight *
                       (double)(gameOfLife.size.height - (mox.y + 1)));

            // Highlight selected mox?
            if (i == currentMox)
            {
               imageGraphics.setColor(Color.lightGray);
            }
            else
            {
               imageGraphics.setColor(Color.white);
            }
            imageGraphics.fillRect(x2 + 1, y2 + 1, (int)cellWidth - 1,
                                   (int)cellHeight - 1);
            if (mox.species == Mox.SPECIES.FORAGER.getValue())
            {
               imageGraphics.setColor(ForagerMox.FORAGER_COLOR);
            }
            else if (mox.species == Mox.SPECIES.PREDATOR.getValue())
            {
               imageGraphics.setColor(PredatorMox.PREDATOR_COLOR);
            }
            else
            {
               imageGraphics.setColor(Color.black);
            }
            if (mox.direction == Mox.DIRECTION.NORTH.getValue())
            {
               vx[0] = x2 + (int)(cellWidth * 0.5f);
               vy[0] = y2;
               vx[1] = x2;
               vy[1] = y2 + (int)cellHeight;
               vx[2] = x2 + (int)cellWidth;
               vy[2] = y2 + (int)cellHeight;
            }
            else if (mox.direction == Mox.DIRECTION.EAST.getValue())
            {
               vx[0] = x2 + (int)(cellWidth);
               vy[0] = y2 + (int)(cellHeight * 0.5f);
               vx[1] = x2;
               vy[1] = y2;
               vx[2] = x2;
               vy[2] = y2 + (int)cellHeight;
            }
            else if (mox.direction == Mox.DIRECTION.SOUTH.getValue())
            {
               vx[0] = x2 + (int)(cellWidth * 0.5f);
               vy[0] = y2 + (int)cellHeight;
               vx[1] = x2;
               vy[1] = y2;
               vx[2] = x2 + (int)cellWidth;
               vy[2] = y2;
            }
            else
            {
               vx[0] = x2;
               vy[0] = y2 + (int)(cellHeight * 0.5f);
               vx[1] = x2 + (int)cellWidth;
               vy[1] = y2;
               vx[2] = x2 + (int)cellWidth;
               vy[2] = y2 + (int)cellHeight;
            }
            imageGraphics.fillPolygon(vx, vy, 3);
         }

         // Refresh display.
         graphics.drawImage(image, 0, 0, this);
      }


      // Canvas mouse listener.
      class CanvasMouseListener extends MouseAdapter
      {
         // Mouse pressed.
         public void mousePressed(MouseEvent evt)
         {
            int     i;
            int     x;
            int     y;
            double  cellWidth  = (double)canvasSize.width / (double)gameOfLife.size.width;
            double  cellHeight = (double)canvasSize.height / (double)gameOfLife.size.height;
            Mox     mox;
            boolean moxSelected;

            x = (int)((double)evt.getX() / cellWidth);
            y = gameOfLife.size.height - (int)((double)evt.getY() / cellHeight) - 1;

            if ((x >= 0) && (x < gameOfLife.size.width) &&
                (y >= 0) && (y < gameOfLife.size.height))
            {
               lastX = x;
               lastY = y;

               // Selecting mox?
               moxSelected = false;
               for (i = 0; i < moxen.size(); i++)
               {
                  mox = moxen.get(i);
                  if (mox.isAlive && (mox.x == x) && (mox.y == y))
                  {
                     moxSelected = true;
                     if (currentMox == -1)
                     {
                        currentMox = i;
                        moxenDashboards.get(currentMox).setVisible(true);
                     }
                     else
                     {
                        if (i == currentMox)
                        {
                           moxenDashboards.get(currentMox).setVisible(false);
                           currentMox = -1;
                        }
                        else
                        {
                           moxenDashboards.get(currentMox).setVisible(false);
                           currentMox = i;
                           moxenDashboards.get(currentMox).setVisible(true);
                        }
                     }
                     break;
                  }
               }
               if (!moxSelected && (currentMox != -1))
               {
                  moxenDashboards.get(currentMox).setVisible(false);
                  currentMox = -1;
               }

               if (!moxSelected)
               {
                  synchronized (gameOfLife.lock)
                  {
                     if (gameOfLife.cells[x][y] > 0)
                     {
                        gameOfLife.cells[x][y] = 0;
                     }
                     else
                     {
                        gameOfLife.cells[x][y] = 1;
                     }
                  }
               }

               // Refresh display.
               update();
            }
         }
      }

      // Canvas mouse motion listener.
      class CanvasMouseMotionListener extends MouseMotionAdapter
      {
         // Mouse dragged.
         public void mouseDragged(MouseEvent evt)
         {
            int    x;
            int    y;
            double cellWidth  = (double)canvasSize.width / (double)gameOfLife.size.width;
            double cellHeight = (double)canvasSize.height / (double)gameOfLife.size.height;

            x = (int)((double)evt.getX() / cellWidth);
            y = gameOfLife.size.height - (int)((double)evt.getY() / cellHeight) - 1;

            if ((x >= 0) && (x < gameOfLife.size.width) &&
                (y >= 0) && (y < gameOfLife.size.height))
            {
               if ((x != lastX) || (y != lastY))
               {
                  lastX = x;
                  lastY = y;

                  synchronized (gameOfLife.lock)
                  {
                     if (gameOfLife.cells[x][y] > 0)
                     {
                        gameOfLife.cells[x][y] = 0;
                     }
                     else
                     {
                        gameOfLife.cells[x][y] = 1;
                     }
                  }

                  // Refresh display.
                  update();
               }
            }
         }
      }
   }

   // Control panel.
   class MoxControls extends JPanel implements ActionListener, ChangeListener
   {
      // Components.
      JButton    resetButton;
      JLabel     stepCounter;
      JSlider    speedSlider;
      JButton    stepButton;
      JTextField messageText;

      // Constructor.
      MoxControls()
      {
         setLayout(new BorderLayout());
         setBorder(BorderFactory.createRaisedBevelBorder());

         JPanel panel = new JPanel();
         resetButton = new JButton("Reset");
         resetButton.addActionListener(this);
         panel.add(resetButton);
         panel.add(new JLabel("Speed:   Fast", Label.RIGHT));
         speedSlider = new JSlider(JSlider.HORIZONTAL, MIN_STEP_DELAY,
                                   MAX_STEP_DELAY, MAX_STEP_DELAY);
         speedSlider.addChangeListener(this);
         panel.add(speedSlider);
         panel.add(new JLabel("Stop", Label.LEFT));
         stepButton = new JButton("Step");
         stepButton.addActionListener(this);
         panel.add(stepButton);
         stepCounter = new JLabel("");
         panel.add(stepCounter);
         add(panel, BorderLayout.NORTH);
         panel       = new JPanel();
         messageText = new JTextField("", 50);
         messageText.setEditable(false);
         panel.add(messageText);
         add(panel, BorderLayout.SOUTH);
      }


      // Update step counter display
      void updateStepCounter(int step, int steps)
      {
         stepCounter.setText("Step: " + step + "/" + steps);
      }


      // Speed slider listener.
      public void stateChanged(ChangeEvent evt)
      {
         setStepDelay(speedSlider.getValue());
      }


      // Step button listener.
      public void actionPerformed(ActionEvent evt)
      {
         // Reset?
         if (evt.getSource() == (Object)resetButton)
         {
            gameOfLife.restore();
            int numMox = moxen.size();
            for (int i = 0; i < numMox; i++)
            {
               moxen.get(i).reset();
               moxenDashboards.get(i).update();
            }

            return;
         }

         // Step?
         if (evt.getSource() == (Object)stepButton)
         {
            speedSlider.setValue(MAX_STEP_DELAY);
            step();

            return;
         }
      }
   }
}
