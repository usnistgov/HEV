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
#include "dtk.h"
#define __NOT_LOADED_DTKSERVICE__
#include "if_windows.h"
#include "dtk/dtkService.h"
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include "configParser.h"
#ifndef DTK_ARCH_WIN32_VCPP
#  include <termios.h>
#endif

#ifdef DTK_ARCH_WIN32_VCPP
#  define SEPARATOR ";"
#else
#  define SEPARATOR ":"
#endif

configParser::configParser( )
{
  config_buffer = 0;
}

configParser::~configParser()
{
  if( config_buffer )
    delete [] config_buffer;
}
/*
void configParser::tokenize( const std::string& str, std::vector<std::string>& tokens,
    const std::string& delimiters )
{
    unsigned int startpos = str.find_first_not_of( delimiters, 0 );
    unsigned int endpos = str.find_first_of( delimiters, startpos );
    while( startpos != std::string::npos )
    {
        tokens.push_back( str.substr( startpos, endpos - startpos) );
        startpos = str.find_first_not_of( delimiters, endpos );
        endpos = str.find_first_of( delimiters, startpos );
        if( startpos != std::string::npos && endpos == std::string::npos )
            endpos = str.size();
    }
}
*/
int configParser::loadConfigFile( std::ifstream& config_stream )
{
    std::filebuf* fbuf = config_stream.rdbuf();
    long size = fbuf->pubseekoff( 0, std::ios::end, std::ios::in );
    fbuf->pubseekpos( 0, std::ios::in );
    config_buffer = new char[size+1];
    if( !config_buffer )
    {
      dtkMsg.add( DTKMSG_ERROR, "Unable to allocate sufficient memory for file buffer\n" );
      return -1;
    }

    config_stream.read( config_buffer, size );
    config_stream.close();

    config_buffer[size] = '\0';
    return 0;
}

int configParser::findConfigFile( const std::string& config_file, const std::string& dso_path,
    std::vector<std::string>& config_paths )
{
  std::ifstream config_stream;
  // absolute path
  if( config_file[0] == '/' )
  {
    config_stream.open( config_file.c_str() );
    if( config_stream.is_open() )
    {
      dtkMsg.add( DTKMSG_NOTICE, "Using configuration file %s\n", config_file.c_str() );
      if( loadConfigFile( config_stream ) )
        return -1;
      return 0;
    }
    return -1;
  }

/*
  std::string search_path;
  if( getenv( "DTK_SERVICE_CONFIG_PATH" ) )
  {
    search_path = getenv( "DTK_SERVICE_CONFIG_PATH" );
    search_path += SEPARATOR;
  }
  search_path += dso_path;
*/
//  tokenize( dso_path, config_paths, SEPARATOR );
//  std::string file_name;
  char* file_name = NULL;
  file_name = dtk_getFileFromPath( dso_path.c_str(), config_file.c_str(), (mode_t) 0000);
  if( file_name )
  {
    config_stream.open( file_name );
    if( config_stream.is_open() )
    {
      dtkMsg.add( DTKMSG_NOTICE, "Using configuration file %s\n", file_name );
      if( loadConfigFile( config_stream ) )
      {
        free( file_name );
        return -1;
      }
      free( file_name );
      return 0;
    }
  }
/*
  for( unsigned int i=0;i<config_paths.size();i++ )
  {
    file_name = config_paths[i];
	if( file_name[file_name.size()-1] != '/' )
		file_name += "/";
	file_name += config_file;
    config_stream.open( file_name.c_str() );
    if( config_stream.is_open() )
    {
      dtkMsg.add( DTKMSG_NOTICE, "Using configuration file %s\n", file_name.c_str() );
      if( loadConfigFile( config_stream ) )
        return -1;
      return 0;
    }
  }
*/
  if( file_name )
    free( file_name );

  return -1;
}

int configParser::tokenizeConfigBuffer( std::vector<std::vector<std::string> >& config_tokens )
{
  std::vector<std::string> config_lines, line_tokens;
  tokenize( config_buffer, config_lines, "\n\r" );

  size_t pos;
  std::vector<std::string>::iterator itr;
  for( unsigned int i=0;i<config_lines.size();i++ )
  {
    line_tokens.clear();
    tokenize( config_lines[i], line_tokens, " \t" );

    unsigned int index = 0;
    for( itr=line_tokens.begin();itr!=line_tokens.end();itr++ )
    {
      if( (*itr)[0] == '#' )
      {
        line_tokens.erase( itr, line_tokens.end() );
        break;
      }
      else if( ( pos = (*itr).find( "#" ) ) != std::string::npos )
      {
        (*itr) = (*itr).substr( 0, pos );
        if( itr != line_tokens.end() )
          line_tokens.erase( ++itr, line_tokens.end() );
        break;
      }
      if( index == 0 || index == 2 )
        std::transform( itr->begin(), itr->end(), itr->begin(), (int(*)(int))::tolower );
      index++;
    }
    if( line_tokens.size() )
      config_tokens.push_back( line_tokens );
  }

  return 0;
}

int configParser::populateFields( std::vector<std::vector<std::string> >& config_tokens )
{
  for( unsigned int i=0;i<config_tokens.size();i++ )
  {
    if( config_tokens[i][0] == "servicetype" )
    {
      if( config_tokens[i].size() != 3 )
      {
        dtkMsg.add( DTKMSG_ERROR, "The serviceType definition should contain 3 elements:\n"
            "serviceType Name service - where Name is an arbitrary\n"
            "name used to link serviceOptions with the correct service\n"
            "and service is the name of a dtkConfigService DSO.\n"
            "The serviceType defintion appears as follows:\n" );
        for( unsigned int k=0;k<config_tokens[i].size();k++ )
          dtkMsg.append( "%s ", config_tokens[i][k].c_str() );
        dtkMsg.append( "\n" );
        return -1;
      }
      if( m_configuration[config_tokens[i][1]].m_name == config_tokens[i][1] )
      {
        dtkMsg.add( DTKMSG_ERROR, "There is a duplicate serviceType entry:\n" );
        for( unsigned int k=0;k<config_tokens[i].size();k++ )
          dtkMsg.append( "%s ", config_tokens[i][k].c_str() );
        dtkMsg.append( "\n" );
        return -1;
      }
      serviceConfig current;
      current.m_name = config_tokens[i][1];
      current.m_service_type = config_tokens[i][2];
      m_configuration[current.m_name] = current;
    }
    else if( config_tokens[i][0] == "serviceoption" )
    {
      if( config_tokens[i].size() < 3 )
      {
        dtkMsg.add( DTKMSG_ERROR, "The serviceOption requires a name and at least 1\n"
            "additional parameter. The following parameters were input:\n" );
        for( unsigned int k=0;k<config_tokens[i].size();k++ )
          dtkMsg.append( "%s ", config_tokens[i][k].c_str() );
        dtkMsg.append( "\n" );
        return -1;
      }
      if( m_configuration[config_tokens[i][1]].m_name != config_tokens[i][1] )
      {
        dtkMsg.add( DTKMSG_ERROR, "The serviceType with the name %s must be defined\n"
            "using serviceType before any serviceOptions for that name are specified.\n"
            "The parameters passed are:\n" );
        for( unsigned int k=0;k<config_tokens[i].size();k++ )
          dtkMsg.append( "%s ", config_tokens[i][k].c_str() );
        dtkMsg.append( "\n" );
        return -1;
      }
      if( config_tokens[i][2] == "trackd" || config_tokens[i][2] == "trackdemulation" )
      {
        if( config_tokens[i].size() != 4 )
        {
          dtkMsg.add( DTKMSG_ERROR, "The trackd serviceOption requires a single boolean\n"
              "value following the trackd parameter. The parameters passed are:\n" );
          for( unsigned int k=0;k<config_tokens[i].size();k++ )
            dtkMsg.append( "%s ", config_tokens[i][k].c_str() );
          dtkMsg.append( "\n" );
          return -1;
        }
        if( config_tokens[i][3] == "yes" || config_tokens[i][3] == "true" ||
            config_tokens[i][3] == "on" )
          m_configuration[config_tokens[i][1]].m_trackd = true;
        else if( config_tokens[i][3] == "no" || config_tokens[i][3] == "false" ||
            config_tokens[i][3] == "off" )
          m_configuration[config_tokens[i][1]].m_trackd = false;
        else
        {
          dtkMsg.add( DTKMSG_ERROR, "Valid values for the trackd serviceOption are:\n"
              "\"true\", \"false\", \"on\", \"off\", \"yes\", and \"no\".\n" );
          return -1;
        }
      }
      else if( config_tokens[i][2] == "calibration" )
      {
        if( config_tokens[i].size() < 4 )
        {
          dtkMsg.add( DTKMSG_ERROR, "The calibration serviceOption requires that the name\n"
              "of a dtkCalibration DSO follow along with any parameters which will\n"
              "be passed to the dtkCalibration DSO initCalibration function.\n" );
          return -1;
        }
        m_configuration[config_tokens[i][1]].m_calibrator_name = config_tokens[i][3];
        for( unsigned int j=4;j<config_tokens[i].size();j++ )
        {
          m_configuration[config_tokens[i][1]].m_calibrator_params.push_back( config_tokens[i][j] );
        }
      }
/*      else if( config_tokens[i][2] == "trackdtype" )
      {
        if( config_tokens[i].size() != 4 )
        {
          dtkMsg.add( DTKMSG_ERROR, "The trackdType serviceOption requires a single\n"
              "value following the trackd parameter. Acceptable values are \"head\",\n"
              "\"wand\", and \"none\". The parameters passed are:\n" );
          for( unsigned int k=0;k<config_tokens[i].size();k++ )
            dtkMsg.append( "%s ", config_tokens[i][k].c_str() );
          dtkMsg.append( "\n" );
          return -1;
        }
        if( config_tokens[i][3] == "tracker" || config_tokens[i][3] == "wand" ||
            config_tokens[i][3] == "none" )
          m_configuration[config_tokens[i][1]].m_trackd_type.push_back( config_tokens[i][3] );
        else
        {
          dtkMsg.add( DTKMSG_ERROR, "Valid values for the trackdType serviceOption are:\n"
              "\"head\", \"wand\", and \"none\".\n" );
          return -1;
        }
      }*/
/*      else if( config_tokens[i][2] == "shmname" )
      {
        if( config_tokens[i].size() < 4 || config_tokens[i].size() > 5 )
        {
          dtkMsg.add( DTKMSG_ERROR, "The trackdType serviceOption requires a single\n"
              "value following the trackd parameter. Acceptable values are \"head\",\n"
              "\"wand\", and \"none\". The parameters passed are:\n" );
          for( unsigned int k=0;k<config_tokens[i].size();k++ )
            dtkMsg.append( "%s ", config_tokens[i][k].c_str() );
          dtkMsg.append( "\n" );
          return -1;
        }
        else
          dtkMsg.add( DTKMSG_WARNING, "The shmname serviceOption has not yet been implemented\n" );
      }*/
      else if( config_tokens[i][2] == "port" )
      {
        if( config_tokens[i].size() != 4 )
        {
          dtkMsg.add( DTKMSG_ERROR, "The trackdType serviceOption requires a single\n"
              "value following the trackd parameter. The value should identify the port\n"
              "to which the device is attached (i.e. /dev/ttyS0, com1, etc)\n" );
          for( unsigned int k=0;k<config_tokens[i].size();k++ )
            dtkMsg.append( "%s ", config_tokens[i][k].c_str() );
          dtkMsg.append( "\n" );
          return -1;
        }
        m_configuration[config_tokens[i][1]].m_port = config_tokens[i][3];
      }
      else if( config_tokens[i][2] == "baud" )
      {
        if( config_tokens[i].size() != 4 )
        {
          dtkMsg.add( DTKMSG_ERROR, "The baud serviceOption requires a single\n"
              "value following the baud parameter. The value passed\n"
              "should be a valid baud rate. The parameters passed are:\n" );
          for( unsigned int k=0;k<config_tokens[i].size();k++ )
            dtkMsg.append( "%s ", config_tokens[i][k].c_str() );
          dtkMsg.append( "\n" );
          return -1;
        }
	std::string valid_values;
#ifdef DTK_ARCH_WIN32_VCPP
	valid_values = "0, 110, 300, 600, 1200, 2400, 4800, 9600, 14400,\n";
	valid_values += "19200, 38400, 56000, 57600, 115200, 128000, 256000";

        if( config_tokens[i][3] == "0" )
	  m_configuration[config_tokens[i][1]].m_baud_flag = 0;
	else if( config_tokens[i][3] == "110" )
	  m_configuration[config_tokens[i][1]].m_baud_flag = CBR_110;
	else if( config_tokens[i][3] == "300" )
	  m_configuration[config_tokens[i][1]].m_baud_flag = CBR_300;
	else if( config_tokens[i][3] == "600" )
	  m_configuration[config_tokens[i][1]].m_baud_flag = CBR_600;
	else if( config_tokens[i][3] == "1200" )
	  m_configuration[config_tokens[i][1]].m_baud_flag = CBR_1200;
	else if( config_tokens[i][3] == "2400" )
	  m_configuration[config_tokens[i][1]].m_baud_flag = CBR_2400;
	else if( config_tokens[i][3] == "4800" )
	  m_configuration[config_tokens[i][1]].m_baud_flag = CBR_4800;
	else if( config_tokens[i][3] == "9600" )
	  m_configuration[config_tokens[i][1]].m_baud_flag = CBR_9600;
	else if( config_tokens[i][3] == "14400" )
	  m_configuration[config_tokens[i][1]].m_baud_flag = CBR_14400;
	else if( config_tokens[i][3] == "19200" )
	  m_configuration[config_tokens[i][1]].m_baud_flag = CBR_19200;
	else if( config_tokens[i][3] == "38400" )
	  m_configuration[config_tokens[i][1]].m_baud_flag = CBR_38400;
	else if( config_tokens[i][3] == "56000" )
	  m_configuration[config_tokens[i][1]].m_baud_flag = CBR_56000;
	else if( config_tokens[i][3] == "57600" )
	  m_configuration[config_tokens[i][1]].m_baud_flag = CBR_57600;
	else if( config_tokens[i][3] == "115200" )
	  m_configuration[config_tokens[i][1]].m_baud_flag = CBR_115200;
	else if( config_tokens[i][3] == "128000" )
	  m_configuration[config_tokens[i][1]].m_baud_flag = CBR_128000;
	else if( config_tokens[i][3] == "256000" )
	  m_configuration[config_tokens[i][1]].m_baud_flag = CBR_256000;
#else
	valid_values = "0, 50, 75, 110, 134, 150, 200, 300, 600, 1200,\n";
	valid_values += "1800, 2400, 4800, 9600, 19200, 38400, 57600,\n";
	valid_values += "115200, 230400";
#  ifndef DTK_ARCH_DARWIN
	valid_values += ", 460800, 500000, 576000, 921600,\n";
	valid_values += "1000000, 1152000, 1500000, 2000000, 2500000,\n";
	valid_values += "3000000, 3500000, 4000000";
#  endif

        if( config_tokens[i][3] == "0" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B0;
        else if( config_tokens[i][3] == "50" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B50;
        else if( config_tokens[i][3] == "75" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B75;
        else if( config_tokens[i][3] == "110" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B110;
        else if( config_tokens[i][3] == "134" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B134;
        else if( config_tokens[i][3] == "150" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B150;
        else if( config_tokens[i][3] == "200" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B200;
        else if( config_tokens[i][3] == "300" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B300;
        else if( config_tokens[i][3] == "600" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B600;
        else if( config_tokens[i][3] == "1200" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B1200;
        else if( config_tokens[i][3] == "1800" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B1800;
        else if( config_tokens[i][3] == "2400" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B2400;
        else if( config_tokens[i][3] == "4800" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B4800;
        else if( config_tokens[i][3] == "9600" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B9600;
        else if( config_tokens[i][3] == "19200" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B19200;
        else if( config_tokens[i][3] == "38400" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B38400;
        else if( config_tokens[i][3] == "57600" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B57600;
        else if( config_tokens[i][3] == "115200" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B115200;
        else if( config_tokens[i][3] == "230400" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B230400;
#  ifndef DTK_ARCH_DARWIN
        else if( config_tokens[i][3] == "460800" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B460800;
        else if( config_tokens[i][3] == "500000" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B500000;
        else if( config_tokens[i][3] == "576000" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B576000;
        else if( config_tokens[i][3] == "921600" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B921600;
        else if( config_tokens[i][3] == "1000000" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B1000000;
        else if( config_tokens[i][3] == "1152000" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B1152000;
        else if( config_tokens[i][3] == "1500000" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B1500000;
        else if( config_tokens[i][3] == "2000000" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B2000000;
        else if( config_tokens[i][3] == "2500000" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B2500000;
        else if( config_tokens[i][3] == "3000000" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B3000000;
        else if( config_tokens[i][3] == "3500000" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B3500000;
        else if( config_tokens[i][3] == "4000000" )
          m_configuration[config_tokens[i][1]].m_baud_flag = B4000000;
#  endif
#endif
        else
        {
          dtkMsg.add( DTKMSG_ERROR, "The specified baud rate is not a valid value.\n"
              "Valid values are: %s.\n"
              "Verify that your device can handle the baud rate\n"
              "that you specify or damage could occur.\n", valid_values.c_str() );
          return -1;
        }
        m_configuration[config_tokens[i][1]].m_baud = config_tokens[i][3];
      }
      else
      {
        std::vector<std::string> line;
        for( unsigned int j=2;j<config_tokens[i].size();j++ )
        {
          line.push_back( config_tokens[i][j] );
        }
        m_configuration[config_tokens[i][1]].m_config.push_back( line );
      }
      m_configuration[config_tokens[i][1]].has_options = true;
    }
    else
    {
      dtkMsg.add( DTKMSG_ERROR, "The configuration parameter %s is invalid\n",
          config_tokens[i][0].c_str() );
      return -1;
    }
  }
  return 0;
}

int configParser::parse( const char* file,
    const char* name, const char* config_file,
    const char* dso_path )
{
  if( !config_file )
  {
    dtkMsg.add( DTKMSG_ERROR, "The arg parameter passed with this serviceDSO\n"
        "may not be NULL\n" );
    return -1;
  }
  if( !parse( config_file, dso_path ) )
  {
    return -1;
  }

//***********************************************************************
// Need to make sure that the service file and name exist within the file
// and if not then how to handle it??????????????????????????????????????
//***********************************************************************

  return 0;
}

int configParser::parse( const char* config_file,
    const char* dso_path )
{
  if( !config_file )
  {
    dtkMsg.add( DTKMSG_ERROR, "The arg parameter passed with this serviceDSO\n"
        "may not be NULL\n" );
    return -1;
  }

  std::vector<std::string> config_paths;
  if( findConfigFile( config_file, dso_path, config_paths ) )
  {
    dtkMsg.add( DTKMSG_ERROR, "Unable to locate configuration file %s in:\n",
      config_file );
    for( unsigned int i=0;i<config_paths.size();i++ )
      dtkMsg.append( "%s\n", config_paths[i].c_str() );
    dtkMsg.append( "\n" );
    return -1;
  }

  std::vector<std::vector<std::string> > config_tokens;
  tokenizeConfigBuffer( config_tokens );  

  return populateFields( config_tokens );
}

