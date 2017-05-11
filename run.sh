#export GI_TOKEN=GI
LD_LIBRARY_PATH=`pwd`:$LD_LIBRARY_PATH
LD_PRELOAD=$LD_PRELOAD:./libguiInject.so ../ping/ping
#TODO this needs to be fixed - identify Qt app LD_PRELOAD=$LD_PRELOAD:./libguiInject.so ls

