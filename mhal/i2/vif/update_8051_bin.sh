#!/bin/bash
./bin2header hk51 < infinity_51.bin > 8051.h
TEST="char mcu_commit[]=\""$(git ls-remote ssh://hcgit04:29518/mstar/hk51  HEAD | awk '{print $1}' )"\";"
echo $TEST >> 8051.h
cp 8051.h drv/inc/


mkdir tmp
cd tmp

git archive  --remote=ssh://hcgit04:29518/mstar/hk51  HEAD infinity_51.bin  | tar -x

if cmp -s "../infinity_51.bin" "./infinity_51.bin" ; then
   echo "mcu bin compare pass"
else
   echo "[warning] mcu bin is different from git server"
fi

cd ..
rm -rf tmp
