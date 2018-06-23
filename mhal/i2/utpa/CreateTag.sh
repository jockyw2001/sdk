Utopia_Path=\
(
./
./projects
./mxlib
./mxlib/include
./mxlib/msfs
./modules/msos
./modules/xc
./modules/acp
./modules/ch34
./modules/dac
./modules/dlc
./modules/mfc
./modules/pq
./modules/sc
./modules/ve
./modules/wble
./modules/audio
./modules/demodulator
./modules/dscmb
./modules/graphic
./modules/hdmi
./modules/mfe
./modules/pcmcia
./modules/bdma
./modules/cmdq
./modules/ddc2bi
./modules/flash
./modules/hwi2c
./modules/ipauth
./modules/ir
./modules/mspi
./modules/pwm
./modules/rtc
./modules/sar
./modules/uart
./modules/wdt
./modules/security
./modules/hsl
./modules/mbx
./modules/swi2c
./modules/cpu
./modules/gpio
./modules/irq
./modules/miu
./modules/pm
./modules/pws
./modules/seal
./modules/sem
./modules/smbx
./modules/sys
./modules/urdma
./modules/dmx
./modules/usb
./modules/vd
./modules/gpd
./modules/vdec_v1
./modules/vdec_v2
./modules/vdec_v3
./modules/vdec_lite
./modules/ojpd_vdec_v1
./modules/ojpd_vdec_v2
./modules/mvop
./modules/njpd
./modules/stbdc
./modules/vif
./modules/clkm
./modules/ldm
./modules/dms
./modules/ir_tx
./modules/pvr_iframelut
)

Utopia_commit_message=\
(
UTPA2-700.0.x
projects
mxlib
include
msfs
msos
xc
acp
ch34
dac
dlc
mfc
pq
sc
ve
wble
audio
demodulator
dscmb
graphic
hdmi
mfe
pcmcia
bdma
cmdq
ddc2bi
flash
hwi2c
ipauth
ir
mspi
pwm
rtc
sar
uart
wdt
security
hsl
mbx
swi2c
cpu
gpio
irq
miu
pm
pws
seal
sem
smbx
sys
urdma
dmx
usb
vd
gpd
vdec_v1
vdec_v2
vdec_v3
vdec_lite
ojpd_vdec_v1
ojpd_vdec_v2
mvop
njpd
stbdc
vif
clkm
ldm
dms
ir_tx
pvr_iframelut
)

if [ ! "$1" ];then
	echo Please input a tag.
	exit
fi
if [ "$1" == "help" ];then
	echo -e Usage:
	echo -e CreateTag TAGNAME for add a simple tag
	echo -e CreateTag TAGNAME DESCRIPTION for adding a description
	exit
fi
echo Tag is $1
if [ "$2" ] && [ "$2" != "-c" ];then
	echo Description is $2
	DESCRIPTION=$2
fi

PROJHOME=$PWD
for ((jndex=0; jndex<${#Utopia_Path[@]}; jndex++))
do
cd ${Utopia_Path[$jndex]}
Utopia_commit_message[$jndex]=${Utopia_commit_message[$jndex]}=$(git rev-parse HEAD)
cd $PROJHOME
done

TAGMESSAGE="Created by $USER
Description: $DESCRIPTION"
for ((jndex=0; jndex<${#Utopia_Path[@]}; jndex++))
do
TAGMESSAGE=$TAGMESSAGE"
${Utopia_commit_message[$jndex]}"
done
TAGMESSAGE=$TAGMESSAGE"
Date: $(date '+%Y/%m/%d %H:%M:%S')"
echo "--------------------------------------------------"

for ((jndex=0; jndex<${#Utopia_Path[@]}; jndex++)); do
cd ${Utopia_Path[$jndex]}
git tag -a $1 -m "$(git remote -v | head -n1 | awk '{print $2}' | sed 's/.*\///' | sed 's/\.git//')
	$TAGMESSAGE"
if [ "$?" -ne "0" ];then
	exit 1
fi
cd $PROJHOME
done
git tag $1 -l -n80
