Please read End-to-end argument and think about following questions:

- Use your own word to describe “end-to-end” argument.
- Give at least three cases that are suitable to use this principle.
- Give at least three cases that are NOT suitable to use this principle.
- [Discussion] Consider the design of the file system based on inode (as we learn from class and the lab ). Which part(s) of the design do you think can be removed? In another word, which part(s) of the design is(are) not flexible enough and should be implemented by the user? (Hint: don’t be limited by the FS API.)
- [Discussion] The same question, for the OS.Take your time preparing for this.
***

### My Answer

> Use your own word to describe “end-to-end” argument.

'end-to-end' argument is something stored in the packets. When the receiver client received packet, it will directly ask the sender client to verify if the argument is the corresponding one. All the actions will be done at application layer instead of low-level layer.

“端到端”参数是存储在数据包中的一种参数。当接收方客户端接收到数据包时，会直接询问发送方客户端，验证该参数是否为相应的参数。 所有的操作将在应用层而不是底层完成。



> Give at lease three cases that are suitable to use this principle.

- Delivery guarantees: Using RFNM mechanic at application level

- Secure transmission of data: Encryption and decryption are all provided at application layer and no need of low-level part to provide

- Duplicate message suppression: The application layer determine if it has got a duplicate message by end-to-end communication

- Guaranteeing FIFO message delivery

- Transaction management

- 交付保证：在应用层面使用RFNM机制

- 安全传输数据：加密和解密都在应用层提供，不需要底级部分提供

- 重复消息抑制：应用层确定是否通过端对端通信获得重复消息

- 保证FIFO消息传递

- 交易管理




> Give at lease three cases that are NOT suitable to use this principle.

- On micro-controller like MCU, the CPU and memory not allow too much high level calculation and communication, end-to-end is not suitable.

- When you don't know where your corresponding end is, then end-to-end is not suitable.

- For high security data, users don't want to re-send their message twice.

- 对于像MCU这样的微控制器，CPU和内存不允许太高的计算和通信，端到端是不合适的。

- 当你不知道你的相应的端位置时，那么端到端是不合适的。

- 对于高安全性的数据，用户不希望重复发送消息两次。



> Consider the design of the file system based on inode (as we learn from class and the lab). Which part(s) of the design do you think can be removed? In another word, which part(s) of the design is(are) not flexible enough and should be implemented by the user? (Hint: don’t be limited by the FS API)

The journal behavior. File system would use journal to log the transaction happened before, and it may be not flexible enough. It should offer an API to allow user to change Log's behavior.
日志行为。文件系统会使用日志记录之前发生的事务，可能不够灵活。它应该提供一个API来允许用户更改日志的行为。



> The same question, for the OS.

The desktop environment. It's outside the kernel and can be implement by user, just like KDE and GNOME.

桌面环境。它不在内核之中，可以在用户层实现，例如KDE和GNOME。


#### My question

If the end itself is wrong, like it always fake a right response instead of asking the sender, is it still necessary to implement low-level security guarantee?

如果端本身是错误的，就像它总是假冒正确的回应，而不是问发件人，那么是否仍然有必要实施低层次的安全保证？

***

Here to read the paper: [http://web.mit.edu/Saltzer/www/publications/endtoend/endtoend.pdf](http://web.mit.edu/Saltzer/www/publications/endtoend/endtoend.pdf)
