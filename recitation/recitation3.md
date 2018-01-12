Read the Eraser paper, and think about the following questions as you read the paper:

- According to the lockset algorithm, when does eraser signal a data race? Why is this condition chosen?

- Under what conditions does Eraser report a false positive? What conditions does it produce false negatives?

- Typically, instrumenting a program changes the intra-thread timing (the paper calls it interleaving). This can cause bugs to disappear when you start trying to find them. What aspect of the Eraser design mitigates this problem?

Please raise at least one question of your own for the discussion.
***

### My Answer

> According to the lockset algorithm, when does eraser signal a data race? Why is this condition chosen?

一个变量在刚被分配时，属于Virgin状态，表示这个变量没有被任何线程引用。一旦这个变量被访问了，那么就变为Exclusive状态，表示这个变量已经被访问了，但只有一个线程访问过。在这个状态下，同一个线程的后续读或写不会改变当前状态，也不会更新C(v)。（解决了初始化问题）如果另一个线程访问了这个变量，那么状态改变，如果是读取，那么状态变为Shared状态，这种状态下，C(v)会更新，但即使C(v)变为空也不会报告race出现。（解决了读共享问题，一个变量是只读的，可被多个线程读取，但不会造成race）在Exclusive状态下，变量被第一个线程以外的线程写入了，或者在Shared状态下，变量被线程写入了，那么这个变量就变为Shared-Modified状态。这种状态下，C(v)会更新，并会报告race。

概括来说，在变量处于Shared-Modified状态下，且C(v)在更新的过程中变为空集，则报告race。原因就是Virgin、Exclusive和Shared状态下，只有单线程对变量进行读写或者多线程只对变量进行读取，不会出现race状况。



> Under what conditions does Eraser report a false positive? What conditions does it produce false negative?

误报(false positive)：比如内存重用、私有锁、良性竞争（生产者消费者）、只有测试用例有足够的共享变量读取跟随相应的写入，Eraser才会工作的很好。

漏报(false negative):

（1）当一个线程未使用锁的情况下初始化一个变量，在初始化完成前错误的使该变量能被第二个线程访问；如果第二个线程确实访问了变量，则Eraser报告错误，否则Eraser会漏掉这个错误。

（2）如果线程t1在保持锁定m1时读取v，并且线程t2在保持锁定m2的情况下写入v，则仅当写入在读取之前才会报告违反锁定规则的行为。


> Typically, instrumenting a program changes the intra-thread timing (the paper calls it interleaving). This can cause bugs to disappear when you start trying to find them. What aspect of the Eraser design mitigates this problem?

报告race时，Eraser指出race出现的文件和行号以及所有活动堆叠帧的回溯列表。该报告还包括线程ID，存储器地址，存储器访问类型以及重要的寄存器值，例如程序计数器和堆栈指针。

> Raise your own questions

Eraser是不是缺乏灵活性和普适性？例如它不支持多种不同的锁机制，需要针对特定的案例进行修改。

Eraser太过于浪费内存，对于数据段和堆中的每个32位字，存在一个对应的阴影字，用于包含30位锁定索引和2位状态条件。

Eraser不支持多重保护锁的检测，而现在的工程师都常常使用多重锁来保护数据。


***

Here to read the paper: [http://cseweb.ucsd.edu/~savage/papers/Tocs97.pdf](http://cseweb.ucsd.edu/~savage/papers/Tocs97.pdf)
