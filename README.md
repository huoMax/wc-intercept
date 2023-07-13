<!--
 * @Author: huomax
 * @Date: 2023-06-08 05:38:11
 * @LastEditors: huomax
 * @LastEditTime: 2023-06-08 05:38:11
 * @FilePath: /wgk/wc-intercept/README.md
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
-->
# Underlying Library Functions and System Call Interception Techniques

### 项目名称

高效库函数和系统调用拦截技术

### 项目描述

拦截基础库、系统调用等底层函数，对理解掌握系统行为至关重要，在安全管理、计算容错、异常检测和性能分析中经常使用，但其拦截性能对系统影响较大，实现高效的拦截，是业界和学术界研究的重点。

通过该项目，可以深入理解基础库和OS的工作原理和执行流程。我们聚焦在系统底层函数的拦截，包括libc和系统调用，要求根据配置，拦截不同的函数、不同拦截频率、对接不同的拦截后的处理。通过本项目，可帮助学生深入理解OS中最常见的系统调用原理，掌握系统执行流程、操作系统原理、以及用户与内核交互等内容。

该项目通过实现高性能的拦截系统，降低函数拦截对系统性能的影响，优化拦截系统的可扩展性。

### 相关资源

1. [linux ptrace](https://man7.org/linux/man-pages/man2/ptrace.2.html)
2. [kernel module for system call collection](https://github.com/falcosecurity/libs/tree/master/driver)
3. [eCapture(旁观者): capture SSL/TLS text content without CA cert Using eBPF](https://github.com/gojue/ecapture)

### 所属赛道

2023全国大学生操作系统比赛的“OS功能”挑战赛道

### 参赛要求

（需要更新）

- 以小组为单位参赛，最多三人一个小组，且小组成员是来自同一所高校的本科生（2022年春季学期或之后本科毕业的大一~研三的学生）
- 如学生参加了多个项目，参赛学生选择一个自己参加的项目参与评奖
- 请遵循“2023全国大学生操作系统比赛”的章程和技术方案要求

### 项目导师

胡万明

- email [huwanming@huawei.com](mailto:huwanming@huawei.com)

### 难度

中等

### 特征

- 搭建开发环境，理解底层函数调用执行的原理和流程。
- 支持拦截特定的系统调用。
- 支持拦截基础库（libc）中特定的函数。

### License

# 预期目标

### 注意：选择本项目的同学也可与导师联系，提出自己的新想法，如导师认可，可加入预期目标

通过调研，熟悉现有拦截工具的原理，并理解其对系统性能的影响和原因。设计新的拦截机制，支持灵活配置不同的拦截目标、频率和处理后端，降低拦截所引入的额外开销。

### 第一题：基本的环境搭建和熟悉

- 理解系统调用和libc函数的调用执行流程
- 尝试现有的拦截工具，熟悉其功能和性能

### 第二题：拦截特定的系统调用

- 根据配置，确定拦截的系统调用范围
- 在系统调用前后，采集其相关信息，比如参数；返回值等
- 将拦截到信息对接给后端处理逻辑（具体的后端处理不在本课题范围）

### 第三题：拦截特定的libc函数

- 根据配置，确定拦截的libc函数范围及所需拦截信息
- 将拦截到信息对接给后端处理逻辑（具体的后端处理不在本课题范围）
- 优化对libc和应用的侵入式修改，最好实现应用无感