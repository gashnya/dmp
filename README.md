# Device Mapper Proxy (DMP)

Simple device mapper target that collects some incoming IO statistics.

**Target system**: Ubuntu 23.10

## Requirements

```shell
# kernel headers
$ sudo apt install linux-headers-$(uname -r)

# gcc and make
$ sudo apt install build-essential

# python 3.7+ for manual testing
```

## Build

```shell
# clone this repository
$ git clone git@github.com:gashnya/dmp.git

$ cd dmp

# build the module
$ make
```

## Install

```shell
$ sudo insmod dmp.ko
```

## Example

```shell
# create underlying block device
# table format: "logical_start_sector num_sectors target_type target_args"
$ dmsetup create zero1 --table "0 1024 zero"

# create dmp device
# note: num_sectors should be equal to the num_sectors of the underlying device
$ dmsetup create dmp1 --table "0 1024 dmp /dev/mapper/zero1"
```

*Note*: if you create multiple dmp devices, the statistics will be shared among them.

You can now access and interact with the statistics via `sysfs` of the module:

```shell
# show statistics
$ cat /sys/module/dmp/stat/volumes
read:
 reqs: 176
 avg size: 4096
write:
 reqs: 7
 avg size: 1024
total:
 reqs: 183
 avg size: 3978

# reset statistics
$ sudo bash -c "echo 1 > /sys/module/dmp/stat/reset"
$ cat /sys/module/dmp/stat/volumes
read:
 reqs: 0
 avg size: 0
write:
 reqs: 0
 avg size: 0
total:
 reqs: 0
 avg size: 0
```

## Manual testing

You can check consistency with `sys/block/{name}/stat` using `dmp_check.py` (pass dmp devices names as arguments):

```shell
$ python3 dmp_check.py dm-1
read:
 reqs: 132
 avg size: 4096
write:
 reqs: 1
 avg size: 512
total:
 reqs: 133
 avg size: 4069

correct

# more load
$ sudo dd if=/dev/zero of=/dev/mapper/dmp1 bs=4K count=1
$ python3 dmp_check.py dm-1
read:
 reqs: 177
 avg size: 4096
write:
 reqs: 2
 avg size: 2304
total:
 reqs: 179
 avg size: 4075

correct
```

Recommended usage: check consistency after generating IO load on devices.

*Note*: make sure to specify all dmp devices that you have created.

*Note*: do not reset statistics while manual testing as only the module's statistics will be reset, leading to constant inconsistency.

## Clean
```shell
# remove underlying block device
$ dmsetup remove zero1

# remove dmp device
$ dmsetup remove dmp1

$ sudo rmmod dmp

# remove build output
$ make clean
```
