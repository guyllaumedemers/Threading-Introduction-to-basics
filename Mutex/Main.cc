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
#include <stdio>
#include <thread>

int main(int argc, char* argv[]) {

	// Simple example - 3x Coworkers start the day with their own cup of coffee full. The office coffee machine start full.
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

		const char* CupOwner = "";
		uint32_t number_cup_consumed = 0;

		FCup(const char* Owner) : CupOwner(Owner) {

			last = FClock::now();
		};

		bool IsEmpty() const { return fill_percent <= 0.f; }

		void Tick()
		{
			// we need to be able to check for the state of the coffee machine as the thread would need to stop 

			// Core loop

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
			puts(CupOwner + ": " + fill_percent);
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

	while (!OfficeMachine.IsEmpty()) {

		// wdwdh ?
	}

	return 0;
}