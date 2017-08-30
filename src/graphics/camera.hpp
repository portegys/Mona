//***************************************************************************//
//* File Name: camera.hpp                                                   *//
//* Author:    Tom Portegys, portegys@ilstu.edu                             *//
//* Date Made: 07/25/02                                                     *//
//* File Desc: Class declaration representing a camera and frustum.         *//
//* Rev. Date:                                                              *//
//* Rev. Desc:                                                              *//
//*                                                                         *//
//***************************************************************************//

#ifndef __CAMERA_HPP__
#define __CAMERA_HPP__

#include "baseObject.hpp"

class Camera : public BaseObject
{
public:

   // Constructor.
   Camera()
   {
      cameraHistorySize  = 1;
      cameraHistory      = new struct CameraHistory[cameraHistorySize];
      cameraHistoryIndex = 0;
      extractFrustum();
   }


   // Destructor.
   ~Camera()
   {
      delete cameraHistory;
   }


   // Set viewing frustum.
   void setFrustum(GLfloat angle, GLfloat aspect,
                   GLfloat znear, GLfloat zfar);

   // Set camera view.
   void setView(GLfloat *eye, GLfloat *lookat, GLfloat *up);

   // Update camera view.
   void updateView(GLfloat *eye, GLfloat *lookat, GLfloat *up);

   // Place camera.
   void place();

   // Set camera spring-loaded parameter.
   // 1=fixed, >=springier
   void setCameraSpringiness(int size);

   // Is local point inside frustum (visible)?
   bool pointInFrustum(GLfloat *localPoint);

private:

   // Camera vector history for springiness calculation.
   struct CameraHistory
   {
      GLfloat e[3];                               // eye
      GLfloat f[3];                               // forward
      GLfloat u[3];                               // up
   }
       *cameraHistory;
   int cameraHistorySize;
   int cameraHistoryIndex;

   // Frustum bounding planes.
   GLfloat frustum[6][4];

   // Extract frustum.
   void extractFrustum();
};
#endif
