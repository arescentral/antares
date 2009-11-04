// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#ifndef ANTARES_PTHREAD_HPP_
#define ANTARES_PTHREAD_HPP_

#include <errno.h>
#include <pthread.h>

namespace antares {

class Thread {
  public:
    Thread();
    ~Thread();

    void start(void(*main)());

    template <typename T>
    void start(void(*main)(T), T arg);

    void detach();
    void join();

    static void exit();

  private:
    class BaseDispatcher;

    template <typename T>
    class Dispatcher;

    static void* thread_main(void* v);

    pthread_t _thread;
};

class Condition;

class Mutex {
  public:
    Mutex();
    ~Mutex();

    void lock();
    bool try_lock();
    void unlock();

    void await(Condition* cond);
    bool await_with_timeout(Condition* cond, int64_t abstime);

  private:
    pthread_mutex_t _mu;

    DISALLOW_COPY_AND_ASSIGN(Mutex);
};

class MutexLock {
  public:
    MutexLock(Mutex* mu);
    ~MutexLock();

  private:
    Mutex* _mu;
};

class Condition {
  public:
    Condition();
    ~Condition();

    void signal();
    void broadcast();

  private:
    friend class Mutex;
    pthread_cond_t _cond;
};

inline void pthread_check(const char* method, int error) {
    if (error != 0) {
        char what[256];
        strerror_r(error, what, 256);
        fail("%s: %s", method, what);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Thread

class Thread::BaseDispatcher {
  public:
    virtual ~BaseDispatcher() { }
    virtual void dispatch() = 0;
};

template <typename T>
class Thread::Dispatcher : public BaseDispatcher {
  public:
    Dispatcher(void(*main)(T), T t)
            : _main(main),
              _t(t) { }

    virtual void dispatch() {
        void(*main)(T) = _main;
        T t = _t;
        delete this;
        main(t);
    }

  private:
    void(* const _main)(T);
    const T _t;
};

template <>
class Thread::Dispatcher<void> : public BaseDispatcher {
  public:
    Dispatcher(void(*main)())
            : _main(main) { }

    virtual void dispatch() {
        void(*main)() = _main;
        delete this;
        main();
    }

  private:
    void(* const _main)();
};

inline Thread::Thread() { }

inline Thread::~Thread() { }

inline void Thread::start(void(*main)()) {
    Dispatcher<void>* dispatcher = new Dispatcher<void>(main);
    pthread_check("pthread_create", pthread_create(&_thread, NULL, thread_main, dispatcher));
}

template <typename T>
inline void Thread::start(void(*main)(T), T arg) {
    Dispatcher<T>* dispatcher = new Dispatcher<T>(main, arg);
    pthread_check("pthread_check", pthread_create(&_thread, NULL, thread_main, dispatcher));
}

inline void Thread::detach() {
    pthread_check("pthread_detach", pthread_detach(_thread));
}

inline void Thread::join() {
    void* v;
    pthread_check("pthread_join", pthread_join(_thread, &v));
}

inline void Thread::exit() {
    pthread_exit(NULL);
}

inline void* Thread::thread_main(void* v) {
    BaseDispatcher* dispatcher = reinterpret_cast<BaseDispatcher*>(v);
    dispatcher->dispatch();
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Mutex

inline Mutex::Mutex() {
    pthread_check("pthread_mutex_init", pthread_mutex_init(&_mu, NULL));
}

inline Mutex::~Mutex() {
    pthread_check("pthread_mutex_destroy", pthread_mutex_destroy(&_mu));
}

inline void Mutex::lock() {
    pthread_check("pthread_mutex_lock", pthread_mutex_lock(&_mu));
}

inline bool Mutex::try_lock() {
    int error = pthread_mutex_trylock(&_mu);
    if (error == EBUSY) {
        return false;
    } else {
        pthread_check("pthread_mutex_trylock", error);
        return true;
    }
}

inline void Mutex::unlock() {
    pthread_check("pthread_mutex_unlock", pthread_mutex_unlock(&_mu));
}

inline void Mutex::await(Condition* cond) {
    pthread_cond_wait(&cond->_cond, &_mu);
}

inline bool Mutex::await_with_timeout(Condition* cond, int64_t abstime) {
    timespec time;
    time.tv_sec = abstime / 1000000000ll;
    time.tv_nsec = abstime % 1000000000ll;
    int error = pthread_cond_timedwait(&cond->_cond, &_mu, &time);
    if (error == ETIMEDOUT) {
        return false;
    } else {
        pthread_check("pthread_await_with_timeout", error);
        return true;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// MutexLock

inline MutexLock::MutexLock(Mutex* mu)
        : _mu(mu) {
    _mu->lock();
}

inline MutexLock::~MutexLock() {
    _mu->unlock();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Condition

inline Condition::Condition() {
    pthread_check("pthread_cond_init", pthread_cond_init(&_cond, NULL));
}

inline Condition::~Condition() {
    pthread_check("pthread_cond_destroy", pthread_cond_destroy(&_cond));
}

inline void Condition::broadcast() {
    pthread_check("pthread_cond_broadcast", pthread_cond_broadcast(&_cond));
}

inline void Condition::signal() {
    pthread_check("pthread_cond_signal", pthread_cond_signal(&_cond));
}

}  // namespace antares

#endif  // ANTARES_PTHREAD_HPP_
