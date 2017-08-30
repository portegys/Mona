//***************************************************************************//
//* File Name: spacial.hpp                                                  *//
//* Author:    Tom Portegys, portegys@ilstu.edu                             *//
//* Date Made: 07/25/02                                                     *//
//* File Desc: Class declaration representing spacial properties:           *//
//*             rotation, translation, scale, and speed.                    *//
//* Rev. Date: 3/6/2008                                                     *//
//* Rev. Desc:                                                              *//
//*                                                                         *//
//***************************************************************************//

#ifndef __SPACIAL_HPP__
#define __SPACIAL_HPP__

#ifdef _WIN32
#ifndef WIN32
#define WIN32
#endif
#endif

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <assert.h>
#include "../common/quaternion.hpp"
#include "../common/matrix.hpp"
#include "../common/vector.hpp"

#ifndef _NO_NAMESPACE
using namespace std;
using namespace math;
#define STD    std
#else
#define STD
#endif

#ifndef _NO_TEMPLATE
typedef matrix<GLfloat>   Matrix;
#else
typedef matrix            Matrix;
#endif

class cSpacial
{
public:

   // Rotation.
   GLfloat     m_pitchRate, m_yawRate, m_rollRate;
   GLfloat     m_rotmatrix[4][4];
   cQuaternion *m_qcalc;

   // Position.
   GLfloat m_x, m_y, m_z;

   // Scale.
   GLfloat m_scale;

   // Speed.
   GLfloat m_speed;

   // Constructors.
   cSpacial()
   {
      m_qcalc = new cQuaternion();
      assert(m_qcalc != NULL);
      clear();
   }


   cSpacial(GLfloat pitch, GLfloat yaw, GLfloat roll,
            GLfloat pitchRate, GLfloat yawRate, GLfloat rollRate,
            GLfloat x, GLfloat y, GLfloat z, GLfloat scale, GLfloat speed)
   {
      m_qcalc = new cQuaternion();
      assert(m_qcalc != NULL);
      initialize(pitch, yaw, roll, pitchRate, yawRate, rollRate,
                 x, y, z, scale, speed);
   }


   inline void clear()
   {
      initialize(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   }


   void initialize(GLfloat pitch, GLfloat yaw, GLfloat roll,
                   GLfloat pitchRate, GLfloat yawRate, GLfloat rollRate,
                   GLfloat x, GLfloat y, GLfloat z, GLfloat scale, GLfloat speed);

   // Destructor.
   ~cSpacial() { delete m_qcalc; }

   // Get and set Euler angles.
   GLfloat getPitch();
   GLfloat getYaw();
   GLfloat getRoll();
   void setPitch(GLfloat pitch);
   void setYaw(GLfloat yaw);
   void setRoll(GLfloat roll);

   // Get and set angular rates.
   inline GLfloat getPitchRate() { return(m_pitchRate); }
   inline GLfloat getYawRate() { return(m_yawRate); }
   inline GLfloat getRollRate() { return(m_rollRate); }
   inline void setPitchRate(GLfloat pitchRate) { m_pitchRate = pitchRate; }
   inline void setYawRate(GLfloat yawRate) { m_yawRate = yawRate; }
   inline void setRollRate(GLfloat rollRate) { m_rollRate = rollRate; }

   // Get direction vectors.
   void getRight(GLfloat *v);
   void getUp(GLfloat *v);
   void getForward(GLfloat *v);

   // Get and set position.
   inline GLfloat getX() { return(m_x); }
   inline GLfloat getY() { return(m_y); }
   inline GLfloat getZ() { return(m_z); }
   inline void setX(GLfloat x) { m_x = x; }
   inline void setY(GLfloat y) { m_y = y; }
   inline void setZ(GLfloat z) { m_z = z; }

   // Get and set scale.
   inline GLfloat getScale() { return(m_scale); }
   inline void setScale(GLfloat scale) { m_scale = scale; }

   // Get and set speed.
   inline GLfloat getSpeed() { return(m_speed); }
   inline void setSpeed(GLfloat speed) { m_speed = speed; }

   // Update rotation and translation state.
   void update();

   // Get Euler angles.
   inline void getEulerAngles(float& pitch, float& yaw, float& roll)
   {
      m_qcalc->getEulerAngles(pitch, yaw, roll);
   }


   // Load an axis-angle rotation.
   inline void loadRotation(GLfloat angle, GLfloat *axis)
   {
      m_qcalc->loadRotation(DegreesToRadians(angle), axis);
      build_rotmatrix();
   }


   // Merge an axis-angle rotation.
   inline void mergeRotation(GLfloat angle, GLfloat *axis)
   {
      m_qcalc->mergeRotation(DegreesToRadians(angle), axis);
      build_rotmatrix();
   }


   // Get billboard rotation to given target point.
   void getBillboard(GLfloat *target, GLfloat *rotation);

   // Get billboard rotation from source to target vectors.
   void getBillboard(GLfloat *target, GLfloat *source, GLfloat *rotation);

   // Build rotation matrix from quaternion.
   inline void build_rotmatrix()
   {
      m_qcalc->build_rotmatrix(m_rotmatrix);
   }


   // Get model transformation matrix.
   void getModelTransform(GLfloat *matrix);

   // Get world coordinates from local.
   void localToWorld(GLfloat *local, GLfloat *world);

   // Transform local point.
   inline void transformPoint(GLfloat *point)
   {
      localToWorld(point, point);
   }


   // Inverse transform local point.
   void inverseTransformPoint(GLfloat *point);

   // Point-to-point distance.
   static GLfloat pointDistance(GLfloat *p1, GLfloat *p2);

   // Normalize vector.
   inline static void normalize(GLfloat *v)
   {
      GLfloat d = (sqrt((v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2])));

      v[0] = v[0] / d;
      v[1] = v[1] / d;
      v[2] = v[2] / d;
   }


   // Clone spacial.
   cSpacial *clone();

   // Load and save.
   void load(FILE *fp);
   void save(FILE *fp);
};
#endif
