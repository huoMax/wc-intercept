#!/bin/bash
###
 # @Author: huomax
 # @Date: 2023-05-20 00:09:28
 # @LastEditors: huomax
 # @LastEditTime: 2023-06-07 05:28:39
 # @FilePath: /wgk/wc-intercept/LD_PRELOAD/wc_preload.sh
 # @Description: 
 # 
 # Copyright (c) 2023 by huomax, All Rights Reserved. 
### 
LD_LIBRARY_PATH=LD_LIBRARY_PATH:$PWD/./ ./tracer ./ini.config $@