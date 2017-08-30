// For conditions of distribution and use, see copyright notice in muzz.hpp

// Water pool.

#include "pool.hpp"

// Dimensions.
GLfloat Pool::WIDTH  = 0.025f;
GLfloat Pool::HEIGHT = 0.001f;

// Constructor.
Pool::Pool(float *color, BlockTerrain *terrain, Random *randomizer) : BaseObject()
{
   int       i;
   const int maxtries = 1000;

   m_color[0]   = color[0];
   m_color[1]   = color[1];
   m_color[2]   = color[2];
   m_terrain    = terrain;
   m_randomizer = randomizer;

   // Place the pool on the terrain.
   for (i = 0; i < maxtries; i++)
   {
      place(m_randomizer->RAND());
      if (m_terrain->blocks[(int)(m_placePosition[0] / m_terrain->BLOCK_SIZE)]
          [(int)(m_placePosition[2] / m_terrain->BLOCK_SIZE)].type == BlockTerrain::Block::PLATFORM)
      {
         break;
      }
   }
   if (i == maxtries)
   {
      fprintf(stderr, "Cannot place pool on terrain\n");
      exit(1);
   }

   // Build drawing display.
   display = glGenLists(1);
   glNewList(display, GL_COMPILE);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glDisable(GL_TEXTURE_2D);
   glColor3f(m_color[0], m_color[1], m_color[2]);
   poolBase = gluNewQuadric();
   gluQuadricDrawStyle(poolBase, GLU_FILL);
   gluQuadricNormals(poolBase, GLU_SMOOTH);
   glPushMatrix();
   glTranslatef(0.0f, HEIGHT, 0.0f);
   glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
   gluCylinder(poolBase, WIDTH, WIDTH, HEIGHT, 20, 2);
   glPopMatrix();
   poolSurface = gluNewQuadric();
   gluQuadricDrawStyle(poolSurface, GLU_FILL);
   gluQuadricNormals(poolSurface, GLU_SMOOTH);
   glPushMatrix();
   glTranslatef(0.0f, HEIGHT, 0.0f);
   glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
   gluDisk(poolSurface, 0.0f, WIDTH, 20, 5);
   glPopMatrix();
   glColor3f(1.0f, 1.0f, 1.0f);
   glPopMatrix();
   glEndList();
}


// Destructor.
Pool::~Pool()
{
   gluDeleteQuadric(poolBase);
   gluDeleteQuadric(poolSurface);
   glDeleteLists(display, 1);
}


// Get color.
void Pool::getColor(float *color)
{
   color[0] = m_color[0];
   color[1] = m_color[1];
   color[2] = m_color[2];
}


// Set color.
void Pool::setColor(float *color)
{
   m_color[0] = color[0];
   m_color[1] = color[1];
   m_color[2] = color[2];
}


// Random pool placement on terrain.
void Pool::place(RANDOM placementSeed)
{
   int x, y;

   m_randomizer->RAND_PUSH();
   m_randomizer->SRAND(placementSeed);

   // Pick a random block.
   x = m_randomizer->RAND_CHOICE(m_terrain->WIDTH);
   y = m_randomizer->RAND_CHOICE(m_terrain->HEIGHT);

   // Place.
   place(x, y);

   m_randomizer->RAND_POP();
}


// Place in block terrain units.
void Pool::place(int x, int y)
{
   assert(x >= 0 && x < m_terrain->WIDTH);
   assert(y >= 0 && y < m_terrain->HEIGHT);
   GLfloat fx = ((GLfloat)x * m_terrain->BLOCK_SIZE) + (m_terrain->BLOCK_SIZE * 0.5f);
   GLfloat fy = ((GLfloat)y * m_terrain->BLOCK_SIZE) + (m_terrain->BLOCK_SIZE * 0.5f);
   place(fx, fy);
}


// General placement.
void Pool::place(GLfloat x, GLfloat y)
{
   GLfloat p[3];
   Vector  n;

   p[0] = x;
   p[2] = y;
   m_terrain->getGeometry(p[0], p[2], p[1], n);
   m_placePosition[0] = p[0];
   m_placePosition[1] = p[1];
   m_placePosition[2] = p[2];
   place();
}


// Default placement.
void Pool::place()
{
   clearSpacial();
   setPosition(m_placePosition);
}


// Draw the pool.
void Pool::draw()
{
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


// Load pool.
void Pool::load(char *filename)
{
   FILE *fp;

   if ((fp = FOPEN_READ(filename)) == NULL)
   {
      fprintf(stderr, "Cannot load pool from file %s\n", filename);
      exit(1);
   }
   load(fp);
   FCLOSE(fp);
}


void Pool::load(FILE *fp)
{
   ((BaseObject *)this)->load(fp);
}


// Save pool.
void Pool::save(char *filename)
{
   FILE *fp;

   if ((fp = FOPEN_WRITE(filename)) == NULL)
   {
      fprintf(stderr, "Cannot save pool to file %s\n", filename);
      exit(1);
   }
   save(fp);
   FCLOSE(fp);
}


void Pool::save(FILE *fp)
{
   ((BaseObject *)this)->save(fp);
}
