
#ifndef _META_IMG_
#define _META_IMG_ 1

#include <string>
#include <vector>
#include <map>

#include "VolVis.h"

//
// These functions handle the reading and writing of files in the MetaImage
// format described here: 
//         http://www.itk.org/Wiki/MetaIO/Documentation
// We implemented these functions because the MetaIO library seemed to have
// several important bugs.
//
// We handle only a subset of the format.  The comments below describe
// what we handle.
//
// The MetaImage file format consists of a header section and a data section.
// The header is at the beginning of the file and the data section either
// follows that section or is in a separate file that specified in the header.
//
// We read in files that have header and data in separate files, however we
// write files only in the form in which the header and data are in the same
// file.
//
//
// The MetaImage header is very strictly formatted.
//
// Each line must have the form:
// KeyWord = [value(s)]
//
// The keywords must start at the beginning of the line, followed by a space,
// followed by '=' followed by a space, followed by the value or values.
//
// Extra whitespace before the '=' is not allowed, and the keywords are
// case sensitive.
//
//
// Keywords handled:
//    ObjectType = Image               must be "Image"
//    NDims = N                        number of dimensions; max of 10
//    DimSize = i j k ...              number of elements in each dimension
//    ElementType = MET_XXX            data type of elements.  Must be one of:
//                                       MET_CHAR
//                                       MET_UCHAR
//                                       MET_SHORT
//                                       MET_USHORT
//                                       MET_INT
//                                       MET_UINT
//                                       MET_LONG
//                                       MET_ULONG
//                                       MET_FLOAT
//                                       MET_DOUBLE
//
//
//    ElementNumberOfChannels = N     number of channels of data, default is 1
//
//    BinaryData = True | False       is data stored in binary or ascii
//
//    ElementByteOrderMSB = True | False
//    BinaryDataByteOrderMSB = True | False  
//                             Byte ordering of binary data.
//                             True means big-endian.
//                             Default is False.
//                             Documentation is unclear about the
//                             difference between these two fields.
//                             This software treats these the same on input
//                             and on output uses BinaryDataByteOrderMSB.
//
//    ElementDataFile = fileName | LOCAL    Must be the last line in header.
//                                          If "LOCAL", then the data follows 
//                                          immediately after this line.
//                                          If it's a file name.
//
//    Comment = anything
//    Comment = Levels = N          This is a non-standard extension to specify
//                                  number of MIP levels; default is 1
//
//    HeaderSize = N       Skip N bytes at beginning of data file. if -1 is 
//    specified and the data is binary, then the position of the beginning 
//    of the data is calculated relative to the end of the data file.
//
//
//
// Header lines of any other form are ignored.  No error messages are generated
// when unrecognized header lines are encountered.
//
// In particular, these keywords are ignored:
//
//    ElelmentSpacing
//    ElementSize
//    SequenceID
//    ElementMin
//    ElementMax
//    ElementSize
//    ObjectSubType
//    TransformType
//    Name
//    ID
//    ParentID
//    COlor
//    Position
//    Orientation
//    AnatomicalOrientation
//    Modality
//



namespace VolVis {


int
readMetaImage
  (
  std::string const & inFN,
  int & nDim,
  int * & dims,
  int & nComp,
  int & mipLevels,
  DataType & dType,
  void * & dataPtr ,
  std::vector<std::string> *comments = NULL
  );



int
writeMetaImage
  (
  std::string const & outFN,
  const DataEncoding encoding,
  const int nDim,
  const int dim[],
  const int nComp,
  const int mipLevels,
  const DataType dType,
  void *data ,
  const std::vector<std::string> *comments = NULL
  );



}  // end of namespace VolVis 



#endif
// _META_IMG_


