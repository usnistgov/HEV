#include <iris.h>

bool headActive = false ;
dtkSharedMem* headShm = NULL ;
float head[6] ;
float oldHead[6] ;

bool wandActive = false ;
dtkSharedMem* wandShm = NULL ;
float wand[6] ;
float oldWand[6] ;

bool joystickActive = false ;
dtkSharedMem* joystickShm = NULL ;
float joystick[2] ;
float oldJoystick[2] ;

float headT = .1 ;
float headR = 1.8 ;

float wandT = .1 ;
float wandR = 1.8 ;

float joystickT = .9 ;

void usage()
{
    fprintf(stderr,"Usage: hev-trackerMonitor [ --head t r ] [ --wand t r ] [ --joystick t ]\n") ;
}

int main(int argc, char** argv)
{
    if (argc == 1)
    {
	headActive = wandActive = joystickActive = true ;
    }
    else
    {
	int i = 1 ;
	while (i<argc)
	{
	    if (iris::IsSubstring("--head", argv[i], 3))
	    {
		i++ ;
		if (i+1<argc && iris::StringToFloat(argv[i], &headT) && headT > 0.f &&
		    iris::StringToFloat(argv[i+1], &headR) && headR > 0.f)
		{
		    headActive = true ;
		    i+=2 ;
		}
		else
		{
		    dtkMsg.add(DTKMSG_ERROR, "hev-trackerMonitor: invalid delta value\n") ;
		    usage() ;
		    return 1 ;
		}
	    }
	    else if (iris::IsSubstring("--wand", argv[i], 3))
	    {
		i++ ;
		if (i+1<argc && iris::StringToFloat(argv[i], &wandT) && wandT > 0.f &&
		    iris::StringToFloat(argv[i+1], &wandR) && wandR > 0.f)
		{
		    wandActive = true ;
		    i+=2 ;
		}
		else
		{
		    dtkMsg.add(DTKMSG_ERROR, "hev-trackerMonitor: invalid delta value\n") ;
		    usage() ;
		    return 1 ;
		}
	    }
	    else if (iris::IsSubstring("--joystick", argv[i], 3))
	    {
		i++ ;
		if (i<argc && iris::StringToFloat(argv[i], &joystickT) && joystickT > 0.f)
		{
		    joystickActive = true ;
		    i++ ;
		}
		else
		{
		    dtkMsg.add(DTKMSG_ERROR, "hev-trackerMonitor: invalid delta value\n") ;
		    usage() ;
		    return 1 ;
		}
	    }
	    else
	    {
		usage() ;
		return 1 ;
	    }
	}
    }

    std::string msg = "hev-trackerMonitor:" ;

    if (headActive)
    {
	headShm = new dtkSharedMem(sizeof(head), "head", DTK_CONNECT) ;
	if (headShm->isInvalid())
	{
	    usage() ;
	    return 1 ;
	}
	headShm->read(head) ;
	memcpy(oldHead, head, sizeof(head)) ;
	msg += "  head: T = " + iris::FloatToString(headT) + " R = " + iris::FloatToString(headR) ;
    }

    if (wandActive)
    {
	wandShm = new dtkSharedMem(sizeof(wand), "wand", DTK_CONNECT) ;
	if (wandShm->isInvalid())
	{
	    usage() ;
	    return 1 ;
	}
	wandShm->read(wand) ;
	memcpy(oldWand, wand, sizeof(wand)) ;
	msg += "  wand: T = " + iris::FloatToString(wandT) + " R = " + iris::FloatToString(wandR) ;
    }

    if (joystickActive)
    {
	joystickShm = new dtkSharedMem(sizeof(joystick), "joystick", DTK_CONNECT) ;
	if (joystickShm->isInvalid())
	{
	    usage() ;
	    return 1 ;
	}
	joystickShm->read(joystick) ;
	memcpy(oldJoystick, joystick, sizeof(joystick)) ;
	msg += "  joystick: T = " + iris::FloatToString(joystickT) ;
    }

    dtkMsg.add(DTKMSG_INFO, "%s\n",msg.c_str()) ;
    
    time_t t ;
    char c[26] ;
    while (1)
    {
	t = time(NULL) ;
	ctime_r(&t, c) ;
	c[strlen(c)-1] = '\0' ;
	if (headActive)
	{
	    headShm->read(head) ;
	    for (int i=0; i<3; i++)
	    {
		if (fabs(head[i]-oldHead[i])>=headT)
		{
		    printf("%s: head[%d]: %f %f (%f)\n",c,i,oldHead[i],head[i],head[i]-oldHead[i]) ;
		}
	    }
	    for (int i=3; i<6; i++)
	    {
		if (fabs(head[i]-oldHead[i])>=headR)
		{
		    printf("%s: head[%d]: %f %f (%f)\n",c,i,oldHead[i],head[i],head[i]-oldHead[i]) ;
		}
	    }
	    memcpy(oldHead, head, sizeof(head)) ;
	}
	if (wandActive)
	{
	    wandShm->read(wand) ;
	    for (int i=0; i<3; i++)
	    {
		if (fabs(wand[i]-oldWand[i])>=wandT)
		{
		    printf("%s: wand[%d]: %f %f (%f)\n",c,i,oldWand[i],wand[i],wand[i]-oldWand[i]) ;
		}
	    }
	    for (int i=3; i<6; i++)
	    {
		if (fabs(wand[i]-oldWand[i])>=wandR)
		{
		    printf("%s: wand[%d]: %f %f (%f)\n",c,i,oldWand[i],wand[i],wand[i]-oldWand[i]) ;
		}
	    }
	    memcpy(oldWand, wand, sizeof(wand)) ;
	}
	if (joystickActive)
	{
	    joystickShm->read(joystick) ;
	    for (int i=0; i<2; i++)
	    {
		if (fabs(joystick[i]-oldJoystick[i])>=joystickT)
		{
		    printf("%s: joystick[%d]: %f %f (%f)\n",c,i,oldJoystick[i],joystick[i],joystick[i]-oldJoystick[i]) ;
		}
	    }
	    memcpy(oldJoystick, joystick, sizeof(joystick)) ;
	}
	fflush(stdout) ;
	usleep(iris::GetUsleep()) ;	
    }

    return 0 ;
}
