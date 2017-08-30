// For conditions of distribution and use, see copyright notice in muzz.hpp

// A muzz is a simple robotic creature that uses a mona neural network
// for a brain.

#include "muzz.hpp"

// Identifier dispenser.
int Muzz::idDispenser = 0;

// Muzz needs.
Mona::NEED Muzz::      INIT_HUNGER      = 1.0;
Mona::NEED Muzz::      INIT_THIRST      = 1.0;
const Mona::NEED Muzz::EAT_GOAL_VALUE   = 1.0;
const Mona::NEED Muzz::DRINK_GOAL_VALUE = 1.0;

// Base size.
const GLfloat Muzz::BASE_SIZE = 0.025f;

// Hover height.
const GLfloat Muzz::HOVER_HEIGHT = 0.0f;

// Movement constraint parameters.
const GLfloat Muzz::MAX_HEIGHT_CHANGE    = 0.75f;
const GLfloat Muzz::MAX_ANGLE_ADJUSTMENT = 30.0f;

// Constructor.
Muzz::Muzz(float *color, BlockTerrain *terrain,
           RANDOM placementSeed, Random *randomizer) : BaseObject()
{
   GLfloat       d1, d2, d3, a;
   Vector        vertices[8];
   const GLfloat flatten = 0.5f;
   const GLfloat border  = BASE_SIZE * 0.025f;

   id = idDispenser;
   idDispenser++;

   m_color[0]   = color[0];
   m_color[1]   = color[1];
   m_color[2]   = color[2];
   m_terrain    = terrain;
   m_randomizer = randomizer;

   // Place muzz on the terrain.
   place(placementSeed);

   // Build display.
   display = glGenLists(1);
   glNewList(display, GL_COMPILE);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glDisable(GL_TEXTURE_2D);

   // Draw the base.
   glTranslatef(BASE_SIZE * -0.5f, 0.0f, BASE_SIZE * -0.5f);
   glColor3f(1.0f, 1.0f, 1.0f);
   drawBlock(0.0f, BASE_SIZE, 0.0f, BASE_SIZE * flatten, 0.0f, BASE_SIZE);
   glColor3f(0.0f, 0.0f, 0.0f);

   // Draw the front 'v' pattern.
   drawBlock(0.0f, BASE_SIZE, 0.0f, border, BASE_SIZE, BASE_SIZE + border);
   drawBlock(0.0f, BASE_SIZE, (BASE_SIZE * flatten) - border, BASE_SIZE * flatten, BASE_SIZE, BASE_SIZE + border);
   drawBlock(-border, 0.0f, 0.0f, BASE_SIZE * flatten, BASE_SIZE, BASE_SIZE + border);
   drawBlock(BASE_SIZE, BASE_SIZE + border, 0.0f, BASE_SIZE * flatten, BASE_SIZE, BASE_SIZE + border);
   glPushMatrix();
   glTranslatef(0.0f, (BASE_SIZE * flatten) - border, 0.0f);
   d1 = (BASE_SIZE * flatten) - border;
   d2 = BASE_SIZE * 0.5f;
   d3 = sqrt((d1 * d1) + (d2 * d2));
   a  = 90.0f - RadiansToDegrees(asin(d2 / d3));
   glRotatef(-a, 0.0f, 0.0f, 1.0f);
   drawBlock(0.0f, d3, 0.0f, border, BASE_SIZE, BASE_SIZE + border);
   glPopMatrix();
   glPushMatrix();
   glTranslatef(BASE_SIZE, (BASE_SIZE * flatten) - border, 0.0f);
   glRotatef(a, 0.0f, 0.0f, 1.0f);
   drawBlock(-d3, 0.0f, 0.0f, border, BASE_SIZE, BASE_SIZE + border);
   glPopMatrix();

   // Draw the back inverted 'v' pattern.
   drawBlock(0.0f, BASE_SIZE, 0.0f, border, -border, 0.0f);
   drawBlock(0.0f, BASE_SIZE, (BASE_SIZE * flatten) - border, BASE_SIZE * flatten, -border, 0.0f);
   drawBlock(-border, 0.0f, 0.0f, BASE_SIZE * flatten, -border, 0.0f);
   drawBlock(BASE_SIZE, BASE_SIZE + border, 0.0f, BASE_SIZE * flatten, -border, 0.0f);
   glPushMatrix();
   glTranslatef(BASE_SIZE * 0.5f, (BASE_SIZE * flatten) - border, 0.0f);
   glRotatef(-a, 0.0f, 0.0f, 1.0f);
   drawBlock(0.0f, d3, 0.0f, border, -border, 0.0f);
   glPopMatrix();
   glPushMatrix();
   glTranslatef(BASE_SIZE * 0.5f, (BASE_SIZE * flatten) - border, 0.0f);
   glRotatef(a, 0.0f, 0.0f, 1.0f);
   drawBlock(-d3, 0.0f, 0.0f, border, -border, 0.0f);
   glPopMatrix();

   // Draw the left diagnonal pattern.
   drawBlock(-border, 0.0f, 0.0f, border, 0.0f, BASE_SIZE);
   drawBlock(-border, 0.0f, (BASE_SIZE * flatten) - border, BASE_SIZE * flatten, 0.0f, BASE_SIZE);
   glPushMatrix();
   d1 = (BASE_SIZE * flatten) - border;
   d2 = BASE_SIZE;
   d3 = sqrt((d1 * d1) + (d2 * d2));
   a  = RadiansToDegrees(asin(d1 / d3));
   glRotatef(-a, 1.0f, 0.0f, 0.0f);
   drawBlock(-border, 0.0f, 0.0f, border, 0.0f, d3);
   glPopMatrix();

   // Draw the right diagnonal pattern.
   drawBlock(BASE_SIZE, BASE_SIZE + border, 0.0f, border, 0.0f, BASE_SIZE);
   drawBlock(BASE_SIZE, BASE_SIZE + border, (BASE_SIZE * flatten) - border, BASE_SIZE * flatten, 0.0f, BASE_SIZE);
   glPushMatrix();
   d1 = (BASE_SIZE * flatten) - border;
   d2 = BASE_SIZE;
   d3 = sqrt((d1 * d1) + (d2 * d2));
   a  = RadiansToDegrees(asin(d1 / d3));
   glRotatef(-a, 1.0f, 0.0f, 0.0f);
   drawBlock(BASE_SIZE, BASE_SIZE + border, 0.0f, border, 0.0f, d3);
   glPopMatrix();

   // Draw the "turret".
   glColor3f(m_color[0], m_color[1], m_color[2]);
   turretCylinder = gluNewQuadric();
   gluQuadricDrawStyle(turretCylinder, GLU_FILL);
   gluQuadricNormals(turretCylinder, GLU_SMOOTH);
   glPushMatrix();
   glTranslatef(BASE_SIZE * 0.5f, BASE_SIZE * 0.75f, BASE_SIZE * 0.5f);
   glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
   gluCylinder(turretCylinder, BASE_SIZE * 0.2f, BASE_SIZE * 0.2f, BASE_SIZE * 0.25f, 25, 5);
   glPopMatrix();
   turretTopInner = gluNewQuadric();
   gluQuadricDrawStyle(turretTopInner, GLU_FILL);
   gluQuadricNormals(turretTopInner, GLU_SMOOTH);
   glPushMatrix();
   glTranslatef(BASE_SIZE * 0.5f, BASE_SIZE * 0.75f, BASE_SIZE * 0.5f);
   glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
   gluDisk(turretTopInner, 0.0f, BASE_SIZE * 0.15f, 25, 5);
   glPopMatrix();
   glColor3f(0.5f, 0.5f, 0.5f);
   turretTopOuter = gluNewQuadric();
   gluQuadricDrawStyle(turretTopOuter, GLU_FILL);
   gluQuadricNormals(turretTopOuter, GLU_SMOOTH);
   glPushMatrix();
   glTranslatef(BASE_SIZE * 0.5f, BASE_SIZE * 0.75f, BASE_SIZE * 0.5f);
   glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
   gluDisk(turretTopOuter, BASE_SIZE * 0.15f, BASE_SIZE * 0.2f, 25, 5);
   glPopMatrix();
   turretEye = gluNewQuadric();
   gluQuadricDrawStyle(turretEye, GLU_FILL);
   gluQuadricNormals(turretEye, GLU_SMOOTH);
   glPushMatrix();
   glTranslatef(BASE_SIZE * 0.5f, BASE_SIZE * 0.63f, BASE_SIZE * 0.65f);
   gluSphere(turretEye, BASE_SIZE * 0.1f, 10, 10);
   glPopMatrix();
   turretRightEar = gluNewQuadric();
   gluQuadricDrawStyle(turretRightEar, GLU_FILL);
   gluQuadricNormals(turretRightEar, GLU_SMOOTH);
   glPushMatrix();
   glTranslatef(BASE_SIZE * 0.3f, BASE_SIZE * 0.63f, BASE_SIZE * 0.5f);
   gluSphere(turretRightEar, BASE_SIZE * 0.05f, 10, 10);
   glPopMatrix();
   turretLeftEar = gluNewQuadric();
   gluQuadricDrawStyle(turretLeftEar, GLU_FILL);
   gluQuadricNormals(turretLeftEar, GLU_SMOOTH);
   glPushMatrix();
   glTranslatef(BASE_SIZE * 0.7f, BASE_SIZE * 0.63f, BASE_SIZE * 0.5f);
   gluSphere(turretLeftEar, BASE_SIZE * 0.05f, 10, 10);
   glPopMatrix();

   glColor3f(1.0f, 1.0f, 1.0f);
   glPopMatrix();
   glEndList();

   // Initialize brain.
   brain = new Mona();
   assert(brain != NULL);
   initBrain(brain);

   // Initialize needs.
   setNeed(FOOD, INIT_HUNGER);
   setNeed(WATER, INIT_THIRST);

   // Allocate brain sensors.
   brainSensors.resize(NUM_BRAIN_SENSORS);

   // Initialize foraging status.
   gotFood      = gotWater = false;
   gotFoodCycle = gotWaterCycle = -1;
}


// Default constructor.
Muzz::Muzz(BlockTerrain *terrain) : BaseObject()
{
   int i;

   id = idDispenser;
   idDispenser++;

   for (i = 0; i < 3; i++)
   {
      m_color[i] = 0.0f;
   }
   for (i = 0; i < 3; i++)
   {
      m_placePosition[i] = 0.0f;
   }
   m_placeDirection = 0.0f;
   m_terrain        = terrain;
   m_randomizer     = NULL;
   display          = (GLuint)(-1);
   turretCylinder   = NULL;
   turretTopInner   = NULL;
   turretTopOuter   = NULL;
   turretEye        = NULL;
   turretRightEar   = NULL;
   turretLeftEar    = NULL;
   brain            = NULL;
   gotFood          = gotWater = false;
   gotFoodCycle     = gotWaterCycle = -1;
}


// Destructor.
Muzz::~Muzz()
{
   if (display != (GLuint)(-1))
   {
      glDeleteLists(display, 1);
   }
   if (turretCylinder != NULL)
   {
      gluDeleteQuadric(turretCylinder);
   }
   if (turretTopInner != NULL)
   {
      gluDeleteQuadric(turretTopInner);
   }
   if (turretTopOuter != NULL)
   {
      gluDeleteQuadric(turretTopOuter);
   }
   if (turretEye != NULL)
   {
      gluDeleteQuadric(turretEye);
   }
   if (turretRightEar != NULL)
   {
      gluDeleteQuadric(turretRightEar);
   }
   if (turretLeftEar != NULL)
   {
      gluDeleteQuadric(turretLeftEar);
   }
   if (brain != NULL)
   {
      delete brain;
   }
}


// Initialize muzz brain.
void Muzz::initBrain(Mona *brain)
{
   int i;

   vector<bool>         sensorMask;
   vector<bool>         bits;
   vector<Mona::SENSOR> goalSensors;

   // Initialize network.
   brain->initNet(NUM_BRAIN_SENSORS, NUM_RESPONSES, NUM_NEEDS,
                  brain->randomSeed);
   sensorMask.clear();
   for (i = 0; i < NUM_BRAIN_SENSORS; i++)
   {
      sensorMask.push_back(true);
   }
   brain->addSensorMode(sensorMask);
   sensorMask.clear();
   for (i = 0; i < NUM_BRAIN_SENSORS; i++)
   {
      if (i < NUM_BRAIN_SENSOR_MODE_1)
      {
         sensorMask.push_back(true);
      }
      else
      {
         sensorMask.push_back(false);
      }
   }
   brain->addSensorMode(sensorMask);
   sensorMask.clear();
   for (i = 0; i < NUM_BRAIN_SENSORS; i++)
   {
      if (i < NUM_BRAIN_SENSOR_MODE_1)
      {
         sensorMask.push_back(false);
      }
      else
      {
         sensorMask.push_back(true);
      }
   }
   brain->addSensorMode(sensorMask);

   // Add sensory goals.
   goalSensors.resize(NUM_BRAIN_SENSORS);
   for (i = 0; i < NUM_BRAIN_SENSORS; i++)
   {
      bits.push_back(false);
   }
   setBoolBits(bits, TERRAIN_SENSOR_0, 3, PLATFORM);
   setBoolBits(bits, OBJECT_SENSOR_0, 5, MUSHROOM +
               (MAX_OTHER - MIN_OTHER));
   for (i = 0; i < NUM_BRAIN_SENSORS; i++)
   {
      if (bits[i])
      {
         goalSensors[i] = 1.0f;
      }
      else
      {
         goalSensors[i] = 0.0f;
      }
   }
   brain->homeostats[FOOD]->addGoal(goalSensors, SENSOR_MODE_2,
                                    EAT, EAT_GOAL_VALUE);
   setBoolBits(bits, OBJECT_SENSOR_0, 5, POOL +
               (MAX_OTHER - MIN_OTHER));
   for (i = 0; i < NUM_BRAIN_SENSORS; i++)
   {
      if (bits[i])
      {
         goalSensors[i] = 1.0f;
      }
      else
      {
         goalSensors[i] = 0.0f;
      }
   }
   brain->homeostats[WATER]->addGoal(goalSensors, SENSOR_MODE_2,
                                     DRINK, DRINK_GOAL_VALUE);
}


// Set binary value in bools.
void Muzz::setBoolBits(vector<bool>& bits, int begin,
                       int length, int value)
{
   for (int i = 0; i < length; i++)
   {
      if ((value % 2) == 1)
      {
         bits[begin + i] = true;
      }
      else
      {
         bits[begin + i] = false;
      }
      value = (unsigned int)value >> 1;
   }
}


// Get color.
void Muzz::getColor(float *color)
{
   color[0] = m_color[0];
   color[1] = m_color[1];
   color[2] = m_color[2];
}


// Set color.
void Muzz::setColor(float *color)
{
   m_color[0] = color[0];
   m_color[1] = color[1];
   m_color[2] = color[2];
}


// Get terrain.
BlockTerrain *Muzz::getTerrain()
{
   return(m_terrain);
}


// Get randomizer.
Random *Muzz::getRandomizer()
{
   return(m_randomizer);
}


// Random muzz placement on terrain.
void Muzz::place(RANDOM placementSeed)
{
   int x, y;
   BlockTerrain::Block::DIRECTION direction;

   m_randomizer->RAND_PUSH();
   m_randomizer->SRAND(placementSeed);

   // Pick a random block.
   x = m_randomizer->RAND_CHOICE(m_terrain->WIDTH);
   y = m_randomizer->RAND_CHOICE(m_terrain->HEIGHT);

   // Pick a random direction.
   direction = (BlockTerrain::Block::DIRECTION)m_randomizer->RAND_CHOICE(4);

   // Place.
   place(x, y, direction);

   m_randomizer->RAND_POP();
}


// Place in block terrain units.
void Muzz::place(int x, int y, BlockTerrain::Block::DIRECTION direction)
{
   assert(x >= 0 && x < m_terrain->WIDTH);
   assert(y >= 0 && y < m_terrain->HEIGHT);
   GLfloat fx = ((GLfloat)x * m_terrain->BLOCK_SIZE) + (m_terrain->BLOCK_SIZE * 0.5f);
   GLfloat fy = ((GLfloat)y * m_terrain->BLOCK_SIZE) + (m_terrain->BLOCK_SIZE * 0.5f);
   switch (direction)
   {
   case BlockTerrain::Block::NORTH:
      place(fx, fy, 0.0f);
      break;

   case BlockTerrain::Block::EAST:
      place(fx, fy, 90.0f);
      break;

   case BlockTerrain::Block::SOUTH:
      place(fx, fy, 180.0f);
      break;

   case BlockTerrain::Block::WEST:
      place(fx, fy, 270.0f);
      break;

   default:
      assert(false);
   }
}


// General placement.
void Muzz::place(GLfloat x, GLfloat y, GLfloat direction)
{
   GLfloat p[3];
   Vector  n;

   p[0] = x;
   p[2] = y;
   m_terrain->getGeometry(p[0], p[2], p[1], n);
   m_placePosition[0] = p[0];
   m_placePosition[1] = p[1];
   m_placePosition[2] = p[2];
   m_placeDirection   = direction;
   place();
}


// Default placement.
void Muzz::place()
{
   clearSpacial();
   setPosition(m_placePosition);
   setYaw(m_placeDirection);
   forward(0.0f);
}


// Move forward.
void Muzz::forward(GLfloat step)
{
   moveOverTerrain(MOVE_FORWARD, step);
}


// Move backward.
void Muzz::backward(GLfloat step)
{
   moveOverTerrain(MOVE_FORWARD, -step);
}


// Turn right.
void Muzz::right(GLfloat angle)
{
   moveOverTerrain(TURN_RIGHT, angle);
}


// Turn left.
void Muzz::left(GLfloat angle)
{
   moveOverTerrain(TURN_LEFT, angle);
}


// Conform movement to terrain topology.
// Return false if move is invalid.
bool Muzz::moveOverTerrain(MOVE_TYPE type, GLfloat amount)
{
   int     i;
   GLfloat r, c0[3], c1[3], c2[3], c3[3], nc0[3], nc1[3], nc2[3], nc3[3];
   GLfloat oldpos[3], pos[3], oldfwd[3], fwd[3], up[3], normal[3], rotation[4];
   Vector  v1, v2, n;
   bool    ok;

   // Clear transforms for coordinate conversions.
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   // Make sure maximum height change is not exceeded for all 4 corners.
   r     = BASE_SIZE * 0.5f;
   c0[0] = -r;
   c0[1] = 0.0f;
   c0[2] = r;
   localToWorld(c0, c0);
   c1[0] = r;
   c1[1] = 0.0f;
   c1[2] = r;
   localToWorld(c1, c1);
   c2[0] = r;
   c2[1] = 0.0f;
   c2[2] = -r;
   localToWorld(c2, c2);
   c3[0] = -r;
   c3[1] = 0.0f;
   c3[2] = -r;
   localToWorld(c3, c3);
   switch (type)
   {
   case MOVE_FORWARD:
      getPosition(oldpos);
      getForward(fwd);
      cSpacial::normalize(fwd);
      for (i = 0; i < 3; i++)
      {
         pos[i] = oldpos[i] + (fwd[i] * amount);
      }
      setPosition(pos);
      break;

   case TURN_LEFT:
      amount = -amount;

   case TURN_RIGHT:
      addYaw(amount);
      break;
   }
   do
   {
      ok = true;
      GLfloat max = MAX_HEIGHT_CHANGE * m_terrain->BLOCK_SIZE;
      nc0[0] = -r;
      nc0[1] = 0.0f;
      nc0[2] = r;
      localToWorld(nc0, nc0);
      m_terrain->getGeometry(c0[0], c0[2], c0[1], n);
      m_terrain->getGeometry(nc0[0], nc0[2], nc0[1], n);
      if (fabs(c0[1] - nc0[1]) > max)
      {
         ok = false;
         break;
      }
      nc1[0] = r;
      nc1[1] = 0.0f;
      nc1[2] = r;
      localToWorld(nc1, nc1);
      m_terrain->getGeometry(c1[0], c1[2], c1[1], n);
      m_terrain->getGeometry(nc1[0], nc1[2], nc1[1], n);
      if (fabs(c1[1] - nc1[1]) > max)
      {
         ok = false;
         break;
      }
      nc2[0] = r;
      nc2[1] = 0.0f;
      nc2[2] = -r;
      localToWorld(nc2, nc2);
      m_terrain->getGeometry(c2[0], c2[2], c2[1], n);
      m_terrain->getGeometry(nc2[0], nc2[2], nc2[1], n);
      if (fabs(c2[1] - nc2[1]) > max)
      {
         ok = false;
         break;
      }
      nc3[0] = -r;
      nc3[1] = 0.0f;
      nc3[2] = -r;
      localToWorld(nc3, nc3);
      m_terrain->getGeometry(c3[0], c3[2], c3[1], n);
      m_terrain->getGeometry(nc3[0], nc3[2], nc3[1], n);
      if (fabs(c3[1] - nc3[1]) > max)
      {
         ok = false;
         break;
      }
   } while (false);
   if (!ok)
   {
      switch (type)
      {
      case MOVE_FORWARD:
         setPosition(oldpos);
         break;

      case TURN_LEFT:
      case TURN_RIGHT:
         addYaw(-amount);
         break;
      }
      return(false);
   }

   // Set height to average height of corners.
   getPosition(pos);
   pos[1] = (nc0[1] + nc1[1] + nc2[1] + nc3[1]) / 4.0f;
   setPosition(pos);

   // Save original heading for later.
   getForward(oldfwd);

   // New up vector is normal to base plane.
   v1.x = nc1[0] - nc0[0];
   v1.y = nc1[1] - nc0[1];
   v1.z = nc1[2] - nc0[2];
   v2.x = nc2[0] - nc0[0];
   v2.y = nc2[1] - nc0[1];
   v2.z = nc2[2] - nc0[2];
   n    = v1 ^ v2;
   n.Normalize();
   normal[0] = n.x;
   normal[1] = n.y;
   normal[2] = n.z;

   // Smooth out rotation.
   getUp(up);
   getBillboard(normal, up, rotation);
   if ((fabs(rotation[0]) > tol) || (fabs(rotation[1]) > tol) || (fabs(rotation[2]) > tol))
   {
      if (rotation[3] > MAX_ANGLE_ADJUSTMENT)
      {
         rotation[3] = MAX_ANGLE_ADJUSTMENT;
      }
      else if (rotation[3] < -MAX_ANGLE_ADJUSTMENT)
      {
         rotation[3] = -MAX_ANGLE_ADJUSTMENT;
      }
      mergeRotation(rotation[3], rotation);
   }

   // Set new height to "hover" above terrain.
   pos[1] += HOVER_HEIGHT;
   setPosition(pos);

   return(true);
}


// Set the camera to muzz's viewpoint.
void Muzz::aimCamera(Camera& camera)
{
   GLfloat up[3], pos[3];

   camera.setSpacial(getSpacial());
   camera.getUp(up);
   cSpacial::normalize(up);
   camera.getPosition(pos);
   for (int i = 0; i < 3; i++)
   {
      pos[i] += (up[i] * BASE_SIZE * 0.6f);
   }
   camera.setPosition(pos);
   camera.place();
}


// Draw the muzz.
void Muzz::draw()
{
   assert(display != (GLuint)(-1));

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   // Transform.
   glTranslatef(m_spacial->getX(), m_spacial->getY(), m_spacial->getZ());
   glMultMatrixf(&m_spacial->m_rotmatrix[0][0]);
   GLfloat scale = m_spacial->getScale();
   glScalef(scale, scale, scale);

   // Draw.
   glDisable(GL_TEXTURE_2D);
   glCallList(display);

   glPopMatrix();
}


// Highlight the muzz.
void Muzz::highlight()
{
   assert(display != (GLuint)(-1));

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   // Transform.
   glTranslatef(m_spacial->getX(), m_spacial->getY(), m_spacial->getZ());
   glMultMatrixf(&m_spacial->m_rotmatrix[0][0]);
   GLfloat scale = m_spacial->getScale();
   glScalef(scale, scale, scale);

   // Draw enclosing semi-tranparent sphere.
   glDisable(GL_TEXTURE_2D);
   glColor4f(m_color[0], m_color[1], m_color[2], 0.5f);
   glEnable(GL_BLEND);
   glutSolidSphere(BASE_SIZE, 20, 20);
   glDisable(GL_BLEND);

   glPopMatrix();
}


// Reset muzz.
void Muzz::reset()
{
   assert(brain != NULL);
   brain->clearWorkingMemory();
   setNeed(FOOD, INIT_HUNGER);
   setNeed(WATER, INIT_THIRST);

   // Reset foraging results.
   gotFood      = gotWater = false;
   gotFoodCycle = gotWaterCycle = -1;

   // Re-place.
   place();
}


// Reset muzz need.
void Muzz::resetNeed(int need)
{
   assert(brain != NULL);
   if (need == FOOD)
   {
      setNeed(FOOD, INIT_HUNGER);
      gotFood      = false;
      gotFoodCycle = -1;
   }
   else
   {
      setNeed(WATER, INIT_THIRST);
      gotWater      = false;
      gotWaterCycle = -1;
   }
}


// Clear muzz need.
void Muzz::clearNeed(int need)
{
   assert(brain != NULL);
   if (need == FOOD)
   {
      setNeed(FOOD, 0.0);
      gotFood = true;
   }
   else
   {
      setNeed(WATER, 0.0);
      gotWater = true;
   }
}


// Sensory-response cycle.
int Muzz::cycle(int *sensors, int cycleNum)
{
   int i, response;

   vector<bool> bits;

   assert(brain != NULL);

   // Set up brain sensors.
   bits.resize(NUM_BRAIN_SENSORS);
   if (sensors[FORWARD_SENSOR] == OPEN)
   {
      bits[FORWARD_SENSOR] = true;
   }
   else
   {
      bits[FORWARD_SENSOR] = false;
   }
   if (sensors[RIGHT_SENSOR] == OPEN)
   {
      bits[RIGHT_SENSOR] = true;
   }
   else
   {
      bits[RIGHT_SENSOR] = false;
   }
   if (sensors[LEFT_SENSOR] == OPEN)
   {
      bits[LEFT_SENSOR] = true;
   }
   else
   {
      bits[LEFT_SENSOR] = false;
   }
   setBoolBits(bits, TERRAIN_SENSOR_0, 3, sensors[TERRAIN_SENSOR]);
   if (sensors[OBJECT_SENSOR] >= MIN_OTHER)
   {
      setBoolBits(bits, OBJECT_SENSOR_0, 5,
                  sensors[OBJECT_SENSOR] - MIN_OTHER);
   }
   else
   {
      setBoolBits(bits, OBJECT_SENSOR_0, 5,
                  sensors[OBJECT_SENSOR] + (MAX_OTHER - MIN_OTHER));
   }
   for (i = 0; i < NUM_BRAIN_SENSORS; i++)
   {
      if (bits[i])
      {
         brainSensors[i] = 1.0f;
      }
      else
      {
         brainSensors[i] = 0.0f;
      }
   }

   // Get response.
   response = (int)brain->cycle(brainSensors);

   // Note food and water consumption.
   if ((sensors[OBJECT_SENSOR] == MUSHROOM) && (response == EAT))
   {
      if (!gotFood)
      {
         gotFood      = true;
         gotFoodCycle = cycleNum;
      }
   }
   if ((sensors[OBJECT_SENSOR] == POOL) && (response == DRINK))
   {
      if (!gotWater)
      {
         gotWater      = true;
         gotWaterCycle = cycleNum;
      }
   }

   return(response);
}


// Print muzz brain in XML format.
void Muzz::printBrain(FILE *out)
{
   assert(brain != NULL);
   fprintf(out, "Muzz brain:\n");
#ifdef MONA_TRACKING
   brain->print((Mona::TRACKING_FLAGS)Mona::TRACK_DRIVE, out);
#else
   brain->print(out);
#endif
}


// Print muzz response potentials.
void Muzz::printResponsePotentials(FILE *out)
{
   assert(brain != NULL);
   fprintf(out, "Muzz response potentials:\n");
   for (int i = 0; i < NUM_RESPONSES; i++)
   {
      switch (i)
      {
      case WAIT:
         fprintf(out, "WAIT = %f\n", brain->responsePotentials[i]);
         break;

      case FORWARD:
         fprintf(out, "FORWARD = %f\n", brain->responsePotentials[i]);
         break;

      case RIGHT:
         fprintf(out, "RIGHT = %f\n", brain->responsePotentials[i]);
         break;

      case LEFT:
         fprintf(out, "LEFT = %f\n", brain->responsePotentials[i]);
         break;

      case EAT:
         fprintf(out, "EAT = %f\n", brain->responsePotentials[i]);
         break;

      case DRINK:
         fprintf(out, "DRINK = %f\n", brain->responsePotentials[i]);
         break;
      }
   }
}


// Load muzz.
void Muzz::load(char *filename)
{
   FILE *fp;

   if ((fp = FOPEN_READ(filename)) == NULL)
   {
      fprintf(stderr, "Cannot load muzz from file %s\n", filename);
      exit(1);
   }
   load(fp);
   FCLOSE(fp);
}


void Muzz::load(FILE *fp)
{
   assert(brain != NULL);
   ((BaseObject *)this)->load(fp);
   FREAD_INT(&id, fp);
   FREAD_BOOL(&gotFood, fp);
   FREAD_BOOL(&gotWater, fp);
   FREAD_INT(&gotFoodCycle, fp);
   FREAD_INT(&gotWaterCycle, fp);
   brain->load(fp);
   FREAD_FLOAT(&m_color[0], fp);
   FREAD_FLOAT(&m_color[1], fp);
   FREAD_FLOAT(&m_color[2], fp);
   FREAD_FLOAT(&m_placePosition[0], fp);
   FREAD_FLOAT(&m_placePosition[1], fp);
   FREAD_FLOAT(&m_placePosition[2], fp);
   FREAD_FLOAT(&m_placeDirection, fp);
}


// Save muzz.
void Muzz::save(char *filename)
{
   FILE *fp;

   if ((fp = FOPEN_WRITE(filename)) == NULL)
   {
      fprintf(stderr, "Cannot save muzz to file %s\n", filename);
      exit(1);
   }
   save(fp);
   FCLOSE(fp);
}


void Muzz::save(FILE *fp)
{
   assert(brain != NULL);
   ((BaseObject *)this)->save(fp);
   FWRITE_INT(&id, fp);
   FWRITE_BOOL(&gotFood, fp);
   FWRITE_BOOL(&gotWater, fp);
   FWRITE_INT(&gotFoodCycle, fp);
   FWRITE_INT(&gotWaterCycle, fp);
   brain->save(fp);
   FWRITE_FLOAT(&m_color[0], fp);
   FWRITE_FLOAT(&m_color[1], fp);
   FWRITE_FLOAT(&m_color[2], fp);
   FWRITE_FLOAT(&m_placePosition[0], fp);
   FWRITE_FLOAT(&m_placePosition[1], fp);
   FWRITE_FLOAT(&m_placePosition[2], fp);
   FWRITE_FLOAT(&m_placeDirection, fp);
}
