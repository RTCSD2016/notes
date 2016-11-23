> 开发环境搭建主要参考了 http://rtt-lwr.readthedocs.io/en/latest/rtpc/xenomai.html ，如果在PC机上直接安装ubuntu 14.04，则省掉windows上虚拟机安装步骤。

## 一、在windows操作系统中安装VirtualBox虚拟机软件

下载地址： https://www.virtualbox.org/wiki/Downloads


## 二、在虚拟机上安装Ubuntu 14.04

- 下载ubuntu映像文件

到Ubuntu官网上下载映像文件 ubuntu-14.04.3-desktop-amd64.iso

- 创建新虚拟机并安装ubuntu14.04

打开VirtualBox，点击“新建”按钮创建新的虚拟机

1） 在第一个对话框中设置

虚拟电脑名称为rtcsd（也可自己起名字）

类型为Linux

版本为Ubuntu(64bit)

2） 在第一个对话框中，内存大小可使用默认值

3） 在第三个对话框中，选择“现在创建虚拟硬盘”

4） 在第四个对话框中，选择“VDI（VirtualBox磁盘映像）”

5） 在第五个对话框中，选择“动态分配”虚拟硬盘。

6） 在第六个对话框中，设置虚拟硬盘大小，因为要进行Linux内核编译，最好把大小设为32GB

创建虚拟机后点“启动”按钮，弹出“选择启动盘”对话框，选中之前下载的ubuntu-14.04.3-desktop-amd64.iso文件，按对话框上的“启动”按钮继续。

进入Ubuntu安装界面，选择“Install Ubuntu”开始安装。

安装完成后重启虚拟机，确认可以连接互联网。

- 更新源，如有连接问题，可编辑 /etc/apt/sources.list，更换或添加源服务器：

```
sudo apt-get update
```
- 设置VirtualBox虚拟机与windows的共享目录

1）点击菜单 设备->安装增强功能，会弹出对话框要求输入密码。观察提示信息，等待安装完成。

2）在windows下建立一个共享目录，如在D盘建立 "D:\myshare"。

3）点击菜单 设备->共享文件夹，点击添加共享文件夹， 设置共享文件夹路径为 "D:\myshare"，命名一个共享文件夹名称如“vboxshare”，选中"固定分配"选项框，不选"只读分配"和"自动挂载"。

4）重启ubuntu。

5）在终端中输入：sudo mount -t vboxsf vboxshare ~/share 即可实现共享文件夹的设置，实现在虚拟机下的ubuntu和windows共享文件的操作。

## 三、安装Xenomai 2.6.5

- 在home目录下创建一个内核编译的工作目录

```
mkdir xenomai
cd xenomai
```

- 下载xenomai源代码并解压缩到当前目录

```
wget http://xenomai.org/downloads/xenomai/stable/xenomai-2.6.5.tar.bz2
tar xfvj xenomai-2.6.5.tar.bz2
```

- 下载Linux内核3.18.30源代码并解压缩到当前目录

```
    wget https://www.kernel.org/pub/linux/kernel/v3.x/linux-3.18.20.tar.gz
    tar xfv linux-3.18.20.tar.gz
```

- 下载ubuntu的内核编译工具

```
    sudo apt install kernel-package
    sudo apt-get install libncurses-dev
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

如提示分辨率不够无法进入图形化配置界面，则确认是否安装成功了虚拟机的增强功能。进入配置界面后进行相关选项的修改：

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

可适度增加相关资源的配置数量。

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

修改完成后保存退出。

- 编译内核：

```
    sudo CONCURRENCY_LEVEL=$(nproc) make-kpkg --rootcmd fakeroot --initrd kernel_image kernel_headers
```

- 安装内核

```   
    cd ..
    sudo dpkg -i linux-headers-3.18.20-xenomai-2.6.5_3.18.20-xenomai-2.6.5-10.00.Custom_amd64.deb linux-image-3.18.20-xenomai-2.6.5_3.18.20-xenomai-2.6.5-10.00.Custom_amd64.deb
```

- 配置GRUB，编辑/etc/default/grub文件

```
sudo gedit /etc/default/grub
```

相关选项设置为：

```
    GRUB_DEFAULT=saved
    GRUB_SAVEDEFAULT=true
    GRUB_HIDDEN_TIMEOUT_QUIET=true
    GRUB_TIMEOUT=5
    GRUB_CMDLINE_LINUX_DEFAULT="quiet splash xeno_nucleus.xenomai_gid=1234 xenomai.allowed_group=1234"
    GRUB_CMDLINE_LINUX=""
```

- 更新 GRUB 并重启

```
    sudo update-grub
    sudo reboot
```
- 虚拟机启动时出现GRUB菜单，选择xenomai选项进入系统，在命令行环境下执行：

```
uname -r
```

如显示3.18.20-xenomai-2.6.5，则表明xenomai内核已安装成功。

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
    ' >> ~/.bashrc
```

至此xenomai开发环境已建立起来。
