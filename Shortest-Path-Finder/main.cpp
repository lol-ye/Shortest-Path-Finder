#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "HashTable.h"
#include "Graph.h"
#include <algorithm>
#include "raylib.h"

using namespace std;

void printDivider()
{
    for(int i = 0; i < 30; i++){cout<<"=";}
    cout << endl;
}

struct datFileInfo
{
    int lat1;
    int lat2;
    int lon1;
    int lon2;
    string fileName;
    int rows, columns;
};

bool inRange(namedPlace start, namedPlace end, int lat1, int lat2, int lon1, int lon2)
{
    return (start.latitude <= lat1 && start.latitude >= lat2 &&
            start.longitude >= lon1 && start.longitude <= lon2) &&
           (end.latitude <= lat1 && end.latitude >= lat2 &&
            end.longitude >= lon1 && end.longitude <= lon2);
}

double calculateArea(int lat1, int lat2, int lon1, int lon2)
{
    double latDiff = abs(lat1 - lat2);
    double lonDiff = abs(lon1 - lon2);
    return latDiff * lonDiff;
}

datFileInfo getDatFile(namedPlace start, namedPlace end)
{
    datFileInfo bestFile{};
    double smallestArea = 0.0;

    fstream fin("Shortest-Path-Finder/data/coverage.txt");
    if (!fin) {
        cout << "Error opening coverage.txt" << endl;
        return datFileInfo{};
    }

    int lat1, lat2, lon1, lon2;
    string file;

    while(fin >> lat1 >> lat2 >> lon1 >> lon2 >> file)
    {
        if(inRange(start, end, lat1, lat2, lon1, lon2))
        {
            if(bestFile.fileName.empty())
            {
                bestFile.fileName = file;
                bestFile.lat1 = lat1;
                bestFile.lat2 = lat2;
                bestFile.lon1 = lon1;
                bestFile.lon2 = lon2;
                smallestArea = calculateArea(lat1, lat2, lon1, lon2);
            }
            else
            {
                double currentArea = calculateArea(lat1, lat2, lon1, lon2);
                if(currentArea < smallestArea)
                {
                    smallestArea = currentArea;
                    bestFile.fileName = file;
                    bestFile.lat1 = lat1;
                    bestFile.lat2 = lat2;
                    bestFile.lon1 = lon1;
                    bestFile.lon2 = lon2;
                }
            }
        }
    }
    fin.close();
    return bestFile;
}

// raylib only has HSV, so convert the HLS values the original colouring used
Color colorFromHLS(float h, float l, float s)
{
    float v = l + s * min(l, 1 - l);
    return ColorFromHSV(h * 360, v == 0 ? 0 : 2 * (1 - l / v), v);
}

Image drawMap(datFileInfo& datFile)
{
    ifstream fin("Shortest-Path-Finder/data/" + datFile.fileName, ios::in | ios::binary);
    if (!fin) {
        cout << "Error opening " << datFile.fileName << endl;
        return Image{};
    }

    string key;
    int value;

    int bytesPerPixel = 0, secondsPerPixel = 0;
    int leftLongSeconds = 0, topLatSeconds = 0;
    int minVal = 0, maxVal = 0, specialVal = 0;

    for (int i = 0; i < 9; ++i)
    {
        fin >> key >> value;
        if      (key == "rows"){datFile.rows = value;}            
        else if (key == "columns"){datFile.columns = value;}
        else if (key == "bytesperpixel")   bytesPerPixel   = value;
        else if (key == "secondsperpixel") secondsPerPixel = value;
        else if (key == "leftlongseconds") leftLongSeconds = value;
        else if (key == "toplatseconds")   topLatSeconds   = value;
        else if (key == "min")             minVal          = value;
        else if (key == "max")             maxVal          = value;
        else if (key == "specialval")      specialVal      = value;
    }

    fin.seekg(datFile.columns * bytesPerPixel, ios::beg);   // header is padded to one row

    vector<short> pixelData(datFile.rows * datFile.columns);
    fin.read(reinterpret_cast<char*>(pixelData.data()),
             datFile.rows * datFile.columns * bytesPerPixel);
    if (!fin) {
        cout << "Error reading pixel data\n";
        return Image{};
    }

    InitWindow(datFile.columns, datFile.rows, "Shortest Path");   // must come before text drawing: it loads the default font
    Image map = GenImageColor(datFile.columns, datFile.rows, BLACK);   // width = columns, height = rows

    for (int r = 0; r < datFile.rows; r++)
    {
        for (int c = 0; c < datFile.columns; c++)
        {
            short h = pixelData[r * datFile.columns + c];

            if (h == specialVal)
            {
                ImageDrawPixel(&map, c, r, BLUE);
                continue;
            }
            else
            {
                float t = float(h - minVal) / float(maxVal - minVal);
                if (t < 0) t = 0;
                if (t > 1) t = 1;

                float hue        = 0.33f - 0.23f * t;   // green → brown
                float lightness  = 0.3f  + 0.7f  * t;   // dark → bright
                float saturation = 1.0f;

                ImageDrawPixel(&map, c, r, colorFromHLS(hue, lightness, saturation));
            }
        }
    }
    fin.close();
    return map;
}

pair<int,int> latLonToXY(double latitude, double longitude, const datFileInfo& datFile)
{
    int latLimit = datFile.lat1 - datFile.lat2;
    int lonLimit = datFile.lon2 - datFile.lon1;
    double effectiveLat = latitude - datFile.lat2;
    double effectiveLon = longitude - datFile.lon1;
    int x = effectiveLon / lonLimit * datFile.columns;
    int y = datFile.rows- (effectiveLat / latLimit * datFile.rows);

    return {x, y};
}

//Label centred just above (x, y)
void drawLabel(Image& map, const string& text, pair<int,int> xy)
{
    int width = MeasureText(text.c_str(), 10);
    ImageDrawText(&map, text.c_str(), xy.first - width / 2, xy.second - 12, 10, RED);
}

void drawPath(Image& map, vector<connection*> path, namedPlace start, namedPlace end, const datFileInfo& datFile)
{
    pair<int,int> startXY = latLonToXY(start.latitude, start.longitude, datFile);
    pair<int,int> endXY = latLonToXY(end.latitude, end.longitude, datFile);
    drawLabel(map, start.name, startXY);

    Vector2 pen = {(float)startXY.first, (float)startXY.second};
    for(connection* conn : path)
    {
        pair<int,int> connXY = latLonToXY(conn->inter2->latitude, conn->inter2->longitude, datFile);
        Vector2 next = {(float)connXY.first, (float)connXY.second};
        ImageDrawLineEx(&map, pen, next, 3, BLACK);
        pen = next;
    }
    ImageDrawLineEx(&map, pen, {(float)endXY.first, (float)endXY.second}, 3, BLACK);
    drawLabel(map, end.name, endXY);
}

//Opens a window showing the map until the user closes it
void showMap(Image& map)
{
    Texture2D texture = LoadTextureFromImage(map);
    SetTargetFPS(30);
    while(!WindowShouldClose())
    {
        BeginDrawing();
        DrawTexture(texture, 0, 0, WHITE);
        EndDrawing();
    }
    UnloadTexture(texture);
    CloseWindow();
    UnloadImage(map);
}

int main()
{
    HashTable HT;
    Graph G;

    HT.buildTable();

    string startName;
    string endName;
    string startState;
    string endState;
    string line;

    //Start Place
    cout << "Where would you like to begin? ";
    getline(cin, line);
    //Check if the last two chars are a valid state code
    if(stateAbbreviationConvert(line.substr(line.length()-2)) == "[ERROR]")
    {
        startName = line;
        startName.erase(startName.find_last_not_of(" ") + 1);
        vector<string> possibleStates = HT.listStates(startName);
        //Don't ask user for which state if there is only one option
        if(possibleStates.size() == 1)
        {
            startState = possibleStates[0];
        }
        else
        {
            printDivider();
            for(string state : possibleStates){ cout << state << endl;}
            printDivider();
            cout << "Which state? ";
            getline(cin, line);
            startState = line;
        }
    }
    else
    {
        startState = line.substr(line.length() - 2);    //State Abbreviation is last 2 characters
        startName = line.substr(0, line.length() - 3);  //Rest is place name
        startName.erase(startName.find_last_not_of(" ") + 1);   //Trim trailing spaces
    }
    
    //Destination
    cout << "Where would you like to go? ";
    getline(cin, line);
    if(stateAbbreviationConvert(line.substr(line.length()-2)) == "[ERROR]")
    {
        endName = line;
        endName.erase(endName.find_last_not_of(" ") + 1);
        vector<string> possibleStates = HT.listStates(endName);
        if(possibleStates.size() == 1)
        {
            endState = possibleStates[0];
        }
        else
        {
            printDivider();
            for(string state : possibleStates){ cout << state << endl;}
            printDivider();
            cout << "Which state? ";
            getline(cin, line);
            endState = line;
        }
    }
    else
    {
        endState = line.substr(line.length() - 2);
        endName = line.substr(0, line.length() - 3);  //Rest is place name
        endName.erase(endName.find_last_not_of(" ") + 1);   //Trim trailing spaces
    }
    

    namedPlace startPlace = HT.findItem(startName, startState);
    namedPlace endPlace = HT.findItem(endName, endState);
    intersection* currentIntersection = G.findClosestIntersection(startPlace);
    intersection* finalIntersection = G.findClosestIntersection(endPlace);

    
    vector<connection*> shortestPath = G.findShortestRoute(currentIntersection, finalIntersection);
    for(int i = 0; i < 30; i++){cout<<"=";}
    cout<<endl;
    double totalDistance = 0;

    //Error handling for same place
    if(startPlace.name == endPlace.name && startPlace.state == endPlace.state)
    {
        cout << "You are already at your destination: " << endPlace.name << " " << endPlace.state << endl;
        return 0;
    }
    
    //Distance of startplace to its nearest intersection
    double startDistance = distanceBetweenPoints(startPlace.latitude, startPlace.longitude, currentIntersection->latitude, currentIntersection->longitude);
    string startDirection = determineDirection(startPlace.latitude, startPlace.longitude, currentIntersection->latitude, currentIntersection->longitude);
    cout << "Starting point is intersection #" << G.getCode(currentIntersection) << ", " << startDistance << " miles " << startDirection << " of " << startPlace.name << " " << startPlace.state << endl;
    totalDistance += startDistance;
    
    //Check if there is only one connection necessary so the other loop doesn't break trying to look for the nonexistent next connection
    if(shortestPath.size() == 1)
    {
        string direction = determineDirection(currentIntersection->latitude, currentIntersection->longitude, finalIntersection->latitude, finalIntersection->longitude);
        cout << "then take " << shortestPath[0]->name << " " << direction << " for " << shortestPath[0]->length << " miles to" << endPlace.name << " " << endPlace.state << endl;
        totalDistance = shortestPath[0]->length;
    }
    else
    {
        double totalConnectionDistance = 0.0;
        for(int i = 0; i < shortestPath.size() - 1; i++)
        {
            connection* currentConnection = shortestPath[i];
            connection* nextConnection = shortestPath[i+1];
            intersection* nextIntersection = G.findCommonIntersection(currentConnection, nextConnection);
            string direction = determineDirection(currentIntersection->latitude,currentIntersection->longitude, nextIntersection->latitude, nextIntersection->longitude);
            if(currentConnection->name == nextConnection->name)
            {
                totalConnectionDistance += currentConnection->length;
            }
            else
            {
                totalConnectionDistance += currentConnection->length;
                cout << "then take " << currentConnection->name << " " << direction << " for " << totalConnectionDistance << " miles to intersection #" << G.getCode(nextIntersection) << " to " << nextIntersection->name << " " << nextIntersection->stateCode << endl;
                totalDistance += totalConnectionDistance;
                totalConnectionDistance = 0.0;
            }
        }
    }
    cout << "Total Distance: " << totalDistance << endl;
    datFileInfo datFile = getDatFile(startPlace, endPlace);
    Image map = drawMap(datFile);
    if (map.data == nullptr) return 1;
    drawPath(map, shortestPath, startPlace, endPlace, datFile);
    showMap(map);

    return 0;
} 
