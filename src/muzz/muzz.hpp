/*
 * Copyright (c) 2010 Tom Portegys (portegys@gmail.com). All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY TOM PORTEGYS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// A muzz is a simple robotic creature controlled by a mona neural network.

#ifndef MUZZ_HPP
#define MUZZ_HPP

#include "../graphics/graphics.h"
#include "blockTerrain.hpp"
#include "mushroom.hpp"
#include "pool.hpp"
#include "../mona/mona.hpp"
#include <map>

class Muzz : public BaseObject
{
public:

   // Identifier.
   int id;

   // Identifier dispenser.
   static int idDispenser;

   // Muzz world sensors.
   enum
   {
      FORWARD_SENSOR=0,
      RIGHT_SENSOR  =1,
      LEFT_SENSOR   =2,
      TERRAIN_SENSOR=3,
      OBJECT_SENSOR =4,
      NUM_SENSORS   =5
   };
   enum
   {
      OPEN  =0,
      CLOSED=1
   };
   enum
   {
      PLATFORM =0,
      WALL     =1,
      DROP     =2,
      RAMP_UP  =3,
      RAMP_DOWN=4
   };
   enum
   {
      MUSHROOM =0,
      POOL     =1,
      MUZZ     =2,
      EMPTY    =3,
      MIN_OTHER=(int)'A',
      MAX_OTHER=(int)'['
   };

   // Brain sensors.
   enum
   {
      NUM_BRAIN_SENSORS      =11,
      NUM_BRAIN_SENSOR_MODE_1=3,
      NUM_BRAIN_SENSOR_MODE_2=8
   };
   enum
   {
      // Binary encoding of terrain types:
      TERRAIN_SENSOR_0=3,
      TERRAIN_SENSOR_1=4,
      TERRAIN_SENSOR_2=5,

      // Binary encoding of object values:
      OBJECT_SENSOR_0 =6,
      OBJECT_SENSOR_1 =7,
      OBJECT_SENSOR_2 =8,
      OBJECT_SENSOR_3 =9,
      OBJECT_SENSOR_4 =10
   };

   // Sensor modes.
   enum
   {
      SENSOR_MODE_0=0,
      SENSOR_MODE_1=1,
      SENSOR_MODE_2=2
   };

   // Muzz responses.
   enum
   {
      WAIT          =0,
      FORWARD       =1,
      RIGHT         =2,
      LEFT          =3,
      EAT           =4,
      DRINK         =5,
      NUM_RESPONSES = 6
   };

   // Muzz needs.
   enum
   {
      FOOD     =0,
      WATER    =1,
      NUM_NEEDS=2
   };
   static Mona::NEED       INIT_HUNGER;
   static Mona::NEED       INIT_THIRST;
   static const Mona::NEED EAT_GOAL_VALUE;
   static const Mona::NEED DRINK_GOAL_VALUE;

   // Base size.
   static const GLfloat BASE_SIZE;

   // Hover height.
   static const GLfloat HOVER_HEIGHT;

   // Movement constraint parameters.
   static const GLfloat MAX_HEIGHT_CHANGE;
   static const GLfloat MAX_ANGLE_ADJUSTMENT;

   // Constructors/destructor.
   Muzz(float *color, BlockTerrain *terrain,
        RANDOM placementSeed, Random *randomizer);
   Muzz(BlockTerrain *terrain = NULL);
   ~Muzz();

   // Initialize a muzz brain.
   static void initBrain(Mona *brain);

   // Set binary value in bools.
   static void setBoolBits(vector<bool>& bits, int begin,
                           int length, int value);

   // Color.
   void getColor(float *color);
   void setColor(float *color);

   // Get terrain.
   BlockTerrain *getTerrain();

   // Get randomizer.
   Random *getRandomizer();

   // Place muzz on terrain.
   void place(RANDOM placementSeed);
   void place(int x, int y, BlockTerrain::Block::DIRECTION direction);
   void place(GLfloat x, GLfloat y, GLfloat direction);
   void place();

   inline GLfloat getPlaceX() { return(m_placePosition[0]); }
   inline GLfloat getPlaceY() { return(m_placePosition[2]); }
   inline GLfloat getPlaceDirection() { return(m_placeDirection); }

   // Movement.
   void forward(GLfloat step);
   void backward(GLfloat step);
   void right(GLfloat angle);
   void left(GLfloat angle);

   // Conform movement to terrain topology.
   typedef enum
   {
      MOVE_FORWARD, TURN_RIGHT, TURN_LEFT
   } MOVE_TYPE;
   bool moveOverTerrain(MOVE_TYPE type, GLfloat amount);

   // Set the camera to muzz's viewpoint.
   void aimCamera(Camera& camera);

   // Draw.
   void draw();

   // Highlight the muzz.
   void highlight();

   // Needs.
   double getNeed(int need)
   {
      return(brain->getNeed(need));
   }


   void setNeed(int need, double value)
   {
      brain->setNeed(need, value);
   }


   // Reset muzz.
   void reset();

   // Reset muzz need.
   void resetNeed(int need);

   // Clear muzz need.
   void clearNeed(int need);

   // Sensory-response cycle.
   int cycle(int *sensors, int cycleNum = (-1));

   // Foraging results.
   bool gotFood;
   bool gotWater;
   int  gotFoodCycle;
   int  gotWaterCycle;

   // Load muzz.
   void load(char *filename);
   void load(FILE *);

   // Save muzz.
   void save(char *filename);
   void save(FILE *);

   // Print brain.
   void printBrain(FILE *out = stdout);
   void printResponsePotentials(FILE *out = stdout);

   // Brain.
   Mona *brain;

   // Brain sensors.
   vector<Mona::SENSOR> brainSensors;

   // Properties.
   float        m_color[3];
   GLfloat      m_placePosition[3];
   GLfloat      m_placeDirection;
   BlockTerrain *m_terrain;
   Random       *m_randomizer;

   // OpenGL display for drawing.
   GLuint display;

   // "Turret" components.
   GLUquadricObj *turretCylinder, *turretTopInner, *turretTopOuter,
   *turretEye, *turretRightEar, *turretLeftEar;
};
#endif
