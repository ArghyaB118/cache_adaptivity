/*
########################################################################
#
# Copyright (c) 2002 The Regents of the University of Texas System.
# All rights reserved. See end of file.
#
########################################################################
*/

/* Kent Milfeld 9/27/2002 TACC (Texas Advanced Computing Center) */


#ifdef RRT_TIMER

#include <stdio.h>
#include <sys/time.h> 
#include <time.h> 

double rrt_base = 0.0e0;

double rrt_timer(void)
{

   timebasestruct_t tbs;
   double time;
                                              /* Get the time */
   read_real_time(&tbs, TIMEBASE_SZ);
                                              /* Convert to to sec, nsec. */
   time_base_to_time(&tbs, TIMEBASE_SZ);
                                              /* Create new sec. base */
   if(rrt_base == 0.0e0) rrt_base = tbs.tb_high;

   return( (double)(tbs.tb_high - rrt_base) + 1.0E-09*(double)tbs.tb_low  );
}
#endif


#ifdef RUSAGE_TIMER

#include <stdio.h>
#include <sys/time.h>  
#include <sys/resource.h>

double rusage_secbase=0.0E0;

double rusage_timer(void)
{
   
   double t, sec;
   int    ierr ;
                        /* Allocate structure */
   struct rusage sr; 
                        /* Call getrusage */
 
   if ( (ierr=getrusage(RUSAGE_SELF,&sr)) != 0 ){
      printf(" ERROR: getrusage failed with error %d.\n",ierr);
   }

   if(rusage_secbase == 0.0E0) 
         rusage_secbase = (double)(sr.ru_utime.tv_sec+sr.ru_stime.tv_sec);
   sec = (double)(sr.ru_utime.tv_sec +sr.ru_stime.tv_sec) - rusage_secbase;
 
   t   = (double)( sec +
                 (double)(sr.ru_utime.tv_usec+sr.ru_stime.tv_usec)/1.0E6 );
   return(t);
}
 
#endif


#ifdef TMS_TIMER

#include <sys/param.h>
#include <sys/types.h>
#include <sys/times.h>
#include <time.h>

double tms_timer()
{
    double sec;
    struct tms realbuf;
    times(&realbuf);
    sec = (double)( realbuf.tms_stime + realbuf.tms_utime )/(double)CLK_TCK;
    return ( sec);
}
   
#endif



#ifdef GTOD_TIMER

   
#include <stdio.h>
#include <sys/time.h>
#include <time.h>     /* Cray 4.3BSD*/

double gtod_secbase = 0.0E0;

double gtod_timer()
{
   struct timeval tv;
   struct timezone Tzp;
   double sec;

   gettimeofday(&tv, &Tzp);

               /*Always remove the LARGE sec value
                 for improved accuracy  */
   if(gtod_secbase == 0.0E0) 
      gtod_secbase = (double)tv.tv_sec;
   sec = (double)tv.tv_sec - gtod_secbase;

   return sec + 1.0E-06*(double)tv.tv_usec;
}

#endif

/*
########################################################################
#
# Copyright (c) 2001 The Regents of the University of Texas System.
# All rights reserved.
#
# Redistribution and use in source and binary forms are permitted provided
# that the above copyright notice and this paragraph are duplicated in all
# such forms and that any documentation, advertising materials,  and other
# materials  related to such  distribution  and use  acknowledge  that the
# software  was  developed  by the  University of Texas.  The  name of the
# University may not be  used to endorse or promote  products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
# MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
########################################################################
#
# $Id: timers.c,v 1.3 2005/08/16 02:49:25 cvsuser Exp $
# $Source: C:/cvs/www/www.tacc.utexas.edu/services/userguides/porting/timers/timers.c,v $
#
*/
