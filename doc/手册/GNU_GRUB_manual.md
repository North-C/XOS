# OUROS-GNU GRUB manual 2.06

这是 GNU GRUB 的文档，GNU GRUB 即 GRand Unified Bootloader，是一个灵活而强大的引导加载程序，适用于各种架构。

此版本记录了 2.06 版。

本手册适用于 GNU GRUB（2.06 版，2021 年 5 月 10 日）。****

Copyright © 1999,2000,2001,2002,2004,2006,2008,2009,2010,2011,2012,2013 Free Software Foundation, Inc.

> Permission is granted to copy, distribute and/or modify this document under the terms of the GNU Free Documentation License, Version 1.2 or any later version published by the Free Software Foundation; with no Invariant Sections.

| 目录                                                         |      | 简介                                                 |
| ------------------------------------------------------------ | ---- | ---------------------------------------------------- |
| • [Introduction](https://www.gnu.org/software/grub/manual/grub/grub.html#Introduction): |      | Capturing the spirit of GRUB                         |
| • [Naming convention](https://www.gnu.org/software/grub/manual/grub/grub.html#Naming-convention): |      | Names of your drives in GRUB                         |
| • [OS-specific notes about grub tools](https://www.gnu.org/software/grub/manual/grub/grub.html#OS_002dspecific-notes-about-grub-tools): |      | Some notes about OS-specific behaviour of GRUB tools |
| • [Installation](https://www.gnu.org/software/grub/manual/grub/grub.html#Installation): |      | Installing GRUB on your drive                        |
| • [Booting](https://www.gnu.org/software/grub/manual/grub/grub.html#Booting): |      | How to boot different operating systems              |
| • [Configuration](https://www.gnu.org/software/grub/manual/grub/grub.html#Configuration): |      | Writing your own configuration file                  |
| • [Theme file format](https://www.gnu.org/software/grub/manual/grub/grub.html#Theme-file-format): |      | Format of GRUB theme files                           |
| • [Network](https://www.gnu.org/software/grub/manual/grub/grub.html#Network): |      | Downloading OS images from a network                 |
| • [Serial terminal](https://www.gnu.org/software/grub/manual/grub/grub.html#Serial-terminal): |      | Using GRUB via a serial line                         |
| • [Vendor power-on keys](https://www.gnu.org/software/grub/manual/grub/grub.html#Vendor-power_002don-keys): |      | Changing GRUB behaviour on vendor power-on keys      |
| • [Images](https://www.gnu.org/software/grub/manual/grub/grub.html#Images): |      | GRUB image files                                     |
| • [Core image size limitation](https://www.gnu.org/software/grub/manual/grub/grub.html#Core-image-size-limitation): |      | GRUB image files size limitations                    |
| • [Filesystem](https://www.gnu.org/software/grub/manual/grub/grub.html#Filesystem): |      | Filesystem syntax and semantics                      |
| • [Interface](https://www.gnu.org/software/grub/manual/grub/grub.html#Interface): |      | The menu and the command-line                        |
| • [Environment](https://www.gnu.org/software/grub/manual/grub/grub.html#Environment): |      | GRUB environment variables                           |
| • [Commands](https://www.gnu.org/software/grub/manual/grub/grub.html#Commands): |      | The list of available builtin commands               |
| • [Internationalisation](https://www.gnu.org/software/grub/manual/grub/grub.html#Internationalisation): |      | Topics relating to language support                  |
| • [Security](https://www.gnu.org/software/grub/manual/grub/grub.html#Security): |      | Authentication, authorisation, and signatures        |
| • [Platform limitations](https://www.gnu.org/software/grub/manual/grub/grub.html#Platform-limitations): |      | The list of platform-specific limitations            |
| • [Platform-specific operations](https://www.gnu.org/software/grub/manual/grub/grub.html#Platform_002dspecific-operations): |      | Platform-specific operations                         |
| • [Supported kernels](https://www.gnu.org/software/grub/manual/grub/grub.html#Supported-kernels): |      | The list of supported kernels                        |
| • [Troubleshooting](https://www.gnu.org/software/grub/manual/grub/grub.html#Troubleshooting): |      | Error messages produced by GRUB                      |
| • [Invoking grub-install](https://www.gnu.org/software/grub/manual/grub/grub.html#Invoking-grub_002dinstall): |      | How to use the GRUB installer                        |
| • [Invoking grub-mkconfig](https://www.gnu.org/software/grub/manual/grub/grub.html#Invoking-grub_002dmkconfig): |      | Generate a GRUB configuration file                   |
| • [Invoking grub-mkpasswd-pbkdf2](https://www.gnu.org/software/grub/manual/grub/grub.html#Invoking-grub_002dmkpasswd_002dpbkdf2): |      | Generate GRUB password hashes                        |
| • [Invoking grub-mkrelpath](https://www.gnu.org/software/grub/manual/grub/grub.html#Invoking-grub_002dmkrelpath): |      | Make system path relative to its root                |
| • [Invoking grub-mkrescue](https://www.gnu.org/software/grub/manual/grub/grub.html#Invoking-grub_002dmkrescue): |      | Make a GRUB rescue image                             |
| • [Invoking grub-mount](https://www.gnu.org/software/grub/manual/grub/grub.html#Invoking-grub_002dmount): |      | Mount a file system using GRUB                       |
| • [Invoking grub-probe](https://www.gnu.org/software/grub/manual/grub/grub.html#Invoking-grub_002dprobe): |      | Probe device information for GRUB                    |
| • [Invoking grub-script-check](https://www.gnu.org/software/grub/manual/grub/grub.html#Invoking-grub_002dscript_002dcheck): |      | Check GRUB script file for syntax errors             |
| • [Obtaining and Building GRUB](https://www.gnu.org/software/grub/manual/grub/grub.html#Obtaining-and-Building-GRUB): |      | How to obtain and build GRUB                         |
| • [Reporting bugs](https://www.gnu.org/software/grub/manual/grub/grub.html#Reporting-bugs): |      | Where you should send a bug report                   |
| • [Future](https://www.gnu.org/software/grub/manual/grub/grub.html#Future): |      | Some future plans on GRUB                            |
| • [Copying This Manual](https://www.gnu.org/software/grub/manual/grub/grub.html#Copying-This-Manual): |      | Copying This Manual                                  |
| • [Index](https://www.gnu.org/software/grub/manual/grub/grub.html#Index): |      |                                                      |

## 1 Introduction - 简介

### 1.1 概述

简而言之，*boot loader* 是计算机启动时运行的第一个软件程序。它负责加载并将控制权转移到操作系统内核软件（例如 Linux 或 GNU Mach）。内核初始化操作系统的其余部分（例如 GNU system）。

GNU GRUB 是一个非常强大的引导加载程序，它可以加载各种各样的 free operating system，还可以使用 chain-loading 功能加载 proprietary operating system。该程序和本手册都与该计算机平台紧密绑定，但将来可能会解决移植到其他平台的问题。

GRUB 的重要特性之一是**灵活性**。 GRUB 了解文件系统和内核可执行文件的格式，因此您可以按照自己喜欢的方式加载任意操作系统，而无需记录内核在磁盘上的物理位置。因此，只需指定内核的文件名、以及内核所在文件系统的驱动器和分区，就可以加载内核。

使用 GRUB 引导时，您可以使用命令行界面（see [Command-line interface](https://www.gnu.org/software/grub/manual/grub/grub.html#Command_002dline-interface)）或菜单界面（see [Menu interface](https://www.gnu.org/software/grub/manual/grub/grub.html#Menu-interface)）。使用命令行界面，您可以手动键入内核的驱动器规格和文件名。在菜单界面中，您只需使用方向键选择一个操作系统。该菜单基于您预先准备的配置文件（see [Configuration](https://www.gnu.org/software/grub/manual/grub/grub.html#Configuration)）。在菜单中，您可以切换到命令行模式，反之亦然。您甚至可以在使用菜单项之前对其进行编辑。

在接下来的章节中，您将逐步学习如何为 GRUB 指定驱动器、分区和文件名（see [Naming convention](https://www.gnu.org/software/grub/manual/grub/grub.html#Naming-convention)），如何在驱动器上安装 GRUB（see [Installation](https://www.gnu.org/software/grub/manual/grub/grub.html#Installation)），以及如何引导您的操作系统（see [Booting](https://www.gnu.org/software/grub/manual/grub/grub.html#Booting)）。

> 原文：In the following chapters, you will learn how to specify a drive, a partition, and a file name (see [Naming convention](https://www.gnu.org/software/grub/manual/grub/grub.html#Naming-convention)) to GRUB, how to install GRUB on your drive (see [Installation](https://www.gnu.org/software/grub/manual/grub/grub.html#Installation)), and how to boot your OSes (see [Booting](https://www.gnu.org/software/grub/manual/grub/grub.html#Booting)), step by step.

### 1.2 GRUB 历史

GRUB originated in 1995 when Erich Boleyn was trying to boot the GNU Hurd with the University of Utah’s Mach 4 microkernel (now known as GNU Mach). Erich and Brian Ford designed the Multiboot Specification (see [Motivation](http://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Top) in The Multiboot Specification), because they were determined not to add to the large number of mutually-incompatible PC boot methods.

Erich then began modifying the FreeBSD boot loader so that it would understand Multiboot. He soon realized that it would be a lot easier to write his own boot loader from scratch than to keep working on the FreeBSD boot loader, and so GRUB was born.

Erich added many features to GRUB, but other priorities prevented him from keeping up with the demands of its quickly-expanding user base. In 1999, Gordon Matzigkeit and Yoshinori K. Okuji adopted GRUB as an official GNU package, and opened its development by making the latest sources available via anonymous CVS. See [Obtaining and Building GRUB](https://www.gnu.org/software/grub/manual/grub/grub.html#Obtaining-and-Building-GRUB), for more information.

Over the next few years, GRUB was extended to meet many needs, but it quickly became clear that its design was not keeping up with the extensions being made to it, and we reached the point where it was very difficult to make any further changes without breaking existing features. Around 2002, Yoshinori K. Okuji started work on PUPA (Preliminary Universal Programming Architecture for GNU GRUB), aiming to rewrite the core of GRUB to make it cleaner, safer, more robust, and more powerful. PUPA was eventually renamed to GRUB 2, and the original version of GRUB was renamed to GRUB Legacy. Small amounts of maintenance continued to be done on GRUB Legacy, but the last release (0.97) was made in 2005 and at the time of writing it seems unlikely that there will be another.

By around 2007, GNU/Linux distributions started to use GRUB 2 to limited extents, and by the end of 2009 multiple major distributions were installing it by default.

### 1.3 与以前版本的区别（Differences from previous versions）

GRUB2 是对 GRUB（参见历史）的重写，尽管它与以前的版本（现在称为 GRUB Legacy）有许多共同之处。 GRUB Legacy 的用户可能需要一些指导才能使用新版本。

* 配置文件有一个新名称（`grub.cfg` 而不是 `menu.lst` 或 `grub.conf`）、新语法（see [Configuration](https://www.gnu.org/software/grub/manual/grub/grub.html#Configuration)）和许多新命令（see [Commands](https://www.gnu.org/software/grub/manual/grub/grub.html#Commands)）。尽管大多数 GRUB Legacy 用户不会觉得语法太奇怪，但是不能直接复制旧的配置。
* grub.cfg 通常由 grub-mkconfig 自动生成(see [Simple configuration](https://www.gnu.org/software/grub/manual/grub/grub.html#Simple-configuration))，这使得版本化的内核升级变得更加容易。
* GRUB device names 中的分区号（partition numbers）现在从 1 开始，而不是 0（see [Naming convention](https://www.gnu.org/software/grub/manual/grub/grub.html#Naming-convention)）。
* 配置文件现在用更完整的脚本语言编写：可以使用变量、条件和循环。
* 通过使用 GRUB 中的 `save_env` 和 `load_env` 命令以及 `grub_editenv` utility，可以在重新启动后使用少量持久存储。这不是在所有配置中都可用（see [Environment block](https://www.gnu.org/software/grub/manual/grub/grub.html#Environment-block)）。
* GRUB2 有更可靠的方法来查找它自己的文件和多磁盘系统上的目标内核文件，并且具有使用文件系统标签或通用唯一标识符 (UUID) 查找设备的命令（see [search](https://www.gnu.org/software/grub/manual/grub/grub.html#search)）。
* 除了 GRUB Legacy 支持的 PC BIOS 系统之外，GRUB2 还可用于其他几种类型的系统：PC EFI、PC coreboot、PowerPC、SPARC 和 MIPS Lemote Yeeloong 均受支持。
* 支持更多文件系统，包括但不限于 ext4、HFS+ 和 NTFS。
* GRUB2 可以直接从 LVM 和 RAID 设备读取文件。
* 提供图形终端和图形菜单系统。
* GRUB2 的界面可以翻译，包括菜单条目名称。
* 组成 GRUB 的图像文件（参见图像）已重新组织；Stage 1、Stage 1.5 和 Stage 2 已不复存在。
* GRUB2 将许多设施放在动态加载的模块中，允许 core image 更小，并允许以更灵活的方式构建 core image。

### 1.4 GRUB 功能（GRUB feature）

GRUB 的主要要求是它符合 Multiboot 规范，在 ` Multiboot Specification` 中的 `Motivation` 中进行了详细描述。

其他目标，按大致的重要性顺序列出：

* 基本功能必须对最终的用户简单明了。
* 丰富的功能支持内核专家和设计者。
* 向后兼容引导 FreeBSD、NetBSD、OpenBSD 和 Linux。专有内核（例如 DOS、Windows NT 和 OS/2）通过链式加载功能得到支持。

除了特定的兼容性模式（chain-loading 和 Linux `piggyback` format）外，所有内核都将以 Multiboot 规范中大致相同的状态启动。目前仅支持加载 1MB 或以上的内核。任何低于该边界的加载尝试只会导致立即失败和错误消息（报告问题）。

除了上述要求之外，GRUB 还具有以下特性（请注意，Multiboot 规范并没有要求 GRUB 支持的所有特性）：

* 识别多种可执行格式（**Recognize multiple executable formats**）

  支持许多 a.out 变体和 ELF。符号表也会被加载。

* 支持 non-Multiboot 内核（**Support non-Multiboot kernels**）

  支持许多缺乏 Multiboot 合规性的各种 free 32-bit kernels（主要是 FreeBSD、NetBSD2、OpenBSD 和 Linux）。还支持Chain-loading of other boot loaders。

* 加再多个模块（**Load multiples modules**）

  支持加载多个引导模块。

* 加载配置文件（**Load a configuration file**）

  支持带有预设引导命令的 human-readable 配置文件。还可以动态加载另一个配置文件并将预设配置文件嵌入到 GRUB 映像文件中。命令列表（see [Commands](https://www.gnu.org/software/grub/manual/grub/grub.html#Commands)）是命令行支持的那些命令的超集。[Configuration](https://www.gnu.org/software/grub/manual/grub/grub.html#Configuration)中提供了一个示例配置文件。

* 提供菜单界面（**Provide a menu interface**）

  A menu interface listing preset boot commands, with a programmable timeout, is available. There is no fixed limit on the number of boot entries, and the current implementation has space for several hundred.

* 拥有灵活的命令行界面（**Have a flexible command-line interface**）

  A fairly flexible command-line interface, accessible from the menu, is available to edit any preset commands, or write a new boot command set from scratch. If no configuration file is present, GRUB drops to the command-line.

  The list of commands (see [Commands](https://www.gnu.org/software/grub/manual/grub/grub.html#Commands)) are a subset of those supported for configuration files. Editing commands closely resembles the Bash command-line (see [Command Line Editing](https://www.gnu.org/software/grub/manual/grub/features.html#Command-Line-Editing) in Bash Features), with `TAB`-completion of commands, devices, partitions, and files in a directory depending on context.

* 支持多种文件系统类型（**Support multiple filesystem types**）

  Support multiple filesystem types transparently, plus a useful explicit blocklist notation. The currently supported filesystem types are *Amiga Fast FileSystem (AFFS)*, *AtheOS fs*, *BeFS*, *BtrFS* (including raid0, raid1, raid10, gzip and lzo), *cpio* (little- and big-endian bin, odc and newc variants), *Linux ext2/ext3/ext4*, *DOS FAT12/FAT16/FAT32*, *exFAT*, *F2FS*, *HFS*, *HFS+*, *ISO9660* (including Joliet, Rock-ridge and multi-chunk files), *JFS*, *Minix fs* (versions 1, 2 and 3), *nilfs2*, *NTFS* (including compression), *ReiserFS*, *ROMFS*, *Amiga Smart FileSystem (SFS)*, *Squash4*, *tar*, *UDF*, *BSD UFS/UFS2*, *XFS*, and *ZFS* (including lzjb, gzip, zle, mirror, stripe, raidz1/2/3 and encryption in AES-CCM and AES-GCM). See [Filesystem](https://www.gnu.org/software/grub/manual/grub/grub.html#Filesystem), for more information.

* 支持自动解压（**Support automatic decompression**）

  可以解压 `gzip` 或 `xz` 压缩过的文件。此功能对用户是自动且透明的（即所有功能都对指定文件的未压缩内容进行操作）。这大大减少了文件大小和加载时间，对于软盘来说是一个特别大的好处。

  可以想象，某些内核模块应该以压缩状态加载，因此可以指定不同的 module-loading 命令来避免解压缩需要以压缩状态加载的内核模块。

* 访问任何已安装设备上的数据（**Access data on any installed device**）

  支持从 BIOS 识别的软盘或硬盘读取数据，与根设备的设置无关。

* 不受驱动几何体平移的影响（**Be independent of drive geometry translations**）

  Unlike many other boot loaders, GRUB makes the particular drive translation irrelevant. A drive installed and running with one translation may be converted to another translation without any adverse effects or changes in GRUB’s configuration.

* 检测所有已安装的 RAM（**Detect all installed RAM**）

  GRUB can generally find all the installed RAM on a PC-compatible machine. It uses an advanced BIOS query technique for finding all memory regions. As described on the Multiboot Specification (see [Motivation](http://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Top) in The Multiboot Specification), not all kernels make use of this information, but GRUB provides it for those who do.

* 支持逻辑块地址模式（**Support Logical Block Address mode**）

  In traditional disk calls (called *CHS mode*), there is a geometry translation problem, that is, the BIOS cannot access over 1024 cylinders, so the accessible space is limited to at least 508 MB and to at most 8GB. GRUB can’t universally solve this problem, as there is no standard interface used in all machines. However, several newer machines have the new interface, Logical Block Address (*LBA*) mode. GRUB automatically detects if LBA mode is available and uses it if available. In LBA mode, GRUB can access the entire disk.

* 支持网络引导（**Support network booting**）

  GRUB基本上是一个基于磁盘的引导加载程序，但也支持网络。您可以使用 `TFTP` 协议从网络加载操作系统映像。

* 支持远程终端（**Support remote terminals**）

  为了支持没有控制台的计算机，GRUB 提供了远程终端支持，以便您可以从远程主机控制 GRUB。目前仅支持串行终端。

### 1.5 引导加载程序的角色（The role of a boot loader）

以下是 GRUB 狂热者 Gordon Matzigkeit 的一段话：

>Some people like to acknowledge both the operating system and kernel when they talk about their computers, so they might say they use “GNU/Linux” or “GNU/Hurd”. Other people seem to think that the kernel is the most important part of the system, so they like to call their GNU operating systems “Linux systems.”
>
>I, personally, believe that this is a grave injustice, because the *boot loader* is the most important software of all. I used to refer to the above systems as either “LILO”[5](https://www.gnu.org/software/grub/manual/grub/grub.html#FOOT5) or “GRUB” systems.
>
>Unfortunately, nobody ever understood what I was talking about; now I just use the word “GNU” as a pseudonym for GRUB.
>
>So, if you ever hear people talking about their alleged “GNU” systems, remember that they are actually paying homage to the best boot loader around… GRUB!

作为 GRUB 维护者，我们（通常）不会鼓励 Gordon 的狂热程度，但它有助于记住引导加载程序应该得到认可。我们希望您喜欢使用 GNU GRUB，就像我们编写它一样。

## 2 命名约定（Naming convention）

GRUB 中使用的设备语法与您之前在操作系统中看到的略有不同，您需要了解它才能指定`驱动器/分区`。

> 驱动器指硬盘或软盘。

看下面的例子和解释：

```text
(fd0)
```

首先，GRUB 要求设备名称用‘(’和‘)’括起来。 “fd”部分表示它是一张软盘。数字“0”是驱动器号，从零开始计数。这个表达式意味着 GRUB 将使用整个软盘。

```text
(hd0,msdos2)
```

在这里，“hd”表示它是硬盘驱动器。第一个整数'0'表示驱动器号，即第一个硬盘，字符串'msdos'表示分区方案，而第二个整数'2'表示分区号（或 PC slice number，BSD 术语）。**分区号从 1 开始计算，而不是从 0 开始计算**（就像以前版本的 GRUB 一样）。此表达式表示第一个硬盘驱动器的第二个分区。在这种情况下，GRUB 使用磁盘的一个分区，而不是整个磁盘。

```text
(hd0,msdos5)
```

这指定了第一个硬盘驱动器的第一个扩展分区。请注意，**扩展分区的分区号从“5”开始计算，与硬盘上的主分区的实际数量无关。**

```text
(hd1,msdos1,bsd1)
```

这意味着第二个硬盘的第一个 PC slice number 上的 BSD 'a' 分区。

当然，要真正使用 GRUB 访问磁盘或分区，您需要在命令中使用 device specification，例如“set root=(fd0)”或“parttool (hd0,msdos3) hidden-”。为了帮助您找出哪个数字指定了您想要的分区，GRUB 命令行（see [Command-line interface](https://www.gnu.org/software/grub/manual/grub/grub.html#Command_002dline-interface)）选项具有参数补全（argument completion）功能。这意味着，例如，您只需键入：

```text
set root=(
```

后跟一个 `TAB`，GRUB 将显示驱动器、分区或文件名的列表。因此，即使对语法知之甚少，也很容易确定目标分区的名称。

请注意，GRUB 不区分 IDE 和 SCSI - 它只是从零开始计算驱动器编号，而不管它们的类型。通常，任何 IDE 驱动器号都小于任何 SCSI 驱动器号，但如果您通过在 BIOS 中交换 IDE 和 SCSI 驱动器来更改引导顺序，则情况并非如此。

现在的问题是，如何指定文件？再次考虑一个例子：

```text
(hd0,msdos1)/vmlinuz
```

这指定了名为“vmlinuz”的文件，该文件位于第一个硬盘驱动器的第一个分区上。请注意，参数补全也适用于文件名。

这很容易，承认吧。现在阅读下一章，了解如何在驱动器上实际安装 GRUB。

## 3 特定操作系统上 GRUB 工具说明（OS-specific notes about grub tools）

On OS which have device nodes similar to Unix-like OS GRUB tools use the OS name. E.g. for GNU/Linux:

```bash
# grub-install /dev/sda
```

On AROS we use another syntax. For volumes:

```text
//:<volume name>
```

E.g.

```text
//:DH0
```

For disks we use syntax:

```text
//:<driver name>/unit/flags
```

E.g.

```text
# grub-install //:ata.device/0/0
```

On Windows we use UNC path. For volumes it’s typically

```text
\\?\Volume{<GUID>}
\\?\<drive letter>:
```

E.g.

```text
\\?\Volume{17f34d50-cf64-4b02-800e-51d79c3aa2ff}
\\?\C:
```

For disks it’s

```text
\\?\PhysicalDrive<number>
```

E.g.

```text
# grub-install \\?\PhysicalDrive0
```

> 请注意，根据您的shell，您可能需要进一步转义反斜杠。

When compiled with cygwin support then cygwin drive names are automatically when needed. E.g.

```bash
# grub-install /dev/sda
```

## 4 安装（Installation）

为了将 GRUB 安装为引导加载程序，您需要首先在类 UNIX 操作系统下安装 GRUB system 和utilities（see [Obtaining and Building GRUB](https://www.gnu.org/software/grub/manual/grub/grub.html#Obtaining-and-Building-GRUB)）。您可以从源代码压缩包中执行此操作，也可以作为操作系统的包来执行此操作。

完成此操作后，您需要在类 UNIX 操作系统上使用程序 grub-install（see [Invoking grub-install](https://www.gnu.org/software/grub/manual/grub/grub.html#Invoking-grub_002dinstall)）在驱动器（软盘或硬盘）上安装引导加载程序。

GRUB 带有 boot images，通常放在目录 `/usr/lib/grub/<cpu>-<platform>` 中（对于基于 BIOS 的机器 /usr/lib/grub/i386-pc）。此后，最初放置 GRUB 映像的目录（通常是 `/usr/lib/grub/<cpu>-<platform>`）将被称为映像目录，该目录将被称为引导目录，引导程序需要找到它们（usually /boot）。

* 使用 grub-install 安装 GRUB
* 制作可使用 GRUB 引导的 CD-ROM
* 设备映射
* BIOS 安装（ [BIOS installation](https://www.gnu.org/software/grub/manual/grub/grub.html#BIOS-installation)）

### 4.1 使用 grub-install 安装 GRUB

有关在 PC BIOS 平台上安装 GRUB 的位置的信息，see [BIOS installation](https://www.gnu.org/software/grub/manual/grub/grub.html#BIOS-installation)。

为了在类 UNIX 操作系统（例如 GNU）下安装 GRUB，请以超级用户 (root) 身份调用程序 grub-install（see [Invoking grub-install](https://www.gnu.org/software/grub/manual/grub/grub.html#Invoking-grub_002dinstall)）。

用法基本上很简单。您只需为程序指定一个参数，即引导加载程序的安装位置。例如，在 Linux 下，以下命令会将 GRUB 安装到第一个 IDE 磁盘的 MBR 中：

```bash
# grub-install /dev/sda
```

同样，在 GNU/Hurd 下，这具有相同的效果：

```bash
# grub-install /dev/hd0
```

但是上面所有的例子都假设 GRUB 应该把镜像放在 /boot 目录下。如果希望 GRUB 将映像放在 /boot 以外的目录下，则需要指定选项 `--boot-directory`。典型用法是创建带有文件系统的 GRUB 引导软盘。这是一个例子：

```bash
# mke2fs /dev/fd0
# mount -t ext2 /dev/fd0 /mnt
# mkdir /mnt/boot
# grub-install --boot-directory=/mnt/boot /dev/fd0
# umount /mnt
```

> mk2fs：用于建立 ext2 文件系统
>
> mount：用于挂在 Linux 系统外的文件，-t 指定文件系统类型，将 /dev/fd0 挂载到 /mnt
>
> mkdir：创建目录
>
> umount：卸载

某些 BIOS 存在一个缺陷，将 USB 驱动器的第一个分区公开为软盘而不是将 USB 驱动器公开为硬盘（他们称之为“USB-FDD”引导）。在这种情况下，您需要像这样安装：

```bash
# losetup /dev/loop0 /dev/sdb1
# mount /dev/loop0 /mnt/usb
# grub-install --boot-directory=/mnt/usb/bugbios --force --allow-floppy /dev/loop0
```

只要它们位于不同的目录中，此安装就不会与标准安装冲突。

在用于固定磁盘安装的 EFI 系统上，您必须安装 EFI 系统分区。如果你将它挂载在 /boot/efi ，那么你不需要任何特殊参数：

```bash
# grub-install
```

否则，您需要指定 EFI 系统分区的挂载位置：

```bash
# grub-install --efi-directory=/mnt/efi
```

对于可移动安装，您必须使用 `--removable` 并指定 `--boot-directory` 和 `--efi-directory`：

```bash
# grub-install --efi-directory=/mnt/usb --boot-directory=/mnt/usb/boot --removable
```

### 4.2 制作可使用 GRUB 引导的 CD-ROM

GRUB supports the *no emulation mode* in the El Torito specification[6](https://www.gnu.org/software/grub/manual/grub/grub.html#FOOT6). This means that you can use the whole CD-ROM from GRUB and you don’t have to make a floppy or hard disk image file, which can cause compatibility problems.

For booting from a CD-ROM, GRUB uses a special image called cdboot.img, which is concatenated with core.img. The core.img used for this should be built with at least the ‘iso9660’ and ‘biosdisk’ modules. Your bootable CD-ROM will usually also need to include a configuration file grub.cfg and some other GRUB modules.

To make a simple generic GRUB rescue CD, you can use the `grub-mkrescue` program (see [Invoking grub-mkrescue](https://www.gnu.org/software/grub/manual/grub/grub.html#Invoking-grub_002dmkrescue)):

```
$ grub-mkrescue -o grub.iso
```

You will often need to include other files in your image. To do this, first make a top directory for the bootable image, say, ‘iso’:

```
$ mkdir iso
```

Make a directory for GRUB:

```
$ mkdir -p iso/boot/grub
```

If desired, make the config file grub.cfg under iso/boot/grub (see [Configuration](https://www.gnu.org/software/grub/manual/grub/grub.html#Configuration)), and copy any files and directories for the disc to the directory iso/.

Finally, make the image:

```
$ grub-mkrescue -o grub.iso iso
```

This produces a file named grub.iso, which then can be burned into a CD (or a DVD), or written to a USB mass storage device.

The root device will be set up appropriately on entering your grub.cfg configuration file, so you can refer to file names on the CD without needing to use an explicit device name. This makes it easier to produce rescue images that will work on both optical drives and USB mass storage devices.

### 4.3 BIOS 驱动器和 OS 设备之间的映射

If the device map file exists, the GRUB utilities (`grub-probe`, etc.) read it to map BIOS drives to OS devices. This file consists of lines like this:

```
(device) file
```

device is a drive specified in the GRUB syntax (see [Device syntax](https://www.gnu.org/software/grub/manual/grub/grub.html#Device-syntax)), and file is an OS file, which is normally a device file.

Historically, the device map file was used because GRUB device names had to be used in the configuration file, and they were derived from BIOS drive numbers. The map between BIOS drives and OS devices cannot always be guessed correctly: for example, GRUB will get the order wrong if you exchange the boot sequence between IDE and SCSI in your BIOS.

Unfortunately, even OS device names are not always stable. Modern versions of the Linux kernel may probe drives in a different order from boot to boot, and the prefix (/dev/hd* versus /dev/sd*) may change depending on the driver subsystem in use. As a result, the device map file required frequent editing on some systems.

GRUB avoids this problem nowadays by using UUIDs or file system labels when generating grub.cfg, and we advise that you do the same for any custom menu entries you write. If the device map file does not exist, then the GRUB utilities will assume a temporary device map on the fly. This is often good enough, particularly in the common case of single-disk systems.

However, the device map file is not entirely obsolete yet, and it is used for overriding when current environment is different from the one on boot. Most common case is if you use a partition or logical volume as a disk for virtual machine. You can put any comments in the file if needed, as the GRUB utilities assume that a line is just a comment if the first character is ‘#’.

### 4.4 BIOS 安装（BIOS installation）部分内容

**MBR**

传统上用于 PC BIOS 平台的分区表格式称为**主引导记录 (MBR)** 格式；这是允许最多四个主分区和附加逻辑分区的格式。使用这种分区表格式，安装 GRUB 有两种方法：它可以嵌入在 MBR 和第一个分区之间的区域（有各种名称，例如“引导轨道”、“MBR 间隙”或“嵌入区域”，通常至少为 1000 KiB），或者 core image 可以安装在文件系统中，组成它的块列表可以存储在该分区的第一个扇区中。

现代工具通常会留下至少 1023 KiB 的 MBR 间隙。这个数量足以涵盖大多数配置。因此这个值是 GRUB 团队推荐的。

从历史上看，许多工具只留下 31 KiB 的空间。这不足以解析 Btrfs、ZFS、RAID 或 LVM 等可靠的困难结构，或者使用 ahci 等困难的磁盘访问方法。因此，如果尝试安装到较小的 MBR 间隙中，GRUB 会发出警告，除非在少数被授予的配置中。The grandfathered config must：

******

使用 biosdisk 作为 /boot 的磁盘访问模块，不使用任何额外的分区映射来访问 /boot，/boot 必须位于以下文件系统之一：AFFS, AFS, BFS, cpio, newc, odc, ext2/3/4, FAT, exFAT, F2FS, HFS, uncompressed HFS+, ISO9660, JFS, Minix, Minix2, Minix3, NILFS2, NTFS, ReiserFS, ROMFS, SFS, tar, UDF, UFS1, UFS2, XFS。

******

MBR gap 几乎没有技术问题。没有办法在嵌入区域中保留完全安全的空间，并且已知某些专有软件会使用它来使用户难以绕过许可限制。GRUB 通过其他软件的检测扇区并避开它们并使用 Reed-Solomon 编码保护自己的扇区。

GRUB 团队建议 MBR gap 至少为 1000 KiB

如果不可能，GRUB 支持强烈建议反对的后备解决方案。安装到文件系统意味着 GRUB 很容易受到文件系统特性（如尾部打包）甚至是激进的 fsck 实现移动其块的影响，因此这种方法非常脆弱；并且这种方法只能在 /boot 文件系统与 BIOS 启动所在的同一磁盘上时使用，这样 GRUB 就不必依赖猜测 BIOS 驱动器号。

**GRUB 开发团队一般建议在第一个分区之前嵌入 GRUB，除非你有特殊要求。**您**必须确保第一个分区从磁盘开始至少开始 1000 KiB**（2000 个扇区）；在现代磁盘上，无论如何在更大的边界上对齐分区通常是一种性能优势，因此第一个分区可能从磁盘开始处开始 1 MiB。

> 原文：The GRUB development team generally recommends embedding GRUB before the first partition, unless you have special requirements. You must ensure that the first partition starts at least 1000 KiB (2000 sectors) from the start of the disk; on modern disks, it is often a performance advantage to align partitions on larger boundaries anyway, so the first partition might start 1 MiB from the start of the disk.

**GPT**

Some newer systems use the GUID Partition Table (GPT) format.

> MBR 和 GPT 简介
>
> https://static.kancloud.cn/digest/system/149353
>
> **简单说明☆**：
>
> 说了这么多，如果你是第一看这方面的东西的话，很有可能看不太懂，没关系！这里有简单的介绍。
>
> 1. MBR分区表：Master Boot Record，即硬盘主引导记录分区表，只支持容量在 2.1TB 以下的硬盘，超过2.1TB的硬盘只能管理2.1TB，最多只支持4个主分区或三个主分区和一个扩展分区，扩展分区下可以有多个逻辑分区。
> 2. GPT分区表：GPT，全局唯一标识分区表(GUID Partition Table)，与MBR最大4个分区表项的限制相比，GPT对分区数量没有限制，但Windows最大仅支持128个GPT分区，GPT可管理硬盘大小达到了18EB。只有基于UEFI平台的主板才支持GPT分区引导启动。
>
> GPT分区表下的隐藏分区：
>
> ①. ESP分区：EFI system partition，该分区用于采用了EFI BIOS的电脑系统，用来启动操作系统。分区内存放引导管理程序、驱动程序、系统维护工具等。如果电脑采用了EFI系统，或当前磁盘用于在EFI平台上启动操作系统，则应建议ESP分区。
>
> ②. MSR分区：即微软保留分区，是GPT磁盘上用于保留空间以备用的分区，例如在将磁盘转换为动态磁盘时需要使用这些分区空间。

## 5 Booting

GRUB 可以以一致的方式加载兼容 Multiboot 的内核，但对于某些免费操作系统，您需要使用一些 OS-specific magic。

### 5.1 How to boot operating systems

GRUB 有两种不同的引导方法。其中之一是**直接加载操作系统**，另一种是**链式加载另一个引导加载程序，然后加载操作系统**。一般来说，前者更可取，因为您不需要安装或维护其他引导加载程序，而且 GRUB 足够灵活，可以从任意磁盘/分区中加载操作系统。但是，有时需要后者，因为 GRUB 本身并不支持所有现有的操作系统。

* 直接加载操作系统（Loading an operating system directly）
* chain-loading

#### 5.1.1 How to boot an OS directly with GRUB

Multiboot (see [Motivation](http://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Top) in The Multiboot Specification) is the native format supported by GRUB. For the sake of convenience, there is also support for Linux, FreeBSD, NetBSD and OpenBSD. If you want to boot other operating systems, you will have to chain-load them (see [Chain-loading](https://www.gnu.org/software/grub/manual/grub/grub.html#Chain_002dloading)).

FIXME: this section is incomplete.

1. Run the command `boot` (see [boot](https://www.gnu.org/software/grub/manual/grub/grub.html#boot)).

However, DOS and Windows have some deficiencies, so you might have to use more complicated instructions. See [DOS/Windows](https://www.gnu.org/software/grub/manual/grub/grub.html#DOS_002fWindows), for more information.

#### 5.1.2 Chain-loading an OS

不支持 Multiboot 且在 GRUB 中没有特定支持（Linux、FreeBSD、NetBSD 和 OpenBSD 提供特定支持）的操作系统必须是链式加载的，这涉及加载另一个引导加载程序并在实模式下跳转到它。

该`chainloader`命令（参见[chainloader](https://www.gnu.org/software/grub/manual/grub/grub.html#chainloader)）用于设置它。通常还需要加载一些 GRUB 模块并设置适当的根设备。综上所述，对于第一个硬盘的第一个分区上的 Windows 系统，我们得到这样的结果：

```
menuentry "Windows" { 
	insmod chain 
	insmod ntfs 
	set root=(hd0,1) 
	chainloader +1 
}
```

在具有多个硬盘的系统上，可能需要额外的解决方法。请参阅[DOS/Windows](https://www.gnu.org/software/grub/manual/grub/grub.html#DOS_002fWindows)。

只有 PC BIOS 和 EFI 平台支持链式加载。

## 6 Writing your own configuration file

GRUB is configured using grub.cfg, usually located under /boot/grub. This file is quite flexible, but most users will not need to write the whole thing by hand.

* [Simple configuration](https://www.gnu.org/software/grub/manual/grub/grub.html#Simple-configuration) - Recommended for most users
* [Root Identifcation Heuristics](https://www.gnu.org/software/grub/manual/grub/grub.html#Root-Identifcation-Heuristics) - Summary on how the root file system is identified.
* [Shell-like scripting](https://www.gnu.org/software/grub/manual/grub/grub.html#Shell_002dlike-scripting) - For power users and developers
* [Multi-boot manual config](https://www.gnu.org/software/grub/manual/grub/grub.html#Multi_002dboot-manual-config) - For non-standard multi-OS scenarios
* [Embedded configuration](https://www.gnu.org/software/grub/manual/grub/grub.html#Embedded-configuration) - Embedding a configuration file into GRUB

### 6.1 Simple configuration handling

程序grub-mkconfig（请参阅调用 grub-mkconfig）生成的grub.cfg适用于大多数情况的文件。它适合在升级发行版时使用，它将发现可用的内核并尝试为它们生成菜单条目。

### 6.3 Writing full configguratipn files directly

grub.cfg是用 GRUB 的内置脚本语言编写的，其语法与 GNU Bash 和其他 Bourne shell 衍生产品的语法非常相似。

#### Words



















