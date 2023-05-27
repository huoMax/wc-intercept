###
 # @Author: huomax
 # @Date: 2023-05-27 23:47:05
 # @LastEditors: huomax
 # @LastEditTime: 2023-05-28 00:05:27
 # @FilePath: /wgk/wc-intercept/ptrace/tracer.sh
 # @Description: 执行跟踪程序
 # 
 # Copyright (c) 2023 by huomax, All Rights Reserved. 
### 

LD_LIBRARY_PATH=LD_LIBRARY_PATH:$PWD/./ ./obj/tracer main $@
