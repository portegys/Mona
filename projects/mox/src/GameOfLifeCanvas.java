/**
 * Game of Life canvas.
 */

import java.awt.*;
import java.awt.event.*;

public class GameOfLifeCanvas extends Canvas
{
   // Game of life.
   GameOfLife gameOfLife;

   // Buffered display.
   private Dimension canvasSize;
   private Graphics  graphics;
   private Image     image;
   private Graphics  imageGraphics;

   // Message and font.
   private String      message;
   private Font        font = new Font("Helvetica", Font.BOLD, 12);
   private FontMetrics fontMetrics;
   private int         fontAscent;
   private int         fontWidth;
   private int         fontHeight;

   // Last cell visited by mouse.
   private int lastX = -1;

   // Last cell visited by mouse.
   private int lastY = -1;

   // Constructor.
   public GameOfLifeCanvas(GameOfLife gameOfLife, Dimension canvasSize)
   {
      this.gameOfLife = gameOfLife;

      // Configure canvas.
      this.canvasSize = canvasSize;
      setBounds(0, 0, canvasSize.width, canvasSize.height);
      addMouseListener(new CanvasMouseListener());
      addMouseMotionListener(new CanvasMouseMotionListener());
   }


   // Display automaton.
   void display()
   {
      int    x;
      int    y;
      int    x2;
      int    y2;
      double cellWidth;
      double cellHeight;

      if (graphics == null)
      {
         graphics      = getGraphics();
         image         = createImage(canvasSize.width, canvasSize.height);
         imageGraphics = image.getGraphics();
         graphics.setFont(font);
         fontMetrics = graphics.getFontMetrics();
         fontAscent  = fontMetrics.getMaxAscent();
         fontWidth   = fontMetrics.getMaxAdvance();
         fontHeight  = fontMetrics.getHeight();
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
         cellWidth  = (double)canvasSize.width / (double)gameOfLife.size.width;
         cellHeight = (double)canvasSize.height / (double)gameOfLife.size.height;
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
                  case 2:
                     imageGraphics.setColor(Color.green);
                     break;

                  case 3:
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

      // Draw message.
      drawMessage();

      // Refresh display.
      graphics.drawImage(image, 0, 0, this);
   }


   // Set message.
   public void setMessage(String s)
   {
      message = s;
   }


   // Draw message.
   private void drawMessage()
   {
      if ((message != null) && !message.equals(""))
      {
         imageGraphics.setFont(font);
         imageGraphics.setColor(Color.black);
         imageGraphics.drawString(message,
                                  (canvasSize.width - fontMetrics.stringWidth(message)) / 2,
                                  canvasSize.height / 2);
      }
   }


   // Canvas mouse listener.
   class CanvasMouseListener extends MouseAdapter
   {
      // Mouse pressed.
      public void mousePressed(MouseEvent evt)
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
            lastX = x;
            lastY = y;

            synchronized (gameOfLife.lock)
            {
               switch (gameOfLife.cells[x][y])
               {
               case 0:
                  gameOfLife.cells[x][y] = GameOfLife.BLUE_CELL_COLOR_VALUE;
                  break;

               case GameOfLife.BLUE_CELL_COLOR_VALUE:
                  gameOfLife.cells[x][y] = GameOfLife.GREEN_CELL_COLOR_VALUE;
                  break;

               case GameOfLife.GREEN_CELL_COLOR_VALUE:
                  gameOfLife.cells[x][y] = 0;
                  break;
               }
            }
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
                  switch (gameOfLife.cells[x][y])
                  {
                  case 0:
                     gameOfLife.cells[x][y] = GameOfLife.BLUE_CELL_COLOR_VALUE;
                     break;

                  case GameOfLife.BLUE_CELL_COLOR_VALUE:
                     gameOfLife.cells[x][y] = GameOfLife.GREEN_CELL_COLOR_VALUE;
                     break;

                  case GameOfLife.GREEN_CELL_COLOR_VALUE:
                     gameOfLife.cells[x][y] = 0;
                     break;
                  }
               }
            }
         }
      }
   }
}
