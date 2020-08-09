
#include "m2.h"
#include "mapObjects.h"
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "mapObjects.h"
#include "X11/keysymdef.h"
#include <X11/keysym.h>

#include <stddef.h>
#include "graphics.h"
#include "OSMDatabaseAPI.h"
#include <math.h>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <unordered_map>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/register/point.hpp> 
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <limits>
#include <X11/Xutil.h>

node::node(){
    fromNode = 0;
    fromSegment = 0;
    travelTime = INFINITY;
    
    //projectedTime = find_distance_between_two_points(startPos, endPos)/1000/100*60;  //REMEMBER TO CHANGE THIS
    timeFromDestination = 0;
    expanded = false; //goes true if already expanded
    insertedInPOIVector = false;// goes true if inserted in poi vector
}
node::node(unsigned node, unsigned prevSegment, unsigned start, unsigned goal){
    fromNode = node;
    fromSegment = prevSegment;
    travelTime = find_street_segment_travel_time(prevSegment);
    startPos = map -> pos_from_intersectionID[start];
    endPos = map -> pos_from_intersectionID[goal];
    timeFromDestination = find_distance_between_two_points(startPos, endPos)/(maxSpeedLimit/3.6);  //REMEMBER TO CHANGE THIS
    //timeFromDestination = 0;
    expanded = false; //goes true if already expanded
}

node::node(unsigned node, unsigned prevSegment){
    
    timeFromDestination = 0;
    fromNode = node;
    fromSegment = prevSegment;
    insertedInPOIVector = true;
    travelTime = find_street_segment_travel_time(prevSegment);
    expanded = false;
}
