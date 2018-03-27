Some example commads taken from the  nvidia-settings-user-guide.txt

  2. 2 Systems, each with 2 GPUs, 1 Frame Lock board and 1 display device per
     GPU syncing from the first system's first display device:
     
       # - Make sure frame lock sync is disabled
       nvidia-settings -a myserver:0[gpu:0]/FrameLockEnable=0
       nvidia-settings -a myserver:0[gpu:1]/FrameLockEnable=0
       nvidia-settings -a myslave1:0[gpu:0]/FrameLockEnable=0
       nvidia-settings -a myslave1:0[gpu:1]/FrameLockEnable=0
     
       # - Query the enabled displays on the GPUs
       nvidia-settings -q myserver:0[gpu:0]/EnabledDisplays
       nvidia-settings -q myserver:0[gpu:1]/EnabledDisplays
       nvidia-settings -q myslave1:0[gpu:0]/EnabledDisplays
       nvidia-settings -q myslave1:0[gpu:1]/EnabledDisplays
     
       # - Check the refresh rate is the same for all displays
       nvidia-settings -q myserver:0[gpu:0]/RefreshRate
       nvidia-settings -q myserver:0[gpu:1]/RefreshRate
       nvidia-settings -q myslave1:0[gpu:0]/RefreshRate
       nvidia-settings -q myslave1:0[gpu:1]/RefreshRate
     
       # - Make sure the display device we want as master is masterable
       nvidia-settings -q myserver:0[gpu:0]/FrameLockMasterable
     
       # - Set the master display device (CRT-0)
       nvidia-settings -a myserver:0[gpu:0]/FrameLockMaster=0x00000001
     
       # - Disable the house sync signal on the master device
       nvidia-settings -a myserver:0[framelock:0]/FrameLockUseHouseSync=0
     
       # - Set the slave display devices
       nvidia-settings -a myserver:0[gpu:1]/FrameLockSlaves=0x00000001
       nvidia-settings -a myslave1:0[gpu:0]/FrameLockSlaves=0x00000001
       nvidia-settings -a myslave1:0[gpu:1]/FrameLockSlaves=0x00000001
     
       # - Enable frame lock on server
       nvidia-settings -a myserver:0[gpu:0]/FrameLockEnable=1
     
       # - Enable frame lock on slave devices
       nvidia-settings -a myserver:0[gpu:1]/FrameLockEnable=1
       nvidia-settings -a myslave1:0[gpu:0]/FrameLockEnable=1
       nvidia-settings -a myslave1:0[gpu:1]/FrameLockEnable=1
     
       # - Toggle the test signal
       nvidia-settings -a myserver:0[gpu:0]/FrameLockTestSignal=1
       nvidia-settings -a myserver:0[gpu:0]/FrameLockTestSignal=0
