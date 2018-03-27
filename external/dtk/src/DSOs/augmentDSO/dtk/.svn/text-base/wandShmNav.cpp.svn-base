/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2003  Virginia Tech
 *
 * This is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License (GPL) as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software, in the top level source directory in a file
 * named "COPYING.GPL"; if not, see it at:
 * http://www.gnu.org/copyleft/gpl.html or write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 */

/*

John Kelso, kelso@nist.gov, 3/12/08

a navigation based on offsets of the wand and the state of shared memory
files:

"wandShmNav" or whatever DTK_WANDSHMNAV is set to, 1 byte

"wand" or whatever DTK_WANDSHMNAV_PICKER is set to, 24 bytes, representing
six XYZHPR floats

when the shared memory file "wandShmNav":

- if zero, nothing happens

- if bit zero is set, navigation is based on the difference between the wand's
XYZ this frame and the last frame
    
- if bit one is set, navigation is based on the difference between the wand's
HPR this frame and the last frame
    
- if bits zero and one are set, navigation is based on the difference between the wand's
XYZHPR this frame and the last frame

the navigation XYZ is set to the difference between the XYZ the last frame
and the wand's current XYZ, multiplied by the response

the navigation HPR is set to the difference between the HPR the last frame
and the wand's current HPR, centered around the pivot point, if one exists

to support using an external app to set/unset the shared memory used by
this DSO, you can specify a command to run when activate() and
deactivate() are invoked.  This is implmented only on Linux, mostly
because that's all I use.

- the envvar DTK_WANDSHMNAV_ACTIVATE is the name of the command to run when
activate() is called

- the envvar DTK_WANDSHMNAV_DEACTIVATE is the name of the command to run
when deactivate() is called

- the envvar DTK_WANDSHMNAV_EULER specifies that the HPR angles should be
treated as Euler angles- that is, a compund rotation involving all three
angles, as opposed to each angle being applied individually.

*/

#include <math.h>

#include <dtk.h>
#if defined DTK_ARCH_LINUX || defined DTK_ARCH_IRIX
# include <values.h>
#endif

#ifdef DTK_ARCH_WIN32_VCPP
# define M_PI  (3.1415926f)
#endif

#include <dtk/dtkDSO_loader.h>

class wandShmNav : public dtkNav
{
public:
    wandShmNav(dtkManager *);
    int postFrame(void) ;
    int activate(void);
    int deactivate(void);

private:
    dtkSharedMem *_stateShm ;
    unsigned char _state ;
    unsigned char _oldState ;
    dtkSharedMem *_wandShm ;
    float _wand[6] ;
    float _oldWand[6] ;
    bool euler ;
#ifdef DTK_ARCH_LINUX
    char *_activateCommand ;
    char *_deactivateCommand ;
#endif
    
} ;

wandShmNav::wandShmNav(dtkManager *m) :
    dtkNav(m, "wandShmNav")
{
    // get the shared memory for the state- this one needs to defined
    std::string _stateShmName = "wandShmNav" ;
    if (getenv("DTK_WANDSHMNAV"))
    {
	_stateShmName = getenv("DTK_WANDSHMNAV") ;
    }
    _stateShm = new dtkSharedMem(sizeof(_state), _stateShmName.c_str()) ;
    if (_stateShm->isInvalid())
    {
	dtkMsg.add(DTKMSG_ERROR,
		   "wandShmNav::postConfig: failed:"
		   " couldn't open shared memory segment \"%s\"\n",
		   _stateShmName.c_str()) ;
	return ;
    }
    _oldState = 0 ;

    // get the shared memory for the state- this one needs to defined
    std::string _wandShmName = "wand" ;
    if (getenv("DTK_WANDSHMNAV_PICKER"))
    {
	_wandShmName = getenv("DTK_WANDSHMNAV_PICKER") ;
    }
    _wandShm = new dtkSharedMem(sizeof(_wand), _wandShmName.c_str()) ;
    if (_wandShm->isInvalid())
    {
	dtkMsg.add(DTKMSG_ERROR,
		   "wandShmNav::postConfig: failed:"
		   " couldn't open shared memory segment \"%s\"\n",
		   _wandShmName.c_str()) ;
	return ;
    }

    if (getenv("DTK_WANDSHMNAV_EULER")) euler = true ;
    else euler = false ;

#ifdef DTK_ARCH_LINUX
    _activateCommand = getenv("DTK_WANDSHMNAV_ACTIVATE") ;
    _deactivateCommand = getenv("DTK_WANDSHMNAV_DEACTIVATE") ;
#endif

    setDescription("dtk Navigation using wand and shared memory");

}

// for navigation update
int wandShmNav::postFrame(void)
{  
    _stateShm->read(&_state) ;

    if (_state)
    {
	_wandShm->read(_wand) ;

	// getting turned on after being off
	if (_state != _oldState) 
	{
	    memcpy(_oldWand, _wand, sizeof(_wand)) ;
	}


	// get current location
	dtkMatrix locMat ;
	locMat.coord(location) ;

	// do rotation
	if (_state & 2)
	{
	    // translate to pivot point, if any
	    if (_pivot)
	    {
		locMat.translate(-(*_pivot).x,
				 -(*_pivot).y,
				 -(*_pivot).z);
	    }
	    

	    dtkMatrix rotMat ;

	    if (euler)
	    {
		rotMat.rotateHPR(_oldWand[3], _oldWand[4], _oldWand[5]) ;
		rotMat.invert() ;
		rotMat.rotateHPR(_wand[3], _wand[4], _wand[5]) ;
	    }
	    else
	    {
		rotMat.rotateHPR(_wand[3]-_oldWand[3], _wand[4]-_oldWand[4], _wand[5]-_oldWand[5]) ;
	    }

	    locMat.mult(&rotMat) ;

	    
	    // translate back from pivot point, if any
	    if (_pivot)
	    {
		locMat.translate((*_pivot).x,
				 (*_pivot).y,
				 (*_pivot).z);
	    }
	    
	}

	// then translation
	if (_state & 1)
	{
	    dtkMatrix transMat ;
	    transMat.translate((_wand[0] - _oldWand[0])*_response, 
			       (_wand[1] - _oldWand[1])*_response, 
			       (_wand[2] - _oldWand[2])*_response) ;

	    locMat.mult(&transMat) ;
	}

	// and put back into location
	locMat.coord(&location) ;

	// save wand for next frame
	memcpy(_oldWand, _wand, sizeof(_wand)) ;
	
    }

    _oldState = _state ;

    return dtkNav::postFrame();
}

int wandShmNav::activate(void)
{
#ifdef DTK_ARCH_LINUX
    // reset the wand position & state
    _wandShm->read(_wand) ;
    memcpy(_oldWand, _wand, sizeof(_wand)) ;
    _oldState = 0 ;
    
    if (_activateCommand)
    {
	dtkMsg.add(DTKMSG_NOTICE,
		   "wandShmNav::activate: doing a fork/exec of \"%s\"\n", _activateCommand) ;
	pid_t pid  = fork() ;
	if (pid == -1)
	{
	    perror("picker::picker: Fork failed") ;
	    exit(1) ;
	}
	else if (pid == 0)
	{
	    int ret = execl("/bin/sh", "sh", "-c", _activateCommand, NULL) ;
	    if (ret) perror("wandShmNav::deactivate: Child failed to exec command") ;
	    exit(1) ;
	}
    }
#endif

    return dtkNav::activate() ;
}


int wandShmNav::deactivate(void)
{
#ifdef DTK_ARCH_LINUX
    if (_deactivateCommand)
    {
	dtkMsg.add(DTKMSG_NOTICE,
		   "wandShmNav::deactivate: doing a fork/exec of \"%s\"\n", _deactivateCommand) ;
	pid_t pid  = fork() ;
	if (pid == -1)
	{
	    perror("picker::picker: Fork failed") ;
	    exit(1) ;
	}
	else if (pid == 0)
	{
	    int ret = execl("/bin/sh", "sh", "-c", _deactivateCommand, NULL) ;
	    if (ret) perror("wandShmNav::deactivate: Child failed to exec command") ;
	    exit(1) ;
	}
    }
#endif

    return dtkNav::deactivate() ;
}


/************ DTK C++ dtkAugment loader/unloader functions ***************
 *
 * All DTK dtkAugment DSO files are required to declare these two
 * functions.  These function are called by the loading program to get
 * your C++ objects loaded.
 *
 *************************************************************************/

static dtkAugment *dtkDSO_loader(dtkManager *manager, void *p)
{
    return new wandShmNav(manager);
}

static int dtkDSO_unloader(dtkAugment *augment)
{
    delete augment;
    return DTKDSO_UNLOAD_CONTINUE;
}
