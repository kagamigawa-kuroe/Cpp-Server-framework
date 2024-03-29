#### 基于线程的协程调度 

首先，这个协程调度功能是基于线程调度的，在每个Scheduler类中，你需要指明线程的数量，然后会有一个vector记录所有开辟出的线程，同时也有一个vector记录所有加入的待执行的协程。

Scheduler类通过 **void schedule(Fiber fiber, int thread = -1)** 函数，加入向协程vector中加入一个新的线程，或是直接加入一个函数。

每个线程都会有一个**调度协程** 这个调度协程的内容为一个**run**函数，该函数首先会新开出一个**idle**协程，意为空闲协程，就是一个线程在没有任务执行的时候会去跑的协程。在这之后，**run**函数会轮询查看待做的协程vector中是否含有当前线程的可作的任务，**如果存在**，就执行他，做完后循环这个过程，继续查看任务，**如果不存在**，就会进入idle协程，在该协程内，一般情况下，我们会直接把该idle协程调度到后台，然后切换回调度协程继续进行我们的查找循环，但如果我们对scheduler调用了**stop函数**，就意味着我们想要结束这个调度器，我们就会直接让该**idle协程继续进行下去**，这样就会让该线程结束。

所以，一个普通的线程会有两个协程，一个是**调度协程**，一个是**空闲协程**，剩下的是从任务列表中取出进行的短暂协程，一般来说，我们会直接用直接**Fiber**类中的**主协程**作为调度协程，但是，对于主线程来说，是个例外，如果我们想让**主线程**也作为一个可以执行任务的对象，那么我们就需要让主线程在所有线程都被创建后，然后让主线程执行**调度fiber**，也就是**run函数**，但是注意，run函数是一个死循环，是通过idle，接收到主线程调用stop（）函数的信号，才会退出，并且结束线程，如果我们直接在主线程中执行了**run函数**，那么主线程本身就陷入死循环了，无法继续调用stop函数来结束线程，那么程序的逻辑就不成立了，这里有两种解决办法: 

1. 让主线程不做任何fiber任务，仅仅作为一个管理者存在
2. 主线程也会做任务，但是主线程的任务要在调用了stop函数之后才能启动，也就是说，主线程的调度协程run，要在stop函数中启动。

可以看出其实第二种做法并不太好，毕竟在实战场景中怎么能一直拖延任务呢。