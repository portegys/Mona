// For conditions of distribution and use, see copyright notice in mona.hpp

/*
 * Maze-learning mouse application.
 * See MazeMouse.java description.
 */
package mona;

import java.applet.Applet;
import java.applet.AudioClip;
import java.awt.*;
import java.awt.event.*;
import java.awt.geom.*;
import java.io.*;
import java.net.*;
import java.util.*;
import javax.swing.*;
import javax.swing.event.*;
import mona.Mona;


public class MazeMouseApplication extends JFrame
implements ActionListener, ItemListener, Runnable
{
   // Version (SCCS "what" format).
   static final String MAZE_MOUSE_VERSION = "@(#)Maze Learning Mouse, Version 1.1";

   // Dimensions.
   static final int WIDTH  = 700;
   static final int HEIGHT = 500;

   // Room types.
   static final int START_ROOM = 0;
   static final int BEGIN_MAZE = 1;
   static final int MAZE_ROOM  = 2;
   static final int END_MAZE   = 3;
   static final int GOAL_ROOM  = 4;
   static final int DEAD_ROOM  = 5;

   // Responses.
   static final int TAKE_DOOR_0 = 0;
   static final int TAKE_DOOR_1 = 1;
   static final int TAKE_DOOR_2 = 2;
   static final int WAIT        = 3;
   static final int HOP         = 4;

   // Cheese need and goal.
   static final double CHEESE_NEED = 1.0;
   static final double CHEESE_GOAL = 0.5;

   // Modes
   static final int TRAIN = 0;
   static final int RUN   = 1;

   // Random seed.
   static final int DEFAULT_RANDOM_SEED = 7671;

   // Step delay (ms).
   static final int STEP_DELAY = 1000;

   // Display update frequency (ms).
   static final int DISPLAY_DELAY = 50;

   // Mouse and cheese images.
   final static String MOUSE_IMAGE_FILE  = "mouse.gif";
   final static String CHEESE_IMAGE_FILE = "cheese.jpg";

   // Mouse squeak sound.
   final static String SQUEAK_SOUND_FILE = "squeak.wav";

   final static String[] HELP_TEXT =
   {
      "Create a path for the mouse to follow to the cheese by clicking on a sequence of rooms from one of the",
      "start rooms through the maze to one of the cheese goal rooms. Then train the mouse by selecting train",
      "mode and running a few trials. You run a trial by clicking the start trial control which starts the",
      "mouse moving through the rooms. When the mouse finishes moving, click stop trial. When the mouse is",
      "training, it is being shown the correct responses to make in each room. Now use test mode to test the",
      "mouse to see if it can find the cheese on its own!",
      "",
      "Now try something interesting! First click reset to make the mouse forget everything.",
      "Then select just one of the start rooms, the entry, exit, and one of the goal rooms. The mouse will",
      "\"hop\" over the maze. Train that a few times and test the mouse on it.",
      "",
      "Now train a path through just the maze by selecting the entry room, three connected maze rooms, and the",
      "exit room. Don't bother to test that now since the mouse didn't find any cheese that would make it want",
      "to do that on its own.",
      "",
      "Now comes the tricky part. Combine the paths that you trained separately into one path by selecting all",
      "the rooms from the start to the entry, then through the maze to the exit, then to the cheese goal. Test",
      "the mouse a few times to see if it can find the cheese. You may see it do an extra hop at the entry that",
      "it doesn't need to, but that's OK. If the mouse can find the cheese that means that it can combine things",
      "that it knows together to solve the problem!",
      "",
      "You can try other fancy stuff like training more than one start->entry->exit->goal room path, or changing",
      "the maze path while keeping the start->entry->exit->goal room path the same.",
      "",
      "Type 'p' to print the network."
   };

   // Mona.
   Mona mona;
   float[] sensors;

   // Maze.
   Room[][] maze;
   Dimension roomSize;
   int       beginMazeIndex;
   int       endMazeIndex;
   int       mouseX;
   int       mouseY;

   // Are maze room doors all visible?
   // Some of these doors are blocked.
   boolean SeeAllMazeDoors = false;

   // Display.
   Dimension screenSize;
   Canvas    mazeCanvas;
   Dimension mazeSize;
   Graphics  canvasGraphics;
   Image     mazeImage;
   Graphics  imageGraphics;
   String    error;

   // Controls.
   JPanel    controls;
   JCheckBox start;
   Choice    mode;
   TextField response;
   JCheckBox mute;
   JButton   reset;
   JButton   help;
   boolean   helpDisplay;

   // Font.
   Font        font;
   FontMetrics fontMetrics;
   int         fontAscent;
   int         fontWidth;
   int         fontHeight;

   // Images and sound.
   Image     mouseImage;
   Image     cheeseImage;
   AudioClip squeakSound;

   // Threads.
   private Thread displayThread;
   private Thread stepThread;
   Toolkit        toolkit;

   // Constructor.
   public MazeMouseApplication(int randomSeed)
   {
      // Set title.
      setTitle("Maze-learning Mouse");
      setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

      // Create mona.
      createMona(randomSeed);

      // Access toolkit.
      toolkit = Toolkit.getDefaultToolkit();

      // Create the maze display.
      JPanel screen = (JPanel)getContentPane();
      screen.setLayout(new BorderLayout());
      screenSize = new Dimension(WIDTH, HEIGHT);
      mazeSize   = new Dimension(screenSize.width,
                                 (int)((double)screenSize.height * .90));
      mazeCanvas = new Canvas();
      mazeCanvas.setSize(mazeSize.width, mazeSize.height);
      mazeCanvas.addMouseListener(new MazeMouseListener());
      mazeCanvas.addKeyListener(new MazeKeyListener());
      screen.add(mazeCanvas, BorderLayout.NORTH);
      error = null;

      // Add controls.
      controls = new JPanel();
      controls.setLayout(new FlowLayout());
      start = new JCheckBox("Start Trial");
      start.addItemListener(this);
      start.setToolTipText("Go mouse!");
      controls.add(start);
      controls.add(new JLabel("Mode:"));
      mode = new Choice();
      mode.add("Train");
      mode.add("Test");
      controls.add(mode);
      controls.add(new JLabel("Mouse Response:"));
      response = new TextField(10);
      response.setEditable(false);
      controls.add(response);
      mute = new JCheckBox("Mute");
      mute.setToolTipText("No squeaking!");
      controls.add(mute);
      reset = new JButton("Reset");
      reset.addActionListener(this);
      controls.add(reset);
      help = new JButton("Help");
      help.addActionListener(this);
      controls.add(help);
      screen.add(controls, BorderLayout.SOUTH);
      helpDisplay = false;

      // Build maze.
      int x;
      int y;
      Room[] doors;
      roomSize       = new Dimension(mazeSize.width / 15, mazeSize.height / 11);
      maze           = new Room[7][];
      maze[0]        = new Room[3];
      x              = roomSize.width;
      y              = (mazeSize.height / 2) - (roomSize.height / 2);
      maze[0][1]     = new Room(x, y, START_ROOM, 0, 1);
      maze[0][0]     = new Room(x, y - (roomSize.height * 2), START_ROOM, 0, 0);
      maze[0][2]     = new Room(x, y + (roomSize.height * 2), START_ROOM, 0, 2);
      beginMazeIndex = 1;
      maze[1]        = new Room[1];
      x             += (roomSize.width * 2);
      y              = (mazeSize.height / 2) - (roomSize.height / 2);
      maze[1][0]     = new Room(x, y, BEGIN_MAZE, 1, 0);
      doors          = new Room[3];
      doors[0]       = null;
      doors[1]       = null;
      doors[2]       = maze[1][0];
      maze[0][0].setDoors(doors);
      doors    = new Room[3];
      doors[0] = null;
      doors[1] = maze[1][0];
      doors[2] = null;
      maze[0][1].setDoors(doors);
      doors    = new Room[3];
      doors[0] = maze[1][0];
      doors[1] = null;
      doors[2] = null;
      maze[0][2].setDoors(doors);
      maze[2]    = new Room[3];
      x         += (roomSize.width * 2);
      y          = (mazeSize.height / 2) - (roomSize.height / 2);
      maze[2][1] = new Room(x, y, MAZE_ROOM, 2, 1);
      maze[2][0] = new Room(x, y - (roomSize.height * 2), MAZE_ROOM, 2, 0);
      maze[2][2] = new Room(x, y + (roomSize.height * 2), MAZE_ROOM, 2, 2);
      doors      = new Room[3];
      doors[0]   = maze[2][0];
      doors[1]   = maze[2][1];
      doors[2]   = maze[2][2];
      maze[1][0].setDoors(doors);
      maze[3]    = new Room[5];
      x         += (roomSize.width * 2);
      y          = (mazeSize.height / 2) - (roomSize.height / 2);
      maze[3][2] = new Room(x, y, MAZE_ROOM, 3, 2);
      maze[3][1] = new Room(x, y - (roomSize.height * 2), MAZE_ROOM, 3, 1);
      maze[3][0] = new Room(x, y - (roomSize.height * 4), MAZE_ROOM, 3, 0);
      maze[3][3] = new Room(x, y + (roomSize.height * 2), MAZE_ROOM, 3, 3);
      maze[3][4] = new Room(x, y + (roomSize.height * 4), MAZE_ROOM, 3, 4);
      doors      = new Room[3];
      doors[0]   = maze[3][0];
      doors[1]   = maze[3][1];
      doors[2]   = maze[3][2];
      maze[2][0].setDoors(doors);
      doors    = new Room[3];
      doors[0] = maze[3][1];
      doors[1] = maze[3][2];
      doors[2] = maze[3][3];
      maze[2][1].setDoors(doors);
      doors    = new Room[3];
      doors[0] = maze[3][2];
      doors[1] = maze[3][3];
      doors[2] = maze[3][4];
      maze[2][2].setDoors(doors);
      maze[4]    = new Room[3];
      x         += (roomSize.width * 2);
      y          = (mazeSize.height / 2) - (roomSize.height / 2);
      maze[4][1] = new Room(x, y, MAZE_ROOM, 4, 1);
      maze[4][0] = new Room(x, y - (roomSize.height * 2), MAZE_ROOM, 4, 0);
      maze[4][2] = new Room(x, y + (roomSize.height * 2), MAZE_ROOM, 4, 2);
      doors      = new Room[3];
      doors[0]   = null;
      doors[1]   = null;
      doors[2]   = maze[4][0];
      maze[3][0].setDoors(doors);
      doors    = new Room[3];
      doors[0] = null;
      doors[1] = maze[4][0];
      doors[2] = maze[4][1];
      maze[3][1].setDoors(doors);
      doors    = new Room[3];
      doors[0] = maze[4][0];
      doors[1] = maze[4][1];
      doors[2] = maze[4][2];
      maze[3][2].setDoors(doors);
      doors    = new Room[3];
      doors[0] = maze[4][1];
      doors[1] = maze[4][2];
      doors[2] = null;
      maze[3][3].setDoors(doors);
      doors    = new Room[3];
      doors[0] = maze[4][2];
      doors[1] = null;
      doors[2] = null;
      maze[3][4].setDoors(doors);
      endMazeIndex = 5;
      maze[5]      = new Room[1];
      x           += (roomSize.width * 2);
      y            = (mazeSize.height / 2) - (roomSize.height / 2);
      maze[5][0]   = new Room(x, y, END_MAZE, 5, 0);
      doors        = new Room[3];
      doors[0]     = null;
      doors[1]     = null;
      doors[2]     = maze[5][0];
      maze[4][0].setDoors(doors);
      doors    = new Room[3];
      doors[0] = null;
      doors[1] = maze[5][0];
      doors[2] = null;
      maze[4][1].setDoors(doors);
      doors    = new Room[3];
      doors[0] = maze[5][0];
      doors[1] = null;
      doors[2] = null;
      maze[4][2].setDoors(doors);
      maze[6]    = new Room[3];
      x         += (roomSize.width * 2);
      y          = (mazeSize.height / 2) - (roomSize.height / 2);
      maze[6][1] = new Room(x, y, GOAL_ROOM, 6, 1);
      maze[6][0] = new Room(x, y - (roomSize.height * 2), GOAL_ROOM, 6, 0);
      maze[6][2] = new Room(x, y + (roomSize.height * 2), GOAL_ROOM, 6, 2);
      doors      = new Room[3];
      doors[0]   = maze[6][0];
      doors[1]   = maze[6][1];
      doors[2]   = maze[6][2];
      maze[5][0].setDoors(doors);
      doors    = new Room[3];
      doors[0] = null;
      doors[1] = null;
      doors[2] = null;
      maze[6][0].setDoors(doors);
      maze[6][1].setDoors(doors);
      maze[6][2].setDoors(doors);
      mouseX = mouseY = 0;
      maze[0][0].hasMouse = true;
      maze[0][0].selected = true;
      maze[endMazeIndex + 1][2].hasCheese = true;
      maze[endMazeIndex + 1][2].selected  = true;

      // Show app.
      pack();
      setVisible(true);

      // Get graphics.
      if ((canvasGraphics = mazeCanvas.getGraphics()) == null)
      {
         System.err.println("Cannot get canvas graphics");
         System.exit(1);
      }
      mazeImage     = createImage(mazeSize.width, mazeSize.height);
      imageGraphics = mazeImage.getGraphics();

      // Set font.
      font = new Font("Helvetica", Font.PLAIN, 12);
      canvasGraphics.setFont(font);
      imageGraphics.setFont(font);
      fontMetrics = canvasGraphics.getFontMetrics();
      fontAscent  = fontMetrics.getMaxAscent();
      fontWidth   = fontMetrics.getMaxAdvance();
      fontHeight  = fontMetrics.getHeight();

      // Get mouse and cheese images.
      Image mouse = null;
      mouseImage = null;

      Image cheese = null;
      cheeseImage = null;

      int w1;
      int h1;
      int w2;
      int h2;

      URL imgURL = MazeMouse.class .getResource(MOUSE_IMAGE_FILE);

      if (imgURL != null)
      {
         mouse = new ImageIcon(imgURL).getImage();
      }

      if (mouse == null)
      {
         error = "Cannot get image: " + MOUSE_IMAGE_FILE;
      }

      imgURL = MazeMouse.class .getResource(CHEESE_IMAGE_FILE);

      if (imgURL != null)
      {
         cheese = new ImageIcon(imgURL).getImage();
      }

      if (cheese == null)
      {
         error = "Cannot get image: " + CHEESE_IMAGE_FILE;
      }

      if (mouse != null)
      {
         w1 = mouse.getWidth(this);
         h1 = mouse.getHeight(this);

         if ((w1 <= 0) || (h1 <= 0))
         {
            error = "Invalid image: " + MOUSE_IMAGE_FILE;
         }
         else
         {
            w2         = roomSize.width / 2;
            h2         = roomSize.height / 2;
            mouseImage = createImage(w2, h2);

            Graphics graphics = mouseImage.getGraphics();
            graphics.drawImage(mouse, 0, 0, w2 - 1, h2 - 1, 0, 0, w1 - 1,
                               h1 - 1, Color.white, this);
         }
      }

      if (cheese != null)
      {
         w1 = cheese.getWidth(this);
         h1 = cheese.getHeight(this);

         if ((w1 <= 0) || (h1 <= 0))
         {
            error = "Invalid image: " + CHEESE_IMAGE_FILE;
         }
         else
         {
            w2          = roomSize.width / 2;
            h2          = roomSize.height / 2;
            cheeseImage = createImage(w2, h2);

            Graphics graphics = cheeseImage.getGraphics();
            graphics.drawImage(cheese, 0, 0, w2 - 1, h2 - 1, 0, 0, w1 - 1,
                               h1 - 1, Color.white, this);
         }
      }

      // Load mouse squeak sound.
      squeakSound = null;

      URL url = MazeMouse.class .getResource(SQUEAK_SOUND_FILE);

      if (url != null)
      {
         squeakSound = Applet.newAudioClip(url);
      }

      if (squeakSound == null)
      {
         error = "Cannot load sound file: " + SQUEAK_SOUND_FILE;
      }
      else
      {
         // Playing and stopping ensures sound completely loaded.
         squeakSound.play();
         squeakSound.stop();
      }

      if (displayThread == null)
      {
         displayThread = new Thread(this);
         displayThread.setPriority(Thread.MIN_PRIORITY);
         displayThread.start();
      }

      if ((stepThread == null) && (error == null))
      {
         stepThread = new Thread(this);
         stepThread.setPriority(Thread.MIN_PRIORITY);
         stepThread.start();
      }
   }


   // Create Mona.
   void createMona(int randomSeed)
   {
      Vector<String> keys   = new Vector<String>();
      Vector<Object> values = new Vector<Object>();
      keys.add("NUM_SENSORS");
      values.add("5");
      keys.add("NUM_RESPONSES");
      values.add((HOP + 1) + "");
      keys.add("NUM_NEEDS");
      values.add("1");
      keys.add("MAX_MEDIATOR_LEVEL");
      values.add("1");
      keys.add("RANDOM_SEED");
      values.add(randomSeed + "");
      mona = new Mona(keys, values);

      // Set a long second effect interval
      // for a higher level mediator.
      mona.setEffectEventIntervals(1, 2);
      mona.setEffectEventInterval(1, 0, 2, 0.5f);
      mona.setEffectEventInterval(1, 1, 10, 0.5f);

      // Set need and goal for cheese.
      mona.setNeed(0, CHEESE_NEED);
      sensors    = new float[5];
      sensors[0] = 0.0f;
      sensors[1] = 0.0f;
      sensors[2] = 0.0f;
      sensors[3] = (float)GOAL_ROOM;
      sensors[4] = 1.0f;
      mona.addGoal(0, sensors, 0, CHEESE_GOAL);
   }


   // Run.
   public void run()
   {
      // Display loop.
      while ((Thread.currentThread() == displayThread) &&
             !displayThread.isInterrupted())
      {
         display();

         try {
            Thread.sleep(DISPLAY_DELAY);
         }
         catch (InterruptedException e) {
            break;
         }
      }

      // Step loop.
      while ((Thread.currentThread() == stepThread) &&
             !stepThread.isInterrupted())
      {
         step();

         try {
            Thread.sleep(STEP_DELAY);
         }
         catch (InterruptedException e) {
            break;
         }
      }
   }


   // Display.
   synchronized void display()
   {
      // Clear display.
      imageGraphics.setColor(Color.white);
      imageGraphics.fillRect(0, 0, mazeSize.width, mazeSize.height);

      if (helpDisplay)
      {
         imageGraphics.setColor(Color.black);
         for (int i = 0; i < HELP_TEXT.length; i++)
         {
            imageGraphics.drawString(HELP_TEXT[i], 1, (i * (fontHeight + 1)) + fontHeight);
         }
      }
      else
      {
         // Draw maze.
         for (int i = 0; i < maze.length; i++)
         {
            for (int j = 0; j < maze[i].length; j++)
            {
               maze[i][j].draw(imageGraphics);
            }
         }

         // Label maze.
         int h = fontHeight / 2;
         imageGraphics.drawString("Start", maze[0][0].x + 4, maze[0][0].y - h);
         imageGraphics.drawString("Entry", maze[beginMazeIndex][0].x + 4,
                                  maze[beginMazeIndex][0].y - h);
         imageGraphics.drawString("Maze", maze[beginMazeIndex + 1][0].x + 6,
                                  maze[beginMazeIndex + 1][0].y - h);
         imageGraphics.drawString("Maze", maze[beginMazeIndex + 2][0].x + 6,
                                  maze[beginMazeIndex + 2][0].y - h);
         imageGraphics.drawString("Maze", maze[beginMazeIndex + 3][0].x + 6,
                                  maze[beginMazeIndex + 3][0].y - h);
         imageGraphics.drawString("Exit", maze[endMazeIndex][0].x + 6,
                                  maze[endMazeIndex][0].y - h);
         imageGraphics.drawString("Goal", maze[endMazeIndex + 1][0].x + 6,
                                  maze[endMazeIndex + 1][0].y - h);

         // Show error message?
         if (error != null)
         {
            imageGraphics.setColor(Color.white);

            int w = fontMetrics.stringWidth(error);
            imageGraphics.fillRect(((mazeSize.width - w) / 2) - 2,
                                   (mazeSize.height / 2) - (fontAscent + 2), w + 4, fontHeight +
                                   4);
            imageGraphics.setColor(Color.black);
            imageGraphics.drawString(error, (mazeSize.width - w) / 2,
                                     mazeSize.height / 2);
         }
      }

      // Refresh display.
      canvasGraphics.drawImage(mazeImage, 0, 0, this);
   }


   // Step.
   void step()
   {
      if (!start.isSelected() || (mouseX == -1) || (error != null))
      {
         return;
      }

      // Determine correct response.
      int cx = -1;
      int cy = -1;
      int cr = -1;
      int r  = -1;
      int i;
      int j;

      for (i = mouseX + 1; (i < maze.length) && (cx == -1); i++)
      {
         for (j = 0; (j < maze[i].length) && (cx == -1); j++)
         {
            if (maze[i][j].selected)
            {
               cx = i;
               cy = j;
            }
         }
      }

      if (cx == (mouseX + 1))
      {
         for (i = 0; (i < maze[mouseX][mouseY].doors.length) && (cr == -1);
              i++)
         {
            if ((maze[mouseX][mouseY].doors[i] != null) &&
                (maze[mouseX][mouseY].doors[i].cx == cx) &&
                (maze[mouseX][mouseY].doors[i].cy == cy))
            {
               cr = i;

               break;
            }
         }
      }
      else
      {
         if (cx != -1)
         {
            cr = HOP;
         }
         else
         {
            cx = mouseX;
            cy = mouseY;
            cr = WAIT;
         }
      }

      if ((mode.getSelectedIndex() == TRAIN) || (cr == WAIT))
      {
         mona.overrideResponse(cr);
      }
      else
      {
         mona.clearResponseOverride();
      }

      // Initiate sensory/response cycle.
      for (i = 0; i < maze[mouseX][mouseY].doors.length; i++)
      {
         if (maze[mouseX][mouseY].doors[i] != null)
         {
            sensors[i] = 1.0f;
         }
         else if (SeeAllMazeDoors && (mouseX > 1) && (mouseX < 5))
         {
            // Maze room door is visible although possibly blocked.
            sensors[i] = 1.0f;
         }
         else
         {
            sensors[i] = 0.0f;
         }
      }

      sensors[3] = (float)maze[mouseX][mouseY].type;

      if (maze[mouseX][mouseY].hasCheese)
      {
         sensors[4] = 1.0f;

         // Eat the cheese!
         maze[mouseX][mouseY].hasCheese = false;

         if ((squeakSound != null) && !mute.isSelected())
         {
            squeakSound.play();
         }
      }
      else
      {
         sensors[4] = 0.0f;
      }

      // Move mouse based on response.
      r = mona.cycle(sensors);
      if (r < WAIT)
      {
         response.setText("Door " + r);
      }
      else if (r == WAIT)
      {
         response.setText("Wait");
      }
      else
      {
         response.setText("Hop");
      }

      // Move mouse.
      if (r == cr)
      {
         maze[mouseX][mouseY].hasMouse = false;
         mouseX = cx;
         mouseY = cy;
         maze[mouseX][mouseY].hasMouse = true;
      }
   }


   // Button listener.
   public void actionPerformed(ActionEvent evt)
   {
      if (evt.getSource() == help)
      {
         helpDisplay = !helpDisplay;
         if (helpDisplay)
         {
            help.setText("Back");
         }
         else
         {
            help.setText("Help");
         }
         return;
      }

      if (error != null)
      {
         return;
      }

      if (evt.getSource() == reset)
      {
         // Erase mouse memory.
         mona.clearWorkingMemory();
         mona.clearLongTermMemory();

         // Clear maze.
         for (int i = 0; i < maze.length; i++)
         {
            for (int j = 0; j < maze[i].length; j++)
            {
               maze[i][j].selected  = false;
               maze[i][j].hasMouse  = false;
               maze[i][j].hasCheese = false;
            }
         }

         mouseX = mouseY = 0;
         maze[0][0].selected = true;
         maze[0][0].hasMouse = true;
         maze[endMazeIndex + 1][2].selected  = true;
         maze[endMazeIndex + 1][2].hasCheese = true;
         start.setSelected(false);
         start.setText("Start Trial");
         response.setText("");
      }
   }


   // CheckBox listener.
   public void itemStateChanged(ItemEvent evt)
   {
      if (error != null)
      {
         return;
      }

      if (evt.getSource() == start)
      {
         if (start.isSelected())
         {
            start.setText("Stop Trial");

            // Reset need.
            mona.setNeed(0, CHEESE_NEED);

            // Clear working memory.
            mona.clearWorkingMemory();
         }
         else
         {
            start.setText("Start Trial");
            response.setText("");

            // Place cheese in selected goal room.
            for (int i = 0; i < maze[endMazeIndex + 1].length; i++)
            {
               if (maze[endMazeIndex + 1][i].selected)
               {
                  maze[endMazeIndex + 1][i].hasCheese = true;

                  break;
               }
            }

            // Move mouse back to leftmost selected room.
            if (mouseX != -1)
            {
               for (int i = 0; i < maze.length; i++)
               {
                  for (int j = 0; j < maze[i].length; j++)
                  {
                     if (maze[i][j].selected)
                     {
                        maze[mouseX][mouseY].hasMouse = false;
                        maze[i][j].hasMouse           = true;
                        mouseX = i;
                        mouseY = j;

                        return;
                     }
                  }
               }
            }
         }
      }
   }


   // Maze room.
   class Room extends Rectangle {
      int     type;
      int     cx;
      int     cy;
      boolean selected;
      boolean hasMouse;
      boolean hasCheese;
      Room[] doors;

      Room(int x, int y, int type, int cx, int cy)
      {
         super(x, y, roomSize.width, roomSize.height);
         this.type = type;
         this.cx   = cx;
         this.cy   = cy;
         selected  = hasMouse = hasCheese = false;
      }


      // Set room doors to other rooms.
      void setDoors(Room[] doors)
      {
         this.doors = new Room[doors.length];

         for (int i = 0; i < doors.length; i++)
         {
            this.doors[i] = doors[i];
         }
      }


      // Draw the room.
      void draw(Graphics g)
      {
         Room to;

         g.setColor(Color.black);

         if (selected)
         {
            g.fillRect(x, y, width, height);
            g.setColor(Color.white);
            g.fillRect(x + 4, y + 4, width - 8, height - 8);
            g.setColor(Color.black);
         }
         else
         {
            g.drawRect(x, y, width, height);
         }

         if (hasCheese && (cheeseImage != null))
         {
            g.drawImage(cheeseImage, x + (width / 4), y + (height / 4),
                        mazeCanvas);
         }

         if (hasMouse && (mouseImage != null))
         {
            g.drawImage(mouseImage, x + (width / 4), y + (height / 4),
                        mazeCanvas);
         }

         for (int i = 0; i < doors.length; i++)
         {
            if ((to = doors[i]) != null)
            {
               g.drawLine(x + width, y + (height / 2), to.x,
                          to.y + (to.height / 2));
            }
         }
      }
   }

   // Mouse press.
   class MazeMouseListener extends MouseAdapter {
      public void mousePressed(MouseEvent evt)
      {
         if (error != null)
         {
            return;
         }

         // Request keyboard focus.
         requestFocus();

         // Select/deselect room.
         boolean found = false;
         int     cx    = 0;
         int     cy    = 0;

         for (int i = 0; (i < maze.length) && !found; i++)
         {
            for (int j = 0; (j < maze[i].length) && !found; j++)
            {
               if (maze[i][j].contains(evt.getX(), evt.getY()))
               {
                  found = true;
                  cx    = i;
                  cy    = j;
               }
            }
         }

         if (!found)
         {
            return;
         }

         if (start.isSelected())
         {
            toolkit.beep();

            return;
         }

         maze[cx][cy].selected = !maze[cx][cy].selected;

         // Move mouse/cheese if necessary.
         if (maze[cx][cy].selected)
         {
            for (int i = 0; i < maze[cx].length; i++)
            {
               if ((i != cy) && maze[cx][i].selected)
               {
                  maze[cx][i].selected = false;

                  if (maze[cx][i].hasMouse)
                  {
                     mouseX = mouseY = -1;
                  }

                  maze[cx][i].hasMouse  = false;
                  maze[cx][i].hasCheese = false;
               }
            }

            if ((mouseX != -1) && (mouseX > cx))
            {
               maze[mouseX][mouseY].hasMouse = false;
               mouseX = mouseY = -1;
            }

            if (mouseX == -1)
            {
               maze[cx][cy].hasMouse = true;
               mouseX = cx;
               mouseY = cy;
            }

            if (cx == (endMazeIndex + 1))
            {
               maze[cx][cy].hasCheese = true;
            }
         }
         else
         {
            if (maze[cx][cy].hasMouse)
            {
               mouseX = mouseY = -1;

               for (int i = cx + 1; (i < maze.length) && (mouseX == -1);
                    i++)
               {
                  for (int j = 0; (j < maze[i].length) && (mouseX == -1);
                       j++)
                  {
                     if (maze[i][j].selected)
                     {
                        maze[i][j].hasMouse = true;
                        mouseX = i;
                        mouseY = j;
                     }
                  }
               }
            }

            maze[cx][cy].hasMouse  = false;
            maze[cx][cy].hasCheese = false;
         }
      }
   }

   // Key press.
   class MazeKeyListener extends KeyAdapter {
      public void keyPressed(KeyEvent evt)
      {
         int key = evt.getKeyCode();

         switch (key)
         {
         case 'p':
         case 'P':
            // Print neural network.
            mona.print();
            break;
         }
      }
   }

   // Main.
   public static void main(String[] args)
   {
      String usage      = "Usage: java mona.MazeMouseApplication [-randomSeed <random seed>]";
      int    randomSeed = DEFAULT_RANDOM_SEED;

      for (int i = 0; i < args.length; i++)
      {
         if (args[i].equals("-randomSeed"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid random seed");
               System.err.println(usage);
               System.exit(1);
            }
            randomSeed = Integer.parseInt(args[i]);
         }
         else
         {
            System.err.println(usage);
            System.exit(1);
         }
      }

      MazeMouseApplication mazeMouse = new MazeMouseApplication(randomSeed);
   }
}
