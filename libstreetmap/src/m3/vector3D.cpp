/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   vector3D.cpp
 * Author: khanzaf2
 * 
 * Created on March 18, 2018, 11:28 PM
 */

#include "vector3D.h"
#include "m1.h"
#include "map_data.h"
#include "mapObjects.h"
#include "nodeInfo.h"
#include "X11/keysymdef.h"
#include <X11/keysym.h>
#include <iostream>

#include <stddef.h>
#include <math.h>
#include <algorithm>

vector3D::vector3D() {
    x = 0;
    y = 0;
    z = 0;
}

vector3D::vector3D(float _x, float _y, float _z) {
    x = _x;
    y = _y;
    z = _z;
}

vector3D::vector3D(const vector3D& orig) {
}

vector3D::vector3D(unsigned tailID, unsigned headID) {
    std::cout << "average_lat: " << average_lat << std::endl;
    
    LatLon tailPos = map -> pos_from_intersectionID[tailID];
    LatLon headPos = map -> pos_from_intersectionID[headID];
    
    float x1 = tailPos.lon() * cos(average_lat);
    float y1 = tailPos.lat();
    
    float x2 = headPos.lon() * cos(average_lat);
    float y2 = headPos.lat();
    
    x = x2 - x1;
    y = y2 - y1;
    z = 0;
}

vector3D::~vector3D() {
}



vector3D& vector3D::cross_product(vector3D& A,vector3D& B) {
    //vector3D vector;
    this->x = (A.y * B.z) - (A.z * B.y);
    this->y = (A.z * B.x) - (A.x * B.z);
    this->z = (A.x * B.y) - (A.y * B.x);
    
    if(this->z < 0) {
        this->z = -1;
    }
    else if (this->z > 0) {
        this->z = 1;
    }
    
    return *this;
}