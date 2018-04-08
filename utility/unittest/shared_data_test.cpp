#include "utility/shared_data.h"
#include <future>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <assert.h>

using namespace beam;

class Barrier {
private:
    std::mutex _mutex;
    std::condition_variable _cv;
    size_t _count;
public:
    explicit Barrier(size_t count) : _count(count) { }

    void wait() {
        std::unique_lock<std::mutex> lock{_mutex};
        if (--_count == 0) {
            _cv.notify_all();
        } else {
            _cv.wait(lock, [this] { return _count == 0; });
        }
    }
};

struct SomeStatus {
    size_t x=0;
    size_t y=0;
    size_t z=0;

    bool consistent() const {
        return z == x*y;
    }
};

using SharedStatus = SharedData<SomeStatus>;

int main() {
    using namespace std;

    SharedStatus status;

    vector<future<void>> futures;

    size_t nIterations = 100500;
    size_t nReaders = 100;
    // + 2 writers: the 1st modifies x, the second modifies y
    Barrier barrier(nReaders + 2);

    // 1st writer
    futures.push_back(std::async(
        std::launch::async,
        [&barrier,&status,nIterations]() {
            barrier.wait();
            for (size_t i=1; i<=nIterations; ++i) {
                auto data = status.write();
                assert(data);
                data->x = i;
                data->z = i * data->y;
            }
        }
    ));

    // 2nd writer
    futures.push_back(std::async(
        std::launch::async,
        [&barrier,&status,nIterations]() {
            barrier.wait();
            for (size_t i=1; i<=nIterations; ++i) {
                auto data = status.write();
                assert(data);
                data->y = i;
                data->z = i * data->x;
            }
        }
    ));

    for (size_t i=0; i<nReaders; ++i) {
        futures.push_back(std::async(
            std::launch::async,
            [&barrier,&status,nIterations]() {
                barrier.wait();
                for (size_t i=1; i<=nIterations; ++i) {
                    const auto data = status.read();
                    assert(data);
                    assert(data->consistent());
                }
            }
        ));
    }

    // wait all
    for (auto& f : futures) {
        f.get();
    }
}

