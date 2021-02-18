#include "Item.h"


char abc[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

default_random_engine generator(static_cast<unsigned long int>(time(nullptr)));
uniform_int_distribution<int> distributionLetters(0, 25);
uniform_int_distribution<int> distributionNumbers(0, 99);

Item::Item() {
	Birds* myBirds = new Birds;
	//default_random_engine generator(static_cast<unsigned long int>(time(nullptr)));
	//uniform_int_distribution<int> distributionLetters(0, 26);
	//uniform_int_distribution<int> distributionNumbers(0, 99);

	Group = abc[distributionLetters(generator)];
	Subgroup = distributionNumbers(generator);
	Name = myBirds->getRandomBird();
	Timestamp = Timestamp.CreateRandomDate(Date(1,1,2000), Date(31, 12, 2021));
	if (myBirds)
		delete myBirds;
}

Item::Item(Birds &birds)
{
	Group = abc[distributionLetters(generator)];
	Subgroup = distributionNumbers(generator);
	Name = birds.getRandomBird();
	Timestamp = Timestamp.CreateRandomDate(Date(1, 1, 2000), Date(31, 12, 2021));
}

Item::Item(char group, int subGroup, string bird, Date time) {
	Group = group;
	Subgroup = subGroup;
	Name = bird;
	Timestamp = time;
}

Item::Item(const Item &Original) { // copy constructor
	Group = Original.Group;
	Subgroup = Original.Subgroup;
	Name = Original.Name;
	Timestamp = Original.Timestamp;
}

Item::~Item() {
	//cout << "Item object deleted" << endl;
}

void Item::drawItem() {
	cout << Group << ':' << endl << Subgroup << ' ' << Name << ' ' << Timestamp.ToString() << endl;
}
