//***************************************************************************//
//* File Name: frameRate.cpp                                                *//
//*    Author: Chris McBride chris_a_mcbride@hotmail.com                    *//
//* Date Made: 04/06/02                                                     *//
//* File Desc: Frame rate counter for any project, windows specific.        *//
//* Rev. Date: 11/26/02                                                     *//
//* Rev. Desc: Added frame rate independence and UNIX functionality (TEP)   *//
//*                                                                         *//
//***************************************************************************//

#include "frameRate.hpp"

// Initial speed factor.
float FrameRate::initialSpeedFactor = 0.0f;

// Maximum speed factor.
float FrameRate::maxSpeedFactor = 5.0f;

// Constructor.
FrameRate::FrameRate(float targetFPS)
{
   this->targetFPS = targetFPS;
   FPS             = targetFPS;
   speedFactor     = initialSpeedFactor;
   m_frameCount    = 0;
   m_lastTime      = gettime();
}


// Update: call per frame.
void FrameRate::update()
{
   TIME currentTime, delta;

   // Count the frame.
   m_frameCount++;

   // Get the time delta.
   currentTime = gettime();
   delta       = (currentTime - m_lastTime) / 1000;

   // Time to recalculate frame rate?
   if (delta >= FRAME_RECALC_FREQUENCY)
   {
      // Calculate new values.
      FPS = (float)m_frameCount / (float)delta;
      if (FPS > 0.0f)
      {
         speedFactor = targetFPS / FPS;
      }
      else
      {
         speedFactor = 0.0f;
      }
      if (speedFactor > maxSpeedFactor)
      {
         speedFactor = maxSpeedFactor;
      }
      m_frameCount = 0;
      m_lastTime   = currentTime;
   }
}


// Reset.
void FrameRate::reset()
{
   FPS          = targetFPS;
   speedFactor  = initialSpeedFactor;
   m_frameCount = 0;
   m_lastTime   = gettime();
}
