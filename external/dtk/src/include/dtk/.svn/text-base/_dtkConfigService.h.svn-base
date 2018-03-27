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

#include <string>
#include <map>

/*! \class dtkConfigService _dtkConfigService.h dtk.h
 *
 *  \brief Virtual base class for DSO loaded DTK services which use standard
 *   configuration options.
 *
 * This pure virtual base class is used to construct DTK dynamic
 * shared object (DSO) loadable configurable services.  An object of this class can
 * be created in a running DTK server by being loaded as a DSO in a
 * DTK server, <b>dtk-server</b>.
 *
 * A derived classes constructor and over written
 * <b>dtkService::serve</b> method are required.  All other
 * <b>dtkService</b> virtual methods are not required to be over
 * written to make a usable DTK server loadable DSO service.
 * All configuration options are accessible through the accessor
 * functions provided by this class including a pointer to any
 * dtkCalibration objects that have been loaded for use with
 * this service.
 */

class dtkCalibration;
class ServiceManager;

class DTKAPI dtkConfigService : public dtkService
{
public:
  dtkConfigService( const char* configfile = 0 );
  ~dtkConfigService();

  friend class ServiceManager;

  inline std::string getServiceType() { return m_service_type; };
  inline std::string getName() { return m_name; };
  inline bool getTrackd() { return m_trackd; };
  inline std::vector<std::string> getTrackdType() { return m_trackd_type; };
  inline std::vector<std::string> getShmName() { return m_shm_name; };
  inline std::map<std::string,int> getShmSize() { return m_shm_size; };
  inline std::string getPort() { return m_port; };
  inline std::string getBaud() { return m_baud; };
  inline int getBaudFlag() { return m_baud_flag; };
  inline std::vector<std::vector<std::string> > getConfig() { return m_config; };
  inline dtkCalibration* getCalibrator() { return m_calibrator; };
  inline std::vector<std::string> getCalibratorParams() { return m_calibrator_params; };
  inline std::string getCalibratorName() { return m_calibrator_name; };
  inline std::string getCalibrationConfigPath()
      { return m_calibration_config_path; };

private:
  std::string m_service_type;
  std::string m_name;
  bool m_trackd;
  std::vector<std::string> m_trackd_type;
  std::vector<std::string> m_shm_name;
  std::map<std::string,int> m_shm_size;
  std::string m_port;
  std::string m_baud;
  int m_baud_flag;
  std::vector<std::vector<std::string> > m_config;
  dtkCalibration* m_calibrator;
  std::vector<std::string> m_calibrator_params;
  std::string m_calibrator_name;
  void* m_calibratorLoaderFunc;
  std::string m_calibration_config_path;
};

