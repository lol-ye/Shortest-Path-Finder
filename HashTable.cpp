#include "HashTable.h"

using namespace std;

string stateAbbreviationConvert(string abbrev)
{
    pair<string, string> states[] = {
        {"AL", "Alabama"}, {"AK", "Alaska"}, {"AZ", "Arizona"}, {"AR", "Arkansas"},
        {"CA", "California"}, {"CO", "Colorado"}, {"CT", "Connecticut"}, {"DE", "Delaware"},
        {"FL", "Florida"}, {"GA", "Georgia"}, {"HI", "Hawaii"}, {"ID", "Idaho"},
        {"IL", "Illinois"}, {"IN", "Indiana"}, {"IA", "Iowa"}, {"KS", "Kansas"},
        {"KY", "Kentucky"}, {"LA", "Louisiana"}, {"ME", "Maine"}, {"MD", "Maryland"},
        {"MA", "Massachusetts"}, {"MI", "Michigan"}, {"MN", "Minnesota"}, {"MS", "Mississippi"},
        {"MO", "Missouri"}, {"MT", "Montana"}, {"NE", "Nebraska"}, {"NV", "Nevada"},
        {"NH", "New Hampshire"}, {"NJ", "New Jersey"}, {"NM", "New Mexico"}, {"NY", "New York"},
        {"NC", "North Carolina"}, {"ND", "North Dakota"}, {"OH", "Ohio"}, {"OK", "Oklahoma"},
        {"OR", "Oregon"}, {"PA", "Pennsylvania"}, {"RI", "Rhode Island"}, {"SC", "South Carolina"},
        {"SD", "South Dakota"}, {"TN", "Tennessee"}, {"TX", "Texas"}, {"UT", "Utah"},
        {"VT", "Vermont"}, {"VA", "Virginia"}, {"WA", "Washington"}, {"WV", "West Virginia"},
        {"WI", "Wisconsin"}, {"WY", "Wyoming"}
    };

    for (pair<string,string> state : states)
    {
        if (state.first == abbrev)
        {
            return state.second;
        }
    }
    return "[ERROR]";
}

//Constructor
HashTable::HashTable()
{
    hashGroups = 53;
    table = new node*[hashGroups];
    for (int i = 0; i < hashGroups; i++)
    {
        table[i] = nullptr;
    }
    itemCount = 0;
}

//Destructor
HashTable::~HashTable()
{
    for (int i = 0; i < hashGroups; i++)
    {
        node* cell = table[i];
        while (cell)
        {
            node* temp = cell;
            cell = cell->next;
            delete temp;
        }
    }
    delete[] table;
}

//Find next prime number greater than n
int nextPrime(int n)
{
    n = (n % 2 == 0) ? n + 1 : n + 2;
    while (true)
    {
        bool isPrime = true;
        for (int i = 3; i * i <= n; i += 2)
        {
            if (n % i == 0)
            {
                isPrime = false;
                break;
            }
        }
        if (isPrime) return n;
        n += 2;
    }
}

//Rehash the table when load factor exceeds threshold
void HashTable::rehash()
{
    int oldHashGroups = hashGroups;
    hashGroups = nextPrime(2 * hashGroups);
    node** oldTable = table;
    table = new node*[hashGroups];
    for (int i = 0; i < hashGroups; i++)
    {
        table[i] = nullptr;
    }
    itemCount = 0;

    for (int i = 0; i < oldHashGroups; i++)
    {
        node* cell = oldTable[i];
        while (cell)
        {
            insertItem(cell->data);
            cell = cell->next;
        }
    }

    for (int i = 0; i < oldHashGroups; i++)
    {
        node* cell = oldTable[i];
        while (cell)
        {
            node* temp = cell;
            cell = cell->next;
            delete temp;
        }
    }
    delete[] oldTable;
}

//Takes place's name as key
int HashTable::hashFunction(const string& key)
{
    unsigned int hash = 0;
    const unsigned int prime = 31;
    for (char ch : key)
    {
        hash = hash * prime + ch;
    }
    return hash % hashGroups;
}

void HashTable::insertItem(namedPlace place)
{
    double loadFactor = (double)itemCount / hashGroups;
    if (loadFactor > 0.75) rehash();

    int hashValue = hashFunction(place.name);
    node* newNode = new node{place, nullptr};

    newNode->next = table[hashValue];
    table[hashValue] = newNode;
    itemCount++; // Increment item count on successful
}

void HashTable::displayTable()
{
    for (int i = 0; i < hashGroups; i++)
    {
        node* cell = table[i];
        if (!cell)
        {
            cout << "Cell " << i << " is empty." << endl;
        }
        else
        {
            cout << "Cell " << i << " contains: ";
            while (cell)
            {
                cout << cell->data.name << " -> ";
                cell = cell->next;
            }
            cout << "nullptr" << endl;
        }
    }
}

vector<string> HashTable::listStates(const string& placeName)
{
    vector<string> states;
    bool placeFound = false;
    int hashValue = hashFunction(placeName);
    node* cell = table[hashValue];
    while(cell)
    {
        if(cell->data.name == placeName)
        {
            placeFound = true;
            states.push_back(cell->data.state);
        }
        cell = cell->next;
    }
    return states;
}

namedPlace HashTable::findItem(const string& placeName, const string& state)
{
    bool placeFound = false;
    int hashValue = hashFunction(placeName);
    node* cell = table[hashValue];
    while(cell)
    {
        if(cell->data.state == state && cell->data.name == placeName)
        {
            return cell->data;
        }
        cell = cell->next;
    }

    if(!placeFound)
    {
        cout << "Place not found." << endl;
    }

    return namedPlace{};
}

void HashTable::buildTable()
{
    ifstream fin("Shortest-Path-Finder/named-places.txt");
    if (!fin)
    {
        cerr << "Error opening file." << endl;
        return;
    }
    string line;
    while(getline(fin, line))
    {
        // Parse fixed-width format
        int numericCode = stoi(line.substr(0, 8));
        string state = line.substr(8, 2);
        string name = line.substr(10, 49);
        int population = stoi(line.substr(59, 10));
        double area = stod(line.substr(69, 11));
        double latitude = stod(line.substr(80, 10));
        double longitude = stod(line.substr(90, 11));
        int code = stoi(line.substr(101, 5));
        double distance = stod(line.substr(106));
        
        // Trim trailing spaces from name
        name.erase(name.find_last_not_of(" ") + 1);
        
        // Create namedPlace using designated initialization
        namedPlace place = {numericCode, state, name, population, area, latitude, longitude, code, distance};
        
        // Insert into hash table
        insertItem(place);
    }
    fin.close();
}