/*
    Copyright (c) 2007-2013 Contributors as noted in the AUTHORS file

    This file is part of 0MQ.

    0MQ is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    0MQ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __ZMQ_YPIPE_HPP_INCLUDED__
#define __ZMQ_YPIPE_HPP_INCLUDED__

#include "atomic_ptr.hpp"
#include "yqueue.hpp"

//  Lock-free queue implementation.
//  Only a single thread can read from the pipe at any specific moment.
//  Only a single thread can write to the pipe at any specific moment.
//  T is the type of the object in the queue.
//  N is granularity of the pipe, i.e. how many items are needed to
//  perform next memory allocation.

template <typename T, int N>
class ypipe_t
{
public:
    //  Initialises the pipe.
    inline ypipe_t()
    {
        //  Insert terminator element into the queue.
        queue.push(); //yqueue_t的尾指针加1，开始back_chunk为空，现在back_chunk指向第一个chunk_t块的第一个位置

        //  Let all the pointers to point to the terminator.
        //  (unless pipe is dead, in which case c is set to NULL).
        r = w = f = &queue.back(); //就是让r、w、f、c四个指针都指向这个back迭代器
        c.set(&queue.back());  // 以非线程安全的方式设置值，因为这里只有一个线程能执行这里代码。
    }

    //  The destructor doesn't have to be virtual. It is mad virtual
    //  just to keep ICC and code checking tools from complaining.
    inline virtual ~ypipe_t()
    {
    }

    //  Following function (write) deliberately copies uninitialised data
    //  when used with zmq_msg. Initialising the VSM body for
    //  non-VSM messages won't be good for performance.

#ifdef ZMQ_HAVE_OPENVMS
#pragma message save
#pragma message disable(UNINIT)
#endif

    //  Write an item to the pipe.  Don't flush it yet. If incomplete is
    //  set to true the item is assumed to be continued by items
    //  subsequently written to the pipe. Incomplete items are neverflushed down the stream.
    // 写入数据，incomplete参数表示写入是否还没完成，在没完成的时候不会修改flush指针，即这部分数据不会让读线程看到。
    inline void write(const T &value_, bool incomplete_)
    {
        //  Place the value to the queue, add new terminator element.
        queue.back() = value_;   // 这里back()表示队列中最后一个元素的下一个空位置
        queue.push();

        //  Move the "flush up to here" poiter.
        if (!incomplete_)
        {
            f = &queue.back(); // 记录要刷新的位置
            // printf("1 f:%p, w:%p\n", f, w);
        }
        else
        {
            //  printf("0 f:%p, w:%p\n", f, w);
        }
    }

#ifdef ZMQ_HAVE_OPENVMS
#pragma message restore
#endif

    //  Pop an incomplete item from the pipe. Returns true is such
    //  item exists, false otherwise.
    inline bool unwrite(T *value_)
    {
        if (f == &queue.back())  // 没有未刷新的写入数据就不能回退操作了
            return false;
        queue.unpush();
        *value_ = queue.back();
        return true;
    }

    //  Flush all the completed items into the pipe. Returns false if
    //  the reader thread is sleeping. In that case, caller is obliged to
    //  wake the reader up before using the pipe again.
    // 刷新所有已经完成的数据到管道，返回false意味着读线程在休眠，在这种情况下调用者需要唤醒读线程。
    // 批量刷新的机制， 写入批量后唤醒读线程；
    // 反悔机制 unwrite
    inline bool flush()   // 返回false表示需要唤醒消费者线程
    {
        //  If there are no un-flushed items, do nothing.
        if (w == f) // 不需要刷新，即是还没有新元素加入
            return true;

        //  Try to set 'c' to 'f'.
        // read时如果没有数据可以读取则c的值会被置为NULL
        // 如果 (c == w) ? (old_c = c, c = f, return old_c) : (return c)
        if (c.cas(w, f) != w) // 尝试将c设置为f，即是准备更新w的位置
        {   // 进入这里表明????????????
            // 在有数据的情况下c==w是成立的，当不等时说明c==NULL表示队列之前为空
            // 因此需要提醒读消费者线程队列有数据了，可以唤醒线程取数据了。
            // 设置NULL只会在读数据的时候设置。
            // 在ypipe构造函数时初始化c==w的
            // 而只在flush函数和check_read函数中会更新c的值
            // 其中flush函数也就是这里，如果c!=w，即c==NULL
            // 这种情况下更新c不需要原子操作，因为消费者线程正处于睡眠状态。

            //  Compare-and-swap was unseccessful because 'c' is NULL.
            //  This means that the reader is asleep. Therefore we don't
            //  care about thread-safeness and update c in non-atomic
            //  manner. We'll return false to let the caller know
            //  that reader is sleeping.
            c.set(f); // 更新w的位置
            w = f;
            return false; //线程看到flush返回false之后会发送一个消息给读线程，这需要写业务去做处理
        }
        else  // 读端还有数据可读取
        {    
            //  Reader is alive. Nothing special to do now. Just move
            //  the 'first un-flushed item' pointer to 'f'.
            w = f;             // 只需要更新w的位置，因为c已经在if (c.cas(w, f) != w)条件中更新了
            return true;
        }
    }

    //  Check whether item is available for reading.
    // 这里面有两个点，一个是检查是否有数据可读，一个是预取
    inline bool check_read()
    {
        //  Was the value prefetched already? If so, return.
        // 判断前几次read时是否预取过数据且预取的数据是否读完
        // 如果还有预取的数据就直接返回true表明可以直接读数据了
        // 下面为什么要判断r!=null??????????
        if (&queue.front() != r && r) //判断是否在前几次调用read函数时已经预取数据了return true;
            return true;


        // 下面需要重新预取数据了，执行到这里必然 &queue.front() == r
        //  There's no prefetched value, so let us prefetch more values.
        //  Prefetching is to simply retrieve the
        //  pointer from c in atomic fashion. If there are no
        //  items to prefetch, set c to NULL (using compare-and-swap).
        // 两种情况
        // 1. 如果c值和queue.front()地址值相等， 返回c值并将c值置为NULL，此时没有可预取的数据了
        //    如果生成者生产数据时发现c==Null则需要提醒消费者可以消费数据了。这种情况下 (&queue.front() == r == c) -> c == NULL && r == c 依然 &queue.front() == r
        // 2. 如果c值和queue.front()地址不相等， 表示返回c值，此时预取成功了。这种情况 (&queue.front() == r != c) -> r = c, c = null 从而 &queue.front() != r
        r = c.cas(&queue.front(), NULL); //尝试预取数据

        //  If there are no elements prefetched, exit.
        //  During pipe's lifetime r should never be NULL, however,
        //  it can happen during pipe shutdown when items are being deallocated.
        if (&queue.front() == r || !r) //判断是否成功预取数据，这里判断r == null感觉没什么用，因为整个逻辑中r就一直不等于null
            return false;

        //  There was at least one value prefetched.
        return true;
    }

    //  Reads an item from the pipe. Returns false if there is no value.
    //  available.
    inline bool read(T *value_)
    {
        //  Try to prefetch a value.
        if (!check_read())  // 如果队列为空则预取失败。对于消费者线程可以暂时让出处理器或睡眠等待被唤醒
            return false;

        //  There was at least one value prefetched.
        //  Return it to the caller.
        *value_ = queue.front();
        queue.pop();
        return true;
    }

    //  Applies the function fn to the first elemenent in the pipe
    //  and returns the value returned by the fn.
    //  The pipe mustn't be empty or the function crashes.
    // 将函数 fn 应用于管道中的第一个元素，并返回 fn 返回的值。管道不能为空，否则函数会崩溃。
    inline bool probe(bool (*fn)(T &))
    {
        bool rc = check_read();
        // zmq_assert(rc);

        return (*fn)(queue.front());
    }

protected:
    //  Allocation-efficient queue to store pipe items.
    //  Front of the queue points to the first prefetched item, back of
    //  the pipe points to last un-flushed item. Front is used only by
    //  reader thread, while back is used only by writer thread.
    yqueue_t<T, N> queue;

    //  Points to the first un-flushed item. This variable is used
    //  exclusively by writer thread.
    T *w; //指向第一个未刷新的元素,只被写线程使用

    //  Points to the first un-prefetched item. This variable is used
    //  exclusively by reader thread.
    T *r; //指向第一个还没预提取的元素，只被读线程使用

    //  Points to the first item to be flushed in the future.
    T *f; //指向下一轮要被刷新的一批元素中的第一个

    //  The single point of contention between writer and reader thread.
    //  Points past the last flushed item. If it is NULL,
    //  reader is asleep. This pointer should be always accessed using
    //  atomic operations.
    atomic_ptr_t<T> c; //读写线程共享的指针，指向每一轮刷新的起点（看代码的时候会详细说）。当c为空时，表示读线程睡眠（只会在读线程中被设置为空）

    //  Disable copying of ypipe object.
    ypipe_t(const ypipe_t &);
    const ypipe_t &operator=(const ypipe_t &);
};

#endif
