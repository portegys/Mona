/*
 * Triangle mesh heightmap.
 */

#include "meshHeightmap.hpp"
#include <assert.h>

// Parameters.
const int QuadTree::MAX_POLYGONS = 8;             // Maximum polygons per leaf node.
const int QuadTree::MAX_DEPTH    = 14;            // Maximum tree depth.

// Polygon constructor.
Poly::Poly(vector<Vector>& vertices)
{
   int    i;
   Vector v1, v2, normal;
   float  d;

   // Store vertices.
   assert(vertices.size() > 2);
   for (i = 0; i < (int)vertices.size(); i++)
   {
      this->vertices.push_back(vertices[i]);
   }

   // Generate 3D plane.
   v1     = vertices[1] - vertices[0];
   v2     = vertices[2] - vertices[0];
   normal = v1 ^ v2;
   normal.Normalize();
   d = (-normal.x * vertices[0].x) + (-normal.y * vertices[0].y) +
       (-normal.z * vertices[0].z);
   plane = Plane(normal, d);

   // Determine 2D bounds in the XZ plane.
   bounds.min.x = bounds.max.x = vertices[0].x;
   bounds.min.z = bounds.max.z = vertices[0].z;
   for (i = 1; i < (int)vertices.size(); i++)
   {
      if (vertices[i].x < bounds.min.x)
      {
         bounds.min.x = vertices[i].x;
      }
      if (vertices[i].x > bounds.max.x)
      {
         bounds.max.x = vertices[i].x;
      }
      if (vertices[i].z < bounds.min.z)
      {
         bounds.min.z = vertices[i].z;
      }
      if (vertices[i].z > bounds.max.z)
      {
         bounds.max.z = vertices[i].z;
      }
   }
}


// Is point inside polygon?
// Adapted from Rourke's Computational Geometry FAQ.
bool Poly::contains(float x, float z)
{
   int  i, j, size;
   bool ret = false;

   for (i = 0, size = (int)vertices.size(), j = size - 1; i < size; j = i++)
   {
      if ((((vertices[i].z <= z) && (z < vertices[j].z)) ||
           ((vertices[j].z <= z) && (z < vertices[i].z))) &&
          (x < (vertices[j].x - vertices[i].x) * (z - vertices[i].z) /
           (vertices[j].z - vertices[i].z) + vertices[i].x))
      {
         ret = !ret;
      }
   }
   return(ret);
}


// Print polygon.
void Poly::print()
{
   printf("Polygon:\n");
   printf("\tvertices: ");
   for (int i = 0; i < (int)vertices.size(); i++)
   {
      printf("(%f,%f,%f) ", vertices[i].x, vertices[i].y, vertices[i].z);
   }
   printf("\n");
   printf("\tplane: normal=(%f,%f,%f),d=%f\n", plane.normal.x, plane.normal.y, plane.normal.z, plane.d);
   printf("\tbounds: xmin=%f,xmax=%f,zmin=%f,zmax=%f\n",
          bounds.min.x, bounds.max.x, bounds.min.z, bounds.max.z);
}


// Quadtree constructor.,
QuadTree::QuadTree(Bounds& bounds)
{
   root         = NULL;
   this->bounds = bounds;
}


// Destructor.
QuadTree::~QuadTree()
{
   if (root != NULL)
   {
      delete root;
   }
   for (int i = 0; i < (int)polygons.size(); i++)
   {
      delete polygons[i];
   }
}


// Insert polygon.
bool QuadTree::insert(Poly *polygon)
{
   if (!bounds.contains(polygon->bounds))
   {
      return(false);
   }

   // Insert into tree.
   if (root == NULL)
   {
      root = new QuadNode(0, bounds);
      assert(root != NULL);
   }
   root->insert(polygon);

   // Add to global list.
   polygons.push_back(polygon);

   return(true);
}


// Search for polygons containing coordinate.
void QuadTree::search(float x, float z, vector<Poly *>& found)
{
   if (root != NULL)
   {
      root->search(x, z, found);
   }
}


// Print polygons in tree.
void QuadTree::print()
{
   printf("Polygons in tree:\n");
   if (root != NULL)
   {
      root->print();
   }
}


// Constructors.
QuadNode::QuadNode(int depth, Bounds& bounds)
{
   this->depth = depth;
   for (int i = 0; i < 4; i++)
   {
      children[i] = NULL;
   }
   this->bounds = bounds;
}


// Destructor.
QuadNode::~QuadNode()
{
   for (int i = 0; i < 4; i++)
   {
      if (children[i] != NULL)
      {
         delete children[i];
      }
   }
}


// Insert polygon.
void QuadNode::insert(Poly *polygon)
{
   int    i, j;
   Bounds childBounds;
   float  rx, rz;

   // OK at this depth?
   if ((depth >= QuadTree::MAX_DEPTH) ||
       ((polygons.size() < QuadTree::MAX_POLYGONS) && (children[0] == NULL)))
   {
      polygons.push_back(polygon);
      return;
   }

   // Create children?
   if (children[0] == NULL)
   {
      rx = (bounds.max.x - bounds.min.x) / 2.0f;
      rz = (bounds.max.z - bounds.min.z) / 2.0f;
      childBounds.min.x = bounds.min.x;
      childBounds.max.x = bounds.max.x - rx;
      childBounds.min.z = bounds.min.z;
      childBounds.max.z = bounds.max.z - rz;
      children[0]       = new QuadNode(depth + 1, childBounds);
      assert(children[0] != NULL);
      childBounds.min.x = bounds.min.x;
      childBounds.max.x = bounds.max.x - rx;
      childBounds.min.z = bounds.min.z + rz;
      childBounds.max.z = bounds.max.z;
      children[1]       = new QuadNode(depth + 1, childBounds);
      assert(children[1] != NULL);
      childBounds.min.x = bounds.min.x + rx;
      childBounds.max.x = bounds.max.x;
      childBounds.min.z = bounds.min.z + rz;
      childBounds.max.z = bounds.max.z;
      children[2]       = new QuadNode(depth + 1, childBounds);
      assert(children[2] != NULL);
      childBounds.min.x = bounds.min.x + rx;
      childBounds.max.x = bounds.max.x;
      childBounds.min.z = bounds.min.z;
      childBounds.max.z = bounds.max.z - rz;
      children[3]       = new QuadNode(depth + 1, childBounds);
      assert(children[3] != NULL);
   }

   // Insert polygons into children.
   polygons.push_back(polygon);
   for (i = 0; i < (int)polygons.size(); i++)
   {
      for (j = 0; j < 4; j++)
      {
         if (children[j]->bounds.intersects(polygons[i]->bounds))
         {
            children[j]->insert(polygons[i]);
         }
      }
   }
   polygons.clear();
}


// Search for polygons containing coordinate.
void QuadNode::search(float x, float z, vector<Poly *>& found)
{
   int i;

   for (i = 0; i < (int)polygons.size(); i++)
   {
      if (polygons[i]->contains(x, z))
      {
         found.push_back(polygons[i]);
      }
   }

   if (children[0] == NULL)
   {
      return;
   }
   for (i = 0; i < 4; i++)
   {
      if (children[i]->bounds.contains(x, z))
      {
         children[i]->search(x, z, found);
      }
   }
}


// Recursively print polygons in node.
void QuadNode::print()
{
   int i;

   printf("Polygons in node at depth=%d, bounds: xmin=%f,xmax=%f,zmin=%f,zmax=%f\n",
          depth, bounds.min.x, bounds.max.x, bounds.min.z, bounds.max.z);
   for (i = 0; i < (int)polygons.size(); i++)
   {
      polygons[i]->print();
   }

   if (children[0] == NULL)
   {
      return;
   }
   for (i = 0; i < 4; i++)
   {
      children[i]->print();
   }
}


// Given point contained in this?
bool Bounds::contains(float x, float z)
{
   if (x < min.x)
   {
      return(false);
   }
   if (x > max.x)
   {
      return(false);
   }
   if (z < min.z)
   {
      return(false);
   }
   if (z > max.z)
   {
      return(false);
   }
   return(true);
}


// Is given bounds contained within this?
bool Bounds::contains(Bounds& bounds)
{
   if (!contains(bounds.min.x, bounds.min.z))
   {
      return(false);
   }
   if (!contains(bounds.max.x, bounds.min.z))
   {
      return(false);
   }
   if (!contains(bounds.min.x, bounds.max.z))
   {
      return(false);
   }
   if (!contains(bounds.max.x, bounds.max.z))
   {
      return(false);
   }
   return(true);
}


// Does given rectangle intersect this?
bool Bounds::intersects(Bounds& bounds)
{
   if (contains(bounds))
   {
      return(true);
   }
   if (bounds.contains(*this))
   {
      return(true);
   }
   if ((((min.x >= bounds.min.x) && (min.x <= bounds.max.x)) ||
        ((max.x >= bounds.min.x) && (max.x <= bounds.max.x))) &&
       (((bounds.min.z >= min.z) && (bounds.min.z <= max.z)) ||
        ((bounds.max.z >= min.z) && (bounds.max.z <= max.z))))
   {
      return(true);
   }
   if ((((bounds.min.x >= min.x) && (bounds.min.x <= max.x)) ||
        ((bounds.max.x >= min.x) && (bounds.max.x <= max.x))) &&
       (((min.z >= bounds.min.z) && (min.z <= bounds.max.z)) ||
        ((max.z >= bounds.min.z) && (max.z <= bounds.max.z))))
   {
      return(true);
   }
   return(false);
}
