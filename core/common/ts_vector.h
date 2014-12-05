/*
 * Simple vector (Thread safe implementation)
 *
 *                               Impl. by Eru
 */

#ifndef _CORELIB_COMMON_TS_VECTOR_H_
#define _CORELIB_COMMON_TS_VECTOR_H_


// Interface
template<class T>
class ITSVector {
protected:
    size_t capacity;
    T **ary;

public:
    size_t count;

    ITSVector() : count(0), capacity(32) {
        ary = new T *[capacity];
    }

    ~ITSVector() {
        for (size_t i = 0; i < count; ++i)
            delete ary[i];
        delete[] ary;
    }

    virtual bool empty() {
        return count == 0;
    }

    virtual void lock() = 0;

    virtual void unlock() = 0;

    virtual bool erase(size_t idx) {
        delete ary[idx];
//		for (size_t i=idx+1; i<count; ++i)
//			ary[i-1] = ary[i];
        memmove(&ary[idx], &ary[idx + 1], sizeof(T *) * (--count - idx));

        return true;
    }

    virtual bool clear() {
        for (size_t i = 0; i < count; ++i) {
            delete ary[i];
        }
        count = 0;

        return true;
    }

    virtual bool find(const T &tgt) {
        for (size_t i = 0; i < count; ++i) {
            if (*ary[i] == tgt)
                return true;
        }

        return false;
    }

    virtual bool find(const T &tgt, size_t *idx) {
        for (size_t i = 0; i < count; ++i) {
            if (*ary[i] == tgt) {
                *idx = i;
                return true;
            }
        }

        return false;
    }

    virtual bool push_back(const T &val) {
        T *ptr = new T(val);

        if (count + 1 > capacity) {
            T **ptr = new T *[capacity * 2];
            if (!ptr)
                return false;

            for (size_t i = 0; i < capacity; ++i)
                ptr[i] = ary[i];
            delete[] ary;

            ary = ptr;
            capacity <<= 1;
        }

        ary[count++] = ptr;

        return true;
    }

    virtual T &at(size_t idx) {
        if (idx >= count)
            throw std::out_of_range("out of bounds");

        return *ary[idx];
    }

    virtual T &operator[](size_t idx) {
        return at(idx);
    }
};


#endif
