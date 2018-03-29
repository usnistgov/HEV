#ifndef __IRIS_RING_BUFFER__
#define __IRIS_RING_BUFFER__

#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>

namespace iris
{

    /**
     * \brief The RingBuffer class is a fixed-size, thread-safe ringbuffer.
     */
    template <typename T, size_t N>
    class RingBuffer {
    public:
        /**
         * \brief Push a value into the buffer.
         */
        void push(T val) {
            {
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
                while (_write+1 == _read) { // read side full; can't push
                    _fullCond.wait(&_mutex);
                }

                _buffer[_write] = val;
                _write = (_write+1)%N;
            }

            _emptyCond.signal();
        }

        /**
         * \brief Pop a value from the buffer.
         */
        T pop() {
            T val;

            {
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
                while (_read == _write) { // write side empty; can't pop
                    _emptyCond.wait(&_mutex);
                }

                val = _buffer[_read];
                _read = (_read+1)%N;
            }

            _fullCond.signal();
            return val;
        }

        size_t size() const { return N; }
        bool empty() const { return _read==_write; }
        unsigned read() const { return _read; }
        unsigned write() const { return _write; }

        void clear()
        {
            {
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
                _read = _write = 0;
            }
            _emptyCond.signal();
        }

        RingBuffer() : _read(0), _write(0) {}

    private:
        OpenThreads::Mutex _mutex;
        OpenThreads::Condition _fullCond, _emptyCond;
        unsigned _read, _write;
        T _buffer[N];
    } ;

}

#endif
