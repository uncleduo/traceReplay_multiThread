# traceReplay_multiThread
## 内容
多线程对大trace进行模拟

## 状态
~~使用读者/写者模型读取文件，但是是轮询的，所以下一步需要加上cond~~

读者写者，当缓冲区中的未使用trace达到500后阻塞，使用到100时再唤醒

修改了一些bug

对本地操作(close, closedir)也进行复现，但不计入log
