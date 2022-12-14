#
#  Do not edit this file.  This file is generated from 
#  package.bld.  Any modifications to this file will be 
#  overwritten whenever makefiles are re-generated.
#
#  target compatibility key = iar.targets.arm.M4{1,0,8.32,2
#
ifeq (,$(MK_NOGENDEPS))
-include package/lib/lib/iar/m4/json_debug/package/package_ti.utils.json.orm4.dep
package/lib/lib/iar/m4/json_debug/package/package_ti.utils.json.orm4.dep: ;
endif

package/lib/lib/iar/m4/json_debug/package/package_ti.utils.json.orm4: | .interfaces
package/lib/lib/iar/m4/json_debug/package/package_ti.utils.json.orm4: package/package_ti.utils.json.c lib/iar/m4/json_debug.a.mak
	@$(RM) $@.dep
	$(RM) $@
	@$(MSG) clrm4 $< ...
	LC_ALL=C $(iar.targets.arm.M4.rootDir)/bin/iccarm  --silent --aeabi --cpu=Cortex-M4 --diag_suppress=Pa050,Go005 --endian=little -e --thumb  -DALLOW_PARSING__TEMPLATE -DALLOW_PARSING__JSON -D USE__STANDARD_LIBS  -I/vagrant/Q3_ENG_SDK_250919/sdk_root/source/ti/posix/iar -Dxdc_target_name__=M4 -Dxdc_target_types__=iar/targets/arm/std.h -Dxdc_bld__profile_debug -Dxdc_bld__vers_1_0_8_32_2 --debug --dlib_config $(iar.targets.arm.M4.rootDir)/inc/c/DLib_Config_Normal.h  $(XDCINCS)  -o $@  $<
	
	-@$(FIXDEP) $@.dep $@.dep
	

package/lib/lib/iar/m4/json_debug/package/package_ti.utils.json.srm4: | .interfaces
package/lib/lib/iar/m4/json_debug/package/package_ti.utils.json.srm4: package/package_ti.utils.json.c lib/iar/m4/json_debug.a.mak
	@$(RM) $@.dep
	$(RM) $@
	@$(MSG) clrm4 $< ...
	LC_ALL=C $(iar.targets.arm.M4.rootDir)/bin/iccarm  --silent --aeabi --cpu=Cortex-M4 --diag_suppress=Pa050,Go005 --endian=little -e --thumb  -DALLOW_PARSING__TEMPLATE -DALLOW_PARSING__JSON -D USE__STANDARD_LIBS  -I/vagrant/Q3_ENG_SDK_250919/sdk_root/source/ti/posix/iar -Dxdc_target_name__=M4 -Dxdc_target_types__=iar/targets/arm/std.h -Dxdc_bld__profile_debug -Dxdc_bld__vers_1_0_8_32_2 --debug --dlib_config $(iar.targets.arm.M4.rootDir)/inc/c/DLib_Config_Normal.h  $(XDCINCS)  -o $@  $<
	
	-@$(FIXDEP) $@.dep $@.dep
	

ifeq (,$(MK_NOGENDEPS))
-include package/lib/lib/iar/m4/json_debug/source/json.orm4.dep
package/lib/lib/iar/m4/json_debug/source/json.orm4.dep: ;
endif

package/lib/lib/iar/m4/json_debug/source/json.orm4: | .interfaces
package/lib/lib/iar/m4/json_debug/source/json.orm4: source/json.c lib/iar/m4/json_debug.a.mak
	@$(RM) $@.dep
	$(RM) $@
	@$(MSG) clrm4 $< ...
	LC_ALL=C $(iar.targets.arm.M4.rootDir)/bin/iccarm  --silent --aeabi --cpu=Cortex-M4 --diag_suppress=Pa050,Go005 --endian=little -e --thumb  -DALLOW_PARSING__TEMPLATE -DALLOW_PARSING__JSON -D USE__STANDARD_LIBS  -I/vagrant/Q3_ENG_SDK_250919/sdk_root/source/ti/posix/iar -Dxdc_target_name__=M4 -Dxdc_target_types__=iar/targets/arm/std.h -Dxdc_bld__profile_debug -Dxdc_bld__vers_1_0_8_32_2 --debug --dlib_config $(iar.targets.arm.M4.rootDir)/inc/c/DLib_Config_Normal.h  $(XDCINCS)  -o $@  $<
	
	-@$(FIXDEP) $@.dep $@.dep
	

package/lib/lib/iar/m4/json_debug/source/json.srm4: | .interfaces
package/lib/lib/iar/m4/json_debug/source/json.srm4: source/json.c lib/iar/m4/json_debug.a.mak
	@$(RM) $@.dep
	$(RM) $@
	@$(MSG) clrm4 $< ...
	LC_ALL=C $(iar.targets.arm.M4.rootDir)/bin/iccarm  --silent --aeabi --cpu=Cortex-M4 --diag_suppress=Pa050,Go005 --endian=little -e --thumb  -DALLOW_PARSING__TEMPLATE -DALLOW_PARSING__JSON -D USE__STANDARD_LIBS  -I/vagrant/Q3_ENG_SDK_250919/sdk_root/source/ti/posix/iar -Dxdc_target_name__=M4 -Dxdc_target_types__=iar/targets/arm/std.h -Dxdc_bld__profile_debug -Dxdc_bld__vers_1_0_8_32_2 --debug --dlib_config $(iar.targets.arm.M4.rootDir)/inc/c/DLib_Config_Normal.h  $(XDCINCS)  -o $@  $<
	
	-@$(FIXDEP) $@.dep $@.dep
	

ifeq (,$(MK_NOGENDEPS))
-include package/lib/lib/iar/m4/json_debug/source/json_engine.orm4.dep
package/lib/lib/iar/m4/json_debug/source/json_engine.orm4.dep: ;
endif

package/lib/lib/iar/m4/json_debug/source/json_engine.orm4: | .interfaces
package/lib/lib/iar/m4/json_debug/source/json_engine.orm4: source/json_engine.c lib/iar/m4/json_debug.a.mak
	@$(RM) $@.dep
	$(RM) $@
	@$(MSG) clrm4 $< ...
	LC_ALL=C $(iar.targets.arm.M4.rootDir)/bin/iccarm  --silent --aeabi --cpu=Cortex-M4 --diag_suppress=Pa050,Go005 --endian=little -e --thumb  -DALLOW_PARSING__TEMPLATE -DALLOW_PARSING__JSON -D USE__STANDARD_LIBS  -I/vagrant/Q3_ENG_SDK_250919/sdk_root/source/ti/posix/iar -Dxdc_target_name__=M4 -Dxdc_target_types__=iar/targets/arm/std.h -Dxdc_bld__profile_debug -Dxdc_bld__vers_1_0_8_32_2 --debug --dlib_config $(iar.targets.arm.M4.rootDir)/inc/c/DLib_Config_Normal.h  $(XDCINCS)  -o $@  $<
	
	-@$(FIXDEP) $@.dep $@.dep
	

package/lib/lib/iar/m4/json_debug/source/json_engine.srm4: | .interfaces
package/lib/lib/iar/m4/json_debug/source/json_engine.srm4: source/json_engine.c lib/iar/m4/json_debug.a.mak
	@$(RM) $@.dep
	$(RM) $@
	@$(MSG) clrm4 $< ...
	LC_ALL=C $(iar.targets.arm.M4.rootDir)/bin/iccarm  --silent --aeabi --cpu=Cortex-M4 --diag_suppress=Pa050,Go005 --endian=little -e --thumb  -DALLOW_PARSING__TEMPLATE -DALLOW_PARSING__JSON -D USE__STANDARD_LIBS  -I/vagrant/Q3_ENG_SDK_250919/sdk_root/source/ti/posix/iar -Dxdc_target_name__=M4 -Dxdc_target_types__=iar/targets/arm/std.h -Dxdc_bld__profile_debug -Dxdc_bld__vers_1_0_8_32_2 --debug --dlib_config $(iar.targets.arm.M4.rootDir)/inc/c/DLib_Config_Normal.h  $(XDCINCS)  -o $@  $<
	
	-@$(FIXDEP) $@.dep $@.dep
	

ifeq (,$(MK_NOGENDEPS))
-include package/lib/lib/iar/m4/json_debug/source/parse_common.orm4.dep
package/lib/lib/iar/m4/json_debug/source/parse_common.orm4.dep: ;
endif

package/lib/lib/iar/m4/json_debug/source/parse_common.orm4: | .interfaces
package/lib/lib/iar/m4/json_debug/source/parse_common.orm4: source/parse_common.c lib/iar/m4/json_debug.a.mak
	@$(RM) $@.dep
	$(RM) $@
	@$(MSG) clrm4 $< ...
	LC_ALL=C $(iar.targets.arm.M4.rootDir)/bin/iccarm  --silent --aeabi --cpu=Cortex-M4 --diag_suppress=Pa050,Go005 --endian=little -e --thumb  -DALLOW_PARSING__TEMPLATE -DALLOW_PARSING__JSON -D USE__STANDARD_LIBS  -I/vagrant/Q3_ENG_SDK_250919/sdk_root/source/ti/posix/iar -Dxdc_target_name__=M4 -Dxdc_target_types__=iar/targets/arm/std.h -Dxdc_bld__profile_debug -Dxdc_bld__vers_1_0_8_32_2 --debug --dlib_config $(iar.targets.arm.M4.rootDir)/inc/c/DLib_Config_Normal.h  $(XDCINCS)  -o $@  $<
	
	-@$(FIXDEP) $@.dep $@.dep
	

package/lib/lib/iar/m4/json_debug/source/parse_common.srm4: | .interfaces
package/lib/lib/iar/m4/json_debug/source/parse_common.srm4: source/parse_common.c lib/iar/m4/json_debug.a.mak
	@$(RM) $@.dep
	$(RM) $@
	@$(MSG) clrm4 $< ...
	LC_ALL=C $(iar.targets.arm.M4.rootDir)/bin/iccarm  --silent --aeabi --cpu=Cortex-M4 --diag_suppress=Pa050,Go005 --endian=little -e --thumb  -DALLOW_PARSING__TEMPLATE -DALLOW_PARSING__JSON -D USE__STANDARD_LIBS  -I/vagrant/Q3_ENG_SDK_250919/sdk_root/source/ti/posix/iar -Dxdc_target_name__=M4 -Dxdc_target_types__=iar/targets/arm/std.h -Dxdc_bld__profile_debug -Dxdc_bld__vers_1_0_8_32_2 --debug --dlib_config $(iar.targets.arm.M4.rootDir)/inc/c/DLib_Config_Normal.h  $(XDCINCS)  -o $@  $<
	
	-@$(FIXDEP) $@.dep $@.dep
	

ifeq (,$(MK_NOGENDEPS))
-include package/lib/lib/iar/m4/json_debug/source/utils.orm4.dep
package/lib/lib/iar/m4/json_debug/source/utils.orm4.dep: ;
endif

package/lib/lib/iar/m4/json_debug/source/utils.orm4: | .interfaces
package/lib/lib/iar/m4/json_debug/source/utils.orm4: source/utils.c lib/iar/m4/json_debug.a.mak
	@$(RM) $@.dep
	$(RM) $@
	@$(MSG) clrm4 $< ...
	LC_ALL=C $(iar.targets.arm.M4.rootDir)/bin/iccarm  --silent --aeabi --cpu=Cortex-M4 --diag_suppress=Pa050,Go005 --endian=little -e --thumb  -DALLOW_PARSING__TEMPLATE -DALLOW_PARSING__JSON -D USE__STANDARD_LIBS  -I/vagrant/Q3_ENG_SDK_250919/sdk_root/source/ti/posix/iar -Dxdc_target_name__=M4 -Dxdc_target_types__=iar/targets/arm/std.h -Dxdc_bld__profile_debug -Dxdc_bld__vers_1_0_8_32_2 --debug --dlib_config $(iar.targets.arm.M4.rootDir)/inc/c/DLib_Config_Normal.h  $(XDCINCS)  -o $@  $<
	
	-@$(FIXDEP) $@.dep $@.dep
	

package/lib/lib/iar/m4/json_debug/source/utils.srm4: | .interfaces
package/lib/lib/iar/m4/json_debug/source/utils.srm4: source/utils.c lib/iar/m4/json_debug.a.mak
	@$(RM) $@.dep
	$(RM) $@
	@$(MSG) clrm4 $< ...
	LC_ALL=C $(iar.targets.arm.M4.rootDir)/bin/iccarm  --silent --aeabi --cpu=Cortex-M4 --diag_suppress=Pa050,Go005 --endian=little -e --thumb  -DALLOW_PARSING__TEMPLATE -DALLOW_PARSING__JSON -D USE__STANDARD_LIBS  -I/vagrant/Q3_ENG_SDK_250919/sdk_root/source/ti/posix/iar -Dxdc_target_name__=M4 -Dxdc_target_types__=iar/targets/arm/std.h -Dxdc_bld__profile_debug -Dxdc_bld__vers_1_0_8_32_2 --debug --dlib_config $(iar.targets.arm.M4.rootDir)/inc/c/DLib_Config_Normal.h  $(XDCINCS)  -o $@  $<
	
	-@$(FIXDEP) $@.dep $@.dep
	

clean,rm4 ::
	-$(RM) package/lib/lib/iar/m4/json_debug/package/package_ti.utils.json.orm4
	-$(RM) package/lib/lib/iar/m4/json_debug/source/json.orm4
	-$(RM) package/lib/lib/iar/m4/json_debug/source/json_engine.orm4
	-$(RM) package/lib/lib/iar/m4/json_debug/source/parse_common.orm4
	-$(RM) package/lib/lib/iar/m4/json_debug/source/utils.orm4
	-$(RM) package/lib/lib/iar/m4/json_debug/package/package_ti.utils.json.srm4
	-$(RM) package/lib/lib/iar/m4/json_debug/source/json.srm4
	-$(RM) package/lib/lib/iar/m4/json_debug/source/json_engine.srm4
	-$(RM) package/lib/lib/iar/m4/json_debug/source/parse_common.srm4
	-$(RM) package/lib/lib/iar/m4/json_debug/source/utils.srm4

lib/iar/m4/json_debug.a: package/lib/lib/iar/m4/json_debug/package/package_ti.utils.json.orm4 package/lib/lib/iar/m4/json_debug/source/json.orm4 package/lib/lib/iar/m4/json_debug/source/json_engine.orm4 package/lib/lib/iar/m4/json_debug/source/parse_common.orm4 package/lib/lib/iar/m4/json_debug/source/utils.orm4 lib/iar/m4/json_debug.a.mak

clean::
	-$(RM) lib/iar/m4/json_debug.a.mak
