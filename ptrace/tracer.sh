###
 # @Author: huomax
 # @Date: 2023-05-27 23:47:05
 # @LastEditors: huomax
 # @LastEditTime: 2023-06-06 06:10:15
 # @FilePath: /wgk/wc-intercept/ptrace/tracer.sh
 # @Description: 执行跟踪程序
 # 
 # Copyright (c) 2023 by huomax, All Rights Reserved. 
### 

LD_LIBRARY_PATH=LD_LIBRARY_PATH:$PWD/./ ./tracer ./ini.config $@
