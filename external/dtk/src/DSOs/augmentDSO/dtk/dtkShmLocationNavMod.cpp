#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

/*!
 * @class dtkShmLocationNavMod
 *
 * @brief DSO which sets the location of the current navigation to
 * a coordinate specified in dtk shared memory.  
 *
 * the dtk shared memory file is "dtk/location", and contains either three
 * or six floats, representing XYZ (3) or XYZHPR (6).  If the shared memory
 * file doesn't exist, one of size six is created.
 *
 * NOTE: HPR is rotated around the pivot point, not just stuffed into location
 *
 * you can override the default shared memory name by specifying it in the
 * envvar DTK_SHM_LOCATION_NAV_MOD
 *
 * @author John Kelso, kelso@nist.gov
 */


class dtkShmLocationNavMod : public dtkAugment
{
public:
    dtkShmLocationNavMod(dtkManager *m) ;
    virtual int postFrame(void) ;
  
private:
    dtkManager *mgr ;
    dtkNavList *navList;
    bool first ;
    dtkSharedMem *locationShm ;
    int shmSize ;
    float location[6] ;
    float oldLocation[6] ;
    dtkCoord locationCoord ;
    dtkMatrix navMat ;
    dtkMatrix modMat ;
    dtkMatrix oldModMatInv ;
    int debug ;
} ;

dtkShmLocationNavMod::dtkShmLocationNavMod(dtkManager *m):
    dtkAugment("dtkShmLocationNavMod")
{
  
    debug = 0 ;
    mgr = m ;
    setDescription("coordinate location navigation modifier") ;
    navList = NULL;

#ifdef DTK_ARCH_WIN32
    char *locationShmName = const_cast<char*>("dtk\\location") ;
#else
    char *locationShmName = const_cast<char*>("dtk/location") ;
#endif

    if (getenv("DTK_SHM_LOCATION_NAV_MOD")) locationShmName = getenv("DTK_SHM_LOCATION_NAV_MOD") ;
  
    // see if shared memory already exists
    locationShm = new dtkSharedMem(locationShmName,0) ;
    if (locationShm->isInvalid())
    {
	locationShm = new dtkSharedMem(sizeof(float)*6, locationShmName) ;
    }
    // it does, but is of wrong size
    if (locationShm && locationShm->isValid() && 
	(locationShm->getSize()!=sizeof(float)*3) && 
	(locationShm->getSize()!=sizeof(float)*6)) 
    {
	dtkMsg.add(DTKMSG_ERROR,"dtkShmLocationNavMod: shared "
		   "memory \"%s\" exists, but wrong size.\n", locationShmName) ;
	return ;
    }
    else if (!locationShm || locationShm->isInvalid())
    {
	dtkMsg.add(DTKMSG_ERROR,"dtkShmLocationNavMod: shared "
		   "memory \"%s\" doesn't exist\n", locationShmName) ;
	locationShm = NULL ; 
    }
  
    first = true ;
    shmSize = locationShm->getSize()/sizeof(float) ;
    memset(location, 0, sizeof(location)) ;
    memset(oldLocation, 0, sizeof(oldLocation)) ;
    locationCoord.set(0.f, 0.f, 0.f, 0.f, 0.f, 0.f) ;
    validate() ;
}


int dtkShmLocationNavMod::postFrame(void)
{
    if (first)
    {

	first = false ;
      
#ifdef __APPLE__
#ifdef check
#undef check
#endif                                // check
#endif                                // __APPLE__
	navList = (dtkNavList *) mgr->check("dtkNavList", DTKNAVLIST_TYPE) ;
	if(!navList)
	{
	    navList = new dtkNavList;
	    if(mgr->add(navList))
	    {
		delete navList;
		navList = NULL;
		return ERROR_;
	    }
	}
    }
  
  
    dtkNav *currentNav = navList->current() ;
    if(currentNav)
    {
	if (locationShm)
	{
	    locationShm->read(location) ;
	    if (memcmp(oldLocation, location, sizeof(location)))
		{
		    
		    locationCoord.set(location) ;
		    if (debug)
		    {
			printf("new offset coord:") ;
			locationCoord.print() ;
		    }

		    navMat.identity() ;
		    modMat.identity() ;

		    // get current navigation
		    navMat.coord(currentNav->location) ;
		    if (debug) 
		    {
			printf("\nold nav location:") ;
			currentNav->location.print() ;
		    }

		    // get specified offset

		    //do we use a pivot point?
		    dtkCoord *pivot = currentNav->pivot() ;
		    if (pivot && shmSize==6)
		    {
			if (debug)
			{
			    printf("pivot:") ;
			    pivot->print() ;
			}
			// move to pivot position
			modMat.translate(-pivot->x,
					 -pivot->y,
					 -pivot->z);
			
			// rotate nav locator 
			modMat.rotateHPR(locationCoord.h, locationCoord.p, locationCoord.r) ;
			
			// move back from pivot position
			modMat.translate(pivot->x,
					 pivot->y,
					 pivot->z);

			// now translate by the offset
			modMat.translate(locationCoord.x, locationCoord.y, locationCoord.z) ;
			
		    }
		    else  // no pivot
		    {
			if (debug) printf("no pivot, or no hpr specified\n") ;
			modMat.coord(locationCoord) ;
		    }


		    if (debug) 
		    {
			printf("new offset:\n") ;
			modMat.print() ;
		    }
		    
		    // get new mod relative to last change
		    dtkMatrix newNavMat ;
		    if (debug) 
		    {
			printf("   1 newNavMat:\n") ;
			newNavMat.print() ;
		    }
		    newNavMat.mult(&navMat) ;
		    if (debug) 
		    {
			printf("   2 newNavMat:\n") ;
			newNavMat.print() ;
		    }
		    newNavMat.mult(&oldModMatInv) ;
		    if (debug) 
		    {
			printf("   3 newNavMat:\n") ;
			newNavMat.print() ;
		    }
		    newNavMat.mult(&modMat) ;
		    if (debug) 
		    {
			printf("   4 newNavMat:\n") ;
			newNavMat.print() ;
		    }
		    newNavMat.coord(&(currentNav->location)) ;
		    if (debug) 
		    {
			printf("new nav location = %f %f %f\n",
			       currentNav->location.x,
			       currentNav->location.y,
			       currentNav->location.z) ;
		    }
		    memcpy(oldLocation, location, sizeof(location)) ;
		    oldModMatInv.copy(&modMat) ;
		    oldModMatInv.invert() ;
		    {
			dtkCoord oldModMatInvCoord ;
			oldModMatInv.coord(&oldModMatInvCoord) ;
			if (debug) 
			{
			    printf("old mod inv:\n") ;
			    oldModMatInvCoord.print() ;
			}
		    }
		    
		}
	}

    }
  
  
    return CONTINUE;
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
    return new dtkShmLocationNavMod(manager);
}


static int dtkDSO_unloader(dtkAugment *augment)
{
    delete augment;
    return DTKDSO_UNLOAD_CONTINUE;
}
