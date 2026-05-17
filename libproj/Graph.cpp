#include "Graph.h"
#include "HashTable.h"

Graph::Graph()
{
    ifstream fin;
    // Load intersections
    fin.open("intersections.txt");
    if (!fin)
    {
        cerr << "Error opening intersections.txt" << endl;
        return;
    }
    string line;
    while (getline(fin, line))
    {
        double lon, lat, elev;
        string state, name;
        lon = stod(line.substr(0, 9));
        lat = stod(line.substr(9, 10));
        elev = stod(line.substr(19, 8));
        state = line.substr(28, 2);
        name = line.substr(30);

        //Trim trailing spaces from name
        name.erase(0, name.find_first_not_of(" "));

        intersection inter(lon, lat, elev, state, name);
        intersections.push_back(inter);
    }
    fin.close();

    // Load connections
    fin.open("connections.txt");
    if (!fin)
    {
        cerr << "Error opening connections.txt" << endl;
        return;
    }

    string name, type;
    int id1, id2;
    double length;
    while(fin >> name >> type >> id1 >> id2 >> length)
    {
        intersection* inter1 = &intersections[id1];
        intersection* inter2 = &intersections[id2];
        connection* conn = new connection(name, type, inter1, inter2, length);
        connections.push_back(conn);
        inter1->connections.push_back(conn);
        inter2->connections.push_back(conn);
    }
    fin.close();
}

Graph::~Graph()
{
    for(int i = 0; i < connections.size(); i++)
    {
        delete connections[i];
    }
    connections.clear();
}

//Helper function to find closest intersection
double distanceBetweenPoints(double lat1, double lon1, double lat2, double lon2)
{
    return sqrt(pow(lat2 - lat1, 2) + pow(lon2 - lon1, 2));
}

intersection* Graph::findClosestIntersection(namedPlace place)
{
    intersection* closestIntersection = nullptr;
    double minDistance = numeric_limits<double>::max();
    for(int i = 0; i < intersections.size(); i++)
    {
        double dist = distanceBetweenPoints(place.latitude, place.longitude, intersections[i].latitude, intersections[i].longitude);
        if(dist < minDistance)
        {
            minDistance = dist;
            closestIntersection = &intersections[i];
        }
    }
    return closestIntersection;
}


const double PI = atan(1) * 4; 
string determineDirection(double lat1, double lon1, double lat2, double lon2)
{
    double dLat = lat2 - lat1;
    double dLon = lon2 - lon1;
    
    // Handle same point
    if (dLat == 0 && dLon == 0)
        return "Same Location";
    
    // Calculate angle in radians
    double angle = atan2(dLon, dLat);
    
    // Convert to degrees (0-360)
    double degrees = angle * 180.0 / PI;
    if (degrees < 0)
        degrees += 360.0;
    
    // Determine direction based on angle
    if (degrees >= 337.5 || degrees < 22.5)
        return "North";
    else if (degrees >= 22.5 && degrees < 67.5)
        return "North-East";
    else if (degrees >= 67.5 && degrees < 112.5)
        return "East";
    else if (degrees >= 112.5 && degrees < 157.5)
        return "South-East";
    else if (degrees >= 157.5 && degrees < 202.5)
        return "South";
    else if (degrees >= 202.5 && degrees < 247.5)
        return "South-West";
    else if (degrees >= 247.5 && degrees < 292.5)
        return "West";
    else  // 292.5 to 337.5
        return "North-West";
}

int Graph::getIndex(intersection* inter)
{
    for (int i = 0; i < intersections.size(); i++)
        {
            if (&intersections[i] == inter)
                return i;
        }
    return -1;
}

vector<connection*> Graph::findShortestRoute(intersection* start, intersection* end)
{
    int n = intersections.size();
    
    // Arrays to track distances and predecessors (indexed by intersection position in vector)
    vector<double> distances(n, numeric_limits<double>::max());
    vector<connection*> predecessors(n, nullptr);
    vector<bool> visited(n, false);
    
    int startIdx = getIndex(start);
    int endIdx = getIndex(end);
    distances[startIdx] = 0.0;
    
    // Main Dijkstra loop
    for (int count = 0; count < n; count++)
    {

        // Find unvisited node with smallest distance
        double minDist = numeric_limits<double>::max();
        int currentIdx = -1;
        
        for (int i = 0; i < n; i++)
        {
            if (!visited[i] && distances[i] < minDist)
            {
                minDist = distances[i];
                currentIdx = i;
            }
        }
        
        if (currentIdx == -1 || currentIdx == endIdx) break;
        
        visited[currentIdx] = true;
        intersection* current = &intersections[currentIdx];
        
        // Check all neighbors
        for (connection* conn : current->connections)
        {
            intersection* neighbor = (conn->inter1 == current) ? conn->inter2 : conn->inter1;
            int neighborIdx = getIndex(neighbor);
            
            if (visited[neighborIdx]) continue;
            
            double newDist = distances[currentIdx] + conn->length;
            
            if (newDist < distances[neighborIdx])
            {
                distances[neighborIdx] = newDist;
                predecessors[neighborIdx] = conn;
            }
        }
    }
    
    // Reconstruct path
    vector<connection*> path;
    int currentIdx = endIdx;
    
    while (currentIdx != startIdx)
    {
        if (predecessors[currentIdx] == nullptr)
        {
            return vector<connection*>();  // No path exists
        }
        
        connection* conn = predecessors[currentIdx];
        path.push_back(conn);
        
        intersection* prevInter = (conn->inter1 == &intersections[currentIdx]) ? conn->inter2 : conn->inter1;
        currentIdx = getIndex(prevInter);
    }
    
    // Reverse the path
    vector<connection*> reversedPath;
    for (int i = path.size() - 1; i >= 0; i--)
    {
        reversedPath.push_back(path[i]);
    }
    
    return reversedPath;
}

intersection* Graph::findCommonIntersection(connection* conn1, connection* conn2)
{
    if (conn1->inter1 == conn2->inter1) return conn1->inter1;
    else if (conn1->inter1 == conn2->inter2) return conn1->inter1;
    else if (conn1->inter2 == conn2->inter1) return conn1->inter2;
    else if (conn1->inter2 == conn2->inter2) return conn1->inter2;
    else return nullptr;
}

int Graph::getCode(intersection* inter)
{
    for (int i = 0; i < intersections.size(); i++)
    {
        if (&intersections[i] == inter)
            return i;
    }
    return -1;
}