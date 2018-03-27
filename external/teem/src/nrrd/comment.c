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

#include "nrrd.h"
#include "privateNrrd.h"

/*
******** nrrdCommentAdd()
**
** Adds a given string to the list of comments
** Leading spaces (' ') and comment chars ('#') are not included.
**
** This function does NOT use biff.
*/
int
nrrdCommentAdd(Nrrd *nrrd, const char *_str) {
  char /* me[]="nrrdCommentAdd", err[512], */ *str;
  int i;
  
  if (!(nrrd && _str)) {
    /*
    sprintf(err, "%s: got NULL pointer", me);
    biffMaybeAdd(NRRD, err, useBiff);
    */
    return 1;
  }
  _str += strspn(_str, " #");
  if (!strlen(_str)) {
    /* we don't bother adding comments with no length */
    return 0;
  }
  str = airStrdup(_str);
  if (!str) {
    /*
    sprintf(err, "%s: couldn't strdup given string", me);
    biffMaybeAdd(NRRD, err, useBiff);
    */
    return 1;
  }
  /* clean out carraige returns that would screw up reader */
  airOneLinify(str);
  i = airArrayIncrLen(nrrd->cmtArr, 1);
  if (-1 == i) {
    /*
    sprintf(err, "%s: couldn't lengthen comment array", me);
    biffMaybeAdd(NRRD, err, useBiff);
    */
    return 1;
  }
  nrrd->cmt[i] = str;
  return 0;
}

/*
******** nrrdCommentClear()
**
** blows away comments, but does not blow away the comment airArray
*/
void
nrrdCommentClear(Nrrd *nrrd) {

  if (nrrd) {
    airArraySetLen(nrrd->cmtArr, 0);
  }
}

/*
******** nrrdCommentScan()
**
** looks through comments of nrrd for something of the form
** " <key> : <val>"
** There can be whitespace anywhere it appears in the format above.
** The division between key and val is the first colon which appears.
** A colon may not appear in the key. Returns a pointer to the beginning
** of "val", as it occurs in the string pointed to by the nrrd struct:
** No new memory is allocated.
**
** If there is an error (including the key is not found), returns NULL,
** but DOES NOT USE BIFF for errors (since such "errors" are more often
** than not actually problems).
*/
char *
nrrdCommentScan(Nrrd *nrrd, const char *key) {
  /* char me[]="nrrdCommentScan";  */
  int i;
  char *cmt, *k, *c, *t, *ret=NULL;

  if (!(nrrd && airStrlen(key)))
    return NULL;

  if (!nrrd->cmt) {
    /* no comments to scan */
    return NULL;
  }
  if (strchr(key, ':')) {
    /* key contains colon- would confuse later steps */
    return NULL;
  }
  for (i=0; i<nrrd->cmtArr->len; i++) {
    cmt = nrrd->cmt[i];
    /* printf("%s: looking at comment \"%s\"\n", me, cmt); */
    if ((k = strstr(cmt, key)) && (c = strchr(cmt, ':'))) {
      /* is key before colon? */
      if (!( k+strlen(key) <= c ))
	goto nope;
      /* is there only whitespace before the key? */
      t = cmt;
      while (t < k) {
	if (!isspace(*t))
	  goto nope;
	t++;
      }
      /* is there only whitespace after the key? */
      t = k+strlen(key);
      while (t < c) {
	if (!isspace(*t))
	  goto nope;
	t++;
      }
      /* is there something after the colon? */
      t = c+1;
      while (isspace(*t)) {
	t++;
      }
      if (!*t)
	goto nope;
      /* t now points to beginning of "value" string; we're done */
      ret = t;
      /* printf("%s: found \"%s\"\n", me, t); */
      break;
    }
  nope:
    ret = NULL;
  }
  return ret;
}

/*
******** nrrdCommentCopy()
**
** copies comments from one nrrd to another
**
** This does NOT use biff.
*/
int
nrrdCommentCopy(Nrrd *nout, const Nrrd *nin) {
  /* char me[]="nrrdCommentCopy", err[512]; */
  int numc, i, E;

  if (!(nout && nin)) {
    /*
    sprintf(err, "%s: got NULL pointer", me);
    biffMaybeAdd(NRRD, err, useBiff);
    */
    return 1;
  }
  if (nout == nin) {
    /* can't satisfy semantics of copying with nout==nin */
    return 2;
  }
  nrrdCommentClear(nout);
  numc = nin->cmtArr->len;
  E = 0;
  for (i=0; i<numc; i++) {
    if (!E) E |= nrrdCommentAdd(nout, nin->cmt[i]);
  }
  if (E) {
    /*
    sprintf(err, "%s: couldn't add all comments", me);
    biffMaybeAdd(NRRD, err, useBiff);
    */
    return 1;
  }
  return 0;
}
