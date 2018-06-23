#!/bin/sh
target=$@
script=_run.sh~
echo "#!/bin/sh" > $script
chmod 755 $script
if [ "$target" == "" ]; then
  find . -name "case*.sh" >> $script;
else
  printf "./%s\n" "$@" >> $script;
fi

time ./$script 2> /tmp/err

echo  ========================================
echo "              TEST RESULT"
echo  ========================================
cat /tmp/err
AnyErr=`grep -i fail /tmp/err`
echo -e "\n\n========================================\n"
if [ "$AnyErr" == "" ];then echo All pass; else echo some error has been found!; fi