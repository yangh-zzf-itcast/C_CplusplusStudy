// 懒汉式遇上多线程问题同步优化.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include<iostream>
#include"windows.h"
#include<stdio.h>
//#include"winbase.h"
#include<process.h>   //
//#include <afxmt.h>  //MFC,CCriticalSection类头文件
#include"myMutex.h"




using namespace std;
//构造函数不是线程安全函数，无法保证多线程安全
//因此要保证线程同步来解决这个问题
//解决方案：二次检查，创建临界区，使用cs.Lock(),cs.Unlock()函数
//static CCriticalSection cs;   //MFC的临界对象

Mutex mutex1;//我的互斥量
class Singleton
{
private:
	//由于构造函数中有休眠，因此懒汉式单例遇到多线程问题时
	//当第一个线程进入构造函数创建对象休眠，还未返回对象，第二个线程并不知晓，因此也进入构造函数，因此所得非单例
	Singleton()
	{
		cout << "Singleton构造函数私有化start---------->" << endl;
		Sleep(1000);
		cout << "Singleton构造函数私有化end---------->" << endl;
	}
public:
	static Singleton *getInstance()
	{
		//InitializeCriticalSection(&m_cs);
		if (m_psl == NULL)                   //静态变量为空时才创建，创建之后不再创建
		{
			synchronized(mutex1)
			{
				//EnterCriticalSection(&m_cs);       //进入临界区
				if (m_psl == NULL)               //二次检查
				{
					count++;
					m_psl = new Singleton;
				}
				//LeaveCriticalSection(&m_cs);       //离开临界区
			}
		}
		return m_psl;
	}

	static Singleton *FreeInstance()
	{
		if (m_psl != NULL)                   //静态变量非空时才释放
		{
			delete m_psl;
			m_psl = NULL;
		}
		return m_psl;
	}
	//线程测试函数
	static void printS()
	{
		cout << "Singleton printS() test" << endl;
	}
	static void printC()
	{
		cout << "count: "<< count << endl;
	}
private:
	//static CRITICAL_SECTION m_cs;
	static Singleton *m_psl;
	static int count;
};
//静态变量初始化
//CRITICAL_SECTION Singleton::m_cs;

Singleton * Singleton::m_psl = NULL;
int  Singleton::count = 0;
void mainTest()
{

	Singleton *p1 = Singleton::getInstance();
	Singleton *p2 = Singleton::getInstance();
	if (p1 == p2)
	{
		cout << "同一个对象" << endl;
	}
	else
	{
		cout << "error" << endl;
	}
	Singleton::FreeInstance();
}

void myThreadFunc(void *)
{
	//cout << "线程体被执行" << endl;        //线程中不会换行
	cout << "线程体被执行\n";
	Singleton::getInstance()->printC();      //将被三个进程同时进行，创建三个对象
											 //多线程下懒汉式单例模式将不再是单例
}

int _tmain(int argc, _TCHAR *ARGV[])
{
	//mainTest();
	/* //开始一个线程,子线程_StartAddress依赖于进程_StackSize
	_ACRTIMP uintptr_t __cdecl _beginthread(
	_In_     _beginthread_proc_type _StartAddress,
	_In_     unsigned               _StackSize,
	_In_opt_ void*                  _ArgList
	);
	*/
	HANDLE hThread[10];    //句柄
	for (int i = 0; i < 3; i++)
	{
		hThread[i] = (HANDLE)_beginthread(myThreadFunc, 0, NULL);   //创建开始一个线程
	}

	//主进程等待所有子线程结束后，才执行
	for (int i = 0; i < 3; i++)
	{
		WaitForSingleObject(hThread[i], INFINITE);
	}
	cout << "等待线程结束" << endl;
	cout << "hello world" << endl;
	system("pause");
	return 0;
}

