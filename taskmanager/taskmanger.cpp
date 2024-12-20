#include "taskmanger.h"
//#include "widStu.h"


std::vector<TaskManger*> TaskManger::mainManger;
std::map<TaskManger*, TaskManger*> TaskManger::mangerMap;
TaskManger::TaskManger()
{
	//hAddTaskEvent= CreateEventA(NULL, FALSE, FALSE, "ManagerAddTask");
}
int TaskManger::init(int threadNum)
{
	if (threadNum < 2) {
		threadNum = 2;
	}
	mainThreads = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)TaskManger::managerMainThread, this, 0, nullptr);

	for (int i = 0; i < threadNum; i++) {
		creatThread();
	}
	_threadNum = threadNum;
	inited = true;
	name = "NULL";
	//addTask(selfClock, this);
	   
	mainManger.push_back(this);
	mangerMap[this] = this;
	iWaitingTask = tasks.size();
	iIdelThread = idelThreadNum();
	//try {
	//	if (!hasGroup(u8"TaskManger")) {
	//		newGroup(u8"TaskManger");
	//	}
	//	//newCVar(&idelPth, "idelPth", C_OUTPUT, 0, name, u8"空闲线程数");
	//}
	//catch (QString e) {
	//	insertLog(-1, "TaskManger", e);
	//}

	return 0;
}
TaskManger::~TaskManger()
{
	//mangerMap.erase(this);
	//mangerMap.erase(mangerMap.find(this));
}
int TaskManger::idelThreadNum()
{
	return threads.size();
}
int TaskManger::allThreadNum()
{
	return _threadNum;
}
int TaskManger::creatThread()
{
	Thread* newThread = new Thread(this);
	newThread->taskParam.lpParameter = nullptr;
	newThread->taskParam.lpParameter = nullptr;
	newThread->xClear = false;
	newThread->state = waitting;
	newThread->nHandle = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)TaskManger::childThread, newThread, 0, nullptr);

	pushBack(newThread);

	return 0;
}
int TaskManger::addTask(void* lpStartAddress, void* lpParameter)
{
	if (lpStartAddress == nullptr) {
		return -1;
	}
	if (xAlarm){
		if (tasks.size() >= iTaskSize) {
			//insertLog(-1, QString::fromStdString(name), u8"超出任务预警限制，请检查程序");
			return -1;
		}
		
	}
	Task* newTask = new Task();
	newTask->lpStartAddress = (LPTHREAD_START_ROUTINE)lpStartAddress;
	newTask->lpParameter = (LPVOID)lpParameter;

	mtx.lock();
	tasks.push_back(*newTask);
	mtx.unlock();
	iWaitingTask = tasks.size();
	iIdelThread = idelThreadNum();

	delete newTask;
	ResumeThread(mainThreads);
	return 0;
}
int TaskManger::addTaskEx(void* lpStartAddress, void* lpParameter)
{
	//CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)lpStartAddress, (LPVOID)lpParameter, 0, nullptr);
	Thread* _thread = new Thread(this);
	_thread->pool = this;
	_thread->taskParam.lpStartAddress = (LPTHREAD_START_ROUTINE)lpStartAddress;
	_thread->taskParam.lpParameter = (LPVOID)lpParameter;
	CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)TaskManger::childThreadEx, _thread, 0, nullptr);
	return 0;
}
int TaskManger::pushBack(Thread* _thread)
{
	threadMtx.lock();
	threads.push_back(_thread);
	threadMtx.unlock();
	return 0;
}
TaskManger::Thread* TaskManger::getThread()
{
	if (threads.size() < 2) {
		return nullptr;
	}
	if (threads.size() > 0) {
		return threads[0];
	}
	else {
		if (xExCreat) {
			creatThread();
			return threads[0];
		}
		else {
			return nullptr;
		}

	}
}
int TaskManger::executeTask(Task _task)
{
	
	TaskManger::Thread* thread;
	thread = getThread();
	if (thread == nullptr) {
		
		return -1;
	}
	threads.erase(threads.begin());
	thread->execute(&_task);

	return 0;
}
void* TaskManger::managerMainThread(TaskManger* self)
{
	while (true) {
		SuspendThread(self->mainThreads);
		self->mtx.lock();
		int exeNum = 0;
		self->threadMtx.lock();
		for (int i = 0; i < self->tasks.size(); i++) {
			int ret = self->executeTask(self->tasks[i]);
			if (ret == 0) {
				exeNum++;
			}
			else {
				break;
			}
		}
		self->threadMtx.unlock();
		for (int i = 0; i < exeNum; i++) {
			self->tasks.erase(self->tasks.begin());
		}
		//self->tasks.clear();
		self->iWaitingTask = self->tasks.size();
		self->iIdelThread = self->idelThreadNum();
		self->mtx.unlock();
	}
	return nullptr;
}
void* TaskManger::childThread(Thread* _thread)
{
	TaskManger* pool = _thread->pool;
	while (true) {
		//ResumeThread(pool->mainThreads);
		SuspendThread(_thread->nHandle);
		_thread->state = busy;

		//TaskManger* pool = _thread->pool;

		if (_thread->pool->xDead) {
			pool->creatThread();
		}	
		if (_thread->taskParam.lpStartAddress == nullptr) {
			//
		}else {
			try {
				_thread->taskParam.lpStartAddress(_thread->taskParam.lpParameter);
			}
			catch (...) {
				_thread->pool->getOut++;
			}
		}

		ResumeThread(pool->mainThreads);
		
		if (!_thread->pool->xDead){
			_thread->state = waitting;
			_thread->pool->pushBack(_thread);
		}


		

		if (_thread->pool->xDead) {
			delete _thread;
			return nullptr;
		}

	}

	//delete _thread;
	return nullptr;
}



TaskManger::Thread::Thread(TaskManger* arg)
{
	pool = arg;
}
TaskManger::Thread::~Thread()
{
}
int TaskManger::Thread::execute(Task* task)
{
	if (state != waitting) {
		return -1;
	}
	taskParam = *task;
	state = busy;
	ResumeThread(nHandle);
	return 0;
}



int TaskManger::taskSzie()
{
	return tasks.size();
}
void TaskManger::setExCreat(bool value)
{
	xExCreat = value;
}

void* TaskManger::selfClock(void* arg)
{
	while (true) {
		TaskManger* self = (TaskManger*)arg;
		ResumeThread(self->mainThreads);
		Sleep(10);
	}
	return  nullptr;
}
void* TaskManger::childThreadEx(Thread* _thread)
{
	_thread->pool->exMtx.lock();
	_thread->pool->exThread++;
	_thread->pool->exMtx.unlock();
	if (_thread->taskParam.lpStartAddress == nullptr) {
	
	}else {
		try {
			_thread->taskParam.lpStartAddress(_thread->taskParam.lpParameter);
		}
		catch (...) {
			_thread->pool->getOut++;
		}
	}

	_thread->pool->exMtx.lock();
	_thread->pool->exThread--;
	_thread->pool->exMtx.unlock();
	delete _thread;

	return nullptr;
}



//中断所有线程任务，并且调用线程的异常结束处理方法,暂时没有实现，需要修改一部分代码
int TaskManger::stopAllThread()
{
	return 0;
}
//中断目标线程，并且调用线程的异常结束处理方法，暂时没有实现，需要修改一部分代码
int TaskManger::stopThread(unsigned long id) 
{
	return 0;
}
