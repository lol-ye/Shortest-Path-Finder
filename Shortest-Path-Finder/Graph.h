#ifndef GRAPH_H
#define GRAPH_H

#include <cmath>
#include <vector>
#include <fstream>
#include "HashTable.h"

using namespace std;

struct intersection;

struct connection
{
    string name;
    string type;
    intersection* inter1;
    intersection* inter2;
    double length;

    connection(){}
    
    connection(const string& nm, const string& tp, intersection* i1, intersection* i2, double len)
    {
        name = nm;
        type = tp;
        inter1 = i1;
        inter2 = i2;
        length = len;
    }
};

struct intersection
{
    double longitude;
    double latitude;
    double elevation;
    string stateCode;
    string name;
    vector<connection*> connections;

    intersection(){}

    intersection(double lon, double lat, double elev, const string& state, const string& nm)
    {
        longitude = lon;
        latitude = lat;
        elevation = elev;
        stateCode = state;
        name = nm;
    }


};

class Graph
{
    private:
        vector<intersection> intersections;
        vector<connection*> connections;
    public:
        Graph();
        ~Graph();
        intersection* findClosestIntersection(namedPlace place);
        vector<connection*> findShortestRoute(intersection* start, intersection* end);
        intersection* findCommonIntersection(connection* conn1, connection* conn2);
        int getIndex(intersection* inter);
        int getCode(intersection* inter);
};

string determineDirection(double lat1, double lon1, double lat2, double lon2);
double distanceBetweenPoints(double lat1, double lon1, double lat2, double lon2);

#endif