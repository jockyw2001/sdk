MODULE_FOLDER=\
(
acp
audio
bdma
ch34
clkm
cmdq
cpu
dac
ddc2bi
demodulator
dlc
dms
dmx
dscmb
flash
gpd
gpio
graphic
hdmi
hsl
hwi2c
ipauth
ir
irq
ldm
mbx
mfc
mfe
miu
msos
mspi
mvop
njpd
ojpd_vdec_v1
ojpd_vdec_v2
pcmcia
pm
pq
pvr_iframelut
pwm
pws
rtc
sar
sc
seal
security
sem
smbx
stbdc
swi2c
sys
uart
urdma
usb
vd
vdec_lite
vdec_v1
vdec_v2
vdec_v3
ve
vif
wble
wdt
xc
ir_tx
)

function ShowHelp(){
	echo -e CreateChip.sh parent_chip new_chip 
}

if [ ! "$1" ] || [ ! "$2" ];then
	echo Please input chip name.
	ShowHelp;
	exit
fi

PROJHOME=$PWD
for ((index=0; index<${#MODULE_FOLDER[@]}; index++))
do
if [ -e $PROJHOME/modules/${MODULE_FOLDER[$index]}/hal/$1 ];then
	echo Module "${MODULE_FOLDER[$index]}" exist
	if ! [ -d $PROJHOME/modules/${MODULE_FOLDER[$index]}/hal/$2 ];then
	cp -rf $PROJHOME/modules/${MODULE_FOLDER[$index]}/hal/$1 $PROJHOME/modules/${MODULE_FOLDER[$index]}/hal/$2
	else
	echo chip "$2" Module "${MODULE_FOLDER[$index]}" folder exist
	fi
fi
done
