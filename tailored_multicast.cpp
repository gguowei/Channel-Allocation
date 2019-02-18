// tailored_multicast.cpp : Proof of concept of the core subgrouping and channel allocation algorithm.
//
// A Visual Studio C++ Console Application programme.
// Used together with file: input.txt
// 
// Wei Guo <wei6.guo@samsung.com>
// 16 Nov 2018	SRUK
//
// 
// Input: 
//	1. Subgroup number to be allocated into;
//	2. the number of UEs for each reported CQI
// Output:
//	1. The lowest CQI correspoinding which the MCS/TBS will be used
//	2. The expected thoughput, maximised
//	3. UE coverage: how many UEs' service are guaranteed
//
// UE coverage: At least 95% of UE should be guaranteed the service. 
// Examplified weight function: f(cqi) = cqi
//


#include "stdafx.h"

#include <iostream>
#include <math.h> 
#include <iomanip>

using namespace std;

int 
	a[16],
	ch,
	totalue = 0,
	thresh = 0,
	val[32768],
	abue[32768],	// How many UEs are abandoned for this allocation.
	optres[16][16];	// [All channels][Selected channels]. Final value for optimal slection.

unsigned short		// 2 octets
	opt[16][16],		// [All channels][Selected channels]. Storing the bitmap of the optimal channels.
	bcsofar[16][16];	// [Highest CQI used][Selected channels]. Storing the bitmap of the optimal channel for each best CQI used.

const float SAT = float(0.95);

void cal()
{
	opt[1][1] = 1;
	opt[1][0] = 0;
	optres[1][1] = a[1];
	optres[1][0] = 0;
	val[1] = a[1];		// Interim data structure
	val[0] = 0;			// Interim data structure 
	abue[1] = 0;		// Interim data structure
	abue[0] = a[1];		// Interim data structure
	bcsofar[1][0] = 1;	// Interim data structure
	bcsofar[0][0] = 0;	// Interim data structure
	bcsofar[1][1] = 1;	// Interim data structure
	bcsofar[0][1] = 0;	// Interim data structure

	unsigned short p = 2;
	for (int pos = 2; pos <= 15; pos++)
	{
		for (int gr = 1; gr <= pos && gr <= ch; ++gr)
		{
			// Choose gr-1 from pos-1, plus the 1 at pos
			int gain1 = optres[pos - 1][gr - 1] + a[pos] * pos;
			int plus1 = gain1;
			if (gr == 1)
			{
				plus1 = a[pos] * pos;
				bcsofar[pos][gr] = p;
				val[bcsofar[pos][gr]] = plus1;
				abue[bcsofar[pos][gr]] = abue[0];
			}
			else
			{
				bcsofar[pos][gr] = p + opt[pos - 1][gr - 1];
				abue[bcsofar[pos][gr]] = abue[opt[pos - 1][gr - 1]];
				val[bcsofar[pos][gr]] = gain1;
			}
			if (abue[bcsofar[pos][gr]] > thresh)
			{
				plus1 = 0;
			}

			// Choose gr from pos-1, plus the 0 at pos
			int plus0 = 0;
			unsigned short best;
			for (int j = gr; j < pos; ++j)
			{
				if (abue[bcsofar[j][gr]] <= thresh)
				{
					int gain0 = val[bcsofar[j][gr]] + a[pos] * j;
					val[bcsofar[j][gr]] = gain0;
					if (gain0 > plus0)
					{
						plus0 = gain0;
						best = bcsofar[j][gr];
					}
				}
			}
						
			if (plus1 >= plus0)
			{
				opt[pos][gr] = bcsofar[pos][gr];
				optres[pos][gr] = plus1;
			}
			else
			{
				opt[pos][gr] = best;
				optres[pos][gr] = plus0;
			}
		}
		abue[0] += a[pos];	// Try the maximum abandoned UEs.
		p <<= 1;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	freopen("input.txt", "r", stdin);

	cin >> ch;
	for (int i = 1; i <= 15; ++i)
	{
		cin >> a[i];
		totalue += a[i];
	}
	thresh = int((1 - SAT) * totalue); // Cannot abandon more than that.

	cout << "UE group:\tNo. of UEs" << endl;
	for (int i = 1; i <= 15; ++i)
		cout << "CQI " << i << "\t:\t" << a[i] << endl;
	cout << "Total UEs: " << totalue << endl << "Guaranteed UEs: " << fixed << setprecision(0) << ceil(totalue * SAT) << " (" << SAT * 100 << "%)" << endl;
	cout << "Allocate " << ch << " multicast channels in SC-PTM mode." << endl << endl;
	
	cal();

	cout << "Optimal channel assignment (CQI# of the channels): ";
	unsigned short p = 1;
	bool abandoned = true;
	int abandonedue = 0;
	for (int i = 1; i <= 15; ++i)
	{
		if (opt[15][ch] & p)
		{
			cout << i << " ";
			abandoned = false;
		}
		else if (abandoned)
			abandonedue += a[i];

		p <<= 1;
	}
	cout << endl << "Max throughput is " << optres[15][ch] << endl;
	int served = totalue - abandonedue;
	cout << "Coverage is " << served << "/" << totalue << " (" << float(served) / totalue * 100 << "%)" << endl << endl;
	return 0;
}

