/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2003  Virginia Tech
 * 
 * This software, the DIVERSE Toolkit library, is free software; you can
 * redistribute it and/or modify it under the terms of the GNU Lesser
 * General Public License (LGPL) as published by the Free Software
 * Foundation; either version 2.1 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software, in the top level source directory in
 * a file named "COPYING.LGPL"; if not, see it at:
 * http://www.gnu.org/copyleft/lesser.html or write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 */
#include "config.h"
#include "_config.h"
#include "privateConstants.h"
#include "dtkConfigure.h"

#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "dtkBase.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "utils.h"
#include "dtkDistribution.h"

void dtkDistribution::remove_object(void)
{
  if(x)
    free(x);
  if(ynorm && ynorm != y)
    free(ynorm);
  if(y)
    free(y);
  if(sum)
    free(sum);
  x = y = ynorm = sum = NULL;
}

dtkDistribution::dtkDistribution(int num_points_in, double min_in,
				 double max_in, double norm_in) :
  dtkBase(DTKDISTRIBUTION_TYPE)
{
  x = y = ynorm = sum = NULL;
  value_count = 0;

  if(num_points_in < 3 || min_in >= max_in ||
     (norm_in <= 0.0 && norm_in != DTK_DISTRO_NO_NORM))
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkDistribution::dtkDistribution(%d,%g,%g,%g) failed. "
		 "Arguments (%d,%g,%g,%g) are out of the"
		 " range of valid arguments.\n",
		 num_points_in, min_in, max_in,norm_in,
		 num_points_in, min_in, max_in,norm_in);
      return;
    }

  num_points = num_points_in;
  min = min_in;
  max = max_in;
  norm = norm_in;
  x_total = 0.0;

  x = (double *) dtk_malloc(sizeof(double)*num_points);
  y = (double *) dtk_malloc(sizeof(double)*num_points);
  if(norm == DTK_DISTRO_NO_NORM)
    ynorm = y;
  else
    ynorm = (double *) dtk_malloc(sizeof(double)*num_points);
  sum = (double *) dtk_malloc(sizeof(double)*num_points);

  delta = (max - min)/(num_points - 2);

  /********** setup the x values **************************/
  // the smallest is x[0] = min
  // the largest is x[num_points-1] = max + delta

  for(int i=0;i<num_points;i++)
    {
      x[i] = min + delta*((double)i);
      y[i] = 0.0;
    }

  if(x && y && ynorm && sum) validate();
}

void dtkDistribution::calculate(void)
{
  if(!sum) return;

  double current_sum = 0.0;
  int i;
  if(norm != DTK_DISTRO_NO_NORM)
    {
      for(i=0;i<num_points;i++)
	sum[i] = (current_sum += (ynorm[i] = norm*y[i]/value_count));
    }
  else
    {
      for(i=0;i<num_points;i++)
	sum[i] = (current_sum += y[i]);
    }
}

void dtkDistribution::setNormal(double norm_in)
{
  if(!sum) return;
  if(norm_in == DTK_DISTRO_NO_NORM && norm != DTK_DISTRO_NO_NORM)
    {
      free(ynorm);
      ynorm = y;
    }
  else if(norm_in != DTK_DISTRO_NO_NORM && norm == DTK_DISTRO_NO_NORM)
    {
      ynorm = (double *) dtk_malloc(sizeof(double)*num_points);
    }
      
  norm = norm_in;
  reset();
}

void dtkDistribution::put(double x_in)
{
  if(!sum) return;
  if(x_in > min && x_in < max)
    {
      int i= (int)( (x_in - min)/delta);
      double ix = (double) i;
      if((x_in  - min)/delta > ix)
	i++;
      (y[i])++;
    }
  else if(x_in <= min)
    (y[0])++;
  else // x_in => max
    (y[num_points-1])++;

  x_total += x_in;

  value_count++;
}

void dtkDistribution::reset(void)
{
  if(!sum) return;
  for(int i=0;i<num_points;i++)
    y[i] = ynorm[i] = sum[i] = 0.0;
  value_count = 0;
  x_total = 0.0;
}

void dtkDistribution::print(FILE *file)
{
  if(!file) file = stdout;

  if(!sum) return;
  fprintf(file, "dtkDistribution loaded %d values\n",value_count);
  calculate();
  int i;
  for(i=0;i<num_points;i++)
    fprintf(file,"%.17g %.17g %.17g %.17g\n",
	    x[i], y[i], ynorm[i], sum[i]);
}

double dtkDistribution::mean(void) const
{
  if(!sum) return 0.0;
  return x_total/value_count;
}

dtkDistribution::~dtkDistribution(void)
{
  remove_object();
}
