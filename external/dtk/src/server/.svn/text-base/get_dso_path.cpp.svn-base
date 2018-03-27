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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dtk.h>

#include "if_windows.h"

#ifdef DTK_ARCH_WIN32_VCPP
#  define SEPARATOR ";"
#else
#  define SEPARATOR ":"
#endif

char *get_service_path(void)
{
  char *server_dso_path;
  char *env = getenv("DTK_SERVICE_PATH");
  if(env)
    {
      server_dso_path = dtk_strdup(env);
    }
  else
    { 
      server_dso_path = (char *) dtk_malloc(strlen(dtkConfig.getString(dtkConfigure::SERVICE_DSO_DIR)) + 
					strlen("." SEPARATOR) + 1);

      sprintf(server_dso_path,"." SEPARATOR "%s", dtkConfig.getString(dtkConfigure::SERVICE_DSO_DIR));
    }
  return server_dso_path;
}

char *get_service_config_path(void)
{
  char *server_dso_path;
  char *env = getenv("DTK_SERVICE_CONFIG_PATH");
  if(env)
    {
      server_dso_path = dtk_strdup(env);
    }
  else
    { 
      server_dso_path = (char *) dtk_malloc(strlen(dtkConfig.getString(dtkConfigure::SERVICE_DSO_DIR)) +
					strlen("." SEPARATOR) + 1);
      sprintf(server_dso_path,"." SEPARATOR "%s", dtkConfig.getString(dtkConfigure::SERVICE_DSO_DIR));
    }
  return server_dso_path;
}

char *get_calibration_path(void)
{
  char *server_dso_path;
  char *env = getenv("DTK_CALIBRATION_PATH");
  if(env)
    {
      server_dso_path = dtk_strdup(env);
    }
  else
    { 
      server_dso_path = (char *) dtk_malloc(strlen(dtkConfig.getString(dtkConfigure::SERVICE_DSO_DIR)) +
					strlen("." SEPARATOR) + 1);
      sprintf(server_dso_path,"." SEPARATOR "%s", dtkConfig.getString(dtkConfigure::SERVICE_DSO_DIR));
    }
  return server_dso_path;
}

char *get_calibration_config_path(void)
{
  char *server_dso_path;
  char *env = getenv("DTK_CALIBRATION_CONFIG_PATH");
  if(env)
    {
      server_dso_path = dtk_strdup(env);
    }
  else
    { 
      server_dso_path = (char *) dtk_malloc(strlen(dtkConfig.getString(dtkConfigure::SERVICE_DSO_DIR)) +
					strlen("." SEPARATOR) + 1);
      sprintf(server_dso_path,"." SEPARATOR "%s", dtkConfig.getString(dtkConfigure::SERVICE_DSO_DIR));
    }
  return server_dso_path;
}

