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
#ifndef __CONFIG_PARSER__
#define __CONFIG_PARSER__

class DTKSERVERAPI configParser
{
public:
  configParser();
  ~configParser();

  //* This is for loading config files and instantiating all DSO objects
  //* i.e. dtk-server -c my.config
  int parse( const char* config_file, const char* dso_path );

  //* This is for loading config files associated with a preinstantiated dtkConfigService
  //* i.e. dtk-loadService somedso somedsoname somearg
  int parse( const char* file, const char* name,
      const char* config_file, const char* dso_path );

  inline std::map<std::string, serviceConfig> getConfiguration() { return m_configuration; };

private:
  std::map<std::string, serviceConfig> m_configuration;
  char* config_buffer;

  // Tokenize string based on arbitrary delimiters
//  void tokenize( const std::string& str, std::vector<std::string>& tokens,
//      const std::string& delimiters = " " );

  int findConfigFile( const std::string& config_file, const std::string& dso_path,
      std::vector<std::string>& config_paths );

  int loadConfigFile( std::ifstream& config_stream );

  int tokenizeConfigBuffer( std::vector<std::vector<std::string> >& config_tokens );

  int populateFields( std::vector<std::vector<std::string> >& config_tokens );
};

#endif

