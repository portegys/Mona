//***************************************************************************//
//* File Name: baseObject.hpp                                               *//
//* Author:    Tom Portegys, portegys@ilstu.edu                             *//
//* Date Made: 07/25/02                                                     *//
//* File Desc: Class declaration representing a basic object.               *//
//* Rev. Date:                                                              *//
//* Rev. Desc:                                                              *//
//*                                                                         *//
//***************************************************************************//

#ifndef __BASE_OBJECT_HPP__
#define __BASE_OBJECT_HPP__

#include "../common/common.h"
#include <GL/glut.h>
#include "spacial.hpp"

class BaseObject
{
public:

   // Constructor.
   BaseObject()
   {
      m_spacial = new cSpacial();
      assert(m_spacial != NULL);
   }


   // Destructor.
   ~BaseObject()
   {
      delete m_spacial;
   }


   // Set spacial state.
   cSpacial *getSpacial() { return(m_spacial); }

   // Set spacial state.
   void setSpacial(GLfloat pitch, GLfloat yaw, GLfloat roll,
                   GLfloat pitchRate, GLfloat yawRate, GLfloat rollRate,
                   GLfloat x, GLfloat y, GLfloat z, GLfloat scale, GLfloat speed);
   void setSpacial(cSpacial *spacial);

   // Clear spacial.
   void clearSpacial() { m_spacial->clear(); }

   // Rotations.
   GLfloat getPitch() { return(m_spacial->getPitch()); }
   GLfloat getYaw()   { return(m_spacial->getYaw()); }
   GLfloat getRoll()  { return(m_spacial->getRoll()); }
   void setPitch(GLfloat pitch) { m_spacial->setPitch(pitch); }
   void setYaw(GLfloat yaw) { m_spacial->setYaw(yaw); }
   void setRoll(GLfloat roll) { m_spacial->setRoll(roll); }
   void addPitch(GLfloat pitch)
   {
      m_spacial->setPitch(m_spacial->getPitch() + pitch);
   }


   void addYaw(GLfloat yaw)
   {
      m_spacial->setYaw(m_spacial->getYaw() + yaw);
   }


   void addRoll(GLfloat roll)
   {
      m_spacial->setRoll(m_spacial->getRoll() + roll);
   }


   // Get direction vectors.
   void getRight(GLfloat *v)   { m_spacial->getRight(v); }
   void getUp(GLfloat *v)      { m_spacial->getUp(v); }
   void getForward(GLfloat *v) { m_spacial->getForward(v); }

   // Position.
   void getPosition(GLfloat *v)
   {
      v[0] = m_spacial->getX();
      v[1] = m_spacial->getY();
      v[2] = m_spacial->getZ();
   }


   void setPosition(GLfloat *v)
   {
      m_spacial->setX(v[0]);
      m_spacial->setY(v[1]);
      m_spacial->setZ(v[2]);
   }


   // Scale.
   GLfloat getScale() { return(m_spacial->getScale()); }
   void setScale(GLfloat scale) { m_spacial->setScale(scale); }

   // Speed.
   GLfloat getSpeed() { return(m_spacial->getSpeed()); }
   void setSpeed(GLfloat speed) { m_spacial->setSpeed(speed); }
   void addSpeed(GLfloat speed)
   {
      m_spacial->setSpeed(m_spacial->getSpeed() + speed);
   }


   // Update.
   void update() { m_spacial->update(); }

   // Get model transformation matrix.
   void getModelTransform(GLfloat *matrix)
   {
      m_spacial->getModelTransform(matrix);
   }


   // Get world coordinates from local.
   void localToWorld(GLfloat *local, GLfloat *world)
   {
      m_spacial->localToWorld(local, world);
   }


   // Transform local point.
   void transformPoint(GLfloat *point)
   {
      m_spacial->transformPoint(point);
   }


   // Inverse transform local point.
   void inverseTransformPoint(GLfloat *point)
   {
      m_spacial->inverseTransformPoint(point);
   }


   // Get billboard (face toward) rotation to target point given in local coordinates.
   // Return axis and angle for rotation to accomplish billboard.
   // rotation[0-2]=axis, rotation[3]=angle
   void getBillboard(GLfloat *target, GLfloat *rotation)
   {
      m_spacial->getBillboard(target, rotation);
   }


   // Get billboard (face toward) rotation from source to target vectors.
   // Return axis and angle for rotation to accomplish billboard.
   // rotation[0-2]=axis, rotation[3]=angle
   void getBillboard(GLfloat *target, GLfloat *source, GLfloat *rotation)
   {
      m_spacial->getBillboard(target, source, rotation);
   }


   // Load an axis-angle rotation into quaternion.
   void loadRotation(GLfloat angle, GLfloat *axis)
   {
      m_spacial->loadRotation(angle, axis);
   }


   // Merge an axis-angle rotation into quaternion.
   void mergeRotation(GLfloat angle, GLfloat *axis)
   {
      m_spacial->mergeRotation(angle, axis);
   }


   // Draw axes.
   void drawAxes(GLfloat span = 1.0f);

   // Set block vertices.
   static void setBlockVertices(Vector *vertices, float xmin, float xmax, float ymin, float ymax, float zmin, float zmax);

   // Draw a block.
   static void drawBlock(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax);

   // Draw block edges.
   static void drawBlockEdges(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax);

   // Load object.
   void load(char *filename);
   void load(FILE *fp);

   // Save object.
   void save(char *filename);
   void save(FILE *fp);

   // Spacial properties.
   cSpacial *m_spacial;
};
#endif
