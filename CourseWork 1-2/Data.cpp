#include "Data.h"

//default_random_engine generator(static_cast<unsigned long int>(time(nullptr)));
//uniform_int_distribution<int> distributionLetters(0, 26);
//uniform_int_distribution<int> distributionNumbers(0, 99);
//uniform_int_distribution<int> distributionLetters(0, 26);

Data::Data(int n) {
	Birds myBirds;
	//list<Item*>* myItems = new list<Item*>;
	unique_ptr<list<Item*>> myItems(new list<Item*>);

	for(int i = 0; i < n; i++)
	{
		myItems->push_back(new Item(myBirds));
	}
	/*myItems->push_back(new Item('Z', 15, "Item1", Date(5,1,2021)));
	myItems->push_back(new Item('Z', 15, "Item2", Date(3, 1, 2021)));
	myItems->push_back(new Item('Z', 15, "Item3", Date(2, 1, 2021)));
	myItems->push_back(new Item('Z', 55, "Item1", Date(1, 1, 2021)));
	myItems->push_back(new Item('Z', 55, "Item2", Date(1, 1, 2021)));*/

	//for (auto& it : *myItems) 
	//{
	//	cout << it->getGroup() << endl << it->getSubgroup() << endl << it->getName() << endl << it->getDate().ToString() << endl << "######################" << endl;
	//}
	//cout << endl << endl << endl;

	for (auto& it : *myItems) {
		auto ds = DataStructure.find(it->getGroup());
		if (ds != DataStructure.end()) 
		{
			continue;
		}
		else {
			map<int, list<Item*>*> *ds = new map<int, list<Item*>*>;
			
			for(auto &it2 : *myItems)
			{
				if(it->getGroup() == it2->getGroup())
				{
					(*ds)[it2->getSubgroup()] = nullptr;
				}
			}
			DataStructure[it->getGroup()] = ds;
		}
	}

	for (auto &it : DataStructure)
	{
		for (auto& it2 : *it.second)
		{
			list<Item*>* itemsList = new list<Item*>;
			for(auto &it3 : *myItems)
			{
				if(it3->getGroup() == it.first && it3->getSubgroup() == it2.first)
				{
					itemsList->push_back(it3);
				}
			}
			it2.second = itemsList;
		}
	}
}

Data::Data() {
	
}

Data::~Data() {
	for (auto& it : DataStructure)
	{
		for (auto& it2 : *it.second)
		{
			for (auto& it3 : *it2.second)
			{
				delete it3;
			}
			delete it2.second;
		}
		delete it.second;
	}
	//cout << "Data object deleted" << endl;
}

void Data::PrintAll() {
	for (auto& it : DataStructure)
	{
		cout << it.first << ":" << endl;
		for (auto& it2 : *it.second)
		{
			for (auto& it3 : *it2.second)
			{
				cout << it2.first << ' ' << it3->getName() << ' ' << it3->getDate().ToString() << endl;
			}
		}
	}
}

int Data::CountItems() {
	int res = 0;
	for (auto& it : DataStructure)
	{
		for (auto& it2 : *it.second)
		{
			for (auto& it3 : *it2.second)
			{
				res++;
			}
		}
	}
	return res;
}

map<int, list<Item*>*>* Data::GetGroup(char c) {
	auto it = DataStructure.find(c);
	if (it != DataStructure.end())
	{
		return (*it).second;
	}
	return nullptr;
}

void Data::PrintGroup(char c) throw (invalid_argument)
{
	try{
		auto it = DataStructure.find(c);
		if (it != DataStructure.end())
		{
			cout << it->first << ":" << endl;
			auto subgroupMap = *it->second;
			for (auto it2 : subgroupMap)
			{
				for (auto it3 : *it2.second)
				{
					cout << it2.first << ": " << it3->getName() << " " << it3->getDate().ToString() << endl;
				}
			}
		} else
		{
			throw invalid_argument("Wrong arguments");
		}
	} catch(const exception &ex)
	{
		cout << ex.what() << endl;
	}
}

int Data::CountGroupItems(char c)
{
	int res = 0;
	auto it = DataStructure.find(c);
	if (it != DataStructure.end())
	{
		auto subgroupMap = *it->second;
		for (auto it2 : subgroupMap)
		{
			for (auto it3 : *it2.second)
			{
				res++;
			}
		}
		return res;
	}
	else
	{
		return 0;
	}
}

list<Item*>* Data::GetSubgroup(char c, int i)
{
	auto it = DataStructure.find(c);
	if (it != DataStructure.end())
	{
		auto subgroupMap = *it->second;
		
		auto it2 = subgroupMap.find(i);
		if (it2 != subgroupMap.end())
		{
			return it2->second;
		}
	}
	return nullptr;
}

void Data::PrintSubgroupByNames(char c, int i) throw (invalid_argument)
{
	try{
		auto it = DataStructure.find(c);
		if (it != DataStructure.end())
		{
			cout << it->first << ":" << endl;
			auto subgroupMap = *it->second;
			auto it2 = subgroupMap.find(i);
		
			if (it2 != subgroupMap.end())
			{
				//cout << it->first << ":" << endl;
				auto myList = *it2->second;

				myList.sort([](Item* const& a, Item* const& b) { return a->getName() < b->getName(); });

				for (auto it3 : myList)
				{
					cout << it2->first << ": " << it3->getName() << " " << it3->getDate().ToString() << endl;
				}
				return;
			}
		}
		throw invalid_argument("Wrong arguments");
	} catch(const exception &ex)
	{
		cout << ex.what() << endl;
	}
}

void Data::PrintSubgroupByDates(char c, int i) throw (invalid_argument)
{
	try{
		auto it = DataStructure.find(c);
		if (it != DataStructure.end())
		{
			cout << it->first << ":" << endl;
			auto subgroupMap = *it->second;
			auto it2 = subgroupMap.find(i);

			if (it2 != subgroupMap.end())
			{
				//cout << it->first << ":" << endl;
				auto myList = *it2->second;

				myList.sort([](Item* const& a, Item* const& b) { return a->getDate() < b->getDate(); });

				for (auto it3 : myList)
				{
					cout << it2->first << ": " << it3->getName() << " " << it3->getDate().ToString() << endl;
				}
				return;
			}
		}
		throw invalid_argument("Wrong arguments");
	} catch (const exception& ex)
	{
		cout << ex.what() << endl;
	}
}

int Data::CountSubgroupItems(char c, int i)
{
	int res = 0;
	auto it = DataStructure.find(c);
	if (it != DataStructure.end())
	{
		auto subgroupMap = *it->second;
		auto it2 = subgroupMap.find(i);

		if (it2 != subgroupMap.end())
		{
			auto itemsList = *it2->second;

			for (auto it3 : itemsList)
			{
				res++;
			}
			return res;
		}
	}
	return 0;
}

Item* Data::GetItem(char c, int i, string s)
{
	int res = 0;
	auto it = DataStructure.find(c);
	if (it != DataStructure.end())
	{
		auto subgroupMap = *it->second;
		auto it2 = subgroupMap.find(i);

		if (it2 != subgroupMap.end())
		{
			auto itemsList = *it2->second;
			
			auto foundItem = find_if(itemsList.begin(), itemsList.end(), [&](Item* item)->bool { return (item->getName() == s); });
			return (*foundItem);
		}
	}
	return nullptr;
}

void Data::PrintItem(char c, int i, string s) throw (invalid_argument)
{
	try{
		auto it = DataStructure.find(c);
		if (it != DataStructure.end())
		{
			auto subgroupMap = *it->second;
			auto it2 = subgroupMap.find(i);

			if (it2 != subgroupMap.end())
			{
				auto itemsList = *it2->second;

				auto foundItem = find_if(itemsList.begin(), itemsList.end(), [&](Item* item)->bool { return (item->getName() == s); });
				if(foundItem != itemsList.end())
				{
					(*foundItem)->drawItem();
					return;
				}
			}
		}
		throw exception("Wrong arguments");
	} catch (const exception &e)
	{
		cout << e.what() << endl;
	}
}

Item* Data::InsertItem(char c, int i, string s, Date d)
{
	Item* newItem = new Item(c, i, s, d);

	auto it = DataStructure.find(c);
	if (it != DataStructure.end())
	{
		auto subgroupMap = it->second;
		auto it2 = (*subgroupMap).find(i);

		if (it2 != (*subgroupMap).end())
		{
			for(auto item : *it2->second)
			{
				if (item->getGroup() == c && item->getSubgroup() == i && item->getName() == s && item->getDate() == d)
				{ 
					cout << "Item already exists" << endl;
					return nullptr;
				}
			}
			it2->second->push_back(newItem);
		} else 
		{
			list<Item*>* newList = new list<Item*>;
			newList->push_back(newItem);
			(*subgroupMap)[i] = newList;
		}
		return newItem;
	}
	else {
		list<Item*>* newList = new list<Item*>;
		newList->push_back(newItem);
		map<int, list<Item*>*>* newSubgroupMap = new map<int, list<Item*>*>;
		(*newSubgroupMap)[i] = newList;
		DataStructure[c] = newSubgroupMap;
		return newItem;
	}
	return nullptr;
}

list<Item*>* Data::InsertSubgroup(char c, int i, initializer_list<Item*> items)
{
	auto it = DataStructure.find(c);
	if (it != DataStructure.end())
	{
		auto subgroupMap = it->second;
		auto it2 = (*subgroupMap).find(i);

		if (it2 != (*subgroupMap).end())
		{
			cout << "This subgroup already exists" << endl;
			return nullptr;
		}
		else
		{
			list<Item*>* newList = new list<Item*>;
			for (auto item : items)
			{
				newList->push_back(item);
			}
			(*subgroupMap)[i] = newList;
			return newList;
		}
	}
	else {
		list<Item*>* newList = new list<Item*>;
		for (auto item : items)
		{
			newList->push_back(item);
		}
		map<int, list<Item*>*>* newSubgroupMap = new map<int, list<Item*>*>;
		(*newSubgroupMap)[i] = newList;
		DataStructure[c] = newSubgroupMap;
		return newList;
	}

	return nullptr;
}

/*
list<Item*>* Data::InsertSubgroup(char c, int i, initializer_list<Item*> items)
{
	auto it = DataStructure.find(c);
	if (it != DataStructure.end())
	{
		auto subgroupMap = it->second;
		auto it2 = (*subgroupMap).find(i);

		if (it2 != (*subgroupMap).end())
		{
			auto dsList = it2->second;
			for (auto item : items)
			{
				if (find(dsList->begin(), dsList->end(), item) == dsList->end()) 
				{
					dsList->push_back(item);
				}
			}
			return dsList;
		} else 
		{
			list<Item*>* newList = new list<Item*>;
			for(auto item : items)
			{ 
				newList->push_back(item);
			}
			(*subgroupMap)[i] = newList;
			return newList;
		}
	}
	else {
		list<Item*>* newList = new list<Item*>;
		for (auto item : items)
		{
			newList->push_back(item);
		}
		map<int, list<Item*>*>* newSubgroupMap = new map<int, list<Item*>*>;
		(*newSubgroupMap)[i] = newList;
		DataStructure[c] = newSubgroupMap;
		return newList;
	}

	return nullptr;
}
*/

map<int, list<Item*>*>* Data::InsertGroup(char c, initializer_list<int> subgroups, initializer_list<initializer_list<Item*>> items)
{
	auto it = DataStructure.find(c);
	if (it != DataStructure.end())
	{
		cout << "This group already exists" << endl;
		return nullptr;
	}
	else
	{
		map<int, list<Item*>*>* newSubgroupMap = new map<int, list<Item*>*>;
		for (auto ilList : items)
		{
			list<Item*>* newList = new list<Item*>;
			for (auto item : ilList)
			{
				newList->push_back(item);
			}
			(*newSubgroupMap)[(*newList->begin())->getSubgroup()] = newList;
		}
		DataStructure[c] = newSubgroupMap;
		return newSubgroupMap;
	}

	return nullptr;
}

/*
map<int, list<Item*>*>* Data::InsertGroup(char c, initializer_list<int> subgroups, initializer_list<initializer_list<Item*>> items)
{
	auto it = DataStructure.find(c);
	if (it != DataStructure.end())
	{
		auto subgroupMap = it->second;
		for(auto subgroupList : subgroups){
			auto it2 = (*subgroupMap).find(subgroupList);

			auto itemsList = find_if(items.begin(), items.end(), [&](list<Item*> const &dsListItem)->bool { return (*dsListItem.begin())->getSubgroup() == subgroupList; });
			if (it2 != (*subgroupMap).end())
			{
				auto dsList = it2->second;
				
				for (auto item : *itemsList)
					{
						if (find(dsList->begin(), dsList->end(), item) == dsList->end())
						{
							dsList->push_back(item);
						}
					}
				return subgroupMap;
			}
			else
			{
				list<Item*>* newList = new list<Item*>;
				for (auto item : *itemsList)
				{
					newList->push_back(item);
				}
				(*subgroupMap)[subgroupList] = newList;
				return subgroupMap;
			}
		}

	} else 
	{
		map<int, list<Item*>*>* newSubgroupMap = new map<int, list<Item*>*>;
		for (auto ilList : items)
		{
			list<Item*>* newList = new list<Item*>;
			for(auto item : ilList)
			{
				newList->push_back(item);
			}
			(*newSubgroupMap)[(*newList->begin())->getSubgroup()] = newList;
		}
		DataStructure[c] = newSubgroupMap;
		return newSubgroupMap;
	}

	return nullptr;
}
*/

bool Data::RemoveItem(char c, int i, string s)
{
	auto it = DataStructure.find(c);
	if (it == DataStructure.end())
		return false;

	auto subgroupMap = it->second;
	auto it2 = subgroupMap->find(i);
	if (it2 == subgroupMap->end())
		return false;

	auto dsList = it2->second;

	auto foundItem = find_if(dsList->begin(), dsList->end(), [&](Item* item)->bool {return (item->getName()).c_str() == s; });
	if (foundItem == dsList->end())
		return false;

	delete *foundItem;
	dsList->erase(foundItem);
	if (dsList->empty())
	{ 
		delete dsList;
		subgroupMap->erase(i);
	}
	if (subgroupMap->empty())
	{ 
		delete subgroupMap;
		DataStructure.erase(it);
	}
}

bool Data::RemoveSubgroup(char c, int i)
{
	auto it = DataStructure.find(c);
	if (it == DataStructure.end())
		return false;

	auto subgroupMap = it->second;
	auto it2 = subgroupMap->find(i);
	if (it2 == subgroupMap->end())
		return false;

	auto dsList = it2->second;

	for(auto it3 : *dsList)
	{
		delete it3;
	}
	delete dsList;
	subgroupMap->erase(i);
	if (subgroupMap->empty())
	{
		delete subgroupMap;
		DataStructure.erase(it);
	}
	return true;
}

bool Data::RemoveGroup(char c)
{
	auto it = DataStructure.find(c);
	if (it == DataStructure.end())
		return false;

	auto subgroupMap = it->second;
	
	for(auto it2 : *subgroupMap)
	{
		auto dsList = it2.second;
		for(auto item : *dsList)
		{
			delete item;
		}
		delete dsList;
	}
	
	delete subgroupMap;
	DataStructure.erase(it);

	return true;
}