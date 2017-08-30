//-----------------------------------------------------------------------
//  Atani neural network controlled robot.
//
//  $File: blockWorld.cs $ $Revision: 1 $
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
   // Simulated world.
   // Largely cannibalized from Trevor Taylor's MazeSimulator.
   public class BlockWorld
   {
      // Pixels less than this value will be counted as walls.
      static int WallCellColorThresh = -10000000;

      // Can significantly reduce number of entities
      // and thus increase frames per second.
      static bool OptimizeBlocks = true;

      // To count blocks after optimization.
      int BlockCounter = 0;

      // Added for better use of bitmaps.
      static bool UseBackgroundColor = true;
      static bool CenterMaze         = true;

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

      // The maze layout.
      public string Maze = "apps/atani/images/ModelLarge.bmp";

      // Ground texture.
      public string GroundTexture = "../../apps/atani/images/cellfloor.jpg";

      // Wall Textures
      //
      // This table of filenames maps the Basic Colors to textures
      // that can be used. A set of files with solid colors are
      // provided.
      //
      // All of these files must be in the apps/atani/images
      // directory under the MSRS installation. Also, there must be
      // 16 files in the list.
      //
      public string[] WallTextures =
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

      // Wall Colors
      // These are specified using the Vector3 format for colors
      // The values below correspond to the bmp files above.
      static public readonly Vector3[] DefaultColors =
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

      public Vector3[] WallColors;

      // Mapping of Colors to Heights
      //
      // There should be an element in the array here for
      // every value in the BasicColor enum.
      // Also, the background color will be ignored
      // regardless of its height.
      // Also, there is no reason why a height has to be big.
      // For example, 0.000001f is not zero, so it will still be
      // used to generate boxes. With an appropriate texture, it
      // could be a pool of water (rectangular though), or some
      // other material on the floor.
      //
      public float[] WallHeights =
      {
         3.0f,
         5.0f,
         10.0f,
         8.0f,
         10.0f,
         15.0f,
         15.0f,
         20.0f,
         0.0f,
         5.0f,
         5.0f,
         5.0f,
         5.0f,
         5.0f,
         5.0f,
         5.0f,
      };

      // Mapping of Colors to Weights (Mass)
      //
      // For walls, specify a mass of zero. This really
      // means infinite, so the walls cannot be moved.
      public float[] WallMasses =
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

      public bool[] UseSphere =
      {
         false,
         false,
         false,
         false,
         false,
         false,
         false,
         false,
         false,
         false,
         false,
         false,
         false,
         false,
         false,
         false
      };

      // Maze Parameters:

      // Size of the wall cubes (meters).
      public float WallBoxSize = 0.98f;

      // Grid size for bitmaps, i.e. one pixel = GridSpacing
      public float GridSpacing = 0.1f;

      // Scale factor applied to heights.
      public float HeightScale = 0.1f;

      public float DefaultHeight = 10.0f;

      // Scale factor applied to spheres.
      public float SphereScale = 0.1f;

      // Constructor.
      public BlockWorld()
      {
         // Initialize the wall colors
         WallColors = new Vector3[16];
         for (int i = 0; i < 16; i++)
         {
            WallColors[i] = new Vector3(DefaultColors[i].X,
                                        DefaultColors[i].Y,
                                        DefaultColors[i].Z);
         }
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

         // These values angle the camera down at 45 degrees
         // looking along the Z axis.
         view.EyePosition = new Vector3(0.0f, 7.0f, -7.0f);
         view.LookAtPoint = new Vector3(0.0f, 5.0f, -5.0f);
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


      int xOffset, yOffset;

      void addMaze()
      {
         float[, ] maze = ParseImage(Maze);
         int height = maze.GetLength(1);
         int width  = maze.GetLength(0);
         int[, ] counters = new int[width, height];
         int        tempX1, tempY1;
         int        tempX2, tempY2;
         BasicColor PixelColor;

         if (CenterMaze)
         {
            xOffset = (int)((-width / 2.0) + 0.5);
            yOffset = (int)((-height / 2.0) + 0.5);
         }
         else
         {
            xOffset = 0;
            yOffset = 0;
         }

         if (OptimizeBlocks)
         {
            int   count;
            int   thisRowCount;
            float currentHeight;

            // Merge horizontal blocks.
            for (int y = 0; y < height; y++)
            {
               int x = 0;
               while (x < width - 1)
               {
                  // At least 2 blocks to merge.
                  if ((maze[x, y] > 0.0f) && (maze[x + 1, y] > 0.0f))
                  {
                     int startX = x;

                     // Only merge wall segments of the same height.
                     count         = 0;
                     currentHeight = maze[x, y];
                     while (x<width && maze[x, y]> 0.0f &&
                            maze[x, y] == currentHeight)
                     {
                        maze[x, y]     = 0.0f;
                        counters[x, y] = 0;
                        x++;
                        count++;
                     }

                     // Just mark the map, do not draw here.
                     counters[startX, y] = count;
                     maze[startX, y]     = currentHeight;
                  }
                  else
                  {
                     if (maze[x, y] > 0.0f)
                     {
                        counters[x, y] = 1;
                     }
                     else
                     {
                        counters[x, y] = 0;
                     }
                     x++;
                  }
               }
               if (x < height)
               {
                  if (maze[x, y] > 0.0f)
                  {
                     counters[x, y] = 1;
                  }
                  else
                  {
                     counters[x, y] = 0;
                  }
               }
            }

            // Merge remaining vertical blocks.
            for (int x = 0; x < width; x++)
            {
               int y = 0;
               while (y < height - 1)
               {
                  // At least 2 blocks to merge.
                  // Must have the same row count and height.
                  if ((counters[x, y] > 0) && (counters[x, y + 1] == counters[x, y]) &&
                      (maze[x, y] == maze[x, y + 1]))
                  {
                     int startY = y;
                     count        = 0;
                     thisRowCount = counters[x, y];

                     // Only merge wall segments of the same height.
                     currentHeight = maze[x, y];
                     while (y < height && counters[x, y] == thisRowCount &&
                            maze[x, y] == currentHeight)
                     {
                        maze[x, y]     = 0.0f;
                        counters[x, y] = 0;
                        y++;
                        count++;
                     }

                     // Add offset.
                     tempY1     = startY + yOffset;
                     tempX1     = x + xOffset;
                     tempY2     = startY + count - 1 + yOffset;
                     tempX2     = x + thisRowCount - 1 + xOffset;
                     PixelColor = ParseColor(img.GetPixel(x, startY));
                     addWall(tempY1, tempX1, tempY2, tempX2, currentHeight, PixelColor);
                  }
                  else
                  {
                     if (counters[x, y] > 0)
                     {
                        tempY1     = y + yOffset;
                        tempX1     = x + xOffset;
                        tempY2     = y + yOffset;
                        tempX2     = x + counters[x, y] - 1 + xOffset;
                        PixelColor = ParseColor(img.GetPixel(x, y));
                        addWall(tempY1, tempX1, tempY2, tempX2, maze[x, y], PixelColor);
                        maze[x, y]     = 0.0f;
                        counters[x, y] = 0;
                     }

                     y++;
                  }
               }
               // Boundary condition.
               if (y < height)
               {
                  if (counters[x, y] > 0)
                  {
                     tempY1     = y + yOffset;
                     tempX1     = x + xOffset;
                     tempY2     = y + yOffset;
                     tempX2     = x + counters[x, y] - 1 + xOffset;
                     PixelColor = ParseColor(img.GetPixel(x, y));
                     addWall(tempY1, tempX1, tempY2, tempX2, maze[x, y], PixelColor);
                     maze[x, y]     = 0.0f;
                     counters[x, y] = 0;
                  }
               }
            }
         }

         // Draw all blocks left over.
         for (int y = 0; y < height; y++)
         {
            for (int x = 0; x < width; x++)
            {
               if (maze[x, y] > 0.0f)
               {
                  // TT -- Add offset
                  tempY1     = y + yOffset;
                  tempX1     = x + xOffset;
                  PixelColor = ParseColor(img.GetPixel(x, y));
                  addWall(tempY1, tempX1, maze[x, y], PixelColor);
                  // This is only for debugging
                  // All blocks should be zero at the end ...
                  maze[x, y] = 0.0f;
               }
            }
         }
      }


      // Simple fuzzy color parsing into the 16 basic colors.
      const int    BlackThreshold = 16;
      const int    WhiteThreshold = 208;
      const int    ColorThreshold = 128;
      const double GreyTolerance  = 0.10;

      BasicColor ParseColor(Color pixel)
      {
         return(ParseColor(pixel.R, pixel.G, pixel.B));
      }


      BasicColor ParseColor(int r, int g, int b)
      {
         int rgbSum = r + g + b;

         // Sort out black and white.
         // The assumption here is that sufficiently
         // intense or dark colors are just white or black.
         if (rgbSum < BlackThreshold * 3)
         {
            return(BasicColor.Black);
         }
         else if (rgbSum > WhiteThreshold * 3)
         {
            return(BasicColor.White);
         }
         else
         {
            // Next check for grey.
            // This compares the range of values to see if they
            // are basically all equal, i.e. no dominant color = grey
            float normMax, normMin, normRange;
            int   valMax, valMin;
            valMax    = Math.Max(r, g);
            valMax    = Math.Max(valMax, b);
            valMin    = Math.Min(r, g);
            valMin    = Math.Min(valMin, b);
            normMax   = (float)valMax;
            normMin   = (float)valMin;
            normMax  /= rgbSum;
            normMin  /= rgbSum;
            normRange = normMax - normMin;
            if (normRange < GreyTolerance)
            {
               if (normMax >= 160)
               {
                  return(BasicColor.Grey);
               }
               else
               {
                  return(BasicColor.DarkGrey);
               }
            }

            // Now we have a more complicated task
            // but it is made easier by the definition
            // of BasicColor.
            byte color = 0;

            // Check for dark versions of the colors.
            if (valMax < 160)
            {
               color += 8;
               if (r >= ColorThreshold / 2)
               {
                  color += 1;
               }
               if (g >= ColorThreshold / 2)
               {
                  color += 2;
               }
               if (b >= ColorThreshold / 2)
               {
                  color += 4;
               }
            }
            else
            {
               // Now check the thresholds for normal colors.
               if (r >= ColorThreshold)
               {
                  color += 1;
               }
               if (g >= ColorThreshold)
               {
                  color += 2;
               }
               if (b >= ColorThreshold)
               {
                  color += 4;
               }
            }
            return((BasicColor)color);
         }
      }


      // Parse the image bitmap into a maze.
      //
      // Modified to allow a background color to use for
      // the bitmap and also to use the color as an
      // index for the height.
      static Bitmap img;
      float[, ] ParseImage(string file)
      {
         img = (Bitmap)Image.FromFile(file);
         float[, ] imgArray = new float[img.Width, img.Height];
         int WallCount = 0;

         Color      BackgroundColor = Color.White;
         Color      PixelColor;
         BasicColor PixelBasicColor;

         if (UseBackgroundColor)
         {
            BackgroundColor = img.GetPixel(0, 0);
         }

         for (int y = 0; y < img.Height; y++)
         {
            for (int x = 0; x < img.Width; x++)
            {
               if (UseBackgroundColor)
               {
                  // Get a basic pixel color.
                  PixelColor      = img.GetPixel(x, y);
                  PixelBasicColor = ParseColor(PixelColor);
                  if ((PixelColor.R != BackgroundColor.R) ||
                      (PixelColor.G != BackgroundColor.G) ||
                      (PixelColor.B != BackgroundColor.B))
                  {
                     // Return the height at this pixel location.
                     imgArray[x, y] = WallHeights[(byte)PixelBasicColor];
                     WallCount++;
                  }
                  else
                  {
                     imgArray[x, y] = 0.0f;
                  }
               }
               else
               {
                  if (img.GetPixel(x, y).ToArgb() < WallCellColorThresh)
                  {
                     imgArray[x, y] = DefaultHeight;
                     WallCount++;
                  }
                  else
                  {
                     imgArray[x, y] = 0.0f;
                  }
               }
            }
         }
         return(imgArray);
      }

      // Adds a simple cube at a specified location in the maze grid.
      void addWall(int row, int col, float height, BasicColor color)
      {
         if (UseSphere[(byte)color])
         {
            SphereShapeProperties cSphereShape = null;
            SingleShapeEntity     sphere       = null;
            float                 radius;

            radius = SphereScale * height / 2.0f;

            // Create simple entity with a single shape.
            cSphereShape = new SphereShapeProperties(
               WallMasses[(byte)color],          // mass in kilograms.
               new Pose(),                       // relative pose
               radius);                          // radius
            cSphereShape.Material = new MaterialProperties("gsphere", 1.0f, 0.01f, 0.01f);

            // Set the color of the sphere according to the bitmap image
            // or the specified color if no bitmap.
            if (WallTextures[(byte)color] == "")
            {
               cSphereShape.DiffuseColor.X = (float)(WallColors[(byte)color].X / 255.0);
               cSphereShape.DiffuseColor.Y = (float)(WallColors[(byte)color].Y / 255.0);
               cSphereShape.DiffuseColor.Z = (float)(WallColors[(byte)color].Z / 255.0);
               cSphereShape.DiffuseColor.W = 1.0f;
            }
            else
            {
               cSphereShape.TextureFileName = WallTextures[(byte)color];
            }

            sphere = new SingleShapeEntity(new SphereShape(cSphereShape),
                                           new Vector3((col * -GridSpacing),
                                                       radius,
                                                       -(row * GridSpacing)));

            // Name the entity. All entities must have unique names
            sphere.State.Name = "ball_" + row + "_" + col;

            // Insert entity in simulation.
            SimulationEngine.GlobalInstancePort.Insert(sphere);
         }
         else
         {
            // Dimensions are in meters
            Vector3 dimensions =
               new Vector3(WallBoxSize * GridSpacing,
                           height * HeightScale,
                           WallBoxSize * GridSpacing);
            BoxShapeProperties cBoxShape = null;
            SingleShapeEntity  box       = null;

            // Create simple immovable entity with a single shape.
            cBoxShape = new BoxShapeProperties(
               WallMasses[(byte)color],          // mass in kilograms.
               new Pose(),                       // relative pose
               dimensions);                      // dimensions
            cBoxShape.Material = new MaterialProperties("gbox", 1.0f, 0.4f, 0.5f);

            // Set the color of the box according to the bitmap image
            // or the specified color if no bitmap.
            if (WallTextures[(byte)color] == "")
            {
               cBoxShape.DiffuseColor.X = (float)(WallColors[(byte)color].X / 255.0);
               cBoxShape.DiffuseColor.Y = (float)(WallColors[(byte)color].Y / 255.0);
               cBoxShape.DiffuseColor.Z = (float)(WallColors[(byte)color].Z / 255.0);
               cBoxShape.DiffuseColor.W = 0.5f;
            }
            else
            {
               cBoxShape.TextureFileName = WallTextures[(byte)color];
            }

            box = new SingleShapeEntity(new BoxShape(cBoxShape),
                                        new Vector3(col * -GridSpacing,
                                                    height * HeightScale / 2,
                                                    -(row * GridSpacing)));

            // Name the entity. All entities must have unique names
            box.State.Name = "wall_" + row + "_" + col;

            // Insert entity in simulation.
            SimulationEngine.GlobalInstancePort.Insert(box);
         }

         BlockCounter++;
      }


      // Adds a long wall in the maze grid.
      // Useful for reducing number of elements in simulation for better performance.
      // The existing code used height to refer to the depth of the wall.
      // Therefore the real height is called boxSize.
      void addWall(int startRow, int startCol, int endRow, int endCol, float boxSize, BasicColor color)
      {
         int width  = Math.Abs(endCol - startCol) + 1;
         int height = Math.Abs(endRow - startRow) + 1;

         float realWidth  = (width * GridSpacing) - (GridSpacing - WallBoxSize * GridSpacing);
         float realHeight = (height * GridSpacing) - (GridSpacing - WallBoxSize * GridSpacing);

         //because the box is placed relative to the center of mass
         float widthOffset  = (Math.Abs(endCol - startCol) * GridSpacing) / 2;
         float heightOffset = -(Math.Abs(endRow - startRow) * GridSpacing) / 2;

         if (UseSphere[(byte)color])
         {
            // This object is a Sphere.
            SphereShapeProperties cSphereShape = null;
            SingleShapeEntity     sphere       = null;
            float                 radius;

            radius = (float)(SphereScale * Math.Sqrt(realWidth * realWidth + realHeight * realHeight) / 2.0f);

            // Create simple entity, with a single shape.
            cSphereShape = new SphereShapeProperties(
               WallMasses[(byte)color],      // mass in kilograms.
               new Pose(),                   // relative pose
               radius);                      // radius
            cSphereShape.Material          = new MaterialProperties("gsphere", 0.9f, 0.05f, 0.1f);
            cSphereShape.Material.Advanced = new MaterialAdvancedProperties();
            cSphereShape.Material.Advanced.RestitutionCombineMode = CoefficientsCombineMode.Max;
            cSphereShape.Material.Advanced.FrictionCombineMode    = CoefficientsCombineMode.Min;
            cSphereShape.Material.Advanced.Spring = new SpringProperties();
            cSphereShape.Material.Advanced.Spring.SpringCoefficient = 0.9f;
            cSphereShape.Material.Advanced.Spring.DamperCoefficient = 0.1f;
            cSphereShape.Advanced = new ShapeAdvancedProperties();
            cSphereShape.Advanced.PhysicsCalculationPasses = 20.0f;

            // Set the color of the sphere according to the bitmap image
            // or the specified color if no bitmap.
            if (WallTextures[(byte)color] == "")
            {
               cSphereShape.DiffuseColor.X = (float)(WallColors[(byte)color].X / 255.0);
               cSphereShape.DiffuseColor.Y = (float)(WallColors[(byte)color].Y / 255.0);
               cSphereShape.DiffuseColor.Z = (float)(WallColors[(byte)color].Z / 255.0);
               cSphereShape.DiffuseColor.W = 1.0f;
            }
            else
            {
               cSphereShape.TextureFileName = WallTextures[(byte)color];
            }

            sphere = new SingleShapeEntity(new SphereShape(cSphereShape),
                                           new Vector3((startCol * -GridSpacing) - widthOffset,
                                                       radius * 2,
                                                       -(startRow * GridSpacing) + heightOffset)
                                           );

            // Name the entity. All entities must have unique names.
            sphere.State.Name = "ball_" + startRow + "_" + startCol;

            // Insert entity in simulation.
            SimulationEngine.GlobalInstancePort.Insert(sphere);
         }
         else
         {
            // This object is a wall (stretched cube).
            Vector3 dimensions =
               new Vector3(realWidth, boxSize * HeightScale, realHeight);
            BoxShapeProperties cBoxShape = null;
            SingleShapeEntity  box       = null;

            cBoxShape = new BoxShapeProperties(
               WallMasses[(byte)color],          // mass in kilograms.
               new Pose(),                       // relative pose
               dimensions);                      // dimensions

            // Walls have the same properties as the ground.
            cBoxShape.Material = new MaterialProperties("gbox", 0.8f, 0.5f, 0.8f);

            // Set the color of the box according to the bitmap image
            // or the specified color if no bitmap.
            if (WallTextures[(byte)color] == "")
            {
               cBoxShape.DiffuseColor.X = (float)(WallColors[(byte)color].X / 255.0);
               cBoxShape.DiffuseColor.Y = (float)(WallColors[(byte)color].Y / 255.0);
               cBoxShape.DiffuseColor.Z = (float)(WallColors[(byte)color].Z / 255.0);
               cBoxShape.DiffuseColor.W = 0.5f;
            }
            else
            {
               cBoxShape.TextureFileName = WallTextures[(byte)color];
            }

            box = new SingleShapeEntity(new BoxShape(cBoxShape),
                                        new Vector3((startCol * -GridSpacing) - widthOffset,
                                                    boxSize * HeightScale / 2,
                                                    -(startRow * GridSpacing) + heightOffset)
                                        );

            // Name the entity. All entities must have unique names.
            box.State.Name = "wall_" + startRow + "_" + startCol;
            SimulationEngine.GlobalInstancePort.Insert(box);
         }

         BlockCounter++;
      }
   }

   // An entity for approximating a table.
   public class TableEntity : MultiShapeEntity
   {
      // Default constructor.
      public TableEntity() { }

      // Custom constructor, programmatically builds
      // physics primitive shapes to describe.
      public TableEntity(Vector3 position)
      {
         State.Pose.Position = position;
         State.Assets.Mesh   = "table_01.obj";
         float tableHeight    = 0.65f;
         float tableWidth     = 1.05f;
         float tableDepth     = 0.7f;
         float tableThinkness = 0.03f;
         float legThickness   = 0.03f;
         float legOffset      = 0.05f;

         // add a shape for the table surface
         BoxShape tableTop = new BoxShape(
            new BoxShapeProperties(30,
                                   new Pose(new Vector3(0, tableHeight, 0)),
                                   new Vector3(tableWidth, tableThinkness, tableDepth))
            );

         // add a shape for the left leg
         BoxShape tableLeftLeg = new BoxShape(
            new BoxShapeProperties(10,     // mass in kg
                                   new Pose(
                                      new Vector3(-tableWidth / 2 + legOffset, tableHeight / 2, 0)),
                                   new Vector3(legThickness, tableHeight + tableThinkness, tableDepth))
            );

         BoxShape tableRightLeg = new BoxShape(
            new BoxShapeProperties(10,     // mass in kg
                                   new Pose(
                                      new Vector3(tableWidth / 2 - legOffset, tableHeight / 2, 0)),
                                   new Vector3(legThickness, tableHeight + tableThinkness, tableDepth))
            );

         BoxShapes = new List<BoxShape>();
         BoxShapes.Add(tableTop);
         BoxShapes.Add(tableLeftLeg);
         BoxShapes.Add(tableRightLeg);
      }


      public override void Update(FrameUpdate update)
      {
         base.Update(update);
      }
   }
}
