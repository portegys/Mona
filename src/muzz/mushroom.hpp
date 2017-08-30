// For conditions of distribution and use, see copyright notice in muzz.hpp

// Mushroom.

#ifndef MUSHROOM_HPP
#define MUSHROOM_HPP

#ifdef WIN32
#include <windows.h>
#endif
#include "../graphics/graphics.h"
#include "blockTerrain.hpp"

class Mushroom : public BaseObject
{
public:

   // Parameters:

   // Dimensions.
   static GLfloat WIDTH, HEIGHT;

   // Constructor/destructor.
   Mushroom(float *color, BlockTerrain *terrain, Random *randomizer);
   ~Mushroom();

   // Is mushroom alive?
   bool isAlive() { return(m_alive); }
   void setAlive(bool alive) { m_alive = alive; }

   // Color.
   void getColor(float *color);
   void setColor(float *color);

   // Place mushroom on terrain.
   void place(RANDOM placementSeed);
   void place(int x, int y);
   void place(GLfloat x, GLfloat y);
   void place();

   inline GLfloat getPlaceX() { return(m_placePosition[0]); }
   inline GLfloat getPlaceY() { return(m_placePosition[2]); }

   // Draw.
   void draw();

   // Load mushroom.
   void load(char *filename);
   void load(FILE *);

   // Save mushroom.
   void save(char *filename);
   void save(FILE *);

private:

   bool         m_alive;
   float        m_color[3];
   GLfloat      m_placePosition[3];
   BlockTerrain *m_terrain;
   Random       *m_randomizer;

   // OpenGL display for drawing.
   GLuint display;

   // Components.
   GLUquadricObj *stem, *cap, *capBorder;
};
#endif
