set -e  #遇到任何错误，脚本立即退出（终止执行）。

rm -rf `pwd`/build/*
cd `pwd`/build/ &&
    cmake .. &&
    make 
cd ..
#-r：递归复制整个目录及内容
cp -r `pwd`/src/include/ `pwd`/lib  #将 src/include/ 目录复制到 lib/ 目录中。