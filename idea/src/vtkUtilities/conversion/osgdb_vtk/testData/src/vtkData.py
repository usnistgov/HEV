#!/usr/bin/env python
"""
VtkData - defines classes for creating *.vtk files in Python.

Status:
  Currently only ASCII output format is supported.
  VtkData provides the following classes:
    VtkData
    CellData
    PointData
  Fully are supported the following geometry/topology types:
    StructuredPoints
    StructuredGrid
    UnstructuredGrid
    RectilinearGrid
    PolyData
  Supported are the following dataset attributes:
    Scalars (only for numComp==1, todo: binary)
    ColorScalars (not implemented)
    LookupTable (todo: binary)
    Vectors (todo: binary)
    Normals (todo: binary)
    Texture (not implemented)
    Tensors (not implemented)
    Field (todo: binary)

Copyright 2001 Pearu Peterson all rights reserved,
Pearu Peterson <pearu@ioc.ee>          
Permission to use, modify, and distribute this software is given under the
terms of the LGPL.  See http://www.fsf.org

NO WARRANTY IS EXPRESSED OR IMPLIED.  USE AT YOUR OWN RISK.
$Revision: 1.1.1.1 $
$Date: 2001/05/18 18:33:10 $
Pearu Peterson
"""

__version__ = "$Id: VtkData.py,v 1.1.1.1 2001/05/18 18:33:10 pearu Exp $"

import types,sys

def _get_3list(lst,default_lst):
    assert len(default_lst)==3
    if lst is None: return default_lst
    if type(lst) in [types.IntType,types.FloatType]:
        return [lst]+default_lst[1:]
    try:
        n = len(lst)
    except TypeError:
        raise TypeError,\
              '_get_3list: expected sequence|int|float but got %s'%(type(lst))
    return (list(lst)[:n]+default_lst[n:])[:3]

def _get_3list_list(lst,default_lst):
    assert len(default_lst)==3
    if lst is None: return [default_lst]
    if type(lst) in [types.IntType,types.FloatType]:
        return [[lst]+default_lst[1:]]
    try:
        n = len(lst)
    except TypeError:
        raise TypeError,\
              '_get_3list_list: expected sequence|int|float but got %s'%(type(lst))
    if n==0: return [default_lst]
    try:
        m = len(lst[0])
    except TypeError:
        return [_get_3list(lst,default_lst)]
    return [_get_3list(l,default_lst) for l in lst]

def _get_list(lst,default_value):
    if lst is None: return default_value
    if type(lst) in [types.IntType,types.FloatType]:
        return [lst]
    try:
        n = len(lst)
    except TypeError:
        raise TypeError,\
              '_get_list: expected sequence|int|float but got %s'%(type(lst))
    assert len(lst)
    ret = []
    for l in lst:
        assert type(l) in [types.IntType,types.FloatType]
        ret.append(l)
    return ret
def _get_intlist(lst,default_value):
    if lst is None: return default_value
    if type(lst) is types.IntType:
        return [lst]
    try:
        n = len(lst)
    except TypeError:
        raise TypeError,\
              '_get_intlist: expected intsequence|int but got %s'%(type(lst))
    assert len(lst)
    ret = []
    for l in lst:
        assert type(l) is types.IntType
        ret.append(l)
    return ret
def _get_intlist_list(lst,default_value):
    if lst is None: return default_value
    if type(lst) is types.IntType:
        return [[lst]]
    try:
        n = len(lst)
    except TypeError:
        raise TypeError,\
              '_get_intlist_list: expected intsequence|int but got %s'%(type(lst))
    assert len(lst)
    try:
        m = len(lst[0])
    except TypeError:
        return [_get_intlist(lst,default_value)]
    return [_get_intlist(l,default_value) for l in lst]

def _get_list_list(lst,default_value):
    if lst is None: return default_value
    if type(lst) in [types.IntType,types.FloatType]:
        return [[lst]]
    try:
        n = len(lst)
    except TypeError:
        raise TypeError,\
              '_get_list_list: expected sequence|int|float but got %s'%(type(lst))
    assert len(lst)
    try:
        m = len(lst[0])
    except TypeError:
        return [[l] for l in _get_list(lst,default_value)]
    return [_get_list(l,default_value) for l in lst]

def _get_datatype(d):
    if type(d) is types.IntType: return 'long'
    if type(d) is types.FloatType: return 'double'
    if type(d) is types.ListType:
        assert len(d)
        for l in d:
            t = _get_datatype(l)
            if t=='double': return t
        return t
    raise TypeError,'_get_datatype: expected int|float|list but got %s'%(type(d))

class _Messages:
    def skipmess(self,m):
        print >>sys.stderr,self.__class__.__name__+':',m,'... skipping.'    
    def errormess(self,m):
        print >>sys.stderr,self.__class__.__name__+':',m,'... giving up.'

class DataSet(_Messages):
    def _check_dimension(self):
        if len(self.dimensions)!=3:
            self.errormess('dimensions argument must be 3-list')
            raise ValueError,'dimensions=%s'%self.dimensions
        for i in range(3):
            if not self.dimensions[i]>=1:
                self.errormess('dimensions[%s] must be >= 1'%i)
                raise ValueError,'dimensions[%s]=%s'%(i,self.dimensions[i])
        if hasattr(self,'points'):
            mdims = self.get_nof_points()
            nofpoints = len(self.points)
            if mdims!=nofpoints:
                self.errormess('Inconsistent nof points (%s) relative to nof points from dimensions (%s)'%(nofpoints,mdims))
                raise ValueError,'dimensions[0]*..*dimensions[2] must be equal to len(points)'
    def _check_spacing(self):
        if len(self.spacing)!=3:
            self.errormess('spacing argument must be 3-list')
            raise ValueError,'spacing=%s'%self.spacing
        for i in range(3):
            if not self.spacing[i]>0:
                self.errormess('spacing[%s] must be > 0'%i)
                raise ValueError,'spacing[%s]=%s'%(i,self.spacing[i])
    def get_nof_points(self):
        return reduce(lambda x,y:x*y,self.dimensions,1)

class StructuredPoints(DataSet):
    def __init__(self,dimensions=[1,1,1],origin=[0,0,0],spacing=[1,1,1]):
        self.dimensions = _get_3list(dimensions,[1,1,1])
        self.origin = _get_3list(origin,[0,0,0])
        self.spacing = _get_3list(spacing,[1,1,1])
        self._check_dimension()
        self._check_spacing()
    def __str__(self):
        ret = ['DATASET STRUCTURED_POINTS',
               'DIMENSIONS %s %s %s'%tuple(self.dimensions),
               'ORIGIN %s %s %s'%tuple(self.origin),
               'SPACING %s %s %s'%tuple(self.spacing)
               ]
        return '\n'.join(ret)+'\n'


class StructuredGrid(DataSet):
    def __init__(self,dimensions=[1,1,1],points=None):
        self.dimensions = _get_3list(dimensions,[1,1,1])
        self.points = _get_3list_list(points,[0,0,0])
        self._check_dimension()
    def __str__(self):
        ret = ['DATASET STRUCTURED_GRID',
               'DIMENSIONS %s %s %s'%tuple(self.dimensions),
               'POINTS %s %s'%(len(self.points),_get_datatype(self.points))
               ]
        for p in self.points:
            ret.append('%s %s %s'%tuple(p))
        return '\n'.join(ret)+'\n'

        
class RectilinearGrid(DataSet):
    def __init__(self,x=None,y=None,z=None):
        self.x = _get_list(x,[0])
        self.y = _get_list(y,[0])
        self.z = _get_list(z,[0])
        self.dimensions = [len(self.x),len(self.y),len(self.z)]
        self._check_dimension()
    def __str__(self):
        ret = ['DATASET RECTILINEAR_GRID',
               'DIMENSIONS %s %s %s'%tuple(self.dimensions),
               'X_COORDINATES %s %s'%(self.dimensions[0],_get_datatype(self.x)),
               ' '.join(map(str,self.x)),
               'Y_COORDINATES %s %s'%(self.dimensions[1],_get_datatype(self.y)),
               ' '.join(map(str,self.y)),
               'Z_COORDINATES %s %s'%(self.dimensions[2],_get_datatype(self.z)),
               ' '.join(map(str,self.z)),
               ]
        return '\n'.join(ret)+'\n'

class PolyData(DataSet):
    vertices = None
    lines = None
    polygons = None
    triangle_strips = None

    def __init__(self,points=None,
                 vertices=None,lines=None,polygons=None,triangle_strips=None):
        self.points = _get_3list_list(points,[0,0,0])
        nof_points = len(self.points)

        ret = []
        for l in _get_intlist_list(vertices,[]):
            if not max(l)<nof_points:
                self.skipmess('Vertex points %s must be less than total nof points (%s)'%(l,nof_points))
                continue
            ret.append(l)
        if ret: self.vertices = ret

        ret = []
        for l in _get_intlist_list(lines,[]):
            if not max(l)<nof_points:
                self.skipmess('Lines points %s must be less than total nof points (%s)'%(l,nof_points))
                continue
            ret.append(l)
        if ret: self.lines = ret

        ret = []
        for l in _get_intlist_list(polygons,[]):
            if not max(l)<nof_points:
                self.skipmess('Polygon points %s must be less than total nof points (%s)'%(l,nof_points))
                continue
            ret.append(l)
        if ret: self.polygons = ret

        ret = []
        for l in _get_intlist_list(triangle_strips,[]):
            if not max(l)<nof_points:
                self.skipmess('Triangle strip points %s must be less than total nof points (%s)'%(l,nof_points))
                continue
            ret.append(l)
        if ret: self.triangle_strips = ret

    def __str__(self):
        ret = ['DATASET POLYDATA',
               'POINTS %s %s'%(len(self.points),_get_datatype(self.points))
               ]
        for p in self.points:
            ret.append('%s %s %s'%tuple(p))
        for (name,v) in [('VERTICES',self.vertices),
                         ('LINES',self.lines),
                         ('POLYGONS',self.polygons),
                         ('TRIANGLE_STRIPS',self.triangle_strips)]:
            if v:
                n = len(v)
                size = reduce(lambda x,y:x+y,[len(l) for l in v],n)
                ret.append('%s %s %s'%(name,n,size))
                ret += ['%s %s'%(len(l),' '.join(map(str,l))) for l in v]
        return '\n'.join(ret)+'\n'
    def get_nof_points(self):
        return len(self.points)

class UnstructuredGrid(DataSet):
    _vtk_cell_types_map = {'vertex':1,'poly_vertex':2,'line':3,'poly_line':4,
                           'triangle':5,'triangle_strip':6,'polygon':7,'pizel':8,
                           'quad':9,'tetra':10,'voxel':11,'hexahedron':12,
                           'wedge':13,'pyramid':14}
    _vtk_cell_nums_map = {'vertex':1,'poly_vertex':-1,'line':2,'poly_line':-1,
                           'triangle':3,'triangle_strip':-1,'polygon':-1,'pizel':4,
                           'quad':4,'tetra':4,'voxel':8,'hexahedron':8,
                           'wedge':6,'pyramid':5}
    
    def __init__(self,points=None,**cells):
        self.points = _get_3list_list(points,[0,0,0])
        nof_points = len(self.points)
        self.cells = {}
        for k,v in cells.items():
            if not self._vtk_cell_types_map.has_key(k):
                self.skipmess('Allowed keywords are %s but got %s'%(self._vtk_cell_types_map.keys(),`k`))
                continue
            if k=='vertex':
                r = [[i] for i in _get_intlist(v,[])]
            else:
                r = _get_intlist_list(cells[k],[])
            ret = []
            for l in r:
                if self._vtk_cell_nums_map[k] != -1:
                    if self._vtk_cell_nums_map[k]!=len(l):
                        self.skipmess('Cell type %s requires %s points but got %s'%(`k`,self._vtk_cell_nums_map[k],l))
                        continue
                if not max(l)<nof_points:
                    self.skipmess('Cell %s points %s must be less that total nof points (%s)'%(`k`,l,nof_points))
                    continue
                ret.append(l)
            if ret:
                self.cells[k] = ret

    def __str__(self):
        ret = ['DATASET UNSTRUCTURED_GRID',
               'POINTS %s %s'%(len(self.points),_get_datatype(self.points))]
        for p in self.points:
            ret.append('%s %s %s'%tuple(p))
        n = 0
        size = 0
        cells,cell_types = [],[]
        for k in self.cells.keys():
            for l in self.cells[k]:
                n += 1
                size += len(l)
                cells.append('%s %s'%(len(l),' '.join(map(str,l))))
                cell_types.append(str(self._vtk_cell_types_map[k]))
        if size:
            ret.append('CELLS %s %s'%(n,size+n))
            ret += cells
            ret.append('CELL_TYPES %s'%(n))
            ret += cell_types
        return '\n'.join(ret)+'\n'
    def get_nof_points(self):
        return len(self.points)

def _to_ascii(data,sep='\n'):
    if type(data) in [types.IntType,types.FloatType]:
        return str(data)
    return sep.join(map(_to_ascii,data,len(data)*[' ']))
def _to_binary(data):
    raise NotImplementedError
    if type(data) in [types.IntType,types.FloatType]:
        return str(data)
    return ''.join(map(_to_binary,data))

class DataSetAttr(_Messages):
    counters = {}
    def _get_default_name(self):
        n = self.__class__.__name__
        try:
            self.counters[n] += 1
        except KeyError:
            self.counters[n] = 0
        return self.__class__.__name__+str(self.counters[n])
    def __len__(self):
        return len(self.data)
    def __str__(self):
        return self.to_string()
    def _get_name(self,name):
        if name is None:
            return self._get_default_name()
        else:
            if type(name) is not types.StringType:
                self.errormess('name must be string')
                raise TypeError,'name=%s'%(type(name))
            name = name.strip().replace(' ','_')
            if not name:
                self.errormess('name must be nonempty string')
                raise ValueError,'name=%s'%(`name`)
            return name
    def _get_lookup_table(self,name):
        if name is None: return 'default'
        if type(name) is not types.StringType:
            self.errormess('lookup_table must be string')
            raise TypeError,'lookup_table=%s'%(type(name))
        name = name.strip().replace(' ','_')
        if not name:
            self.errormess('lookup_table must be nonempty string')
            raise ValueError,'lookup_table=%s'%(`name`)
        return name

class Scalars(DataSetAttr):
    nof_components = 1

    def __init__(self,data,name=None,lookup_table='default'):
        self.name = self._get_name(name)
        self.lookup_table = self._get_lookup_table(name)
        self.data = _get_list(data,[])

    def to_string(self,format='ascii'):
        ret = ['SCALARS %s %s %s'%(self.name,
                                   _get_datatype(self.data),
                                   self.nof_components),
               'LOOKUP_TABLE %s'%(self.lookup_table)]
        if format == 'binary':
            ret.append(_to_binary(self.data))
        else:
            ret.append(_to_ascii(self.data))
        return '\n'.join(ret)+'\n'

class LookupTable(DataSetAttr):
    def __init__(self,data,name=None):
        self.name = self._get_name(name)
        self.data = _get_list_list(data,[0,0,0,1])
    def to_string(self,format='ascii'):
        ret = ['LOOKUP_TABLE %s %s'%(self.name,len(self.data))]
        if format == 'binary':
            ret.append(_to_binary(self.data))
        else:
            ret.append(_to_ascii(self.data))
        return '\n'.join(ret)+'\n'

class Normals(DataSetAttr):
    def __init__(self,data,name=None):
        self.name = self._get_name(name)
        self.data = _get_3list_list(data,[0,0,0])
    def to_string(self,format='ascii'):
        ret = ['NORMALS %s %s'%(self.name,_get_datatype(self.data))]
        if format == 'binary':
            ret.append(_to_binary(self.data))
        else:
            ret.append(_to_ascii(self.data))
        return '\n'.join(ret)+'\n'

class Vectors(DataSetAttr):
    def __init__(self,data,name=None):
        self.name = self._get_name(name)
        self.data = _get_3list_list(data,[0,0,0])
    def to_string(self,format='ascii'):
        ret = ['VECTORS %s %s'%(self.name,_get_datatype(self.data))]
        if format == 'binary':
            ret.append(_to_binary(self.data))
        else:
            ret.append(_to_ascii(self.data))
        return '\n'.join(ret)+'\n'

class Field(DataSetAttr):
    def __init__(self,name=None,**kws):
        self.name = self._get_name(name)
        length = None
        data = {}
        for k,v in kws.items():
            l = _get_list_list(v,[])
            if length is None:
                length = len(l)
            elif length != len(l):
                self.skipmess('array must be of length %s but got %s'%(length,len(l)))
                continue
            data[k] = l
        self.length = length
        self.data = data
    def __len__(self):
        return self.length
    def to_string(self,format='ascii'):
        ret = ['FIELD %s %s'%(self.name,len(self.data))]
        for k,v in self.data.items():
            ret.append('%s %s %s %s'%(k,len(v[0]),len(v),_get_datatype(v)))
            ret.append(_to_ascii(v))
        return '\n'.join(ret)+'\n'

class Data(_Messages):
    data_type = None
    def __init__(self,*args):
        if self.__class__.__name__ not in ['PointData','CellData']:
            raise TypeError,'use PointData or CellData instead of Data'
        if not args:
            raise TypeError,self.__class__.__name__+'() takes at least 1 argument'
        args = list(args)
        length = None
        for a in args:
            if type(a) is not types.InstanceType or not isinstance(a,DataSetAttr):
                self.skipmess('expected DataSetAttr argument but got %s'%(type(a)))
                continue
            if length is None:
                length = len(a)
            elif length != len(a):
                self.skipmess('attribute data %s must be of length %s but got %s'%(`a.__class__.__name__`,length,len(a)))
                continue
        self.length = length
        self.data = args
    def get_nof_points(self):
        return self.length
    def to_string(self,format='ascii'):
        ret = []
        ret.append('%s %s'%(self.data_type,self.length))
        for a in self.data:
            ret.append(a.to_string(format))
        return '\n'.join(ret)+'\n'

class PointData(Data):
    data_type = 'POINT_DATA'

class CellData(Data):
    data_type = 'CELL_DATA'

class VtkData(_Messages):
    structure = None
    point_data = None
    cell_data = None
    def _get_title(self,dict):
        try:
            t = dict['title']
        except KeyError:
            return 'Really cool data'
        if type(t) is not types.StringType:
            self.errormess('title keyword must be string')
            raise TypeError,'title=%s'%(t)
        return t.rstrip()[:256]
    def _get_format(self,dict):
        try:
            t = dict['format']
        except KeyError:
            return 'ascii'
        if t not in ['ascii','binary']:
            self.errormess('format must be ascii|binary')
            raise TypeError,'format=%s'%(t)
        return t

    def __init__(self,*args,**kws):
        self.title = self._get_title(kws)
        self.format = self._get_format(kws)
        for a in args:
            if isinstance(a,DataSet):
                self.structure = a
            elif isinstance(a,PointData):
                self.point_data = a
            elif isinstance(a,CellData):
                self.cell_data = a
            else:
                self.skipmess('expecting DataSet|PointData|CellData but got %s'%(type(a)))
        if self.structure is None:
            raise TypeError,self.__class__.__name__+'expects exactly one DataSet argument'
        if self.point_data is not None and self.structure.get_nof_points() != self.point_data.get_nof_points():
            self.errormess('size of structure = %s'%(self.structure.get_nof_points()))
            self.errormess('size of point_data = %s'%(self.point_data.get_nof_points()))
            raise ValueError,'mismatch of structure and point_data sizes'
        if self.cell_data is not None and self.structure.get_nof_points() != self.point_data.get_nof_points():
            self.errormess('size of structure = %s'%(self.structure.get_nof_points()))
            self.errormess('size of cell_data = %s'%(self.cell_data.get_nof_points()))
            raise ValueError,'mismatch of structure and cell_data sizes'
    def to_file(self,filename):
        """Save VtkData to file `filename[.vtk]'.
        """
        if type(filename) is not types.StringType:
            raise TypeError,'filename must be string but got %s'%(type(filename))
        filename = filename.strip()
        if not filename:
            raise ValueError,'filename must be non-empty string'
        if filename[-4:]!='.vtk':
            filename += '.vtk'
        f = open(filename,'w')
        f.write('# vtk DataFile Version 2.0\n')
        f.write(self.title+'\n')
        f.write(self.format.upper()+'\n')
        f.write(str(self.structure)+'\n')
        if self.point_data:
            f.write(self.point_data.to_string(self.format))
        if self.cell_data:
            f.write(self.cell_data.to_string(self.format))
        f.close()

def _example1():
    """
    First example in 'VTK File Formats'.
    """
    structure = PolyData(points=[[0,0,0],[1,0,0],[1,1,0],[0,1,0],
                                 [0,0,1],[1,0,1],[1,1,1],[0,1,1]],
                         polygons=[[0,1,2,3],[4,5,6,7],[0,1,5,4],
                                   [2,3,7,6],[0,4,7,3],[1,2,6,5]])
    point_values = Scalars([0,1,2,3,4,5,6,7],name='sample_scalars',lookup_table='my_table')
    my_table = LookupTable([[0,0,0,1],[1,0,0,1],[0,1,0,1],[1,1,0,1],
                            [0,0,1,1],[1,0,1,1],[0,1,1,1],[1,1,1,1]],name='my_table')
    cell_values = Scalars([0,1,2,3,4,5],name='cell_scalars')
    cell_normals = Normals([[0,0,-1],[0,0,1],[0,-1,0],
                            [0,1,0],[-1,0,0],[1,0,0]],name='cell_normals')
    field = Field(#'FieldData',
                  cellIds=[0,1,2,3,4,5],
                  faceAttributes=[[0,1],[1,2],[2,3],[3,4],[4,5],[5,6]])

    p = PointData(point_values,my_table)
    c = CellData(cell_values,cell_normals,field)
    vtk = VtkData(structure,p,c)
    vtk.to_file('example1')

def _example2():
    """
    Second example in 'VTK File Formats'.
    """
    vtk = VtkData(StructuredPoints(dimensions=[3,4,6]),
                  PointData(Scalars([0,0,0,0,0,0,0,0,0,0,0,0,
                                     0,5,10,15,20,25,25,20,15,10,5,0,
                                     0,10,20,30,40,50,50,40,30,20,10,0,
                                     0,10,20,30,40,50,50,40,30,20,10,0,
                                     0,5,10,15,20,25,25,20,15,10,5,0,
                                     0,0,0,0,0,0,0,0,0,0,0,0
                                     ])))
    vtk.to_file('example2')

def _example3():
    """
    Third example in 'VTK File Formats'.
    """
    points = [[0,0,0],[1,0,0],[2,0,0],[0,1,0],[1,1,0],[2,1,0],
              [0,0,1],[1,0,1],[2,0,1],[0,1,1],[1,1,1],[2,1,1],
              [0,1,2],[1,1,2],[2,1,2],[0,1,3],[1,1,3],[2,1,3],
              [0,1,4],[1,1,4],[2,1,4],[0,1,5],[1,1,5],[2,1,5],
              [0,1,6],[1,1,6],[2,1,6]
              ]
    vectors = [[1,0,0],[1,1,0],[0,2,0],[1,0,0],[1,1,0],[0,2,0],
               [1,0,0],[1,1,0],[0,2,0],[1,0,0],[1,1,0],[0,2,0],
               [0,0,1],[0,0,1],[0,0,1],[0,0,1],[0,0,1],[0,0,1],
               [0,0,1],[0,0,1],[0,0,1],[0,0,1],[0,0,1],[0,0,1],
               [0,0,1],[0,0,1],[0,0,1]
               ]
    vtk = VtkData(\
        UnstructuredGrid(points,
                         hexahedron=[[0,1,4,3,6,7,10,9],
                                     [1,2,5,4,7,8,11,10]],
                         tetra=[[6,10,9,12],
                                [5,11,10,14]],
                         polygon=[15,16,17,14,13,12],
                         triangle_strip=[18,15,19,16,20,17],
                         quad=[22,23,20,19],
                         triangle=[[21,22,18],
                                   [22,19,18]],
                         line=[26,25],
                         vertex=[24]
                         ),
        PointData(Scalars(range(27)),
                  Vectors(vectors)),
        title = 'Unstructured Grid Example'
        )
    vtk.to_file('example3')

if __name__ == "__main__":
    _example1()
    _example2()
    _example3()

