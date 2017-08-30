/*
 * Triangle mesh heightmap.
 *
 * Description : Stores a set of polygons defining a
 * 3D mesh surface as 2D projections on a quadtree plane
 * in the XZ dimensions. Search using plane coordinates
 * yields the Y (height) dimension value of the mesh.
 *
 * Reference:
 * QOTA: A Fast, Multi-Purpose Algorithm For
 * Terrain Following In Virtual Environments
 * John W. Barrus, Richard C. Waters
 * MERL-TR-96-17 September, 1996
 *
 * Programmer : Tom Portegys <portegys@ilstu.edu>
 */

#ifndef __MESH_HEIGHTMAP_HPP__
#define __MESH_HEIGHTMAP_HPP__

#ifdef _WIN32
#ifndef WIN32
#define WIN32
#endif
#endif

#ifdef WIN32
#include <windows.h>
#else
#include <stdlib.h>
#include <stdio.h>
#endif
#include <vector>
#include "../common/common.h"
using namespace std;

// Axis-aligned bounding rectangle in the XZ plane.
class Bounds
{
public:

   // Constructors.
   Bounds() {}
   Bounds(Vector& min, Vector& max)
   {
      this->min = min;
      this->max = max;
   }


   // Is given point inside the bounding rectangle?
   bool contains(float x, float z);

   // Is given bounds contained within this?
   bool contains(Bounds& bounds);

   // Does given rectangle intersect this?
   bool intersects(Bounds& bounds);

   // Minimum and maximum x and z values.
   Vector min;
   Vector max;
};

// Polygon object.
class Poly
{
public:

   // Polygon plane equation in normal-distance form.
   class Plane
   {
public:

      // Constructors.
      Plane() {}
      Plane(Vector& normal, float d)
      {
         this->normal = normal;
         this->d      = d;
      }


      Vector normal;
      float  d;
   };

   // Constructors.
   // Vertices assumed to be co-planar and
   // wound counterclockwise, i.e., viewing polygon
   // from "outside".
   Poly() {}
   Poly(vector<Vector>& vertices);

   // Is given point inside the polygon projected on the XZ plane?
   bool contains(float x, float z);

   // Print polygon.
   void print();

   // Vectices.
   vector<Vector> vertices;

   // 3D plane defined by vertices.
   Plane plane;

   // 2D bounding rectangle in the XZ plane.
   Bounds bounds;
};

// Node.
class QuadNode
{
public:

   // Constructors.
   QuadNode()
   {
      depth = 0;
      for (int i = 0; i < 4; i++) { children[i] = NULL; }
   }


   QuadNode(int depth, Bounds& bounds);

   // Destructor.
   ~QuadNode();

   // Insert polygon.
   void insert(Poly *polygon);

   // Search for polygons containing coordinate.
   void search(float x, float z, vector<Poly *>& found);

   // Print node.
   void print();

   // Data members.
   int            depth;
   QuadNode       *children[4];
   Bounds         bounds;
   vector<Poly *> polygons;
};

// Quadtree.
class QuadTree
{
public:

   // Parameters.
   static const int MAX_POLYGONS;                 // Maximum polygons per leaf node.
   static const int MAX_DEPTH;                    // Maximum tree depth.

   // Constructor.
   QuadTree() { root = NULL; }
   QuadTree(Bounds& bounds);

   // Destructor.
   ~QuadTree();

   // Insert polygon.
   bool insert(Poly *polygon);

   // Search for polygons containing coordinate.
   void search(float x, float z, vector<Poly *>& found);

   // Print tree.
   void print();

   // Data members.
   QuadNode       *root;
   Bounds         bounds;
   vector<Poly *> polygons;
};
#endif
