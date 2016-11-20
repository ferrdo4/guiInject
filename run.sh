export GI_TOKEN=GI
LD_LIBRARY_PATH=`pwd`:$LD_LIBRARY_PATH
LD_PRELOAD=$LD_PRELOAD:./libguiInject.so ../ping/ping
