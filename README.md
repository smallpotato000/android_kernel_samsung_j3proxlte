# android_kernel_samsung_j3proxlte
*How to Build
*	get Toolchain from android git server , codesourcery and etc .. i.e.arm-eabi-4.8	
*	set environment variables:
*		$ export CROSS_COMPILE= $(android platform directory you download)/android/gcc/prebuilts/linux-x86/arm/arm-eabi-4.8/bin/arm-eabi-
*	or
*		$ export CROSS_COMPILE=/usr/local/toolchain/arm-eabi-4.8/bin/arm-eabi-
* 
*		$ export ARCH=arm
*	make:
*		$ mkdir output
		$ make -C $(pwd) O=output msm8916_sec_defconfig VARIANT_DEFCONFIG=msm8916_sec_j3xprolte_chnopen_defconfig SELINUX_DEFCONFIG=selinux_defconfig
 * Note：msm8916_sec_j3xprolte_chnopen_defconfig for j3110 (Open Edition), msm8916_sec_j3xprolte_chnctc_defconfig for j3119 (CTC Edition).
*		$ make -C $(pwd) O=output 
		$ cp output/arch/arm/boot/zImage arch/arm/boot/zImage
* Output files
*	- Kernel : output/arch/arm/boot/zImage
*	- module : output/drivers/*/*.ko
* How to Clean	
*		$ cd output
		$ make clean
* How to generate image
	Refer to https://github.com/xiaolu/mkbootimg_tools
