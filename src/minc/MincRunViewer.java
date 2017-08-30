// Minc world run viewer.
// Usage: java MincRunViewer <minc world run result file name> | - (stdin)

package mona;

import java.awt.*;
import java.awt.geom.*;
import java.awt.event.*;
import java.io.*;
import java.util.*;
import javax.swing.*;

public class MincRunViewer extends JFrame
implements ActionListener, ItemListener
{
   // Parameters.
   int numMazeMarks;
   int numMazes;
   int randomSeed;
   int generalizationRuns;
   int discriminationRuns;
   int resultRuns;

   // Mazes.
   Vector<Tmaze> mazes;
   int           discriminationMazeIndex;

   // Results.
   int successfulRuns;

   // Display.
   static final Dimension SCREEN_SIZE = new Dimension(600, 700);
   TmazeDisplay           mazeDisplay;
   JScrollPane            displayScroll;
   Vector<Tmaze>          displayMazes;

   // Controls.
   JPanel  controls;
   Choice  viewChoice;
   int     previousViewIndex;
   JLabel  itemLabel;
   Choice  itemChoice;
   int     previousRunIndex;
   JLabel  stepLabel;
   Choice  stepChoice;
   JButton stepButton;

   // Run.
   class Run
   {
      // Step.
      class Step
      {
         int   tick;
         int   mark;
         int   direction;
         int   lensDirection;
         float lensLeftOutput, lensRightOutput;
         int   monaDirection;
         float monaLeftOutput, monaRightOutput;

         Step(int tick, int mark, int direction,
              int lensDirection, float lensLeftOutput, float lensRightOutput,
              int monaDirection, float monaLeftOutput, float monaRightOutput)
         {
            this.tick            = tick;
            this.mark            = mark;
            this.direction       = direction;
            this.lensDirection   = lensDirection;
            this.lensLeftOutput  = lensLeftOutput;
            this.lensRightOutput = lensRightOutput;
            this.monaDirection   = monaDirection;
            this.monaLeftOutput  = monaLeftOutput;
            this.monaRightOutput = monaRightOutput;
         }


         Step()
         {
            tick            = -1;
            mark            = -1;
            direction       = -1;
            lensDirection   = -1;
            lensLeftOutput  = -1.0f;
            lensRightOutput = -1.0f;
            monaDirection   = -1;
            monaLeftOutput  = -1.0f;
            monaRightOutput = -1.0f;
         }
      }

      Vector<Step> path;
      boolean      result;

      Run()
      {
         path   = new Vector<Step>();
         result = false;
      }


      Step newStep()
      {
         return(new Step());
      }


      void addStep(int tick, int mark, int direction,
                   int lensDirection, float lensLeftOutput, float lensRightOutput,
                   int monaDirection, float monaLeftOutput, float monaRightOutput)
      {
         path.add(new Step(tick, mark, direction,
                           lensDirection, lensLeftOutput, lensRightOutput,
                           monaDirection, monaLeftOutput, monaRightOutput));
      }


      void addStep(Step step)
      {
         path.add(step);
      }


      void setStatus(boolean result)
      {
         this.result = result;
      }


      void print()
      {
         Step step;

         System.out.println("Run:");
         for (int i = 0; i < path.size(); i++)
         {
            step = path.get(i);
            System.out.println("tick=" + step.tick + " mark=" + step.mark);
            System.out.print("direction=" + step.direction + " lens=" + step.lensDirection);
            System.out.print(String.format(" [%f %f]", step.lensLeftOutput, step.lensRightOutput));
            System.out.print(" mona=" + step.monaDirection);
            System.out.println(String.format(" [%f %f]", step.monaLeftOutput, step.monaRightOutput));
         }
         if (result == true)
         {
            System.out.println("result=success");
         }
         else
         {
            System.out.println("result=fail");
         }
      }
   };

   // Run results.
   Vector<Run> runs;

   // Constructor.
   public MincRunViewer()
   {
      mazes = new Vector<Tmaze>();
      discriminationMazeIndex = -1;
      runs           = new Vector<Run>();
      successfulRuns = -1;
   }


   // Load from file.
   public boolean load(String resultsFilename)
   {
      Scanner scanner = null;

      try
      {
         File resultsFile = new File(resultsFilename);
         scanner = new Scanner(new FileReader(resultsFile));
         return(load(scanner));
      }
      catch (Exception e) {
         System.err.println("Error: " + e.getMessage());
         return(false);
      }
      finally {
         if (scanner != null)
         {
            scanner.close();
         }
      }
   }


   // Load from scanner.
   public boolean load(Scanner scanner)
   {
      Vector<String> input = new Vector<String>();
      try
      {
         while (scanner.hasNextLine())
         {
            input.add(scanner.nextLine());
         }
      }
      catch (Exception e) {
         System.err.println("Error: " + e.getMessage());
         return(false);
      }
      if (!loadParameters(input)) { return(false); }
      if (!loadMazes(input)) { return(false); }
      if (!loadRuns(input)) { return(false); }
      return(true);
   }


   // Load parameters.
   private boolean loadParameters(Vector<String> input)
   {
      String s;
      String d = "=";

      try
      {
         if (input.size() > 0)
         {
            s = input.get(0).trim();
            input.remove(0);
            if (!s.startsWith("Parameters:"))
            {
               System.err.println("Invalid parameters heading");
               return(false);
            }
         }
         else
         {
            System.err.println("Premature end of parameter input");
            return(false);
         }
         while (input.size() > 0)
         {
            s          = input.get(0).trim();
            String[] t = s.split(d);
            if ((t != null) && (t.length == 2))
            {
               if (t[0].equals("numMazeMarks"))
               {
                  numMazeMarks = Integer.parseInt(t[1]);
               }
               else if (t[0].equals("numMazes"))
               {
                  numMazes = Integer.parseInt(t[1]);
               }
               else if (t[0].equals("numMazes"))
               {
                  numMazes = Integer.parseInt(t[1]);
               }
               else if (t[0].equals("randomSeed"))
               {
                  randomSeed = Integer.parseInt(t[1]);
               }
               else if (t[0].equals("generalizationRuns"))
               {
                  generalizationRuns = Integer.parseInt(t[1]);
               }
               else if (t[0].equals("discriminationRuns"))
               {
                  discriminationRuns = Integer.parseInt(t[1]);
               }
               else if (t[0].equals("resultRuns"))
               {
                  resultRuns = Integer.parseInt(t[1]);
               }
               else{ break; }
            }
            else
            {
               break;
            }
            input.remove(0);
         }
      }
      catch (Exception e) {
         System.err.println("Error loading parameters: " + e.getMessage());
         return(false);
      }
      return(true);
   }


   // Load mazes.
   private boolean loadMazes(Vector<String> input)
   {
      Tmaze  maze;
      String s;
      String d = " ";

      try
      {
         if (input.size() > 0)
         {
            s = input.get(0).trim();
            input.remove(0);
            if (!s.startsWith("Generalization mazes:"))
            {
               System.err.println("Invalid generalization mazes heading");
               return(false);
            }
         }
         else
         {
            System.err.println("Premature end of maze input");
            return(false);
         }
         maze = null;
         while (input.size() > 0)
         {
            s = input.get(0).trim();
            input.remove(0);
            String[] t = s.split(d);
            if ((t != null) && (t.length > 0))
            {
               if (t[0].equals("name:"))
               {
                  maze = new Tmaze();
                  mazes.add(maze);
               }
               else if (t[0].startsWith("I:"))
               {
                  maze.addJunction(getMark(t), getDirection(t), getProbability(t));
               }
               else{ break; }
            }
         }
         if (!s.startsWith("Discrimination maze:"))
         {
            System.err.println("Invalid maze discrimination maze heading");
            return(false);
         }
         maze = null;
         while (input.size() > 0)
         {
            s          = input.get(0).trim();
            String[] t = s.split(d);
            if ((t != null) && (t.length > 0))
            {
               if (t[0].equals("name:"))
               {
                  maze = new Tmaze();
               }
               else if (t[0].startsWith("I:"))
               {
                  maze.addJunction(getMark(t), getDirection(t), getProbability(t));
               }
               else{ break; }
            }
            input.remove(0);
         }
         discriminationMazeIndex = -1;
         if (maze != null)
         {
            for (int i = 0; i < mazes.size(); i++)
            {
               if (maze.isDuplicate(mazes.get(i)) == true)
               {
                  discriminationMazeIndex = i;
                  break;
               }
            }
         }
         if (discriminationMazeIndex == -1)
         {
            System.err.println("Invalid discrimination maze");
            return(false);
         }
      }
      catch (Exception e) {
         System.err.println("Error loading mazes: " + e.getMessage());
         return(false);
      }
      return(true);
   }


   private int getMark(String[] e)
   {
      for (int i = 1; i < e.length; i++)
      {
         if (e[i].equals("1"))
         {
            return(i - 1);
         }
      }
      return(-1);
   }


   private int getDirection(String[] e)
   {
      if (e[e.length - 5].equals("1"))
      {
         return(0);
      }
      else
      {
         return(1);
      }
   }


   private double getProbability(String[] e)
   {
      return(Double.parseDouble(e[e.length - 2]));
   }


   // Load runs.
   private boolean loadRuns(Vector<String> input)
   {
      Run run;

      Run.Step step;
      String   s;
      String   dsp   = " ";
      String   deq   = "=";
      String   dcol  = ":";
      String   dsemi = ";";
      String   dslsh = "/";
      String   dlb   = "\\[";
      String   drb   = "\\]";

      try
      {
         run  = new Run();
         step = null;
         while (input.size() > 0)
         {
            s = input.get(0).trim();
            input.remove(0);
            if (s.startsWith("tick="))
            {
               String[] t = s.split(deq);
               if ((t != null) && (t.length == 2))
               {
                  if (step != null) { run.addStep(step); }
                  step      = run.newStep();
                  step.tick = Integer.parseInt(t[1]);
               }
               else
               {
                  System.err.println("Invalid run format");
                  return(false);
               }
            }
            else if (s.startsWith("inputs:"))
            {
               if (step != null)
               {
                  String[] t = s.split(dsp);
                  if ((t != null) && (t.length > 1))
                  {
                     for (int i = 1; i < t.length; i++)
                     {
                        if (Double.parseDouble(t[i]) > 0.0)
                        {
                           step.mark = i - 1;
                           break;
                        }
                     }
                     if (step.mark < 0)
                     {
                        System.err.println("Invalid run format");
                        return(false);
                     }
                  }
               }
               else
               {
                  System.err.println("Invalid run format");
                  return(false);
               }
            }
            else if (s.startsWith("output="))
            {
               if (step != null)
               {
                  String[] t0 = s.split(dsp);
                  if ((t0 != null) && ((t0.length == 7) || (t0.length == 8)))
                  {
                     String[] t1 = t0[0].split(deq);
                     if ((t1 != null) && (t1.length == 2))
                     {
                        step.direction = Integer.parseInt(t1[1]);
                     }
                     t1 = t0[1].split(deq);
                     if ((t1 != null) && (t1.length == 2))
                     {
                        t1 = t1[1].split(dcol);
                        if ((t1 != null) && (t1.length == 1))
                        {
                           step.lensDirection = Integer.parseInt(t1[0]);
                        }
                     }
                     t1 = t0[2].split(dlb);
                     if ((t1 != null) && (t1.length == 2))
                     {
                        step.lensLeftOutput = (float)Double.parseDouble(t1[1]);
                     }
                     t1 = t0[3].split(drb);
                     if ((t1 != null) && (t1.length == 1))
                     {
                        step.lensRightOutput = (float)Double.parseDouble(t1[0]);
                     }
                     t1 = t0[4].split(deq);
                     if ((t1 != null) && (t1.length == 2))
                     {
                        t1 = t1[1].split(dcol);
                        if ((t1 != null) && (t1.length == 1))
                        {
                           step.monaDirection = Integer.parseInt(t1[0]);
                        }
                     }
                     t1 = t0[5].split(dlb);
                     if ((t1 != null) && (t1.length == 2))
                     {
                        step.monaLeftOutput = (float)Double.parseDouble(t1[1]);
                     }
                     t1 = t0[6].split(drb);
                     if ((t1 != null) && (t1.length == 1))
                     {
                        step.monaRightOutput = (float)Double.parseDouble(t1[0]);
                     }
                  }
               }
               else
               {
                  System.err.println("Invalid run format");
                  return(false);
               }
            }
            else if (s.startsWith("Result run="))
            {
               if (step != null) { run.addStep(step); }
               String[] t = s.split(dsp);
               if ((t != null) && (t.length == 3))
               {
                  if (t[2].equals("success"))
                  {
                     run.result = true;
                  }
                  runs.add(run);
                  run  = new Run();
                  step = null;
               }
               else
               {
                  System.err.println("Invalid run format");
                  return(false);
               }
            }
            else if (s.startsWith("Result summary:"))
            {
               String[] t = s.split(dsemi);
               if ((t != null) && (t.length == 3))
               {
                  String[] t0 = t[0].split(deq);
                  if ((t0 != null) && (t0.length == 2))
                  {
                     String[] t1 = t0[1].split(dslsh);
                     if ((t1 != null) && (t1.length == 2))
                     {
                        successfulRuns = Integer.parseInt(t1[0]);
                     }
                  }
               }
               else
               {
                  System.err.println("Invalid run result summary");
                  return(false);
               }
            }
         }
      }
      catch (Exception e) {
         System.err.println("Error loading runs: " + e.getMessage());
         return(false);
      }
      return(true);
   }


   // Run.
   public void run()
   {
      setTitle("Minc run viewer");
      setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
      JPanel content = (JPanel)getContentPane();
      content.setLayout(new BorderLayout());
      Dimension mazeDisplaySize = new Dimension(SCREEN_SIZE.width,
                                                (int)((double)SCREEN_SIZE.height * .85));
      mazeDisplay  = new TmazeDisplay(mazeDisplaySize);
      displayMazes = mazes;
      mazeDisplay.load(displayMazes);
      displayScroll = new JScrollPane(mazeDisplay,
                                      JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                                      JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
      displayScroll.setPreferredSize(mazeDisplaySize);
      content.add(displayScroll, BorderLayout.SOUTH);
      controls = new JPanel();
      controls.setLayout(new FlowLayout());
      controls.add(new JLabel("View:"));
      viewChoice = new Choice();
      viewChoice.add("Maze");
      viewChoice.add("Run");
      previousViewIndex = 0;
      viewChoice.addItemListener(this);
      controls.add(viewChoice);
      itemLabel = new JLabel("Maze:");
      controls.add(itemLabel);
      itemChoice = new Choice();
      for (int i = 0; i < mazes.size(); i++)
      {
         if (i == discriminationMazeIndex)
         {
            itemChoice.add(i + "*");
         }
         else
         {
            itemChoice.add(i + "");
         }
      }
      itemChoice.add("all");
      previousRunIndex = -1;
      itemChoice.addItemListener(this);
      controls.add(itemChoice);
      stepLabel = new JLabel("Step:");
      stepLabel.setVisible(false);
      controls.add(stepLabel);
      stepChoice = new Choice();
      stepChoice.setVisible(false);
      stepChoice.addItemListener(this);
      controls.add(stepChoice);
      stepButton = new JButton("Step");
      stepButton.setVisible(false);
      stepButton.addActionListener(this);
      controls.add(stepButton);
      content.add(controls, BorderLayout.NORTH);
      setState();
      pack();
      setVisible(true);
   }


   // Choice listener.
   public void itemStateChanged(ItemEvent e)
   {
      setState();
   }


   // Button listener.
   public void actionPerformed(ActionEvent e)
   {
      int i = stepChoice.getSelectedIndex();

      if ((i >= 0) && (i < stepChoice.getItemCount() - 1))
      {
         stepChoice.select(i + 1);
      }
      setState();
   }


   // Set state.
   private void setState()
   {
      int   i, j, k;
      Tmaze maze;
      Run   run;

      Run.Step step;

      i = viewChoice.getSelectedIndex();
      if (i == 0)
      {
         stepLabel.setVisible(false);
         stepChoice.setVisible(false);
         stepButton.setVisible(false);
         for (j = 0; j < mazes.size(); j++)
         {
            mazes.get(j).clearAnnotations();
         }
         if (i != previousViewIndex)
         {
            itemLabel.setText("Maze:");
            itemChoice.removeAll();
            for (j = 0; j < mazes.size(); j++)
            {
               if (j == discriminationMazeIndex)
               {
                  itemChoice.add(j + "*");
               }
               else
               {
                  itemChoice.add(j + "");
               }
            }
            itemChoice.add("all");
         }
         j = itemChoice.getSelectedIndex();
         if (j == itemChoice.getItemCount() - 1)
         {
            displayMazes = mazes;
         }
         else
         {
            displayMazes = new Vector<Tmaze>();
            displayMazes.add(mazes.get(j));
         }
         mazeDisplay.load(displayMazes);
         displayScroll.setViewportView(mazeDisplay);
      }
      else
      {
         stepLabel.setVisible(true);
         stepChoice.setVisible(true);
         stepButton.setVisible(true);
         for (j = 0; j < mazes.size(); j++)
         {
            mazes.get(j).clearAnnotations();
         }
         if (i != previousViewIndex)
         {
            displayMazes = new Vector<Tmaze>();
            displayMazes.add(mazes.get(discriminationMazeIndex));
            mazeDisplay.load(displayMazes);
            displayScroll.setViewportView(mazeDisplay);
            itemLabel.setText("Run:");
            itemChoice.removeAll();
            for (j = 0; j < runs.size(); j++)
            {
               itemChoice.add(j + "");
            }
            previousRunIndex = -1;
         }
         j = itemChoice.getSelectedIndex();
         if (j >= 0)
         {
            run = runs.get(j);
            if (j != previousRunIndex)
            {
               stepChoice.removeAll();
               for (k = 0; k < run.path.size(); k++)
               {
                  stepChoice.add(run.path.get(k).tick + "");
               }
            }
            previousRunIndex = j;
            j = stepChoice.getSelectedIndex();
            if (j >= 0)
            {
               maze = mazes.get(discriminationMazeIndex);
               step = run.path.get(j);
               maze.path.get(step.tick).highlight = true;
               for (k = 0; k < run.path.size(); k++)
               {
                  step = run.path.get(k);
                  maze.path.get(step.tick).annotations = new Vector<String>();
                  maze.path.get(step.tick).annotations.add(
                     String.format("i: [%f %f]", step.lensLeftOutput, step.lensRightOutput));
               }
               for (k = 0; k <= j; k++)
               {
                  step = run.path.get(k);
                  maze.path.get(step.tick).choice = step.direction;
                  maze.path.get(step.tick).annotations.add(
                     String.format("e: [%f %f]", step.monaLeftOutput, step.monaRightOutput));
               }
            }
         }
      }
      previousViewIndex = i;
      repaint();
   }


   public void print()
   {
      int i, j;

      System.out.println("Parameters:");
      System.out.println("numMazeMarks=" + numMazeMarks);
      System.out.println("numMazes=" + numMazes);
      System.out.println("randomSeed=" + randomSeed);
      System.out.println("generalizationRuns=" + generalizationRuns);
      System.out.println("discriminationRuns=" + discriminationRuns);
      System.out.println("resultRuns=" + resultRuns);
      System.out.println("Mazes:");
      for (i = 0; i < mazes.size(); i++)
      {
         mazes.get(i).print();
      }
      System.out.println("discriminationMazeIndex=" + discriminationMazeIndex);
      System.out.println("Runs:");
      for (i = 0; i < runs.size(); i++)
      {
         runs.get(i).print();
      }
      if (resultRuns > 0)
      {
         System.out.print("Result summary: success/total=" + successfulRuns + "/" + runs.size() +
                          " (" + (float)successfulRuns / (float)resultRuns + ")");
      }
      else
      {
         System.out.print("Result summary: success/total=unknown");
      }
      if (mazes.size() > 0)
      {
         for (i = j = 0; i < mazes.size(); i++)
         {
            j += mazes.get(i).path.size();
         }
         System.out.print("; average maze path length=" + (float)j / (float)mazes.size() +
                          "; discrimination maze path length=" + mazes.get(discriminationMazeIndex).path.size());
      }
      System.out.println();
   }


   public static void main(String[] args)
   {
      MincRunViewer viewer = new MincRunViewer();

      if (args.length != 1)
      {
         System.err.println("Usage: java MincRunViewer <minc world run result file name> | - (stdin)");
         return;
      }
      if (args[0].startsWith("-"))
      {
         try
         {
            viewer.load(new Scanner(System.in));
         }
         catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
         }
      }
      else
      {
         viewer.load(args[0]);
      }
      viewer.run();
   }
}
