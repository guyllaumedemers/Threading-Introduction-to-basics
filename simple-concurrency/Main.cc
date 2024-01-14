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
#include <mutex>
#include <stdio.h>
#include <thread>
#include <type_traits>
#include <utility>

#define SLEEP_THREAD_FOR_X_SECOND(Time)  std::this_thread::sleep_for(std::chrono::seconds(Time));

// simple typedef to make this more readable
typedef std::chrono::duration<float, std::milli> FDuration;
typedef std::chrono::steady_clock::time_point FTime;
typedef std::chrono::high_resolution_clock FClock;

typedef std::thread FThread;

inline constexpr static float milli_per_sec = 1000.f;

struct FThreadMaker {

	template<typename TFunctor, typename ... TArgs>
	static FThread StartThread(TFunctor&& function_object, TArgs&& ... args) {

		return std::move(FThread(function_object, std::forward<TArgs>(args)...));
	}
};

struct FCup {

	FTime last;

	float fill_percent = 100.f;
	inline constexpr static float drinking_rate_per_second = 8.f;

	char const* cup_owner = "";

	FCup(char const* owner) : cup_owner(owner) {

		last = FClock::now();
	};

	void Tick(struct FCoffeeMachine& coffee_machine);

	void Print() const
	{
		std::printf("%s: %f%\n", cup_owner, fill_percent);
	}

	bool IsEmpty() const
	{
		return fill_percent <= 0.f;
	}
};

struct FCoffeeMachine {

	// prevent concurrent access to already acquired resources and prevent runtime
	// data race.
	typedef std::mutex FMutex;
	FMutex lock;

	typedef std::lock_guard<FMutex> FLock;

	float fill_percent = 500.f;
	inline constexpr static float cup_filling_rate_per_second = 12.f;

	char const* last_person_who_emptied_machine = "";

	void RefillCup(FCup& coffee_cup);

	void Print(FCup& coffee_cup) const
	{
		std::printf("%s: refilling coffee...\n", coffee_cup.cup_owner);
	}

	bool IsEmpty() const
	{
		return fill_percent <= 0.f;
	}
};

void FCup::Tick(FCoffeeMachine& coffee_machine)
{

	FTime const now = FClock::now();
	FDuration const delta_time = (now - last);
	last = now;

	if (!IsEmpty()) {

		// drinking coffee
		fill_percent -= (drinking_rate_per_second * (delta_time.count() / milli_per_sec));

		// clamp value
		if (fill_percent < 0.f) {

			fill_percent = 0.f;
		}

		Print();

		// we dont want to have this running on each tick so we sleep our current thread to have readable log outputs
		SLEEP_THREAD_FOR_X_SECOND(1);
	}
	else {

		coffee_machine.RefillCup(*this);

		// update timestamp as refill operation took some time to execute and delta_time would be too great between now and last capture.
		last = FClock::now();
	}
}

void FCoffeeMachine::RefillCup(FCup& coffee_cup)
{

	FLock lock_guard(lock);

	FTime last = FClock::now();
	while (!IsEmpty() && coffee_cup.fill_percent < 100.f) {

		FTime const now = FClock::now();
		FDuration const delta_time = (now - last);
		last = now;

		float const delta_coffee_consumed = (cup_filling_rate_per_second * (delta_time.count() / milli_per_sec));
		coffee_cup.fill_percent += delta_coffee_consumed;
		fill_percent -= delta_coffee_consumed;

		// clamp value
		if (coffee_cup.fill_percent > 100.f) {

			coffee_cup.fill_percent = 100.f;
		}

		// clamp value
		if (fill_percent < 0.f) {

			fill_percent = 0.f;
		}

		Print(coffee_cup);

		// we dont want to have this running on each tick so we sleep our current thread to have readable log outputs
		SLEEP_THREAD_FOR_X_SECOND(1);
	}

	if (IsEmpty()) {

		last_person_who_emptied_machine = coffee_cup.cup_owner;
	}
}

int main(int argc, char* argv[]) {

	// Simple example - 3x Coworkers start the day with their coffee cup full. The office coffee machine also start full.
	// 
	//					During the day, they each empty their cup and go to the machine for a refill. Only one person can refill
	//					at a time. Each person, drinking at about the same pace.
	//
	// Goal			  - Which of the three coworker will have to refill the machine?

	FCup CupA = { "CoworkerA" };
	FCup CupB = { "CoworkerB" };
	FCup CupC = { "CoworkerC" };

	FCoffeeMachine OfficeMachine;

	// create anonymous function to be performed on each thread
	auto const lambda = [](std::reference_wrapper<FCup> coffee_cup, std::reference_wrapper<FCoffeeMachine> coffee_machine)
		{

			FCoffeeMachine& Machine = coffee_machine.get();
			while (!Machine.IsEmpty()) {

				coffee_cup.get().Tick(Machine);
			}
		};

	// start thread for each coworkers
	FThread ThreadA = FThreadMaker::StartThread(lambda, std::ref(CupA), std::ref(OfficeMachine));
	FThread ThreadB = FThreadMaker::StartThread(lambda, std::ref(CupB), std::ref(OfficeMachine));
	FThread ThreadC = FThreadMaker::StartThread(lambda, std::ref(CupC), std::ref(OfficeMachine));

	// detach so they run in the background (initial approach - until I realize there's a data race in the statement at line 224)
	// ThreadA.detach();
	// ThreadB.detach();
	// ThreadC.detach();

	// watchout possible data race with the execution here as the thing blocking the main thread is the IsEmpty statement
	// Main thread unblock as soon as this becomes 'true' which means the statement below for : who's last coffee machine user might not have been set yet.
	while (!OfficeMachine.IsEmpty()) {

		// our main thread just wait for other threads to empty the office coffee machine
		// and will close the application once empty.
		std::this_thread::yield();
	}

	// alternative solution to data race at statement line 224.
	ThreadA.join();
	ThreadB.join();
	ThreadC.join();

	std::printf("last person to empty the machine: %s\n", OfficeMachine.last_person_who_emptied_machine);

	return 0;
}