#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=mkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/ScentCollCharsCopy.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/ScentCollCharsCopy.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=PWM.c bluetooth.c main.c owire.c adc.c piezos.c eeprom.c eeprom_async.c i2c.c i2c_async.c pod.c pod_manager.c pod_manager_async.c relay_pwm_manager.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/PWM.o ${OBJECTDIR}/bluetooth.o ${OBJECTDIR}/main.o ${OBJECTDIR}/owire.o ${OBJECTDIR}/adc.o ${OBJECTDIR}/piezos.o ${OBJECTDIR}/eeprom.o ${OBJECTDIR}/eeprom_async.o ${OBJECTDIR}/i2c.o ${OBJECTDIR}/i2c_async.o ${OBJECTDIR}/pod.o ${OBJECTDIR}/pod_manager.o ${OBJECTDIR}/pod_manager_async.o ${OBJECTDIR}/relay_pwm_manager.o
POSSIBLE_DEPFILES=${OBJECTDIR}/PWM.o.d ${OBJECTDIR}/bluetooth.o.d ${OBJECTDIR}/main.o.d ${OBJECTDIR}/owire.o.d ${OBJECTDIR}/adc.o.d ${OBJECTDIR}/piezos.o.d ${OBJECTDIR}/eeprom.o.d ${OBJECTDIR}/eeprom_async.o.d ${OBJECTDIR}/i2c.o.d ${OBJECTDIR}/i2c_async.o.d ${OBJECTDIR}/pod.o.d ${OBJECTDIR}/pod_manager.o.d ${OBJECTDIR}/pod_manager_async.o.d ${OBJECTDIR}/relay_pwm_manager.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/PWM.o ${OBJECTDIR}/bluetooth.o ${OBJECTDIR}/main.o ${OBJECTDIR}/owire.o ${OBJECTDIR}/adc.o ${OBJECTDIR}/piezos.o ${OBJECTDIR}/eeprom.o ${OBJECTDIR}/eeprom_async.o ${OBJECTDIR}/i2c.o ${OBJECTDIR}/i2c_async.o ${OBJECTDIR}/pod.o ${OBJECTDIR}/pod_manager.o ${OBJECTDIR}/pod_manager_async.o ${OBJECTDIR}/relay_pwm_manager.o

# Source Files
SOURCEFILES=PWM.c bluetooth.c main.c owire.c adc.c piezos.c eeprom.c eeprom_async.c i2c.c i2c_async.c pod.c pod_manager.c pod_manager_async.c relay_pwm_manager.c



CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/ScentCollCharsCopy.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=24FJ128GL306
MP_LINKER_FILE_OPTION=,--script=p24FJ128GL306.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/PWM.o: PWM.c  .generated_files/flags/default/2f02f5f759e8790cef562b1cff7d304d421d6df4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/PWM.o.d 
	@${RM} ${OBJECTDIR}/PWM.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  PWM.c  -o ${OBJECTDIR}/PWM.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/PWM.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD5=1  -mno-eds-warn  -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/bluetooth.o: bluetooth.c  .generated_files/flags/default/3a3d3623597c5dd3c74b98d676efae1ae8de115d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/bluetooth.o.d 
	@${RM} ${OBJECTDIR}/bluetooth.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  bluetooth.c  -o ${OBJECTDIR}/bluetooth.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/bluetooth.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD5=1  -mno-eds-warn  -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/a03d203e28f004410721cce615a5e921a1482d75 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD5=1  -mno-eds-warn  -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/owire.o: owire.c  .generated_files/flags/default/4e34e0b94ac7fd16057f855747519685f3497cb9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/owire.o.d 
	@${RM} ${OBJECTDIR}/owire.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  owire.c  -o ${OBJECTDIR}/owire.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/owire.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD5=1  -mno-eds-warn  -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/adc.o: adc.c  .generated_files/flags/default/5600813ca229f0045a66ef0c98df578b086e5d7c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/adc.o.d 
	@${RM} ${OBJECTDIR}/adc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  adc.c  -o ${OBJECTDIR}/adc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/adc.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD5=1  -mno-eds-warn  -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/piezos.o: piezos.c  .generated_files/flags/default/ecfd354150b637d2fff17e433b7f5faceb191c30 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/piezos.o.d 
	@${RM} ${OBJECTDIR}/piezos.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  piezos.c  -o ${OBJECTDIR}/piezos.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/piezos.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD5=1  -mno-eds-warn  -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/eeprom.o: eeprom.c  .generated_files/flags/default/d5b99571ce3627b31d7833d611a67641fea4ace6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/eeprom.o.d 
	@${RM} ${OBJECTDIR}/eeprom.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  eeprom.c  -o ${OBJECTDIR}/eeprom.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/eeprom.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD5=1  -mno-eds-warn  -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/eeprom_async.o: eeprom_async.c  .generated_files/flags/default/64e4f63ca6d5686dba5bf3155ec0e97eab9bf4ea .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/eeprom_async.o.d 
	@${RM} ${OBJECTDIR}/eeprom_async.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  eeprom_async.c  -o ${OBJECTDIR}/eeprom_async.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/eeprom_async.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD5=1  -mno-eds-warn  -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/i2c.o: i2c.c  .generated_files/flags/default/e4d231974ce2b7a74692940fd7c1cff3e8a35378 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/i2c.o.d 
	@${RM} ${OBJECTDIR}/i2c.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  i2c.c  -o ${OBJECTDIR}/i2c.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/i2c.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD5=1  -mno-eds-warn  -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/i2c_async.o: i2c_async.c  .generated_files/flags/default/7934b02de506ab31c5f59558ee6aebe1b03a4ce4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/i2c_async.o.d 
	@${RM} ${OBJECTDIR}/i2c_async.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  i2c_async.c  -o ${OBJECTDIR}/i2c_async.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/i2c_async.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD5=1  -mno-eds-warn  -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/pod.o: pod.c  .generated_files/flags/default/18ded88aa6572e43541da60c5066f04ab625760f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/pod.o.d 
	@${RM} ${OBJECTDIR}/pod.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  pod.c  -o ${OBJECTDIR}/pod.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/pod.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD5=1  -mno-eds-warn  -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/pod_manager.o: pod_manager.c  .generated_files/flags/default/f489acbec85d830fd3781b270ee1d15e887b6e0c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/pod_manager.o.d 
	@${RM} ${OBJECTDIR}/pod_manager.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  pod_manager.c  -o ${OBJECTDIR}/pod_manager.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/pod_manager.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD5=1  -mno-eds-warn  -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/pod_manager_async.o: pod_manager_async.c  .generated_files/flags/default/afd78be0720d383ed1ea39ebea32d9b0ce126440 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/pod_manager_async.o.d 
	@${RM} ${OBJECTDIR}/pod_manager_async.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  pod_manager_async.c  -o ${OBJECTDIR}/pod_manager_async.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/pod_manager_async.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD5=1  -mno-eds-warn  -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/relay_pwm_manager.o: relay_pwm_manager.c  .generated_files/flags/default/834fc2adc58f09e724341bddea7980e754828f8d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/relay_pwm_manager.o.d 
	@${RM} ${OBJECTDIR}/relay_pwm_manager.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  relay_pwm_manager.c  -o ${OBJECTDIR}/relay_pwm_manager.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/relay_pwm_manager.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD5=1  -mno-eds-warn  -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/PWM.o: PWM.c  .generated_files/flags/default/f2bc7d9e66c94b0dde3545585de200177fb5be8f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/PWM.o.d 
	@${RM} ${OBJECTDIR}/PWM.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  PWM.c  -o ${OBJECTDIR}/PWM.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/PWM.o.d"      -mno-eds-warn  -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/bluetooth.o: bluetooth.c  .generated_files/flags/default/ef4ba1fe9aaf33b182711b8b494f1d1b233235c1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/bluetooth.o.d 
	@${RM} ${OBJECTDIR}/bluetooth.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  bluetooth.c  -o ${OBJECTDIR}/bluetooth.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/bluetooth.o.d"      -mno-eds-warn  -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/ec444999ade02d86103243a55b3aea75df304cca .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -mno-eds-warn  -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/owire.o: owire.c  .generated_files/flags/default/1f10399dbbd29f2f3c8968752624d0cc1c62a470 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/owire.o.d 
	@${RM} ${OBJECTDIR}/owire.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  owire.c  -o ${OBJECTDIR}/owire.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/owire.o.d"      -mno-eds-warn  -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/adc.o: adc.c  .generated_files/flags/default/6f3745df3f16f62ba9d277fc82d4d555db789c71 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/adc.o.d 
	@${RM} ${OBJECTDIR}/adc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  adc.c  -o ${OBJECTDIR}/adc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/adc.o.d"      -mno-eds-warn  -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/piezos.o: piezos.c  .generated_files/flags/default/8f92c5903b711189c70832d3aa63f8a71fdcc280 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/piezos.o.d 
	@${RM} ${OBJECTDIR}/piezos.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  piezos.c  -o ${OBJECTDIR}/piezos.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/piezos.o.d"      -mno-eds-warn  -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/eeprom.o: eeprom.c  .generated_files/flags/default/7bcc9fbce0de995ff0b7734225aced604390883 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/eeprom.o.d 
	@${RM} ${OBJECTDIR}/eeprom.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  eeprom.c  -o ${OBJECTDIR}/eeprom.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/eeprom.o.d"      -mno-eds-warn  -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/eeprom_async.o: eeprom_async.c  .generated_files/flags/default/30424ec73d983d4909b2438e9298bb92eaccfaaf .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/eeprom_async.o.d 
	@${RM} ${OBJECTDIR}/eeprom_async.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  eeprom_async.c  -o ${OBJECTDIR}/eeprom_async.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/eeprom_async.o.d"      -mno-eds-warn  -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/i2c.o: i2c.c  .generated_files/flags/default/27a5871cdec38e12ebb3371c93c375807b8fb315 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/i2c.o.d 
	@${RM} ${OBJECTDIR}/i2c.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  i2c.c  -o ${OBJECTDIR}/i2c.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/i2c.o.d"      -mno-eds-warn  -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/i2c_async.o: i2c_async.c  .generated_files/flags/default/98db949003d71ca680a13ef0ce3b3c7035ab184e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/i2c_async.o.d 
	@${RM} ${OBJECTDIR}/i2c_async.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  i2c_async.c  -o ${OBJECTDIR}/i2c_async.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/i2c_async.o.d"      -mno-eds-warn  -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/pod.o: pod.c  .generated_files/flags/default/27048378214fd40133e4d3a2c1434e0e7297ee78 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/pod.o.d 
	@${RM} ${OBJECTDIR}/pod.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  pod.c  -o ${OBJECTDIR}/pod.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/pod.o.d"      -mno-eds-warn  -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/pod_manager.o: pod_manager.c  .generated_files/flags/default/7e1aeb11d65aaffa0b1ee4612a80639dfa9b73a3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/pod_manager.o.d 
	@${RM} ${OBJECTDIR}/pod_manager.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  pod_manager.c  -o ${OBJECTDIR}/pod_manager.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/pod_manager.o.d"      -mno-eds-warn  -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/pod_manager_async.o: pod_manager_async.c  .generated_files/flags/default/4db911c0aa79076e4ac93efdeac32d4520c67c3c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/pod_manager_async.o.d 
	@${RM} ${OBJECTDIR}/pod_manager_async.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  pod_manager_async.c  -o ${OBJECTDIR}/pod_manager_async.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/pod_manager_async.o.d"      -mno-eds-warn  -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/relay_pwm_manager.o: relay_pwm_manager.c  .generated_files/flags/default/9e12c5b5de817e0801e3eb166c9e40de0a2151c4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/relay_pwm_manager.o.d 
	@${RM} ${OBJECTDIR}/relay_pwm_manager.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  relay_pwm_manager.c  -o ${OBJECTDIR}/relay_pwm_manager.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/relay_pwm_manager.o.d"      -mno-eds-warn  -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemblePreproc
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${DISTDIR}/ScentCollCharsCopy.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/ScentCollCharsCopy.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_ICD5=1  -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)   -mreserve=data@0x800:0x81B -mreserve=data@0x81C:0x81D -mreserve=data@0x81E:0x81F -mreserve=data@0x820:0x821 -mreserve=data@0x822:0x823 -mreserve=data@0x824:0x827 -mreserve=data@0x82A:0x84F   -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_ICD5=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/ScentCollCharsCopy.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/ScentCollCharsCopy.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	${MP_CC_DIR}/xc16-bin2hex ${DISTDIR}/ScentCollCharsCopy.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a    -mdfp="${DFP_DIR}/xc16" 
	
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${OBJECTDIR}
	${RM} -r ${DISTDIR}

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(wildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
