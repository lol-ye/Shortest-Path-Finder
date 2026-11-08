#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;


struct namedPlace
{   
    int numericCode;
    string state;
    string name;
    int population;
    double area;
    double latitude;
    double longitude;
    int code;
    double distance;
};

struct node
{
    namedPlace data;
    node* next;
};

class HashTable
{
    private:
        int hashGroups;
        node** table;
        int itemCount;

        void rehash();
    
    public:
        HashTable();
        ~HashTable();
        int hashFunction(const string& key);
        void insertItem(namedPlace place);
        void displayTable();
        vector<string> listStates(const string& placeName);
        namedPlace findItem(const string& placeName, const string& state);
        void buildTable();
};

string stateAbbreviationConvert(string abbrev);

#endif