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

/*! \class dtkCalibration _dtkCalibration.h dtk.h
 *
 *  \brief Virtual base class for DSO loaded DTK calibrator.
 *
 * This pure virtual base class is used to construct DTK dynamic
 * shared object (DSO) loadable calibration for use with dtkConfigService(s).
 */

class DTKAPI dtkCalibration
{
public:
  //! Constructor.
  dtkCalibration(void);

  //! Destructor.
  virtual ~dtkCalibration(void);

  //! Initialize calibration
  /*! This should be used to initialize the calibration. The params parameter will
   *  contain the parameters passed to the calibration through the configuration file.
   *  \param svc the dtkConfigService object which will use this calibration
   *  \param params is a list of parameters passed through the configuration file
   *  \return 0 on success, other for failure
   */
  virtual int initCalibration( dtkConfigService* svc, std::vector<std::string> params ) = 0;

  /*! Performs the calibration correction on the raw data from the tracker
   *  \param tracker_data is a pointer to the raw tracker data
   *  \param data is any additional data that might be sent
   *  \return 0 on success, other for failure
  */
  virtual int rawCalibrate( float* tracker_data, void* data ) = 0;

  /*! Performs the calibration correction on the data after transformation and before
   *  conversion to diverse units since the VRCO shared memory should also be calibrated.
   *  \param tracker_data is a pointer to the raw tracker data
   *  \param data is any additional data that might be sent
   *  \return 0 on success, other for failure
  */
  virtual int calibrate( float* tracker_data, void* data ) = 0;
};

#ifdef __DTKCALIBRATION_LOAD_IT__
static dtkCalibration *dtkDSO_loader(const char *ptr=NULL);
static int dtkDSO_unloader(dtkCalibration *a);

extern "C"
{

#ifdef DTK_ARCH_WIN32_VCPP
  extern __declspec(dllexport) void *__dtkDSO_loader(void);
  extern __declspec(dllexport) void *__dtkDSO_unloader(void);
#endif

  void *__dtkDSO_loader(void)
  {
    return (void *) dtkDSO_loader;
  }
  void *__dtkDSO_unloader(void)
  {
    return (void *) dtkDSO_unloader;
  }
}

#endif


