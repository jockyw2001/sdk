2018-06-22

这份代码是基于HI3516A 070 的SDK里面的sample， 添加rtsp流程后形成的，完整的开发环境在
\\172.19.24.221\share\vmware\HI3516A

共享目录里面是vmware的一个硬盘， 在vmware的ubuntu里面添加这个硬盘，系统启动后，挂载就可以

交叉编译器的路径(目前只安装了uclibc的交叉编译器)
假如挂载到ubuntu系统的/home/work/HI3516A目录， export PATH="/home/work/HI3516A/toolchain/arm-hisiv300-linux/target/bin:$PATH"

使用方法：
替换这个目录下的代码到HI3516A 070 的SDK的sample，
cd sample;make
或者cd sample/rtsp;make /// 单独编译rtsp

rtsp访问方法
rtsp://172.19.24.241/main_stream
rtsp://172.19.24.241/sub_stream0
rtsp://172.19.24.241/sub_stream1