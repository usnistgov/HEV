#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

/*!
 * @class dtkShmPivotNavMod
 *
 * @brief DSO which sets the pivot point of the current navigation to a
 * coordinate specified in dtk shared memory.  If the shared memory
 * file doesn't exist, the origin is used
 *
 * the dtk shared memory file is "dtk/pivot", and contains either three or
 * six floats, but only the first three are used
 *
 * you can override the default shared memory name by specifying it in the
 * envvar DTK_SHM_PIVOT_NAV_MOD
 *
 * @author John Kelso, kelso@nist.gov
 */


class dtkShmPivotNavMod : public dtkAugment
{
public:
  dtkShmPivotNavMod(dtkManager *m) ;
  virtual int postFrame(void) ;
  
private:
  dtkManager *mgr ;
  dtkNavList *navList;
  bool first ;
  dtkSharedMem *pivotShm ;
  float pivot[6] ;
  dtkCoord pivotCoord ;
  
} ;

dtkShmPivotNavMod::dtkShmPivotNavMod(dtkManager *m):
  dtkAugment("dtkShmPivotNavMod")
{
  
  mgr = m ;
  setDescription("coordinate pivot navigation modifier") ;
  navList = NULL;

#ifdef DTK_ARCH_WIN32
  char *pivotShmName = const_cast<char*>("dtk\\pivot") ;
#else
  char *pivotShmName = const_cast<char*>("dtk/pivot") ;
#endif

  if (getenv("DTK_SHM_PIVOT_NAV_MOD")) pivotShmName = getenv("DTK_SHM_PIVOT_NAV_MOD") ;
  
  // see if shared memory already exists
  pivotShm = new dtkSharedMem(pivotShmName,0) ;
  // it does, but is of wrong size
  if (pivotShm && pivotShm->isValid() && 
      (pivotShm->getSize()!=sizeof(float)*3) && 
      (pivotShm->getSize()!=sizeof(float)*6)) 
    {
      dtkMsg.add(DTKMSG_ERROR,"dtkShmPivotNavMod: shared "
		 "memory \"%s\" exists, but wrong size.\n", pivotShmName) ;
      return ;
    }
  else if (!pivotShm || pivotShm->isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR,"dtkShmPivotNavMod: shared "
		 "memory \"%s\" doesn't exist, using origin\n", pivotShmName) ;
      pivotShm = NULL ; 
    }
  
  first = true ;
  pivotCoord.set(0.f, 0.f, 0.f, 0.f, 0.f, 0.f) ;
  validate() ;
}


int dtkShmPivotNavMod::postFrame(void)
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
      if (pivotShm)
	{
	  pivotShm->read(pivot) ;
	  pivotCoord.set(pivot) ;
	}

      // get a pointer to the current pivot, we just copy its hpr
      dtkCoord *pivot = currentNav->pivot() ;
      if (pivot)
	{
	  pivotCoord.h = pivot->h ;
	  pivotCoord.p = pivot->p ;
	  pivotCoord.r = pivot->r ;
	}
      
      // set a non-default pivot
      currentNav->pivot(&pivotCoord) ;
      
      
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
  return new dtkShmPivotNavMod(manager);
}


static int dtkDSO_unloader(dtkAugment *augment)
{
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
}
