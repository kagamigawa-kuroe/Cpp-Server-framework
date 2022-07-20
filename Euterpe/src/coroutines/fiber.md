#### 协程库的实现

---

基本是基于**ucontext.h**库实现的，该库的主要用法：

```c++
// 用户上下文的获取和设置

int getcontext(ucontext_t *ucp);
// 将当前的 context 保存在 ucp 中。成功返回 0，错误时返回 -1 并设置 errno；

int setcontext(const ucontext_t *ucp);
// 恢复用户上下文为 ucp 所指向的上下文，成功调用不用返回。错误时返回 -1 并设置 errno。

// 操纵用户上下文

void makecontext(ucontext_t *ucp, void (*func)(void), int argc, ...);
// 绑定一个上下文和一个函数 每当切换到这个上下文时 函数就会被执行

int swapcontext(ucontext_t *oucp, const ucontext_t *ucp);
// 交换上下文
```

 4 个函数都依赖于 `ucontext_t` 类型，这个类型大致为：

```c++
typedef struct {
    ucontext_t *uc_link;
    sigset_t    uc_sigmask;
    stack_t     uc_stack;
    mcontext_t  uc_mcontext;
    ...
} ucontext_t;
```

uc_link : 当前上下文结束时要恢复到的上下文

uc_stack : 上下文所使用的 stack

uc_mcontext :  其中的mcontext_t

在makecontext之后, 注册了一个上下文和一个函数，每当切换到这个上下文，函数就将被触发。

---

#### 核心机制如上所述 接下来介绍我的封装过程以及用法

----

每一个线程有一个主协程，通过和主协程Swap来实现协程的切换。

Fiber类中有：

**m_id 协程id**

**m_stacksize 协程栈大小**

**m_stat 协程状态 是一个枚举类型**

**m_ctx 协程上下文**

**m_stack 上下文对应的栈**

**m_cb 协进程运行的函数**

---

**主要用法**是

1. 先new一个fiber 传入你想要执行的函数
2. 调用SwapIn方法 抢占主线程 如果主线程现在有程序在执行 则会报错
3. 抢占到线程的协程 在自己想要让出线程的时候 调用YieldToHold或者YieldToReady函数 这两个函数都是封装了 SwapOut函数 只不过切换到后台后设置的状态有所不同
4. 让出线程后 别的协程就可以SwapIn 如此往复

不难看出 这只是最基本的功能 关于协程调度的功能 会专门在协程调度模块中实现

---

 然后来说具体的实现过程：

1. 首先 每个线程都有一个thread_local类型的t_fiber 用于记录指向当前正在运行的fiber 同时 也有一个t_threadfiber 用来指向main fiber

2. 关于协程id的分配 每个线程都有一个s_fiber_id 用来记录现在协程到第几号了 同时也有一个 s_fiber_count来记录现在一共有多少个协程

3. setThis和getThis 将当前运行的fiber 以及获取当前运行的fiber (t_fiber) 这里当getthis返回的值是空时 也就是还没有主fiber时 就会调用Fiber的默认构造函数 来新建一个mainfiber 并赋给t_threadfiber

4. 构造函数 有两个构造函数 1. 无参数的默认构造函数Fiber() 被设置为私有 专门用于创建主线程 2. 而另一个构造函数 参数是一个函数 和 栈空间大小 用于创建一般fiber 创建流程为 先开辟一块堆空间，然后设置m_ctx的堆空间以及对空间大小，最后makecontext(m_ctx,MainFunc) 这里的Mainfunc是对Fiber类中m_cb的封装 后面会解释

5. Mainfunc 先设当前线程的t_fiber为自己 成功后执行函数 执行完后将自己的指针置空 然后将协度状态设置为结束 结束后swapout 让出协程位置

6. SwapIn 函数 抢占线程 首先 setthis 然后 切换为执行状态 并且用swapcontext函数和主协程的context交换 处罚mainfunc函数

   mainfunc函数执行过程中 可以随时通过YieldToHold来暂停协程 切换回main fiber

7. 还有SwapOut函数 将会吧t_fiber设置成main fiber 然后交换当前fiber到main fiber

   

   

