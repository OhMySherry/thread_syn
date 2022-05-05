---
Date : 2022 / 04 / 24
---
[TOC]

# 进程同步：生产者-消费者问题

## 问题描述

​        以生产者-消费者模型为基础，在Windows环境下创建一个控制台进程（或者界面进程），在该进程中创建读者写者线程模拟生产者和消费者。写者线程写入数据，然后将数据放置在一个空缓冲区中供读者线程读取。读者线程从缓冲区中获得数据，然后释放缓冲区。当写者线程写入数据时，如果没有空缓冲区可用，那么写者线程必须等待读者线程释放出一个空缓冲区。当读者线程读取数据时，如果没有满的缓冲区，那么读入线程将被阻塞，直到新的数据被写进去。

​        实验要求设计并实现一个进程，该进程拥有一个生产者线程和一个消费者线程，它们使用N个不同的缓冲区（N为一个确定的数值，本实验中取N=16）。可以作如下的实验尝试，并观察和记录进程同步效果：

（1） 没有信号量时实现生产者线程与消费者线程互斥制约。

（2） 用以下信号量实现生产者线程与消费者线程互斥制约：

创建临界区，用以阻止生产者线程和消费者线程同时操作缓冲区列表，功能同互斥信号量。

（3） 用以下信号量实现生产者线程与消费者线程的同步制约：

一个信号量full，当生产者线程生产出一个物品时可以用它向消费者线程发出信号。

一个信号量empty，消费者线程释放出一个空缓冲区时可以用它向生产者线程发出信号。

（4） 同时使用（2）（3）中的信号量实现生产者-消费者的互斥与同步制约。

## 实验环境

- Windows 11
- Visual Studio Code
- GCC version 8.1.0

## 输入

- 创建1个生产者线程和1个消费者线程，设定缓冲区大小为16，观察运行结果。

## 输出

- 生产者每生产一个资源，剩余资源就加一，最高到16，则停止生产；消费者每消费一个资源，剩余资源就减一，最低到0，则停止消费。

## 测试数据

| **生产者数量** | **消费者数量** | **缓冲区大小** |
| -------------- | -------------- | -------------- |
| **1**          | **1**          | **16**         |

- 测试两种不同情况：缓冲区满和缓冲区空

  - 缓冲区满：生产者线程睡眠时间短于消费者，则一段时间后会导致缓冲区满；

  - 缓冲区空：生产者线程睡眠时间长于消费者，则一段时间后会导致缓冲区空。

## 实验设计

### 数据结构

```cpp
//缓冲区大小
#define N 16
//使用临界区来同步线程
CRITICAL_SECTION cr;
//空信号量
HANDLE emptySemaphore = NULL;
//满信号量
HANDLE fullSemaphore = NULL;
//缓冲区
vector<int> buffer;
```

### 系统框架图

![image-20220424223259278](https://s2.loli.net/2022/04/24/kpoSTVRhcJgbIix.png)

### 流程图

![image-20220424223308558](https://s2.loli.net/2022/04/24/YvhqQCilztWeMoy.png)

## 实验结果与分析

### 结果展示与描述

- 生产速度大于消费速度：

![image-20220424223338540](https://s2.loli.net/2022/04/24/2zRhe1GKPsZbQOS.png)

![image-20220424223348407](https://s2.loli.net/2022/04/24/hiayErH8jufIdGl.png)

- 生产速度小于消费速度：

![image-20220424223402598](https://s2.loli.net/2022/04/24/5OcnNmSCfH6hPQ8.png)

### 结果分析

- 首先创建了empty和full2个信号量以及临界区用来实现进程同步问题,生产者不断生产资源，缓冲区大小为16，缓冲区满时停止生产。只有缓冲区有内容时，消费者才能执行他的动作。
- 生产速度大于消费速度时，一段时间后缓冲区满，生产者停止生产，等到消费者取出一个资源时，生产者再继续生产，因此资源最终在15和16之间跳动；
- 生产速度小于消费速度时，缓冲区空时消费者停止消费，等到生产者生产一个资源时，消费者再继续消费，因此资源会在0和1之间跳动。

### 总结

- 编写多线程程序时，有时不可避免的需要在多个线程之间传递数据，就可能出现多个线程同时访问同一个资源的情况，这时就需要线程间的通信，解决同步与互斥的问题。
- 本实验在Windows环境下，使用临界区的方式实现线程间的互斥。本实验共有1个生产者和1个消费者，结果表明生产者与消费者之间并没有产生矛盾，资源的增加与减少都符合规律。

## 源代码

```cpp
#include <iostream>
#include <Windows.h>
#include <process.h>
#include <vector>
using namespace std;
// 等价于WINAPI，约定使用stdcall函数调用，即被调用者负责清栈
#define STD __stdcall
//缓冲区大小
#define N 16
#define GETMYRAND() (int)(((double)rand() / (double)RAND_MAX) * 3000)
//使用临界区来同步线程
CRITICAL_SECTION cr;
//空信号量
HANDLE emptySemaphore = NULL;
//满信号量
HANDLE fullSemaphore = NULL;
//缓冲区
vector<int> buffer;

//消费者线程
DWORD STD Consumer(void *lp)
{
	while (true)
	{
		//等待判断缓冲区满信号量
		WaitForSingleObject(fullSemaphore, 0xFFFFFFFF);
		//进入临界区，线程同步，功能同互斥量
		EnterCriticalSection(&cr);
		//消费者线程从缓冲区取出消费一个资源
		buffer.pop_back();
		//打印当前缓冲区可用资源数
		cout << "消费一个资源，当前可用资源数：" << buffer.size() << endl;
		//离开临界区
		LeaveCriticalSection(&cr);
		//释放判断缓冲区空的信号量
		ReleaseSemaphore(emptySemaphore, 1, NULL);
		//线程睡眠随机时间
		Sleep(GETMYRAND() + 2000);
	}
	return 0;
}

//生产者线程
DWORD STD Producer(void *lp)
{
	while (true)
	{
		//等待判断缓冲区空信号量
		WaitForSingleObject(emptySemaphore, 0xFFFFFFFF);
		//进入临界区，线程同步，功能互斥量
		EnterCriticalSection(&cr);
		//生产者线程向缓冲区中生成一个资源
		buffer.push_back(1);
		//打印当前缓冲区可用资源数
		cout << "生产一个新资源，当前可用资源数：" << buffer.size() << endl;
		//离开临界区
		LeaveCriticalSection(&cr);
		//释放判断缓冲区满信号量
		ReleaseSemaphore(fullSemaphore, 1, NULL);
		//线程睡眠随机时间
		Sleep(GETMYRAND() + 200);
	}
	return 0;
}

// 主函数
int main()
{
	//创建信号量
	emptySemaphore = CreateSemaphore(NULL, N, N, NULL);
	fullSemaphore = CreateSemaphoreW(NULL, 0, N, NULL);
	//初始化临界区
	InitializeCriticalSection(&cr);
	//开启多线程
	HANDLE handles[2];
	handles[1] = CreateThread(0, 0, &Producer, 0, 0, 0);
	handles[0] = CreateThread(0, 0, &Consumer, 0, 0, 0);
	//等待子线程执行完毕
	WaitForMultipleObjects(2, handles, true, INFINITE);
	//释放子线程
	CloseHandle(handles[0]);
	CloseHandle(handles[1]);
	//释放临界区
	DeleteCriticalSection(&cr);
	return 0;
}
```



