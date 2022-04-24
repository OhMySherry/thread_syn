#include <iostream>
#include <Windows.h>
#include <process.h>
#include <vector>
using namespace std;
// 等价于WINAPI，约定使用stdcall函数调用，既被调用者负责清栈
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