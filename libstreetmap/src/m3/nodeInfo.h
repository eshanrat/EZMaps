/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   nodeInfo.h
 * Author: khanzaf2
 *
 * Created on March 15, 2018, 11:12 PM
 */

#ifndef NODEINFO_H
#define NODEINFO_H

#include "m1.h"
#include "map_data.h"


class node {
public:
    unsigned fromNode;
    unsigned fromSegment;
    double travelTime;
    double timeFromDestination;
    node();
    node(unsigned, unsigned, unsigned, unsigned);
    node(unsigned, unsigned);
    LatLon startPos;
    LatLon endPos;
    bool expanded;
    bool insertedInPOIVector;


private:

};

#endif /* NODEINFO_H */

