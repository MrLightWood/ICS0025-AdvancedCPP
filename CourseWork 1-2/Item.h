#pragma once

#include <string>
#include <random>
#include <iostream>
#include "Date.h"
#include "Birds.h"

using namespace std;

class Item
{
private:
	char Group; // Any from range 'A'...'Z'
	int Subgroup; // Any from range 0...99
	string Name; // Any, but not empty
	Date Timestamp; // Any

public:
	Item(); // Fills the four fields above with preudo-random values
	Item(Birds &birds);
	Item(char, int, string, Date);
	Item(const Item&); // copy constructor
	~Item();
	void drawItem();
	char getGroup() {
		return Group;
	}
	int getSubgroup() {
		return Subgroup;
	}
	string getName() {
		return Name;
	}
	Date getDate() {
		return Timestamp;
	}
};