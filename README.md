# traceReplay_multiThread
## 内容
多线程对大trace进行模拟

## 状态
使用读者/写者模型读取文件，但是是轮询的，所以下一步需要加上cond
