# Recitation 5
## Answer

### Question 1

> Use your own word to describe different types of “RAID”. Why there are so many different types of RAID? Which scenarios are suitable for different types?

RAIDs:

+ RAID1: Every data disk has a copy as check disk in a group. (This improves redundancy by creating a new disk with the same information. You can improve performance on reads because if you are reading a chunk of data, you can break that chunk up into different start places and read these smaller chunks concurrently.)

+ RAID2: Using error correcting codes.Data were bit-interleaved in all data disks in a group, while several check disks were set to determine which disk crashed and recovery it.

+ RAID3: Data were bit-interleaved in all data disks in a group, while only one check disk were set to recovery data. Which disk was crashed was determined by each data disk itself.(Instead of using the more expensive error correcting code, we have any number of data disks and a single check disk which is just the XOR of all the data disks. If the check disk is incorrect, then you know that one of the data disks is incorrect as well.)

+ RAID4: Parity bits are stored on a single data. Data were not bit-interleaved any more, same files were stored in same data disks. Only one disk were set to recovery data, while which disk was crashed was determined by each data disk itself.

+ RAID5: The checksum was not stored in one check disk anymore, it was distributed in every disk instead. Moreover, the disks still need to determine if itself was crashed.You rotate the information of the parity information across the disk. It is unlikely that all the blocks you choose will have the same check disks so hopefully you can spread the check bits evenly across the disks. This prevents your single check disk from failing and losing your redundancy.

The reason of so many RAID types is because both the development of hardware and different scenarios.

The RAID1 is suitable for all scenarios when you don't care about space utility and have a primitive RAID controller.

Both RAID2 and RAID3 are suitable for single large file storage which mostly appeared in supercomputers. RAID3 requires a more advanced RAID controller.

Both RAID4 and RAID5 are suitable for both single large file storage and several small storage which mostly appeared in transaction-processing system. RAID5 required a more advanced RAID controller too.

### Question 2

> Modern RAID arrays use parity information and standby disks to provide a highly reliable storage medium even in the face of hardware failures. A highly reliable system, however, requires more than just a highly reliable storage medium. Consider a networked server handling network transactions (a web server or bank central computer, perhaps). Think about other components of this system whose failure could result in a loss of service. Identify one other component (software or hardware) of such a system that could be appropriately designed along the lines of one or another of the RAID levels, and describe the approach briefly or demonstrate in a picture, indicating how it relates to which RAID level.

In a distributed RPC server, the client sends requests to the master server. If a request requires many actions or functions to be processed, the master server will assign the works to worker server groups. Worker servers in the worker server group can tell the check server in the same group if itself was crashed and then the check server can recover the failure by redo the work according to parity information store in itself like original parameters.

This approach is designed along the 3th layer of RAID layers. The master server acts as the RAID controller, the check servers act as the check disk in each group and the worker servers act as the data disk in each group.

### My question

What if the checksum in the check disk in layer 4 stored seperately according to different data disks and each part provides seperate I/O ports and can R/W concurrently, can the performance be as good as layer 5? Is it easier to implement?
