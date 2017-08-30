//-----------------------------------------------------------------------
//  Atani neural network controlled robot.
//
//  $File: TmazeWorld.cs $ $Revision: 1 $
//-----------------------------------------------------------------------

using Microsoft.Ccr.Core;
using Microsoft.Dss.Core;
using Microsoft.Dss.Core.Attributes;
using Microsoft.Dss.ServiceModel.Dssp;
using Microsoft.Dss.ServiceModel.DsspServiceBase;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Security.Permissions;
using xml = System.Xml;
using Microsoft.Robotics.Simulation;
using Microsoft.Robotics.Simulation.Engine;
using engineproxy = Microsoft.Robotics.Simulation.Engine.Proxy;
using Microsoft.Robotics.Simulation.Physics;
using Microsoft.Robotics.PhysicalModel;

namespace Robotics.Atani
{
   // Simulated T-maze world.
   public class TmazeWorld
   {
      // Simple colors.
      public enum BasicColor : byte
      {
         Black = 0,
         Red      = 1,
         Lime     = 2,
         Yellow   = 3,
         Blue     = 4,
         Magenta  = 5,
         Cyan     = 6,
         White    = 7,
         DarkGrey = 8,
         Maroon   = 9,
         Green    = 10,
         Olive    = 11,
         Navy     = 12,
         Purple   = 13,
         Cobalt   = 14,
         Grey     = 15
      }

      // Ground texture.
      public string GroundTexture = "../../apps/atani/images/cellfloor.jpg";

      // Block Textures
      //
      // This table of filenames maps the Basic Colors to textures
      // that can be used. A set of files with solid colors are
      // provided.
      //
      // All of these files must be in the apps/atani/images
      // directory under the MSRS installation. Also, there must be
      // 16 files in the list.
      //
      public string[] BlockTextures =
      {
         "../../apps/atani/images/SolidBlack.bmp",
         "../../apps/atani/images/SolidRed.bmp",
         "../../apps/atani/images/SolidLime.bmp",
         "../../apps/atani/images/SolidYellow.bmp",
         "../../apps/atani/images/SolidBlue.bmp",
         "../../apps/atani/images/SolidMagenta.bmp",
         "../../apps/atani/images/SolidCyan.bmp",
         "../../apps/atani/images/SolidWhite.bmp",
         "../../apps/atani/images/SolidDarkGrey.bmp",
         "../../apps/atani/images/SolidMaroon.bmp",
         "../../apps/atani/images/SolidGreen.bmp",
         "../../apps/atani/images/SolidOlive.bmp",
         "../../apps/atani/images/SolidNavy.bmp",
         "../../apps/atani/images/SolidPurple.bmp",
         "../../apps/atani/images/SolidCobalt.bmp",
         "../../apps/atani/images/SolidGrey.bmp"
      };

      // Block Colors
      // These are specified using the Vector3 format for colors
      // The values below correspond to the bmp files above.
      static public readonly Vector3[] BlockColors =
      {
         new Vector3(0,     0,   0),
         new Vector3(255,   0,   0),
         new Vector3(0,   255,   0),
         new Vector3(255, 255,   0),
         new Vector3(0,     0, 255),
         new Vector3(255,   0, 255),
         new Vector3(0,   255, 255),
         new Vector3(255, 255, 255),
         new Vector3(128, 128, 128),
         new Vector3(128,   0,   0),
         new Vector3(0,   128,   0),
         new Vector3(128, 128,   0),
         new Vector3(0,     0, 128),
         new Vector3(128,   0, 128),
         new Vector3(0,   128, 128),
         new Vector3(192, 192, 192)
      };

      // Mapping of Colors to Weights (Mass)
      //
      // For walls, specify a mass of zero. This really
      // means infinite, so the walls cannot be moved.
      public float[] BlockMasses =
      {
         0.0f,
         0.0f,
         0.0f,
         0.0f,
         0.0f,
         0.0f,
         0.0f,
         0.0f,
         0.0f,
         0.0f,
         0.0f,
         0.0f,
         0.0f,
         0.0f,
         0.0f,
         0.0f,
      };

      // Maze Parameters:

      // Horizontal size of the wall block (meters).
      public float BlockSize = 1.0f;

      // Height of wall block (meters).
      public float BlockHeight = 1.0f;

      // Maze dimensions in block units.
      public int MazeWidth;
      public int MazeHeight;

      // Maze path.
      public List<Point> MazePath = new List<Point>();

      // Random numbers.
      Random random;

      // Constructor.
      public TmazeWorld(int width, int height, Random random)
      {
         MazeWidth   = width;
         MazeHeight  = height;
         this.random = random;
      }


      // Create world.
      public void create()
      {
         // Set up the initial camera view.
         setupCamera();

         // Add objects (entities) to the simulated world.
         populateWorld();
      }


      // Set up initial view.
      private void setupCamera()
      {
         CameraView view = new CameraView();

         // Place the camera looking directly down on the maze.
         view.EyePosition =
            new Vector3(MazeWidth * BlockSize * 0.5f,
                        Math.Max(MazeWidth, MazeHeight) * BlockSize * 1.5f,
                        MazeHeight * BlockSize * 0.5f);
         view.LookAtPoint =
            new Vector3(MazeWidth * BlockSize * 0.5f,
                        0.0f,
                        MazeHeight * BlockSize * 0.5f);
         SimulationEngine.GlobalInstancePort.Update(view);
      }


      // Build the world in the simulator.
      private void populateWorld()
      {
         addSky();
         addGround();
         addMaze();
      }


      void addSky()
      {
         // Add a sky using a static texture. We will use the sky texture
         // to do per pixel lighting on each simulation visual entity
         SkyEntity sky = new SkyEntity("sky.dds", "sky_diff.dds");

         SimulationEngine.GlobalInstancePort.Insert(sky);
      }


      void addGround()
      {
         HeightFieldShapeProperties hf = new HeightFieldShapeProperties("height field",
                                                                        64,     // number of rows
                                                                        100,    // distance in meters, between rows
                                                                        64,     // number of columns
                                                                        100,    // distance in meters, between columns
                                                                        1,      // scale factor to multiple height values
                                                                        -1000); // vertical extent of the height field. Should be set to large negative values

         // Create array with height samples.
         hf.HeightSamples = new HeightFieldSample[hf.RowCount * hf.ColumnCount];
         for (int i = 0; i < hf.RowCount * hf.ColumnCount; i++)
         {
            hf.HeightSamples[i]        = new HeightFieldSample();
            hf.HeightSamples[i].Height = (short)(Math.Sin(i * 0.01));
         }

         // Create a material for the entire field. We could also specify material per sample.
         hf.Material = new MaterialProperties("ground", 0.8f, 0.5f, 0.8f);

         // Insert ground entity in simulation and specify a texture.
         SimulationEngine.GlobalInstancePort.Insert(new HeightFieldEntity(hf, GroundTexture));
      }


      void addMaze()
      {
         int x, y;

         int[, ] colorIndexes = new int[MazeWidth, MazeHeight];

         // Generate the maze path.
         generateMazePath();

         // Generate the maze world of colored blocks.
         // Reserve black for the goal color.
         for (x = 0; x < MazeWidth; x++)
         {
            for (y = 0; y < MazeHeight; y++)
            {
               colorIndexes[x, y] = random.Next(15) + 1;
            }
         }

         // Create path in the maze world.
         for (int i = 0; i < MazePath.Count; i++)
         {
            colorIndexes[MazePath[i].X, MazePath[i].Y] = -1;
         }

         // Build the maze walls.
         for (y = 0; y < MazeHeight; y++)
         {
            for (x = 0; x < MazeWidth; x++)
            {
               if (colorIndexes[x, y] >= 0)
               {
                  addBlock(x, y, (BasicColor)colorIndexes[x, y]);
               }
            }
         }

         // Create the black goal block facade.
         if (MazePath.Count > 1)
         {
            Vector3 dimensions, position;
            x = MazePath[MazePath.Count - 1].X;
            y = MazePath[MazePath.Count - 1].Y;
            if (MazePath[MazePath.Count - 2].X < x)
            {
               dimensions = new Vector3(BlockSize * 0.1f, BlockHeight, BlockSize);
               position   = new Vector3((x * BlockSize) + (BlockSize * 0.4f),
                                        BlockHeight / 2.0f, y * BlockSize);
            }
            else if (MazePath[MazePath.Count - 2].X > x)
            {
               dimensions = new Vector3(BlockSize * 0.1f, BlockHeight, BlockSize);
               position   = new Vector3((x * BlockSize) - (BlockSize * 0.4f),
                                        BlockHeight / 2.0f, y * BlockSize);
            }
            else if (MazePath[MazePath.Count - 2].Y < y)
            {
               dimensions = new Vector3(BlockSize, BlockHeight, BlockSize * 0.1f);
               position   = new Vector3(x * BlockSize, BlockHeight / 2.0f,
                                        (y * BlockSize) + (BlockSize * 0.4f));
            }
            else
            {
               dimensions = new Vector3(BlockSize, BlockHeight, BlockSize * 0.1f);
               position   = new Vector3(x * BlockSize, BlockHeight / 2.0f,
                                        (y * BlockSize) - (BlockSize * 0.4f));
            }
            addBlock(dimensions, position, BasicColor.Black, "Goal face");
         }
      }


      // Adds a block at a specified location in the maze grid.
      void addBlock(int row, int col, BasicColor color)
      {
         // Units are in meters.
         Vector3 dimensions =
            new Vector3(BlockSize, BlockHeight, BlockSize);

         Vector3 position =
            new Vector3(row * BlockSize, BlockHeight / 2.0f, col * BlockSize);

         addBlock(dimensions, position, color, "block_" + row + "_" + col);
      }


      // Adds a block.
      void addBlock(Vector3 dimensions, Vector3 position, BasicColor color, string name)
      {
         BoxShapeProperties cBoxShape = new BoxShapeProperties(
            BlockMasses[(byte)color],         // mass in kilograms.
            new Pose(),                       // relative pose
            dimensions);                      // dimensions

         cBoxShape.Material = new MaterialProperties("gbox", 1.0f, 0.4f, 0.5f);

         // Set the color of the box according to the bitmap image
         // or the specified color if no bitmap.
         if (BlockTextures[(byte)color] == "")
         {
            cBoxShape.DiffuseColor.X = (float)(BlockColors[(byte)color].X / 255.0f);
            cBoxShape.DiffuseColor.Y = (float)(BlockColors[(byte)color].Y / 255.0f);
            cBoxShape.DiffuseColor.Z = (float)(BlockColors[(byte)color].Z / 255.0f);
            cBoxShape.DiffuseColor.W = 0.5f;
         }
         else
         {
            cBoxShape.TextureFileName = BlockTextures[(byte)color];
         }

         SingleShapeEntity box = new SingleShapeEntity(new BoxShape(cBoxShape), position);

         box.State.Name = name;

         // Insert entity in simulation.
         SimulationEngine.GlobalInstancePort.Insert(box);
      }


      /*
       * A T-maze contains a path of adjacent T-shaped cell patterns.
       * TBE  or  EBT
       *  C        C
       *  S        S
       * where S = start cell, C = channel cell, B = branch cell,
       * T = terminal cell, E = end cell.
       * The end cell of one pattern abuts the start cell of the next
       * pattern such that the TBE cells of the first pattern are colinear with
       * the SCB cells of the second pattern. The cells comprising a pattern have
       * a block height of one, and are surrounded by cells of height two except
       * where connected to another pattern.
       */

      // Directions.
      public enum DIRECTION
      {
         NORTH        = 0,
         EAST         = 1,
         SOUTH        = 2,
         WEST         = 3,
         NO_DIRECTION = 4
      };

      // Tries for space-filling maze.
      int MAZE_GENERATIONS = 10;

      // Generate T-maze path.
      void generateMazePath()
      {
         int i, xmin, xmax, ymin, ymax, xd, yd;

         List<Point> path = new List<Point>();
         Point       s    = new Point(0, 0);

         // Sanity check.
         if ((MazeWidth < 5) || (MazeHeight < 5)) { return; }

         // Create a long random maze path.
         MazePath.Clear();
         for (i = 0; i < MAZE_GENERATIONS; i++)
         {
            path.Clear();
            path.Add(s);
            mapMaze(path, DIRECTION.SOUTH);
            if (path.Count > MazePath.Count)
            {
               MazePath.Clear();
               MazePath = path;
            }
         }

         // Center path in maze.
         xmin = ymin = 0;
         xmax = MazeWidth;
         ymax = MazeHeight;
         getBounds(MazePath, ref xmin, ref ymin, ref xmax, ref ymax);
         xd = 1 - xmin;
         yd = 1 - ymin;
         for (i = 0; i < MazePath.Count; i++)
         {
            MazePath[i] = new Point(MazePath[i].X + xd, MazePath[i].Y + yd);
         }
      }


      // Map a maze.
      bool mapMaze(List<Point> path, DIRECTION direction)
      {
         int       s, c, b, t, e, x, y, x1, y1, x2, y2, xc, yc, xb, yb;
         DIRECTION nextdir;
         Point     cell = new Point(0, 0);

         // Check cell placements.
         s  = path.Count - 1;
         c  = s + 1;
         b  = c + 1;
         t  = b + 1;
         e  = t + 1;
         x  = path[s].X;
         y  = path[s].Y;
         x1 = y1 = x2 = y2 = 0;
         xb = yb = xc = yc = 0;
         switch (direction)
         {
         case DIRECTION.NORTH:
            x1 = x - 1;
            y1 = y + 2;
            if (!checkCell(path, x1, y1)) { return(false); }
            xc = x;
            yc = y + 1;
            if (!checkCell(path, xc, yc)) { return(false); }
            xb = x;
            yb = y + 2;
            if (!checkCell(path, xb, yb)) { return(false); }
            x2 = x + 1;
            y2 = y + 2;
            if (!checkCell(path, x2, y2)) { return(false); }
            break;

         case DIRECTION.EAST:
            x1 = x + 2;
            y1 = y + 1;
            if (!checkCell(path, x1, y1)) { return(false); }
            xc = x + 1;
            yc = y;
            if (!checkCell(path, xc, yc)) { return(false); }
            xb = x + 2;
            yb = y;
            if (!checkCell(path, xb, yb)) { return(false); }
            x2 = x + 2;
            y2 = y - 1;
            if (!checkCell(path, x2, y2)) { return(false); }
            break;

         case DIRECTION.SOUTH:
            x1 = x - 1;
            y1 = y - 2;
            if (!checkCell(path, x1, y1)) { return(false); }
            xc = x;
            yc = y - 1;
            if (!checkCell(path, xc, yc)) { return(false); }
            xb = x;
            yb = y - 2;
            if (!checkCell(path, xb, yb)) { return(false); }
            x2 = x + 1;
            y2 = y - 2;
            if (!checkCell(path, x2, y2)) { return(false); }
            break;

         case DIRECTION.WEST:
            x1 = x - 2;
            y1 = y + 1;
            if (!checkCell(path, x1, y1)) { return(false); }
            xc = x - 1;
            yc = y;
            if (!checkCell(path, xc, yc)) { return(false); }
            xb = x - 2;
            yb = y;
            if (!checkCell(path, xb, yb)) { return(false); }
            x2 = x - 2;
            y2 = y - 1;
            if (!checkCell(path, x2, y2)) { return(false); }
            break;
         }
         cell = new Point(xc, yc);
         path.Add(cell);
         cell = new Point(xb, yb);
         path.Add(cell);
         cell = new Point(0, 0);
         path.Add(cell);
         path.Add(cell);

         // Choose next direction.
         nextdir = DIRECTION.NORTH;
         switch (direction)
         {
         case DIRECTION.NORTH:
         case DIRECTION.SOUTH:
            if ((random.Next() % 2) == 0)
            {
               nextdir = DIRECTION.EAST;
               path[t] = new Point(x1, y1);
               path[e] = new Point(x2, y2);
            }
            else
            {
               nextdir = DIRECTION.WEST;
               path[t] = new Point(x2, y2);
               path[e] = new Point(x1, y1);
            }
            break;

         case DIRECTION.EAST:
         case DIRECTION.WEST:
            if ((random.Next() % 2) == 0)
            {
               nextdir = DIRECTION.SOUTH;
               path[t] = new Point(x1, y1);
               path[e] = new Point(x2, y2);
            }
            else
            {
               nextdir = DIRECTION.NORTH;
               path[t] = new Point(x2, y2);
               path[e] = new Point(x1, y1);
            }
            break;
         }
         if (mapMaze(path, nextdir)) { return(true); }

         // Try reverse direction.
         switch (nextdir)
         {
         case DIRECTION.NORTH:
            nextdir = DIRECTION.SOUTH;
            break;

         case DIRECTION.EAST:
            nextdir = DIRECTION.WEST;
            break;

         case DIRECTION.SOUTH:
            nextdir = DIRECTION.NORTH;
            break;

         case DIRECTION.WEST:
            nextdir = DIRECTION.EAST;
            break;
         }
         cell    = path[e];
         path[e] = path[t];
         path[t] = cell;
         mapMaze(path, nextdir);
         return(true);
      }


      // Is cell placement OK?
      bool checkCell(List<Point> path, int x, int y)
      {
         int i, j, x2, y2, xmin, xmax, ymin, ymax;

         // Check maze bounds.
         xmin = ymin = 0;
         xmax = MazeWidth;
         ymax = MazeHeight;
         getBounds(path, ref xmin, ref ymin, ref xmax, ref ymax);
         if ((x < xmin) && (MazeWidth - (xmax - x) < 3)) { return(false); }
         if ((x > xmax) && (MazeWidth - (x - xmin) < 3)) { return(false); }
         if ((y < ymin) && (MazeHeight - (ymax - y) < 3)) { return(false); }
         if ((y > ymax) && (MazeHeight - (y - ymin) < 3)) { return(false); }

         // Check adjacent cells.
         x2 = x - 1;
         y2 = y;
         for (i = 0, j = path.Count - 1; i < j; i++)
         {
            if ((path[i].X == x2) && (path[i].Y == y2)) { return(false); }
         }
         x2 = x + 1;
         y2 = y;
         for (i = 0, j = path.Count - 1; i < j; i++)
         {
            if ((path[i].X == x2) && (path[i].Y == y2)) { return(false); }
         }
         x2 = x;
         y2 = y - 1;
         for (i = 0, j = path.Count - 1; i < j; i++)
         {
            if ((path[i].X == x2) && (path[i].Y == y2)) { return(false); }
         }
         x2 = x;
         y2 = y + 1;
         for (i = 0, j = path.Count - 1; i < j; i++)
         {
            if ((path[i].X == x2) && (path[i].Y == y2)) { return(false); }
         }
         return(true);
      }


      // Get path bounds.
      void getBounds(List<Point> path,
                     ref int xmin, ref int ymin, ref int xmax, ref int ymax)
      {
         for (int i = 0; i < path.Count; i++)
         {
            if (i == 0)
            {
               xmin = xmax = path[i].X;
               ymin = ymax = path[i].Y;
            }
            else
            {
               if (xmin > path[i].X) { xmin = path[i].X; }
               if (xmax < path[i].X) { xmax = path[i].X; }
               if (ymin > path[i].Y) { ymin = path[i].Y; }
               if (ymax < path[i].Y) { ymax = path[i].Y; }
            }
         }
      }
   }

   // T-maze navigation guide.
   public class TmazeGuide
   {
      public enum MOVEMENT_TYPE
      {
         NOOP    = 0,
         FORWARD = 1,
         RIGHT   = 2,
         LEFT    = 3
      }

                           List<TmazeWorld.DIRECTION> _breadCrumbs;
      List<Point>          _path;
      Point                _position;
      TmazeWorld.DIRECTION _direction;

      public TmazeGuide(ref List<Point> path, Point position,
                        TmazeWorld.DIRECTION direction)
      {
         _breadCrumbs = new List<TmazeWorld.DIRECTION>();
         _path        = new List<Point>();
         _position    = position;
         _direction   = direction;

         // Create direction bread crumbs to end of maze.
         int i, j;
         for (i = 0; i < path.Count; i++)
         {
            _path.Add(path[i]);
            if (i == path.Count - 1)
            {
               _breadCrumbs.Add(TmazeWorld.DIRECTION.NO_DIRECTION);
            }
            else
            {
               // Index next cell.
               // Branch cell leads to end cell.
               j = i + 1;
               if ((i == 2) || ((i > 2) && (((i - 2) % 4) == 0))) { j++; }

               if (path[i].X < path[j].X)
               {
                  _breadCrumbs.Add(TmazeWorld.DIRECTION.EAST);
               }
               else if (path[i].X > path[j].X)
               {
                  _breadCrumbs.Add(TmazeWorld.DIRECTION.WEST);
               }
               else if (path[i].Y < path[j].Y)
               {
                  _breadCrumbs.Add(TmazeWorld.DIRECTION.SOUTH);
               }
               else
               {
                  _breadCrumbs.Add(TmazeWorld.DIRECTION.NORTH);
               }
            }
         }
      }


      // Reset guide.
      public void reset(Point position, TmazeWorld.DIRECTION direction)
      {
         _position  = position;
         _direction = direction;
      }


      // Update position and direction.
      public void forward()
      {
         switch (_direction)
         {
         case TmazeWorld.DIRECTION.NORTH:
            _position.Y--;
            break;

         case TmazeWorld.DIRECTION.EAST:
            _position.X++;
            break;

         case TmazeWorld.DIRECTION.SOUTH:
            _position.Y++;
            break;

         case TmazeWorld.DIRECTION.WEST:
            _position.X--;
            break;
         }
      }


      public void right()
      {
         _direction = (TmazeWorld.DIRECTION)(((int)_direction + 1) % 4);
      }


      public void left()
      {
         int d = (int)_direction - 1;

         if (d < 0) { d = 3; }
         _direction = (TmazeWorld.DIRECTION)d;
      }


      // Get movement toward end of maze.
      public MOVEMENT_TYPE getMovement()
      {
         int i;

         for (i = 0; i < _path.Count; i++)
         {
            if (_position == _path[i]) { break; }
         }
         switch (_direction)
         {
         case TmazeWorld.DIRECTION.NORTH:
            switch (_breadCrumbs[i])
            {
            case TmazeWorld.DIRECTION.NORTH:
               return(MOVEMENT_TYPE.FORWARD);

            case TmazeWorld.DIRECTION.EAST:
               return(MOVEMENT_TYPE.RIGHT);

            case TmazeWorld.DIRECTION.SOUTH:
               return(MOVEMENT_TYPE.RIGHT);

            case TmazeWorld.DIRECTION.WEST:
               return(MOVEMENT_TYPE.LEFT);

            case TmazeWorld.DIRECTION.NO_DIRECTION:
               return(MOVEMENT_TYPE.NOOP);
            }
            break;

         case TmazeWorld.DIRECTION.EAST:
            switch (_breadCrumbs[i])
            {
            case TmazeWorld.DIRECTION.NORTH:
               return(MOVEMENT_TYPE.LEFT);

            case TmazeWorld.DIRECTION.EAST:
               return(MOVEMENT_TYPE.FORWARD);

            case TmazeWorld.DIRECTION.SOUTH:
               return(MOVEMENT_TYPE.RIGHT);

            case TmazeWorld.DIRECTION.WEST:
               return(MOVEMENT_TYPE.LEFT);

            case TmazeWorld.DIRECTION.NO_DIRECTION:
               return(MOVEMENT_TYPE.NOOP);
            }
            break;

         case TmazeWorld.DIRECTION.SOUTH:
            switch (_breadCrumbs[i])
            {
            case TmazeWorld.DIRECTION.NORTH:
               return(MOVEMENT_TYPE.RIGHT);

            case TmazeWorld.DIRECTION.EAST:
               return(MOVEMENT_TYPE.LEFT);

            case TmazeWorld.DIRECTION.SOUTH:
               return(MOVEMENT_TYPE.FORWARD);

            case TmazeWorld.DIRECTION.WEST:
               return(MOVEMENT_TYPE.RIGHT);

            case TmazeWorld.DIRECTION.NO_DIRECTION:
               return(MOVEMENT_TYPE.NOOP);
            }
            break;

         case TmazeWorld.DIRECTION.WEST:
            switch (_breadCrumbs[i])
            {
            case TmazeWorld.DIRECTION.NORTH:
               return(MOVEMENT_TYPE.RIGHT);

            case TmazeWorld.DIRECTION.EAST:
               return(MOVEMENT_TYPE.LEFT);

            case TmazeWorld.DIRECTION.SOUTH:
               return(MOVEMENT_TYPE.LEFT);

            case TmazeWorld.DIRECTION.WEST:
               return(MOVEMENT_TYPE.FORWARD);

            case TmazeWorld.DIRECTION.NO_DIRECTION:
               return(MOVEMENT_TYPE.NOOP);
            }
            break;
         }
         return(MOVEMENT_TYPE.NOOP);
      }
   }
}
