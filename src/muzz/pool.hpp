// For conditions of distribution and use, see copyright notice in muzz.hpp

// Water pool.

#ifndef POOL_HPP
#define POOL_HPP

#include "../graphics/graphics.h"
#include "blockTerrain.hpp"

class Pool : public BaseObject
{
public:

   // Parameters:

   // Dimensions.
   static GLfloat WIDTH, HEIGHT;

   // Constructor/destructor.
   Pool(float *color, BlockTerrain *terrain, Random *randomizer);
   ~Pool();

   // Color.
   void getColor(float *color);
   void setColor(float *color);

   // Place pool on terrain.
   void place(RANDOM placementSeed);
   void place(int x, int y);
   void place(GLfloat x, GLfloat y);
   void place();

   inline GLfloat getPlaceX() { return(m_placePosition[0]); }
   inline GLfloat getPlaceY() { return(m_placePosition[2]); }

   // Draw.
   void draw();

   // Load pool.
   void load(char *filename);
   void load(FILE *);

   // Save pool.
   void save(char *filename);
   void save(FILE *);

private:

   float        m_color[3];
   GLfloat      m_placePosition[3];
   BlockTerrain *m_terrain;
   Random       *m_randomizer;

   // OpenGL display for drawing.
   GLuint display;

   // Components.
   GLUquadricObj *poolBase, *poolSurface;
};
#endif
