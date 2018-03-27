#! /bin/sh

# flTylium.sh  Notes on understanding frame lock on tylium.cam.nist.gov
# February 27, 2014
# steve@nist.gov

# This has been completely re-writtner based on example from:
# http://us.download.nvidia.com/XFree86/Linux-x86_64/331.49/README/framelock.html
# with driver 331.49 installed

# To properly configure Frame Lock, the following steps should be completed:
#
#  1. Make sure Frame Lock Sync is disabled on all GPUs.
#
#  2. Make sure all display devices that are to be Frame Locked have the same refresh rate.
#
#  3. Configure which (display/GPU) device should be the master.
#
#  4. Configure house sync (if applicable).
#
#  5. Configure the slave display devices.
#
#  6. Enable Frame Lock sync on the master GPU.
#
#  7. Enable Frame Lock sync on the slave GPUs.
#
#  8. Toggle the test signal on the master GPU (for testing the hardware connectivity.)
#
# Based on example:
#  2 Systems, each with 2 GPUs, 1 Frame Lock board and 1 display device per GPU syncing
#  from the first system's first display device:


echo "# - Query Frame Locks:"
  nvidia-settings -q framelocks

  echo Press enter to proceed; read x
echo "# - Disable the house sync signal on the master device"
  nvidia-settings -a [framelock:0]/FrameLockUseHouseSync=0
  nvidia-settings -a [framelock:1]/FrameLockUseHouseSync=0

  echo Press enter to proceed; read x

echo "# - Query the house sync signal on all devices"
  nvidia-settings -q [framelock]/FrameLockUseHouseSync

  echo Press enter to proceed; read x

# ----

echo "# - Make sure Frame Lock sync is disabled on all gpus" 
  nvidia-settings -a [gpu]/FrameLockEnable=0

  echo Press enter to proceed; read x

echo "# - Query Frame Lock sync gpus" 
  nvidia-settings -q [gpu]/FrameLockEnable

  echo Press enter to proceed; read x

# ----

echo "# - Query the enabled displays on the GPUs"
  nvidia-settings -q gpus

  echo Press enter to proceed; read x


echo "# - Check the refresh rate is the same for all displays"
  nvidia-settings -q [gpu]/RefreshRate

  echo Press enter to proceed; read x

# ----

echo "# Disable all Frame locks"
  nvidia-settings -a [gpu]/FrameLockDisplayConfig=0

  echo Press enter to proceed; read x

echo "# - Query the valid Frame Lock configurations for the display devices"
  nvidia-settings -q [gpu]/FrameLockDisplayConfig

  echo Press enter to proceed; read x

echo "# - Set the server display device"
  #nvidia-settings -a [dpy:3]/FrameLockDisplayConfig=2
  nvidia-settings -a [gpu:0]/FrameLockDisplayConfig=2

  echo Press enter to proceed; read x

echo "# - Set the slave display devices"
  #nvidia-settings -a [dpy:9]/FrameLockDisplayConfig=1
  #nvidia-settings -a [dpy:15]/FrameLockDisplayConfig=1
  #nvidia-settings -a [dpy:21]/FrameLockDisplayConfig=1

  nvidia-settings -a [gpu:1]/FrameLockDisplayConfig=1
  echo Press enter to proceed; read x
  nvidia-settings -a [gpu:2]/FrameLockDisplayConfig=1
  echo Press enter to proceed; read x
  nvidia-settings -a [gpu:3]/FrameLockDisplayConfig=1
  echo Press enter to proceed; read x


echo "# - Enable Frame Lock on server"
  nvidia-settings -a [gpu:0]/FrameLockEnable=1

  echo Press enter to proceed; read x


echo "# - Enable Frame Lock on slave devices"
  nvidia-settings -a [gpu:1]/FrameLockEnable=1
  echo Press enter to proceed; read x
  nvidia-settings -a [gpu:2]/FrameLockEnable=1
  echo Press enter to proceed; read x
  nvidia-settings -a [gpu:3]/FrameLockEnable=1

  echo Press enter to proceed; read x


echo "# - Query Frame Locks:"
  nvidia-settings -q framelocks

  echo Press enter to proceed; read x


echo "# - Toggle the test signal (on the master GPU)"
  nvidia-settings -a [gpu:0]/FrameLockTestSignal=1
  nvidia-settings -a [gpu:0]/FrameLockTestSignal=0

echo "Done"
