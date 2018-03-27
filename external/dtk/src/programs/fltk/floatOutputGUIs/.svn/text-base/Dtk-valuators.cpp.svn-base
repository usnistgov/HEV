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
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <dtk.h>

#ifdef DTK_ARCH_IRIX
# include <strings.h>
#endif

#include "valuators.h"

#define DEFAULT_NUMBEROFVALUATORS  3   


int is_running = 1;

int return_val = 0;

dtkSharedMem *shm = NULL;
FLOAT_TYPE *buffer = NULL;


static void catcher(int sig)
{
    dtkMsg.add(DTKMSG_INFO,"caught signal %d: exiting.\n",sig);
    is_running = 0;
}

static int Usage(void)
{
    printf("\nUsage: %s [FILE_NAME] [-h|--help]\\\n"
	   " [-N|--number NUM_VALUATORS] [-s|--scale FIELD MIN MAX RESET]\\\n"
	   " [-u|--update-continuous] [-l|--labels LABEL0 LABEL1 ...]\\\n"
	   " [FLTK_OPTIONS]\n\n"
	   "Make some valuators that are connected to DTK\n"
	   "shared memory, or print to stdout if FILE_NAME\n"
	   "is not given.\n\n",PROGRAM_NAME);
    printf("  OPTIONS\n\n"
	   " FILE_NAME                       connect to DTK shared memory file FILE_NAME\n"
	   " -h|--help                       print this help\n"
	   " -l|--labels LABEL0 LABEL1 ...   label the valuators with the labels LABEL0 LABEL1 and so on\n"
	   "                                 -l|--labels must be the last option.\n"
	   " -N|--number NUM_VALUATORS       set up NUM_VALUATORS valuators\n"
	   " -s|--scale FIELD MIN MAX RESET  scale slider number FIELD to have a minimum value of MIN,\n"
	   "                                 a maximum value of MAX and a and reset value of RESET\n"
	   "                                 FIELD is a int starting at 0. MIN, MAX and RESET are %ss.\n"
	   " -u|--update-continuous          start with this program reading and updating continuously\n\n",
	   FLOAT_NAME);

    // prints to stdout
    dtkFLTKOptions_print();

    return 1;
}


static int get_MIN_MAX_RESET(int argc, const char **argv,
			     struct ValuatorLimits *s, int numberOfValuators)
{
    int i;
    //for(i=1;i<argc;i++)
    // printf("%s ",argv[i]);
    // printf("\n");
    for(i=1;i<argc;)
    {
	char *str;
	if((str = dtk_getOpt("-l","--labels",argc,argv,&i)))
	{
	    return 0;
	}
	else if((str = dtk_getOpt("-s","--scale",argc,argv,&i)))
	{
	    errno = 0;
	    int num = atoi(str);
	    if(num < 0 || num >= numberOfValuators)
	    {
		printf("Bad option: -s|--scale\n\n");
		return Usage();
	    }
	    s[num].min = strtod(argv[i++], NULL);
	    if(errno == ERANGE)
	    {
		printf("Bad option: -s|--scale\n\n");
		return Usage();
	    }
	    s[num].max = strtod(argv[i++], NULL);
	    if(errno == ERANGE)
	    {
		printf("Bad option: -s|--scale\n\n");
		return Usage();
	    }
	    s[num].reset = strtod(argv[i++], NULL);
	    if(errno == ERANGE)
	    {
		printf("Bad option: -s|--scale\n\n");
		return Usage();
	    }
	    if(s[num].max == s[num].min)
	    {
		printf("got slider number %d with max=%g min=%g reset=%g\n",
		       num,s[num].max,s[num].min,s[num].reset);
		printf("Bad option: -s|--scale\n\n");
		return Usage();
	    }
	}
	else
	    i++;
    }


#if(0)
    for(i=0;i<numberOfValuators;i++)
	printf("got slider number %d with min=%g max=%g reset=%g\n",
	       i,s[i].min,s[i].max,s[i].reset);
#endif

    return 0;
}

static int do_cmd_line_args(int argc, const char **argv,
                            int *numberOfValuators,
			    char **shm_name, int *labels_index,
			    int *update_continuous)
{
    int i;

    for(i=1;i<argc;)
    {
	char *str;
	if(!strcmp("-u",argv[i]) || !strcmp("--update-continuous",argv[i]))
	{
	    i++;
	    *update_continuous = 1;
	}
	else if((str = dtk_getOpt("-N","--number",argc,argv,&i)))
	{
	    if((*numberOfValuators = atoi(str)) <= 0)
		return Usage();
	}
	else if((str = dtk_getOpt("-l","--labels",argc,argv,&i)))
	{
	    // printf("got labels index = %d\n",i - 1);
	    *labels_index = i - 1;
	    return 0;
	}
	else if((str = dtk_getOpt("-s","--scale",argc,argv,&i)))
	{
	    if(i + 2 >= argc)
	    {
		printf("Bad option: -s|--scale\n\n");
		return Usage();
	    }
	    else
		i += 3; // skip the MIN MAX RESET arg values
	}
	else if(!strcmp("-h",argv[i]) || !strcmp("--help",argv[i]))
	    return Usage();
	else
	    *shm_name = (char *) argv[i++];
    }
    return 0;
}

#define TIMER_CALLBACK 1
#ifdef TIMER_CALLBACK
void timer_callback(void*)
{
    // set by signal catcher- all done when false
    if (!is_running) return ;

    if (update_contButton->value())
    {
	update_valuators();
    }
    Fl::repeat_timeout(0.05, timer_callback) ;
}
#endif

int main(int argc, char **argv)
{
    char *shm_name=NULL;
    int isNewShm = 0;
    int numberOfValuators=-1, labels_index = -1, update_continuous = 0;

    dtkMsg.setPreMessage("%s%s%s", dtkMsg.color.vil,
			 PROGRAM_NAME, dtkMsg.color.end);

    int fltk_argc;
    char **fltk_argv;
    int other_argc;
    char **other_argv;
    // dtkFLTKOptions_get will pull out FLTK options into its return pointer.
    if(dtkFLTKOptions_get(argc, (const char **) argv,
			  &fltk_argc,  &fltk_argv,
			  &other_argc, &other_argv)) return 1;

    if(do_cmd_line_args(other_argc, (const char **) other_argv,
			&numberOfValuators, &shm_name, &labels_index,
			&update_continuous))
	return 1;

    /* printf("shm_name=\"%s\" numberOfValuators=%d\n",
       shm_name, numberOfValuators); */

    size_t size = 0;

    /*************** Get DTK shared memory size **************/
    if(shm_name)
    {
	// connect if it can and do not spew on error.
	shm = new dtkSharedMem(shm_name, 0);
	if(shm->isValid())
	{
	    size = shm->getSize();
	    if(size < sizeof(FLOAT_TYPE))
		return dtkMsg.add(DTKMSG_ERROR,0,1,
				  "DTK shared memory named \"%s\" is smaller"
				  " than a %s.\n", shm_name, FLOAT_NAME);

	    if(numberOfValuators > 0 &&
	       ((size_t) numberOfValuators*sizeof(FLOAT_TYPE)) > size)
		return dtkMsg.add(DTKMSG_ERROR,0,1,
				  "The DTK shared memory \"%s\" is"
				  " size %d which is not large enough for"
				  " %d valuators (%ss).\n",
				  shm_name, size, numberOfValuators, FLOAT_NAME);

	    if(numberOfValuators < 1)
		numberOfValuators = size/sizeof(FLOAT_TYPE);
	}
	else // if(!(shm->isValid()))
	{
	    delete shm;
	    if(numberOfValuators < 1)
		numberOfValuators = DEFAULT_NUMBEROFVALUATORS;
	    size = numberOfValuators*sizeof(FLOAT_TYPE);

	    shm = new dtkSharedMem(size, shm_name);
	    if(shm->isInvalid()) { delete shm; return 1; }
	    isNewShm = 1;
	}
    }
    else 
    {
	if(numberOfValuators < 1)
	    numberOfValuators = DEFAULT_NUMBEROFVALUATORS;
	size = numberOfValuators*sizeof(FLOAT_TYPE);
    }

    struct ValuatorLimits *s = (struct ValuatorLimits *)
	dtk_malloc(sizeof(struct ValuatorLimits)*numberOfValuators);

    int i;
    for(i=0;i<numberOfValuators;i++)
    {
	s[i].start = 0.0;
	s[i].reset = 0.0;
	s[i].min   = -1.0;
	s[i].max   = 1.0;
	s[i].label = strdup("----0----");
	snprintf(s[i].label,7,"-%d-",i);
    }
    if(labels_index > -1)
    {
	i = labels_index-1;
	char *str = dtk_getOpt("-l","--labels",
			       other_argc,(const char **) other_argv,&i);
	if(str)
	    snprintf(s[0].label,7,"%s",str);
	int j=1;
	for(; i<other_argc && j<numberOfValuators; i++)
	{
	    snprintf(s[j++].label,7,"%s",other_argv[i]);
	}
    }

    if(get_MIN_MAX_RESET(other_argc,
			 (const char **) other_argv, s, numberOfValuators))
    {
	if(shm)
	    delete shm;
	return 1;
    }

    if(shm)
	shm->setAutomaticByteSwapping(sizeof(FLOAT_TYPE));

    buffer = (FLOAT_TYPE *) dtk_malloc(size);
    bzero(buffer, size);

    if(isNewShm) /*********** put initial values ************/
    {
	for(i=0;i<numberOfValuators;i++)
	    buffer[i] = (FLOAT_TYPE) s[i].reset;
	if(shm->write(buffer)) { delete shm; return 1; }
    }
    else if(shm) /*********** get initial values ************/
    {
	if(shm->read(buffer)) { delete shm; return 1; }
    }

    if(shm)
    {
	for(i=0;i<numberOfValuators;i++)
	    s[i].start = (double) buffer[i];
    }
    else
    {
	for(i=0;i<numberOfValuators;i++)
	    buffer[i] = (FLOAT_TYPE)(s[i].start = s[i].reset);
    }

    char winLabel[64];
    if(shm_name)
	snprintf(winLabel, (size_t)63, "%s: \"%s\"", PROGRAM_NAME, shm_name);
    else
	snprintf(winLabel, (size_t)63, "%s: stdout", PROGRAM_NAME);

#if(0)
    printf("got shm_name=\"%s\" numberOfValuators=%d\n",
	   shm_name, numberOfValuators);

    for(i=0;i<numberOfValuators;i++)
	printf("slider %d: label=\"%s\" start=%3.3g "
	       "reset=%3.3g min=%3.3g max=%3.3g\n",
	       i, s[i].label, s[i].start, s[i].reset,
	       s[i].min, s[i].max);
#endif

    //  signal(SIGINT, catcher);
    //  signal(SIGTERM, catcher);
#ifndef DTK_ARCH_WIN32_VCPP
    signal(SIGQUIT, catcher);
#endif

    if(make_window(numberOfValuators, s, (const char *) winLabel,
		   update_continuous,fltk_argc, fltk_argv))
    {
	if(shm)
	    delete shm;
	return 1;
    }

#ifdef TIMER_CALLBACK
    Fl::add_timeout(0.05, timer_callback) ;
    Fl::run() ;
#else
    while (is_running) // set by signal catcher
    {
	if(update_contButton->value() &&
	   mainWindow->visible_r())
        {
	    Fl::wait(0.05);
	    update_valuators();
        }
	else
        {
	    while(is_running && ((Fl::wait()>0 && !update_contButton->value())
				 || !mainWindow->visible_r())) ;
	}
    }
#endif  

    if(shm)
	delete shm;

    return return_val;
}
