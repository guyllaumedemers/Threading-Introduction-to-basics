//Copyright(c) 2024 gdemers
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files(the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions :
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include <chrono>
#include <functional>
#include <mutex>
#include <stdio>
#include <thread>
#include <utility>

template<typename T>
using TRef = typename std::ref<T>;

typedef std::thread FThread;

struct FThreadMaker {

	template<typename TFunctor, typename ... TArgs>
	static FThread&& StartThread(TFunctor&& function_object, TArgs&& args...) {

		return FThread(function_object, std::forward<TArgs>(args)...);
	}
};

int main(int argc, char* argv[]) {

	// Simple example - 3x Coworkers start the day with their coffee cup full. The office coffee machine also start full.
	// 
	//					During the day, they each empty their cup and go to the machine for a refill. Only one person can refill
	//					at a time. Each person, drinking at about the same pace.
	//
	// Goal			  - Which of the three coworker will have to refill the machine?

	struct FCup {

		// simple typedef to make this more readable
		typedef std::chrono::duration<double, std::milli> FDuration;
		typedef std::chrono::steady_clock::time_point FTime;
		typedef std::chrono::high_resolution_clock FClock;
		FTime last;

		float fill_percent = 100.f;
		static const float drinking_rate = 4.f;

		const char* cup_owner = "";
		uint32_t number_cup_consumed = 0;

		FCup(const char* owner) : cup_owner(owner) {

			last = FClock::now();
		};

		bool IsEmpty() const { return fill_percent <= 0.f; }

		void Tick()
		{

			const FTime now = FClock::now();
			const FDuration delta_time = (now - last);
			last = now;

			if (!IsEmpty()) {

				fill_percent -= (drinking_rate * delta_time.count());
				Print();
			}
			else {

				// be careful to not increment the cup counter multiple times as mutex might not be initially available
				// and multiple call to this function body gets executed

				// request mutex ownership to fill
			}
		}

		void Print() const
		{
			puts(cup_owner + ": " + fill_percent);
		}
	};

	struct FCoffeeMachine {

		// prevent concurrent access to already acquired resources and prevent runtime
		// data race.
		typedef std::mutex FMutex;
		FMutex lock;

		float fill_percent = 100.f;
		static const float cup_filling_rate = 8.f;

		bool IsEmpty() const { return fill_percent <= 0.f; }
	};

	FCup CupA = { "CoworkerA" };
	FCup CupB = { "CoworkerB" };
	FCup CupC = { "CoworkerC" };

	FCoffeeMachine OfficeMachine;

	// create anonymous function to be performed on each thread
	const auto lambda = [](TRef<FCup> CoffeeCup, TRef<FCoffeeMachine> CoffeeMachine) {

		while (!CoffeeMachine.IsEmpty()) {

			CoffeeCup.Tick();
		}
		};

	// start thread for each coworkers
	FThread ThreadA = FThreadMaker::StartThread(lambda, CupA, OfficeMachine);
	FThread ThreadB = FThreadMaker::StartThread(lambda, CupB, OfficeMachine);
	FThread ThreadC = FThreadMaker::StartThread(lambda, CupC, OfficeMachine);

	while (!OfficeMachine.IsEmpty()) {

		// to be define - not sure what I want to do here
		std::this_thread::yield();
	}

	return 0;
}