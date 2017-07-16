#include <functional>
#include <iostream>

#include "ak/math/Vector.hpp"
#include "ak/thread/RecursiveSpinlock.hpp"
#include "ak/thread/Thread.hpp"


static volatile int counter = 0;
static ak::thread::Spinlock lock;

static void threadTask() {
	for (int i = 0; i < 2000; i++) {
		ak::thread::current().yield();
		auto guard = lock.lock();
		counter++;
		std::cout << ak::thread::current().name() << " " << ak::thread::current().id() << ": " << counter << std::endl;
	}
}

int main() {
	ak::math::Vec2 fVec = ak::math::Vec2{1.0, 2.0};
	ak::math::Vec2 fVec2 = fVec;

	constexpr int THREAD_COUNT = 4;
	ak::thread::Thread thread[THREAD_COUNT];
	for (uint i = 0; i < THREAD_COUNT; i++) thread[i].execute(threadTask);
	for (uint i = 0; i < THREAD_COUNT; i++) thread[i].waitFor();
}
