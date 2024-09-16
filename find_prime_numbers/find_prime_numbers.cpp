#include <iostream>
#include <vector>

int main()
{
	const int nums = 100000;
	const int size_check = 100;

	std::vector<bool> primes;
	primes.reserve(nums);


	for (int i = 1; i <= nums; i += size_check) {

		std::vector<bool> table(100, true);

		if (i == 1) {
			table[0] = false;
			primes.push_back(i + 1);
		}
	}
}