/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   mapObjects.h
 * Author: khanzaf2
 *
 * Created on February 20, 2018, 3:20 PM
 */

#include "map_data.h"

#ifndef MAPOBJECTS_H
#define MAPOBJECTS_H

extern map_data *map;
extern float maxSpeedLimit;

extern double average_lat; 

//for Find and highlight the street intersection
struct isHighlighted {
    std::vector<std::pair<double, double>> intersection_x_y;
    //double xCoordinate;
    //double yCoordinate;
    std::vector<double> latVector;
    std::vector<double> lonVector;
    std::vector<std::string> namesVector;
    bool drawPins = false;
    bool drawBox = false;
};

struct poiInformationBox {
    float POIx = 0;
    float POIy = 0;
    unsigned poiID = 0;
    bool draw = false;
};


#endif /* MAPOBJECTS_H */

