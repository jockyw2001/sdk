INC  += $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/drivers/mstar/include
INC  += ./internal/ceva_vx/VX/include
INC  += ./ceva_vx/st_ceva_vx

LIBS += -L./internal/ceva_vx/VX/lib -lxml2 -lopenvx-cevacv -lopenvx -lvxu -lcevaboot
LIBS += -lm -lstdc++
LIBS += -lmi_ai -lmi_ao -lmi_warp -lmi_vif -lmi_vpe -lmi_venc -lmi_disp -lmi_hdmi -lmi_uac -lAEC_LINUX -lmi_sd -lg711 -lg726 -ladda

#SUBDIRS += ./ceva_vx/st_ceva_vx

ST_DEP := ceva_vx common warp uvc hdmi disp vpe vif fb mess tem sd

LINK_TYPE := dynamic
