#include "hev-constrainedTriangulation.h"

#include <vector>
#include <sstream>


/**
*
* @brief Read a set of 3D points in a SAVG file
*
*/
std::vector<SAVG_point> readSAVGfile (const std::string & filename)
{
   std::vector<SAVG_point> L;
   double pt[3];
   Color c;
   char line[256];
   SAVG_point savg_pt;
   std::ifstream file(filename.c_str());  // open the file

    if(!file.is_open())
   {
      std::cerr << "bad file name, the file doesn't exist !" << std::endl;
      exit(-1);
   }
   else
   {
      while (!file.eof())
      {
         file.getline(line,256);
         if ( strncmp(line,"poi",3) == 0 || strncmp(line,"lin",3) == 0 )
         {
            while(!file.eof() && strncmp(line,"",1))
            {
               file.getline(line,256);
               if (strncmp(line,"",1))
               {
                  std::istringstream iss(line);
                  iss >> pt[0] >> pt[1] >> pt[2];
                  if (iss.rdbuf()->in_avail())
                     iss >> c.r >> c.g >> c.b >> c.a;
                  else
                  {
                     c.r = -1;
                  }
                  savg_pt.p = Point(pt[0],pt[1],pt[2]);
                  savg_pt.c = c;
                  L.push_back(savg_pt);
               }
            }
         }
      }
      file.close();
   }
   return L;
}


// Recursive function which complete the type of the 3 triangles in the neighborhood of fh 
// 1 if the triangle is inside the basin
// 2 otherwise
//
// @param fh face_handle (one triangle of the triangulation)
void fillNeighborsType(face_handle fh)
{
   if (fh->neighbor(0)->get_type() == 0)
   {
      if (fh->is_constrained (0))
      {
         if (fh->get_type() == 1) fh->neighbor(0)->set_type(2);
         else if (fh->get_type() == 2) fh->neighbor(0)->set_type(1);
      }
      else
      {
         fh->neighbor(0)->set_type(fh->get_type());
      }
      fillNeighborsType(fh->neighbor(0));
   }
   
   if(fh->neighbor(1)->get_type() == 0)
   {
      if (fh->is_constrained (1))
      {
         if (fh->get_type() == 1) fh->neighbor(1)->set_type(2);
         else if (fh->get_type() == 2) fh->neighbor(1)->set_type(1);
      }
      else
      {
         fh->neighbor(1)->set_type(fh->get_type());
      }
      fillNeighborsType(fh->neighbor(1));
   }

   if(fh->neighbor(2)->get_type() == 0)
   {
      if (fh->is_constrained (2))
      {
         if (fh->get_type() == 1) fh->neighbor(2)->set_type(2);
         else if (fh->get_type() == 2) fh->neighbor(2)->set_type(1);
      }
      else
      {
         fh->neighbor(2)->set_type(fh->get_type());
      }
      fillNeighborsType(fh->neighbor(2));
   }
}


/**
* @brief Method which triangulate a set of points and save the result in a SAVG file
*
* @param output savg file to save the generated surface
* @param pts vector of the 3D SAVG points we want to triangulate
* @param constraints vector of the 3D SAVG points used for the constraints
*
*/
void surfaceToSAVG(const std::string & output, std::vector<SAVG_point> pts, std::vector<SAVG_point> constraints)
{
   bool withColor = false;
   vertex_handle vh;
   Color c;
   std::ofstream file(output.c_str(), std::ios::out | std::ios::trunc);
 
   if (file)
   {
      ConstrainedDelaunay dt;

      // determines if points have a color component or not
      if ((pts.begin ())->c.r >= 0 && (pts.begin ())->c.r <= 1 && (pts.begin ())->c.g >= 0 && (pts.begin ())->c.g <= 1 && (pts.begin ())->c.b >= 0 && (pts.begin ())->c.b <= 1 && (pts.begin ())->c.a >= 0 && (pts.begin ())->c.a <= 1)
         withColor = true;

      // inserts 3D points
      for(std::vector<SAVG_point>::iterator it = pts.begin () ; it != pts.end () ; ++it)
      {
         vh = dt.push_back(it->p);
         if (withColor)
            vh->info() = it->c;
      }

      // inserts constraints (boundary edges)
      std::vector<SAVG_point>::iterator it = constraints.begin ();
      std::vector<SAVG_point>::iterator it2 = constraints.begin ();
      ++it2;
      while(it2 != constraints.end ())
      {
         dt.insert_constraint (it->p, it2->p);
         ++it;
         ++it2;
      }

      // traverses the triangulation to find the triangles which are outside the domain
        // init : find a face which has a constrained edge and an infinite face as neighbor (so this face is "on the convex hull" of the triangulation)
      face_handle fh = dt.faces_begin();
      while ( ( !fh->is_constrained(0) && !fh->is_constrained(1) && !fh->is_constrained(2) ) || ( !dt.is_infinite(fh->neighbor(0)) && !dt.is_infinite(fh->neighbor(1)) && !dt.is_infinite(fh->neighbor(2)) ) )
      {
         fh++;
      }
      if ((fh->is_constrained(0) && dt.is_infinite(fh->neighbor(0))) || (fh->is_constrained(1) && dt.is_infinite(fh->neighbor(1))) || (fh->is_constrained(2) && dt.is_infinite(fh->neighbor(2))))
      {
         fh->set_type(1); // face inside boundary
      }
      else
      {
         fh->set_type(2); // face outside
      }
         // recursive traversal
      fillNeighborsType(fh);


      // draws the polygons
      for(constrained_faces_iterator it = dt.finite_faces_begin () ; it != dt.finite_faces_end () ; ++it)
      {
         if (it->get_type() == 1)
	 {
            if (withColor)
            {
               file << "polygons " << std::endl;
               c = (*it).vertex(0)->info(); 
               file << (*it).vertex(0)->point() << " " << c.r << " " << c.g << " " << c.b << " " << c.a << std::endl;
               c = (*it).vertex(1)->info(); 
               file << (*it).vertex(1)->point() << " " << c.r << " " << c.g << " " << c.b << " " << c.a << std::endl;
               c = (*it).vertex(2)->info(); 
               file << (*it).vertex(2)->point() << " " << c.r << " " << c.g << " " << c.b << " " << c.a << std::endl;
               file << std::endl;
            }
            else
            {
               file << "polygons 1 1 1 1" << std::endl;
               file << (*it).vertex(0)->point() << std::endl;
               file << (*it).vertex(1)->point() << std::endl;
               file << (*it).vertex(2)->point() << std::endl;
               file << std::endl;
            }
         }
      }
      file.close();
   }
   else
   {
      std::cerr << "void surfaceToSAVG(...) : Can't open the " << output << " file !" << std::endl;
      exit(-1);
   }
}

int main(int argc, char ** argv)
{
  if (argc != 4)
  {
     std::cout << "Usage: hev-constrainedTriangulation points.savg boundary.savg output.savg" << std::endl;
     exit( 0 );
  }

  // read the SAVG points we want to triangulate :
  std::vector<SAVG_point> pts = readSAVGfile (argv[1]);

  // read the boundary points used for the constraints
  std::vector<SAVG_point> constraints = readSAVGfile (argv[2]);

  // constrained triangulation
  surfaceToSAVG(argv[3], pts, constraints);

  return 0;
}

