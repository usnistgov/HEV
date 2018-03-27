#include "hev-triangulation.h"

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


/**
* @brief Method which triangulate a set of points and save the result in a SAVG file
*
* @param output savg file to save the generated surface
* @param pts vector of the 3D SAVG points we want to triangulate
*
*/
void surfaceToSAVG(const std::string & output, std::vector<SAVG_point> pts)
{
   bool withColor = false;
   vertex_handle vh;
   Color c;
   std::ofstream file(output.c_str(), std::ios::out | std::ios::trunc);
 
   if (file)
   {
      Delaunay dt;

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

      // draws the polygons
      for(iterator it = dt.finite_faces_begin () ; it != dt.finite_faces_end () ; ++it)
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
   if ( argc != 3)
   {
      std::cout << "Usage: hev-triangulation points.savg output.savg" << std::endl;
      exit ( 0 );
   }
 
  // read the SAVG points we want to triangulate :
  std::vector<SAVG_point> pts = readSAVGfile (argv[1]);

  // triangulation
  surfaceToSAVG(argv[2], pts);

  return 0;
}
