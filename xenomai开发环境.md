>> 安装步骤参照 http://rtt-lwr.readthedocs.io/en/latest/rtpc/xenomai.html

## 安装虚拟机软件

## 安装Ubuntu 14.04

## 安装Xenomai 2.6.5

- 下载xenomai源代码

```
wget http://xenomai.org/downloads/xenomai/stable/xenomai-2.6.5.tar.bz2
tar xfvj xenomai-2.6.5.tar.bz2
```
- 下载Linux内核3.18.30源代码
```
    wget https://www.kernel.org/pub/linux/kernel/v3.x/linux-3.18.20.tar.gz
    tar xfv linux-3.18.20.tar.gz
```

- 下载相关工具
```
    sudo apt install kernel-package
```

- 给Linux内核打上Xenomai ipipe补丁

```
    cd linux-3.18.20
    ../xenomai-2.6.5/scripts/prepare-kernel.sh
```
按回车选择缺省选项

- 配置内核
```
    make menuconfig
```
- 相关选项的修改：

```
    * Real-time sub-system
      --> Xenomai (Enable)
      --> Nucleus (Enable)
    * Power management and ACPI options
      --> Run-time PM core functionality (Disable)
      --> ACPI (Advanced Configuration and Power Interface) Support
          --> Processor (Disable)
      --> CPU Frequency scaling
          --> CPU Frequency scaling (Disable)
      --> CPU idle
          --> CPU idle PM support (Disable)
    * Pocessor type and features
      --> Processor family
          --> Core 2/newer Xeon (if \"cat /proc/cpuinfo | grep family\" returns 6, set as Generic otherwise)

    Optional Options (RECOMMENDED):
    * General setup
      --> Local version - append to kernel release: -xenomai-2.6.5
      --> Timers subsystem
          --> High Resolution Timer Support (Verify you have HRT ON)
    * Pocessor type and features
      --> Processor family
          --> SMT (Hyperthreading) scheduler support (Disable)
          --> Preemption Model
              --> Voluntary Kernel Preemption (Desktop)
    * Power management and ACPI options
      --> Memory power savings
          --> Intel chipset idle memory power saving driver
```

- 适度增加相关资源的配置数量

```    
        * Real-time sub-system
          --> Number of registry slots
              --> 4096
          --> Size of the system heap
              --> 2048 Kb
          --> Size of the private stack pool
              --> 1024 Kb
          --> Size of private semaphores heap
              --> 48 Kb
          --> Size of global semaphores heap
              --> 48 Kb
```
保存退出

- 编译内核：

```
    CONCURRENCY_LEVEL=$(nproc) make-kpkg --rootcmd fakeroot --initrd kernel_image kernel_headers
```
- 安装内核

```    cd ..
    sudo dpkg -i linux-headers-3.18.20-xenomai-2.6.5_3.18.20-xenomai-2.6.5-10.00.Custom_amd64.deb linux-image-3.18.20-xenomai-2.6.5_3.18.20-xenomai-2.6.5-10.00.Custom_amd64.deb
```

- 配置GRUB

    GRUB_DEFAULT=saved
    GRUB_SAVEDEFAULT=true
    #GRUB_HIDDEN_TIMEOUT=0
    GRUB_HIDDEN_TIMEOUT_QUIET=true
    GRUB_TIMEOUT=5
    GRUB_CMDLINE_LINUX_DEFAULT="quiet splash xeno_nucleus.xenomai_gid=1234 xenomai.allowed_group=1234"
    GRUB_CMDLINE_LINUX=""


- 更新 GRUB 并重启

```
    sudo update-grub
    sudo reboot
```

- 添加非root用户
```
    sudo addgroup xenomai --gid 1234
    sudo addgroup root xenomai
    sudo usermod -a -G xenomai $USER
```

- 安装Xenomai库
```
    cd xenomai-2.6.5/
    ./configure
    make -j$(nproc)
    sudo make install
```

- 更新bashrc文件
```
    echo '
    #### Xenomai
    export XENOMAI_ROOT_DIR=/usr/xenomai
    export XENOMAI_PATH=/usr/xenomai
    export PATH=$PATH:$XENOMAI_PATH/bin
    export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$XENOMAI_PATH/lib/pkgconfig
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$XENOMAI_PATH/lib
    export OROCOS_TARGET=xenomai
    ' >> ~/.bashrc
```
至此xenomai开发环境已建立起来。
