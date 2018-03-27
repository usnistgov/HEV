/**
* @file triangulation.h
* @author Audrey Lemoussu
* @date March 2009
* 
*/

#ifndef HEV_TRIANGULATION_H
#define HEV_TRIANGULATION_H

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Triangulation_euclidean_traits_xy_3.h>
#include <CGAL/Delaunay_triangulation_2.h>

#include <fstream>

typedef struct
{
   double r, g, b, a;
} Color;

// CGAL structures
struct K : CGAL::Exact_predicates_inexact_constructions_kernel {};
typedef CGAL::Triangulation_euclidean_traits_xy_3<K>		Gt;
typedef CGAL::Triangulation_vertex_base_with_info_2<Color, Gt>	Vb;
typedef CGAL::Triangulation_face_base_2<K>			Fb;
typedef CGAL::Triangulation_data_structure_2<Vb,Fb>		TDS;
typedef CGAL::Delaunay_triangulation_2<Gt, TDS>			Delaunay;
typedef Delaunay::Vertex_handle					vertex_handle;
typedef Delaunay::Finite_faces_iterator				iterator;

typedef K::Point_3   Point;


typedef struct
{
   Point p;
   Color c;
} SAVG_point;


std::vector<SAVG_point> readSAVGfile (const std::string & filename);
void surfaceToSAVG(const std::string & output, std::vector<SAVG_point> pts);

#endif
