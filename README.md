# LVGL media player for Allwinner T113S3

# eMP_music

## 环境

```shell
# 请自行修改 build.sh 文件的 STAGING_DIR 环境变量
export STAGING_DIR=/home/hugokkl/tina-sdk/out/t113-pi/staging_dir/target
```

```makefile
# 请自行修改 Makefile 文件的 编译器变量 CC 和 CXX
CC = /home/hugokkl/tina-sdk/prebuilt/gcc/linux-x86/arm/toolchain-sunxi-musl/toolchain/bin/arm-openwrt-linux-gcc
CXX = /home/hugokkl/tina-sdk/prebuilt/gcc/linux-x86/arm/toolchain-sunxi-musl/toolchain/bin/arm-openwrt-linux-g++
```

## 编译

```shell
./build.sh
```

## 运行

可执行文件为：`eMP_music`

## lrc文件格式

注意：一定要加`.10`，否则roller会有bug！

```lrc
[00:00.10] 歌词歌词歌词...
[00:01.10] 歌词歌词歌词...
[00:02.10] 歌词歌词歌词...
[00:05.10] 歌词歌词歌词...
[00:06.10] 歌词歌词歌词...

...

```

