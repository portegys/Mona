//-----------------------------------------------------------------------
//  Atani neural network controlled robot.
//
//  $File: ataniState.cs $ $Revision: 1 $
//-----------------------------------------------------------------------

using Microsoft.Dss.Core;
using Microsoft.Dss.Core.Attributes;
using System;

namespace Robotics.Atani
{
   // Atani state.
   [DataContract]
   public class AtaniState
   {
      // World type.
      public enum WORLD_TYPE
      {
         BLOCK_WORLD = 0,
         TMAZE_WORLD = 1
      };
      [DataMember]
      public WORLD_TYPE _worldType = WORLD_TYPE.TMAZE_WORLD;

      // T-maze dimensions (block units).
      [DataMember]
      public int TMAZE_WIDTH = 9;
      [DataMember]
      public int TMAZE_HEIGHT = 9;

      // Robot selection.
      [DataMember]
      public bool PIONEER3DX_ROBOT = true;
      [DataMember]
      public bool LEGO_NXT_ROBOT = false;
      [DataMember]
      public bool IROBOT = false;

      // Pioneer3DX robot state.
      public Pioneer3DXrobotState _pioneer3DXrobotState = null;

      // Random numbers.
      [DataMember]
      public int    RANDOM_SEED = 2;
      public Random _random     = null;

      // Constructor.
      public AtaniState() { }

      // Initialize.
      public void init()
      {
         // Random numbers.
         _random = new Random(RANDOM_SEED);

         // Create the Pioneer3DX robot.
         // TODO: Lego NXT and IRobots.
         if (PIONEER3DX_ROBOT)
         {
            _pioneer3DXrobotState = new Pioneer3DXrobotState(_random);
         }
      }
   }
}
