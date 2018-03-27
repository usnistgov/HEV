#include <iris.h>

////////////////////////////////////////////////////////////////////////
void float9(char* s, float f) {
    float a ; 
    a = f<0?-f:f ;
    
    if (a==0.0f) 
	sprintf(s,"%-+8.6f",f) ;
    else if (a>=100000000.f || a<=.0001f)
	sprintf(s,"%-+8.2e",f) ;
    else if (a<10.f)
	sprintf(s,"%-+8.6f",f) ;
    else if (a<100.f)
	sprintf(s,"%-+8.5f",f) ;
    else if (a<1000.f)
	sprintf(s,"%-+8.4f",f) ;
    else if (a<10000.f)
	sprintf(s,"%-+8.3f",f) ;
    else if (a<100000.f)
	sprintf(s,"%-+8.2f",f) ;
    else if (a<1000000.f)
	sprintf(s,"%-+8.1f",f) ;
    else if (a<10000000.f)
	sprintf(s,"%-+#8.0f",f) ;
    else if (a<100000000.f)
	sprintf(s,"%-+8.0f",f) ;
}

////////////////////////////////////////////////////////////////////////
// add options for --all --list --loop and specific files on command line

int main(int argc, char **argv)
{
    // send messages to stderr
    dtkMsg.setFile(stderr) ;

    if (argc != 1 && argc != 3) 
    {
	dtkMsg.add(DTKMSG_ERROR, "iris-readState: invalid number of arguments\n") ;
	return 1 ;
    }

    int ticks = iris::GetUsleep() ;
    int c = 1 ;
    if (argc == 3)
    {
	if (iris::IsSubstring("--usleep",argv[c],3))
	{
	    c++ ;
	    if (!iris::StringToInt(argv[c],&ticks)) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-readState: invalid usleep value\n") ;
		return 1 ; 
	    }
	    c++ ;
	}
	else
	{
	    fprintf(stderr,"iris-readState: invalid option\n") ;
	    return 1 ;
	}
    }

    iris::ShmState shmState  ;

    iris::ShmState::DataElement d ;

    std::map<std::string, iris::ShmState::DataElement>::iterator pos ;
    while (1)
    {
	// if you just want to find one
	//pos = shmState.getMap()->find(name) ;
	
	// list all shared memory files
	for (pos=shmState.getMap()->begin(); pos != shmState.getMap()->end(); pos++)
	{
	    std::string name = pos->first ;
	    if (pos != shmState.getMap()->end())
	    {
		iris::ShmState::DataElement d = pos->second ;
		if (d.type == iris::ShmState::MATRIX)
		{
		    osg::Matrix mat ;
		    d.shm->read(mat.ptr()) ;
		    osg::Vec3d t ;
		    osg::Quat q ;
		    osg::Vec3d s ;
		    iris::Decompose(mat, &t, &q, &s) ;
		    double h, p, r ;
		    iris::QuatToEuler(q, &h, &p, &r) ;
		    char txs[20], tys[20], tzs[20], hs[20], ps[20], rs[20], sxs[20], sys[20], szs[20] ;
		    float9(txs,t.x()) ;
		    float9(tys,t.y()) ;
		    float9(tzs,t.z()) ;
		    float9(hs,h) ;
		    float9(ps,p) ;
		    float9(rs,r) ;
		    float9(sxs,s.x()) ;
		    float9(sys,s.y()) ;
		    float9(szs,s.z()) ;
#if 0
		    if (s.x() == 1.f && s.y() == 1.f && s.z() == 1.f) printf("%11.11s: %s %s %s, %s %s %s\n",name.c_str(), txs, tys, tzs, hs, ps, rs) ;
		    else if (s.x() == s.y() && s.x() == s.z() && s.y() == s.z()) printf("%11.11s: %s %s %s, %s %s %s, %s\n",name.c_str(), txs, tys, tzs, hs, ps, rs, sxs) ;
		    else printf("%11.11s: %s %s %s, %s %s %s, %s %s %s\n",name.c_str(), txs, tys, tzs, hs, ps, rs, sxs, sys, szs) ;
#else
		    if (s.x() == s.y() && s.x() == s.z() && s.y() == s.z()) printf("%11.11s: %s %s %s, %s %s %s, %s\n",name.c_str(), txs, tys, tzs, hs, ps, rs, sxs) ;
		    else printf("%11.11s: %s %s %s, %s %s %s, %s %s %s\n",name.c_str(), txs, tys, tzs, hs, ps, rs, sxs, sys, szs) ;
#endif

		}
		else if (d.type == iris::ShmState::FLOAT)
		{
		    float f ;
		    d.shm->read(&f) ;
		    char fs[16] ;
		    float9(fs,f) ;
		    printf("%11.11s: %s\n",name.c_str(),fs) ;
		}
		else if (d.type == iris::ShmState::INT)
		{
		    int i ;
		    d.shm->read(&i) ;
		    printf("%11.11s: %d\n",name.c_str(),i) ;
		}
		else if (d.type == iris::ShmState::STRING)
		{
		    char c[d.shm->getSize()] ;
		    d.shm->read(c) ;
		    printf("%11.11s: %s\n",name.c_str(),c) ;
		}
		else
		{
		    printf("skipping %s\n",name.c_str()) ;
		}
	    }
	}
	usleep(ticks) ;
    }
    return 0 ;
}


