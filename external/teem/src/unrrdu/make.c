/*
  teem: Gordon Kindlmann's research software
  Copyright (C) 2003, 2002, 2001, 2000, 1999, 1998 University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "unrrdu.h"
#include "privateUnrrdu.h"

/* bad Gordon */
extern int _nrrdReadNrrdParse_keyvalue(Nrrd *nrrd, NrrdIO *io, int useBiff);

#define INFO "Create a nrrd (or nrrd header) from scratch"
char *_unrrdu_makeInfoL =
(INFO
 ".  The data can be in one or more files, or coming from stdin. "
 "This provides an easy way of providing the bare minimum "
 "information about some data so as to wrap it in a "
 "nrrd, either to pass on for further unu processing, "
 "or to save to disk.  However, with \"-h\", this creates "
 "only a detached nrrd header file, without ever reading "
 "or writing data. When reading multiple files, each file must contain "
 "the data for one slice along the slowest axis.  Nearly all the options "
 "below refer to the finished nrrd resulting from joining all the slices "
 "together, with the exception of \"-ls\", \"-bs\", and \"-e\", which apply "
 "to every input slice file.  When reading data from many seperate files, it "
 "may be easier to put their filenames in a response file; there can be one "
 "or more filenames per line of the response file. ");

int
unrrduMakeRead(char *me, Nrrd *nrrd, NrrdIO *nio, const char *fname,
	       int lineSkip, int byteSkip, const NrrdEncoding *encoding) {
  char err[AIR_STRLEN_MED];

  nrrdIOInit(nio);
  nio->lineSkip = lineSkip;
  nio->byteSkip = byteSkip;
  nio->encoding = encoding;
  if (!( nio->dataFile = airFopen(fname, stdin, "rb") )) {
    sprintf(err, "%s:  couldn't fopen(\"%s\",\"rb\"): %s\n", 
	    me, fname, strerror(errno));
    biffAdd(me, err); return 1;
  }
  if (nrrdLineSkip(nio)) {
    sprintf(err, "%s: couldn't skip lines", me);
    AIR_FCLOSE(nio->dataFile); biffMove(me, err, NRRD); return 1;
  }
  if (!nio->encoding->isCompression) {
    if (nrrdByteSkip(nrrd, nio)) {
      sprintf(err, "%s: couldn't skip bytes", me);
      AIR_FCLOSE(nio->dataFile); biffMove(me, err, NRRD); return 1;
    }
  }
  if (nio->encoding->read(nrrd, nio)) {
    sprintf(err, "%s: error reading data", me);
    AIR_FCLOSE(nio->dataFile); biffMove(me, err, NRRD); return 1;
  }
  AIR_FCLOSE(nio->dataFile);
  return 0;
}

int
unrrdu_makeMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err, **dataFileNames, **kvp, *content, encInfo[AIR_STRLEN_LARGE];
  Nrrd *nrrd;
  Nrrd **nslice;
  int *size, nameLen, kvpLen, ki, sizeLen, spacingLen, labelLen,
    headerOnly, pret, lineSkip, byteSkip, endian, slc, type,
    encodingType, gotSpacing;
  double *spacing;
  airArray *mop;
  NrrdIO *nio;
  FILE *fileOut;
  char **label;
  const NrrdEncoding *encoding;

  /* so that long lists of filenames can be read from file */
  airStrtokQuoting = AIR_TRUE;
  hparm->respFileEnable = AIR_TRUE;
  hparm->greedySingleString = AIR_TRUE;

  mop = airMopNew();
  nio = nrrdIONew();
  airMopAdd(mop, nio, (airMopper)nrrdIONix, airMopAlways);
  nrrd = nrrdNew();
  airMopAdd(mop, nrrd, (airMopper)nrrdNuke, airMopAlways);
  
  hestOptAdd(&opt, "h", NULL, airTypeBool, 0, 0, &headerOnly, NULL,
	     "Generate header ONLY: don't write out the whole nrrd, "
	     "don't even bother reading the input data, just output the "
	     "detached nrrd header file (usually with a \".nhdr\" "
	     "extension) determined by the options below. "
	     "*NOTE*: The filename given with \"-i\" should probably start "
	     "with \"./\" to indicate that the "
	     "data file is to be found relative to the header file "
	     "(as opposed to the current working directory of whomever "
	     "is reading the nrrd).  Detached headers are incompatible with "
	     "using stdin as the data source, or using multiple data "
	     "files");
  hestOptAdd(&opt, "i", "file", airTypeString, 1, -1, &dataFileNames, "-",
	     "Filename(s) of data file(s); use \"-\" for stdin. ", &nameLen);
  hestOptAdd(&opt, "t", "type", airTypeEnum, 1, 1, &type, NULL,
	     "type of data (e.g. \"uchar\", \"int\", \"float\", "
	     "\"double\", etc.)",
	     NULL, nrrdType);
  hestOptAdd(&opt, "s", "sz0 sz1", airTypeInt, 1, -1, &size, NULL,
	     "number of samples along each axis (and implicit indicator "
	     "of dimension of nrrd)", &sizeLen);
  hestOptAdd(&opt, "sp", "sp0 sp1", airTypeDouble, 1, -1, &spacing, "nan",
	     "spacing between samples on each axis.  Use \"nan\" for "
	     "any non-spatial axes (e.g. spacing between red, green, and blue "
	     "along axis 0 of interleaved RGB image data)", &spacingLen);
  hestOptAdd(&opt, "l", "lb0 lb1", airTypeString, 1, -1, &label, "",
	     "short string labels for each of the axes", &labelLen);
  hestOptAdd(&opt, "c", "content", airTypeString, 1, 1, &content, "",
	     "Specifies the content string of the nrrd, which is built upon "
	     "by many nrrd function to record a history of operations");
  hestOptAdd(&opt, "ls", "lineskip", airTypeInt, 1, 1, &lineSkip, "0",
	     "number of ascii lines to skip before reading data");
  hestOptAdd(&opt, "bs", "byteskip", airTypeInt, 1, 1, &byteSkip, "0",
	     "number of bytes to skip (after skipping ascii lines, if any) "
	     "before reading data.  Can use \"-bs -1\" to skip a binary "
	     "header of unknown length in raw-encoded data");
  strcpy(encInfo,
	 "output file format. Possibilities include:"
	 "\n \b\bo \"raw\": raw encoding"
	 "\n \b\bo \"ascii\": ascii values, one scanline per line of text, "
	 "values within line are delimited by space, tab, or comma"
	 "\n \b\bo \"hex\": two hex digits per byte");
  if (nrrdEncodingGzip->available()) {
    strcat(encInfo, 
	   "\n \b\bo \"gzip\", \"gz\": gzip compressed raw data");
  }
  if (nrrdEncodingBzip2->available()) {
    strcat(encInfo, 
	   "\n \b\bo \"bzip2\", \"bz2\": bzip2 compressed raw data");
  }
  hestOptAdd(&opt, "e", "encoding", airTypeEnum, 1, 1, &encodingType, "raw",
	     encInfo, NULL, nrrdEncodingType);
  hestOptAdd(&opt, "en", "endian", airTypeEnum, 1, 1, &endian,
	     airEnumStr(airEndian, airMyEndian),
	     "Endianness of data; relevent for any data with value "
	     "representation bigger than 8 bits, with a non-ascii encoding: "
	     "\"little\" for Intel and "
	     "friends; \"big\" for everyone else. "
	     "Defaults to endianness of this machine",
	     NULL, airEndian);
  hestOptAdd(&opt, "kv", "key/val", airTypeString, 0, -1, &kvp, "",
	     "key/value string pairs to be stored in nrrd.  Each key/value "
	     "pair must be a single string (put it in \"\"s "
	     "if the key or the value contain spaces).  The format of each "
	     "pair is \"<key>:=<value>\", with no spaces before or after "
	     "\":=\".", &kvpLen);
  OPT_ADD_NOUT(out, "output filename");
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_makeInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);
  encoding = nrrdEncodingArray[encodingType];

  /* given the information we have, we set the fields in the nrrdIO
     so as to simulate having read the information from a header */
  if (!( AIR_IN_CL(1, sizeLen, NRRD_DIM_MAX) )) {
    fprintf(stderr, "%s: # axis sizes (%d) not in valid nrrd dimension "
	    "range ([1,%d])\n", me, sizeLen, NRRD_DIM_MAX);
    airMopError(mop);
    return 1;
  }
  gotSpacing = (spacingLen > 1 ||
		(sizeLen == 1 && AIR_EXISTS(spacing[0])));
  if (gotSpacing && spacingLen != sizeLen) {
    fprintf(stderr,
	    "%s: got different numbers of sizes (%d) and spacings (%d)\n",
	    me, sizeLen, spacingLen);
    airMopError(mop);
    return 1;
  }
  if (airStrlen(label[0]) && sizeLen != labelLen) {
    fprintf(stderr,
	    "%s: got different numbers of sizes (%d) and labels (%d)\n",
	    me, sizeLen, labelLen);
    airMopError(mop);
    return 1;
  }
  if (nameLen > 1 && nameLen != size[sizeLen-1]) {
    fprintf(stderr, "%s: got %d slice filenames but the last axis has %d "
	    "elements\n", me, nameLen, size[sizeLen-1]);
    airMopError(mop);
    return 1;
  }
  nrrd->type = type;
  nrrd->dim = sizeLen;
  nrrdAxisInfoSet_nva(nrrd, nrrdAxisInfoSize, size);
  if (gotSpacing) {
    nrrdAxisInfoSet_nva(nrrd, nrrdAxisInfoSpacing, spacing);
  }
  if (airStrlen(label[0])) {
    nrrdAxisInfoSet_nva(nrrd, nrrdAxisInfoLabel, label);
  }
  if (airStrlen(content)) {
    nrrd->content = airStrdup(content);
  }
  if (kvpLen) {
    for (ki=0; ki<kvpLen; ki++) {
      /* a hack: have to use NrrdIO->line as the channel to communicate
	 the key/value pair, since we have to emulate it having been
	 read from a NRRD header.  But because nio doesn't own the 
	 memory, we must be careful to unset the pointer prior to 
	 NrrdIONuke being called by the mop. */
      nio->line = kvp[ki];
      if (_nrrdReadNrrdParse_keyvalue (nrrd, nio, AIR_TRUE)) {
	airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
	fprintf(stderr, "%s: trouble with key/value %d \"%s\":\n%s",
		me, ki, kvp[ki], err);
	nio->line = NULL; airMopError(mop); return 1;
      }
      nio->line = NULL;
    }
  }
  
  if (headerOnly) {
    /* we don't have to fopen() any input; all we care about
       is the name of the input datafile.  We disallow stdin here */
    if (1 != nameLen) {
      fprintf(stderr, "%s: can't use detached headers with multiple "
	      "data files\n", me);
      airMopError(mop); return 1;
    }
    if (!strcmp("-", dataFileNames[0])) {
      fprintf(stderr, "%s: can't use detached headers with stdin as "
	      "data source\n", me);
      airMopError(mop); return 1;
    }
    nio->lineSkip = lineSkip;
    nio->byteSkip = byteSkip;
    nio->encoding = encoding;
    nio->dataFN = airStrdup(dataFileNames[0]);
    nio->detachedHeader = AIR_TRUE;
    nio->skipData = AIR_TRUE;
    nio->endian = endian;
    /* we open and hand off the output FILE* to the nrrd writer, which
       will not write any data, because of nio->skipData = AIR_TRUE */
    if (!( fileOut = airFopen(out, stdout, "wb") )) {
      fprintf(stderr, "%s: couldn't fopen(\"%s\",\"wb\"): %s\n", 
	      me, out, strerror(errno));
      airMopError(mop); return 1;
    }
    airMopAdd(mop, fileOut, (airMopper)airFclose, airMopAlways);
    /* whatever line and byte skipping is required will be simply
       recorded in the header, and done by the next reader */
    nrrdFormatNRRD->write(fileOut, nrrd, nio);
  } else {
    /* we're not actually using the handy unrrduHestFileCB,
       since we have to open the input data file by hand */
    if (1 == nameLen) {
      if (unrrduMakeRead(me, nrrd, nio, dataFileNames[0],
			 lineSkip, byteSkip, encoding)) {
	airMopAdd(mop, err = biffGetDone(me), airFree, airMopAlways);
	fprintf(stderr, "%s: trouble reading from \"%s\":\n%s",
		me, dataFileNames[0], err);
	airMopError(mop); return 1;
      }
    } else {
      /* create one nrrd for each slice, read them all in, then
	 join them together */
      nslice = (Nrrd **)calloc(nameLen, sizeof(Nrrd *));
      slc = 0;
      if (nslice) {
	airMopAdd(mop, nslice, airFree, airMopAlways);
	for (slc=0; slc<nameLen; slc++) {
	  nslice[slc] = nrrdNew();
	  if (nslice[slc]) {
	    airMopAdd(mop, nslice[slc], (airMopper)nrrdNuke, airMopAlways);
	    nslice[slc]->type = type;
	    nslice[slc]->dim = sizeLen-1;
	    /* the last element of size[] will be ignored */
	    nrrdAxisInfoSet_nva(nslice[slc], nrrdAxisInfoSize, size);
	  } else {
	    break;
	  }
	}
      }
      if (slc != nameLen) {
	fprintf(stderr, "%s: couldn't allocate nslice array!\n", me);
	airMopError(mop); return 1;
      }
      for (slc=0; slc<nameLen; slc++) {
	if (unrrduMakeRead(me, nslice[slc], nio, dataFileNames[slc],
			   lineSkip, byteSkip, encoding)) {
	  airMopAdd(mop, err = biffGetDone(me), airFree, airMopAlways);
	  fprintf(stderr, "%s: trouble reading from \"%s\" "
		  "(file %d of %d):\n%s",
		  me, dataFileNames[slc], slc+1, nameLen, err);
	  airMopError(mop); return 1;
	}
      }
      if (nrrdJoin(nrrd, (const Nrrd**)nslice,
		   nameLen, nrrd->dim-1, AIR_TRUE)) {
	airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
	fprintf(stderr, "%s: trouble joining slices together:\n%s",
		me, err);
	airMopError(mop); return 1;
      }
      /* unfortunately, we have to re-set some peripheral information
	 since we never bothered to set it in any nslice[i]... */
      if (gotSpacing) {
	nrrdAxisInfoSet_nva(nrrd, nrrdAxisInfoSpacing, spacing);
      }
      if (airStrlen(label[0])) {
	nrrdAxisInfoSet_nva(nrrd, nrrdAxisInfoLabel, label);
      }
      if (airStrlen(content)) {
	nrrd->content = airStrdup(content);
      }
    }
    if (1 < nrrdElementSize(nrrd)
	&& encoding->endianMatters
	&& endian != AIR_ENDIAN) {
      /* endianness exposed in encoding, and its wrong */
      nrrdSwapEndian(nrrd);
    }
    /* we are saving normally- no need to subvert nrrdSave() here;
       we just pass it the output filename */
    SAVE(out, nrrd, NULL);
  }

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(make, INFO);
