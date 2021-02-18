#pragma once

#include <random>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

class Birds
{
private:
	default_random_engine generator;
	std::vector<string> BirdsNames;
public:
	Birds(){
		fstream File;
		File.open("..\\ICS0025 Instructors Stuff\\Birds.txt", fstream::in | fstream::binary | fstream::app);
		string buf;

		if (File.good())
		{
			while (getline(File, buf))
			{
				BirdsNames.push_back(buf);
			}
		}
		else
		{
			throw exception("Could not open file");
		}
	}

	void drawBirds() {
		for (auto it = BirdsNames.begin(); it != BirdsNames.end(); it++) {
			cout << *it << endl;
		}
	}

	~Birds() {
		//cout << "Bird object deleted" << endl;
	}

	string getRandomBird() {
		uniform_int_distribution<int> distributionBirds(0, BirdsNames.size());
		string result = BirdsNames.at(distributionBirds(generator));
		result.erase(result.size() - 1);
		return result;
	}
};