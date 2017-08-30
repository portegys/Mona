//***************************************************************************//
//* File Name: camera.cpp                                                   *//
//* Author:    Tom Portegys, portegys@ilstu.edu                             *//
//* Date Made: 07/25/02                                                     *//
//* File Desc: Class implementation details representing                    *//
//*            a camera and frustum.                                        *//
//* Rev. Date:                                                              *//
//* Rev. Desc:                                                              *//
//*                                                                         *//
//***************************************************************************//

#include "camera.hpp"

// Set viewing frustum.
void Camera::setFrustum(GLfloat angle, GLfloat aspect,
                        GLfloat znear, GLfloat zfar)
{
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(angle, aspect, znear, zfar);
   extractFrustum();
   glMatrixMode(GL_MODELVIEW);
}


// Set camera view.
void Camera::setView(GLfloat *eye, GLfloat *lookat, GLfloat *up)
{
   int     i;
   GLfloat t[3], r[4];

   // Set view and initialize orientation history.
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   setPosition(eye);
   for (i = 0; i < 3; i++)
   {
      t[i] = lookat[i] - eye[i];
   }
   m_spacial->getBillboard(t, r);
   if ((fabs(r[0]) >= tol) || (fabs(r[1]) >= tol) || (fabs(r[2]) >= tol))
   {
      m_spacial->loadRotation(DegreesToRadians(r[3]), r);
   }
   else
   {
      if (t[2] < 0.0)
      {
         setYaw(180.0);
      }
   }
   gluLookAt(eye[0], eye[1], eye[2],
             lookat[0], lookat[1], lookat[2],
             up[0], up[1], up[2]);
   for (i = 0; i < cameraHistorySize; i++)
   {
      cameraHistory[i].e[0] = eye[0];
      cameraHistory[i].e[1] = eye[1];
      cameraHistory[i].e[2] = eye[2];
      cameraHistory[i].f[0] = lookat[0];
      cameraHistory[i].f[1] = lookat[1];
      cameraHistory[i].f[2] = lookat[2];
      cameraHistory[i].u[0] = up[0];
      cameraHistory[i].u[1] = up[1];
      cameraHistory[i].u[2] = up[2];
   }
   cameraHistoryIndex = 0;

   // Extract updated frustum.
   extractFrustum();
}


// Update camera view.
void Camera::updateView(GLfloat *eye, GLfloat *lookat, GLfloat *up)
{
   int     i, j;
   GLfloat e[3], f[3], u[3], t[3], r[4];

   // Camera orientation is average of new and old orientations.
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   setPosition(eye);
   for (i = 0; i < 3; i++)
   {
      t[i] = lookat[i] - eye[i];
   }
   m_spacial->getBillboard(t, r);
   if ((fabs(r[0]) >= tol) || (fabs(r[1]) >= tol) || (fabs(r[2]) >= tol))
   {
      m_spacial->loadRotation(DegreesToRadians(r[3]), r);
   }
   else
   {
      if (t[2] < 0.0)
      {
         setYaw(180.0);
      }
   }
   cameraHistoryIndex = (cameraHistoryIndex + 1) % cameraHistorySize;
   i    = cameraHistoryIndex;
   e[0] = cameraHistory[i].e[0] = eye[0];
   e[1] = cameraHistory[i].e[1] = eye[1];
   e[2] = cameraHistory[i].e[2] = eye[2];
   f[0] = cameraHistory[i].f[0] = lookat[0];
   f[1] = cameraHistory[i].f[1] = lookat[1];
   f[2] = cameraHistory[i].f[2] = lookat[2];
   u[0] = cameraHistory[i].u[0] = up[0];
   u[1] = cameraHistory[i].u[1] = up[1];
   u[2] = cameraHistory[i].u[2] = up[2];
   for (j = 1; j < cameraHistorySize; j++)
   {
      i     = (i + 1) % cameraHistorySize;
      f[0] += cameraHistory[i].f[0];
      f[1] += cameraHistory[i].f[1];
      f[2] += cameraHistory[i].f[2];
      u[0] += cameraHistory[i].u[0];
      u[1] += cameraHistory[i].u[1];
      u[2] += cameraHistory[i].u[2];
   }
   f[0] /= (float)cameraHistorySize;
   f[1] /= (float)cameraHistorySize;
   f[2] /= (float)cameraHistorySize;
   u[0] /= (float)cameraHistorySize;
   u[1] /= (float)cameraHistorySize;
   u[2] /= (float)cameraHistorySize;
   setPosition(eye);
   for (i = 0; i < 3; i++)
   {
      t[i] = f[i] - eye[i];
   }
   m_spacial->getBillboard(t, r);
   if ((fabs(r[0]) >= tol) || (fabs(r[1]) >= tol) || (fabs(r[2]) >= tol))
   {
      m_spacial->loadRotation(DegreesToRadians(r[3]), r);
   }
   else
   {
      if (t[2] < 0.0)
      {
         setYaw(180.0);
      }
   }
   gluLookAt(eye[0], eye[1], eye[2],
             f[0], f[1], f[2],
             u[0], u[1], u[2]);

   // Extract updated frustum.
   extractFrustum();
}


// Place camera using spacial state.
void Camera::place()
{
   GLfloat eye[3], forward[3], up[3];

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   getPosition(eye);
   getForward(forward);
   getUp(up);
   gluLookAt(eye[0], eye[1], eye[2],
             eye[0] + forward[0], eye[1] + forward[1], eye[2] + forward[2],
             up[0], up[1], up[2]);

   // Extract updated frustum.
   extractFrustum();
}


// Set camera spring-loaded parameter.
// 1=fixed, >=springier
void Camera::setCameraSpringiness(int size)
{
   int     i;
   GLfloat e[3], f[3], u[3];

   if (size <= 0)
   {
      return;
   }
   e[0] = cameraHistory[cameraHistoryIndex].e[0];
   e[1] = cameraHistory[cameraHistoryIndex].e[1];
   e[2] = cameraHistory[cameraHistoryIndex].e[2];
   f[0] = cameraHistory[cameraHistoryIndex].f[0];
   f[1] = cameraHistory[cameraHistoryIndex].f[1];
   f[2] = cameraHistory[cameraHistoryIndex].f[2];
   u[0] = cameraHistory[cameraHistoryIndex].u[0];
   u[1] = cameraHistory[cameraHistoryIndex].u[1];
   u[2] = cameraHistory[cameraHistoryIndex].u[2];
   delete cameraHistory;
   cameraHistorySize  = size;
   cameraHistory      = new struct CameraHistory[cameraHistorySize];
   cameraHistoryIndex = 0;
   for (i = 0; i < cameraHistorySize; i++)
   {
      cameraHistory[i].e[0] = e[0];
      cameraHistory[i].e[1] = e[1];
      cameraHistory[i].e[2] = e[2];
      cameraHistory[i].f[0] = f[0];
      cameraHistory[i].f[1] = f[1];
      cameraHistory[i].f[2] = f[2];
      cameraHistory[i].u[0] = u[0];
      cameraHistory[i].u[1] = u[1];
      cameraHistory[i].u[2] = u[2];
   }
}


// Is local point in frustum?
bool Camera::pointInFrustum(GLfloat *localPoint)
{
   int     i, j;
   GLfloat m[16];
   Matrix  x(4, 4), p(4, 1), t(4, 1);
   GLfloat worldPoint[3];

   // Convert to world point.
   glGetFloatv(GL_MODELVIEW_MATRIX, m);
   for (i = 0; i < 4; i++)
   {
      for (j = 0; j < 4; j++)
      {
         x(i, j) = m[(j * 4) + i];
      }
   }
   p(0, 0)       = localPoint[0];
   p(1, 0)       = localPoint[1];
   p(2, 0)       = localPoint[2];
   p(3, 0)       = 1.0;
   t             = x * p;
   worldPoint[0] = t(0, 0);
   worldPoint[1] = t(1, 0);
   worldPoint[2] = t(2, 0);

   // Point must lie inside all planes.
   for (i = 0; i < 6; i++)
   {
      if (frustum[i][0] * worldPoint[0] +
          frustum[i][1] * worldPoint[1] +
          frustum[i][2] * worldPoint[2] +
          frustum[i][3] <= 0)
      {
         return(false);
      }
   }
   return(true);
}


// Extracts The Current View Frustum Plane Equations
// Code from: www.markmorley.com/opengl/frustumculling.html
void
Camera::extractFrustum()
{
   float proj[16];
   float modl[16];
   float clip[16];
   float t;

   /* Get the current PROJECTION matrix from OpenGL */
   glGetFloatv(GL_PROJECTION_MATRIX, proj);

   /* Use identity MODELVIEW matrix */
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
   glGetFloatv(GL_MODELVIEW_MATRIX, modl);

   /* Combine the two matrices (multiply projection by modelview) */
   clip[0] = modl[0] * proj[0] + modl[1] * proj[4] + modl[2] * proj[8] + modl[3] * proj[12];
   clip[1] = modl[0] * proj[1] + modl[1] * proj[5] + modl[2] * proj[9] + modl[3] * proj[13];
   clip[2] = modl[0] * proj[2] + modl[1] * proj[6] + modl[2] * proj[10] + modl[3] * proj[14];
   clip[3] = modl[0] * proj[3] + modl[1] * proj[7] + modl[2] * proj[11] + modl[3] * proj[15];

   clip[4] = modl[4] * proj[0] + modl[5] * proj[4] + modl[6] * proj[8] + modl[7] * proj[12];
   clip[5] = modl[4] * proj[1] + modl[5] * proj[5] + modl[6] * proj[9] + modl[7] * proj[13];
   clip[6] = modl[4] * proj[2] + modl[5] * proj[6] + modl[6] * proj[10] + modl[7] * proj[14];
   clip[7] = modl[4] * proj[3] + modl[5] * proj[7] + modl[6] * proj[11] + modl[7] * proj[15];

   clip[8]  = modl[8] * proj[0] + modl[9] * proj[4] + modl[10] * proj[8] + modl[11] * proj[12];
   clip[9]  = modl[8] * proj[1] + modl[9] * proj[5] + modl[10] * proj[9] + modl[11] * proj[13];
   clip[10] = modl[8] * proj[2] + modl[9] * proj[6] + modl[10] * proj[10] + modl[11] * proj[14];
   clip[11] = modl[8] * proj[3] + modl[9] * proj[7] + modl[10] * proj[11] + modl[11] * proj[15];

   clip[12] = modl[12] * proj[0] + modl[13] * proj[4] + modl[14] * proj[8] + modl[15] * proj[12];
   clip[13] = modl[12] * proj[1] + modl[13] * proj[5] + modl[14] * proj[9] + modl[15] * proj[13];
   clip[14] = modl[12] * proj[2] + modl[13] * proj[6] + modl[14] * proj[10] + modl[15] * proj[14];
   clip[15] = modl[12] * proj[3] + modl[13] * proj[7] + modl[14] * proj[11] + modl[15] * proj[15];

   /* Extract the numbers for the RIGHT plane */
   frustum[0][0] = clip[3] - clip[0];
   frustum[0][1] = clip[7] - clip[4];
   frustum[0][2] = clip[11] - clip[8];
   frustum[0][3] = clip[15] - clip[12];

   /* Normalize the result */
   t              = sqrt(frustum[0][0] * frustum[0][0] + frustum[0][1] * frustum[0][1] + frustum[0][2] * frustum[0][2]);
   frustum[0][0] /= t;
   frustum[0][1] /= t;
   frustum[0][2] /= t;
   frustum[0][3] /= t;

   /* Extract the numbers for the LEFT plane */
   frustum[1][0] = clip[3] + clip[0];
   frustum[1][1] = clip[7] + clip[4];
   frustum[1][2] = clip[11] + clip[8];
   frustum[1][3] = clip[15] + clip[12];

   /* Normalize the result */
   t              = sqrt(frustum[1][0] * frustum[1][0] + frustum[1][1] * frustum[1][1] + frustum[1][2] * frustum[1][2]);
   frustum[1][0] /= t;
   frustum[1][1] /= t;
   frustum[1][2] /= t;
   frustum[1][3] /= t;

   /* Extract the BOTTOM plane */
   frustum[2][0] = clip[3] + clip[1];
   frustum[2][1] = clip[7] + clip[5];
   frustum[2][2] = clip[11] + clip[9];
   frustum[2][3] = clip[15] + clip[13];

   /* Normalize the result */
   t              = sqrt(frustum[2][0] * frustum[2][0] + frustum[2][1] * frustum[2][1] + frustum[2][2] * frustum[2][2]);
   frustum[2][0] /= t;
   frustum[2][1] /= t;
   frustum[2][2] /= t;
   frustum[2][3] /= t;

   /* Extract the TOP plane */
   frustum[3][0] = clip[3] - clip[1];
   frustum[3][1] = clip[7] - clip[5];
   frustum[3][2] = clip[11] - clip[9];
   frustum[3][3] = clip[15] - clip[13];

   /* Normalize the result */
   t              = sqrt(frustum[3][0] * frustum[3][0] + frustum[3][1] * frustum[3][1] + frustum[3][2] * frustum[3][2]);
   frustum[3][0] /= t;
   frustum[3][1] /= t;
   frustum[3][2] /= t;
   frustum[3][3] /= t;

   /* Extract the FAR plane */
   frustum[4][0] = clip[3] - clip[2];
   frustum[4][1] = clip[7] - clip[6];
   frustum[4][2] = clip[11] - clip[10];
   frustum[4][3] = clip[15] - clip[14];

   /* Normalize the result */
   t              = sqrt(frustum[4][0] * frustum[4][0] + frustum[4][1] * frustum[4][1] + frustum[4][2] * frustum[4][2]);
   frustum[4][0] /= t;
   frustum[4][1] /= t;
   frustum[4][2] /= t;
   frustum[4][3] /= t;

   /* Extract the NEAR plane */
   frustum[5][0] = clip[3] + clip[2];
   frustum[5][1] = clip[7] + clip[6];
   frustum[5][2] = clip[11] + clip[10];
   frustum[5][3] = clip[15] + clip[14];

   /* Normalize the result */
   t              = sqrt(frustum[5][0] * frustum[5][0] + frustum[5][1] * frustum[5][1] + frustum[5][2] * frustum[5][2]);
   frustum[5][0] /= t;
   frustum[5][1] /= t;
   frustum[5][2] /= t;
   frustum[5][3] /= t;

   glPopMatrix();
}
