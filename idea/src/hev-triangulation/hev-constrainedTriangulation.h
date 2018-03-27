/**
* @file triangulation.h
* @author Audrey Lemoussu
* @date March 2009
* 
*/

#ifndef HEV_CONSTRAINEDTRIANGULATION_H
#define HEV_CONSTRAINEDTRIANGULATION_H

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include "Projection_traits_xy_3.h"
#include <CGAL/Constrained_Delaunay_triangulation_2.h>

#include <fstream>

typedef struct
{
   double r, g, b, a;
} Color;

// CGAL structures
struct K : CGAL::Exact_predicates_inexact_constructions_kernel {};
typedef CGAL::Projection_traits_xy_3<K>				PGt;
typedef CGAL::Triangulation_vertex_base_with_info_2<Color, PGt>	Vb;

template < class GT, class Fb = CGAL::Constrained_triangulation_face_base_2<PGt> >
class My_face_base : public Fb
{
public:
  typedef typename Fb::Vertex_handle  Vertex_handle;
  typedef typename Fb::Face_handle    Face_handle;

  template < class TDS2 >
  struct Rebind_TDS {
    typedef typename Fb::template Rebind_TDS<TDS2>::Other  Fb2;
    typedef My_face_base<GT, Fb2>                        Other;
  };

  My_face_base() { TYPE = 0; }

  My_face_base(Vertex_handle v0, Vertex_handle v1, Vertex_handle v2)
    : Fb(v0, v1, v2) { TYPE = 0; }

  My_face_base(Vertex_handle v0, Vertex_handle v1, Vertex_handle v2, Face_handle n0, Face_handle n1, Face_handle n2)
    : Fb(v0,v1,v2,n0,n1,n2) { TYPE = 0; }

  My_face_base(Vertex_handle v0, Vertex_handle v1, Vertex_handle v2, Face_handle n0, Face_handle n1, Face_handle n2, bool c0, bool c1, bool c2 )
    : Fb(v0,v1,v2,n0,n1,n2) { TYPE = 0; }

private:
  int TYPE; // 0 unknown, 1 inside, 2 outside

public:
  void set_type (const int & t)  { TYPE = t; }
  int get_type () { return TYPE; } 
};

typedef My_face_base<K>						MFb;
typedef CGAL::Triangulation_data_structure_2<Vb,MFb>		TDS;
typedef CGAL::Constrained_Delaunay_triangulation_2<PGt, TDS>	ConstrainedDelaunay;
typedef ConstrainedDelaunay::Finite_faces_iterator		constrained_faces_iterator;
typedef ConstrainedDelaunay::Face_handle			face_handle;
typedef ConstrainedDelaunay::Vertex_handle			vertex_handle;
typedef ConstrainedDelaunay::Finite_vertices_iterator		constrained_vertices_iterator;

typedef K::Point_3   Point;


typedef struct
{
   Point p;
   Color c;
} SAVG_point;


std::vector<SAVG_point> readSAVGfile (const std::string & filename);
void surfaceToSAVG(const std::string & output, std::vector<SAVG_point> pts, std::vector<SAVG_point> constraints);

#endif
