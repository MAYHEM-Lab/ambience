#
_XDCBUILDCOUNT = 
ifneq (,$(findstring path,$(_USEXDCENV_)))
override XDCPATH = /vagrant/Q3_ENG_SDK_250919/sdk_root/kernel/tirtos/packages
override XDCROOT = /opt/ti/xdctools_3_60_00_24_core
override XDCBUILDCFG = /vagrant/Q3_ENG_SDK_250919/sdk_root/source/ti/utils/tiutils.bld
endif
ifneq (,$(findstring args,$(_USEXDCENV_)))
override XDCARGS = TIPOSIX_REPO="/vagrant/Q3_ENG_SDK_250919/sdk_root/source" ti.targets.arm.elf.M4="/opt/ti/ccs-latest/ccs/tools/compiler/ti-cgt-arm_18.12.3.LTS" gnu.targets.arm.M4="/opt/ti/ccs-latest/ccs/tools/compiler/gcc-arm-none-eabi-7-2017-q4-major" iar.targets.arm.M4="/opt/iar/arm/latest"
override XDCTARGETS = 
endif
#
ifeq (0,1)
PKGPATH = /vagrant/Q3_ENG_SDK_250919/sdk_root/kernel/tirtos/packages;/opt/ti/xdctools_3_60_00_24_core/packages;../../..
HOSTOS = Linux
endif
