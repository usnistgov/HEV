/* A simple loadable dtkAugment DSO example
 *
 * This adds a preFrame() callback to the program that loads this and
 * requests to be unloaded after 6 calls to the preFrame().
 *
 * You can compile this and run `dtk-augmentManager dtkAugment' to see
 * how it works.  dtk-augmentManager is just a simple test dtkAugment
 * DSO loader program which is much simpler than the `diversifly'
 * loader program that cames with DPF
 * (http://www.diverse.vt.edu/DPF/).
 */

#include <stdio.h>
#include <dtk.h>

// dtk/dtkDSO_loader.h declares the loader and unloader functions so
// this may be loaded with a object dtkManager.
#include <dtk/dtkDSO_loader.h>


class Example : public dtkAugment
{
public:

  // Constructor
  Example(void);

  // A destructor is not required.

  // Over write the preFrame callback.
  int preFrame(void);

private:

  int count;
};

// Constructor
Example::Example(void)
{
  count = 0;
  validate(); // success
}

int Example::preFrame(void)
{
  printf("calling preFrame() for dtkAugment named %s: %s count=%d\n",
	 getName(), getDescription(), count++);

  if(count > 6)
    return REMOVE_OBJECT;
  else
    return CONTINUE;
}


/************* DTK C++ dtkAugment loader function **************
 *
 * This function has the same interface for all dtkAugment DSOs.
 */

static dtkAugment *dtkDSO_loader(dtkManager *manager, void *p)
{
  return new Example;
}

/*********** DTK C++ dtkAugment unloader function **************
 *
 * This function has the same interface for all dtkAugment DSOs.
 */

static int dtkDSO_unloader(dtkAugment *example)
{
  delete example;
  return DTKDSO_UNLOAD_CONTINUE;
}
