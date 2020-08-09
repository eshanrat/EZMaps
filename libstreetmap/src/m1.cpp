/*
 * Copyright 2018 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated
 * documentation files (the "Software") in course work at the University
 * of Toronto, or for personal use. Other uses are prohibited, in
 * particular the distribution of the Software either publicly or to third
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "mapObjects.h"
#include "m1.h"
#include "map_data.h"

#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include <math.h>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <readline/readline.h>
#include <readline/history.h>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>

map_data *map;


bool load_map(std::string map_path) {
    std::string osm_map_path = map_path.substr(0, map_path.find("."));
    ;

    osm_map_path += ".osm.bin";
    std::cout << osm_map_path << std::endl;
    if (!loadStreetsDatabaseBIN(map_path))
        return false;
    else if (loadOSMDatabaseBIN(osm_map_path)) {
        //Load map related data structure

        map = new map_data;

        std::vector<unsigned> finder = find_intersection_ids_from_street_names("Bridgewood Drive", "Bloor Street");
        std::cout << "Vector Size: " << finder.size() << std::endl;
        for (unsigned i = 0; i < finder.size(); ++i)
            std::cout << "Intersection ID: " << finder[i] << std::endl;

        //getting the poi info
        std::vector<std::string> namesFromID = map -> poi_name_from_id;
        std::vector<LatLon> possFromID = map -> poi_pos_from_id;

        for (unsigned i = 0; i < getNumberOfPointsOfInterest(); i++) {
            std::string point_of_interest_name = namesFromID[i];
            LatLon POI_position = possFromID[i];

            unsigned closestIntersection = find_closest_intersection(POI_position);
            auto found = map -> NameAndIntersectionPOI.find(point_of_interest_name);

            // insert a new closest intersection if the vector itself doesn't exist
            if (found == map -> NameAndIntersectionPOI.end()) {
                std::vector<unsigned> insertElements;
                insertElements.push_back(closestIntersection);
                map -> NameAndIntersectionPOI.insert(std::make_pair(point_of_interest_name, insertElements));
            } else
                (found -> second).push_back(closestIntersection);
        }

        return true;
    } else
        return false;
}

//Close the map (if loaded)

void close_map() {
    //Clean-up your map related data structures here

    delete map;
    map = NULL;

    //Close map
    closeStreetDatabase();
    closeOSMDatabase();
}


//Call unordered map to get streetIDs
//Use set to get unique streetIDs
//Create vector of set, return vector

std::vector<unsigned> find_street_ids_from_name(std::string street_name) {
    return map -> query_streetIDs_from_Name(street_name);
}

//Returns the street segments for the given intersection by calling the vector in vector database

std::vector<unsigned> find_intersection_street_segments(unsigned intersection_id) {
    if (intersection_id < getNumberOfIntersections())
        return map->query_intersection_street_segments(intersection_id);
    else {
        std::cout << "Intersection ID given is out of bounds." << std::endl;
        std::vector<unsigned> empty;
        return empty;
    }
}


//Returns the street names at the given intersection (includes duplicate street
//names in returned vector)
//Use unordered map to retrieve street names

std::vector<std::string> find_intersection_street_names(unsigned intersection_id) {

    if (intersection_id < getNumberOfIntersections()) {
        std::vector<std::string> streetNames;
        if (intersection_id < getNumberOfIntersections()) {
            streetNames = map -> query_streetNames_from_Intersection(intersection_id);
            return streetNames;
        } else
            return streetNames;
    } else {
        std::cout << "Intersection ID given is out of bounds." << std::endl;
        std::vector<std::string> empty;
        return empty;
    }
}


//Returns true if you can get from intersection1 to intersection2 using a single
//street segment (hint: check for 1-way streets too)
//corner case: an intersection is considered to be connected to itself

bool are_directly_connected(unsigned intersection_id1, unsigned intersection_id2) {

    if (intersection_id1 < getNumberOfIntersections() && (intersection_id2 < getNumberOfIntersections())) {
        if (intersection_id1 == intersection_id2) //If both ids are for the same intersection
            return true;

        //Retrieve vector of segments corresponding to that intersection ID
        std::vector<unsigned> segmentsInter1 = map->query_intersection_street_segments(intersection_id1);

        //If one way, the 'to' ID has to be intersection_id2
        //Otherwise if two ways, 'to' or 'from' can be intersection_id2
        for (unsigned i = 0; i < segmentsInter1.size(); i++) {
            if (getStreetSegmentInfo(segmentsInter1[i]).oneWay && getStreetSegmentInfo(segmentsInter1[i]).to == intersection_id2) {
                return true;
            } else if (!getStreetSegmentInfo(segmentsInter1[i]).oneWay && (getStreetSegmentInfo(segmentsInter1[i]).to == intersection_id2 || getStreetSegmentInfo(segmentsInter1[i]).from == intersection_id2)) {
                return true;
            }
        }

        return false;
    } else
        return false;
}


//Returns all intersections reachable by traveling down one street segment
//from given intersection (hint: you can't travel the wrong way on a 1-way street)
//the returned vector should NOT contain duplicate intersections

std::vector<unsigned> find_adjacent_intersections(unsigned intersection_id) {

    if (intersection_id < getNumberOfIntersections()) {
        //Retreive the street segments from the intersection ID
        std::vector<unsigned> segments = map->query_intersection_street_segments(intersection_id);
        std::vector<unsigned> adjacentIntersections;

        for (unsigned i = 0; i < segments.size(); ++i) { //Iterate through segments in intersection
            //Check if are 'to' ID is connected and not the same as the given ID
            if ((are_directly_connected(intersection_id, getStreetSegmentInfo(segments[i]).to) && (getStreetSegmentInfo(segments[i]).to != intersection_id))) {
                //Prevent duplicate IDs to be places
                if (std::find(adjacentIntersections.begin(), adjacentIntersections.end(), getStreetSegmentInfo(segments[i]).to)
                        == adjacentIntersections.end()) {
                    adjacentIntersections.push_back(getStreetSegmentInfo(segments[i]).to); //insert into vector
                }

            }//Check if are 'from' ID is connected and not the same as the given ID
            else if ((are_directly_connected(intersection_id, getStreetSegmentInfo(segments[i]).from) && (getStreetSegmentInfo(segments[i]).from != intersection_id))) {
                //Prevent duplicate IDs to be places
                if (std::find(adjacentIntersections.begin(), adjacentIntersections.end(), getStreetSegmentInfo(segments[i]).from)
                        == adjacentIntersections.end()) {
                    adjacentIntersections.push_back(getStreetSegmentInfo(segments[i]).from); //insert into vector
                }
            }

        }
        return adjacentIntersections;
    } else {
        std::cout << "Intersection ID given is out of bounds." << std::endl;
        std::vector<unsigned> empty;
        return empty;
    }
}


//Returns all street segments for the given street 

std::vector<unsigned> find_street_street_segments(unsigned street_id) {
    if (street_id < getNumberOfStreets())
        return map->query_segmentIDs_from_streetID(street_id);
    else {
        std::cout << "Street ID given is out of bounds." << std::endl;
        std::vector<unsigned> empty;
        return empty;
    }
}

//Returns all intersections along the a given street
//Retrieve intersections from unordered map
//Create a set out of the vector to get all unique values
//Create a vector out of the set and return it

std::vector<unsigned> find_all_street_intersections(unsigned street_id) {
    if (street_id < getNumberOfStreets()) {
        std::vector<unsigned> intersections = map->query_intersectionIDs_from_streetID(street_id);
        std::set<unsigned> intersectionsSet(intersections.begin(), intersections.end());
        std::vector<unsigned> returnIntersections(intersectionsSet.begin(), intersectionsSet.end());
        return returnIntersections;
    } else {
        std::cout << "Street ID given is out of bounds." << std::endl;
        std::vector<unsigned> empty;
        return empty;
    }
}

//Return all intersection ids for two intersecting streets
//This function will typically return one intersection id. 
//However street names are not guarenteed to be unique, so more than 1 intersection id
//may exist

std::vector<unsigned> find_intersection_ids_from_street_names(std::string street_name1, std::string street_name2) {

    std::vector<unsigned> ID_intersection_streets;

    std::vector<unsigned> intersectionOneIDs = map->query_intersectionIDs_from_Name(street_name1);
    std::vector<unsigned> intersectionTwoIDs = map->query_intersectionIDs_from_Name(street_name2);

    //Check to see which intersection vector has the smaller size, parse through the smaller vector
    if (intersectionOneIDs.size() >= intersectionTwoIDs.size()) {
        for (unsigned i = 0; i < intersectionTwoIDs.size(); i++) {
            std::vector<std::string> intersection_street_names = find_intersection_street_names(intersectionTwoIDs[i]);
            for (unsigned j = 0; j < intersection_street_names.size(); j++) {
                //Prevent duplicate entries
                if (intersection_street_names[j] == street_name1 && (std::find(ID_intersection_streets.begin(), ID_intersection_streets.end(), intersectionTwoIDs[i])
                        == ID_intersection_streets.end()))
                    ID_intersection_streets.push_back(intersectionTwoIDs[i]);
            }
        }
    } else {
        for (unsigned i = 0; i < intersectionOneIDs.size(); i++) {
            std::vector<std::string> intersection_street_names = find_intersection_street_names(intersectionOneIDs[i]);
            for (unsigned j = 0; j < intersection_street_names.size(); j++) {
                //Prevent duplicate entries
                if (intersection_street_names[j] == street_name2 && (std::find(ID_intersection_streets.begin(), ID_intersection_streets.end(), intersectionOneIDs[i])
                        == ID_intersection_streets.end()))
                    ID_intersection_streets.push_back(intersectionOneIDs[i]);
            }
        }
    }


    return ID_intersection_streets;
}


//Returns the distance between two coordinates in meters

double find_distance_between_two_points(LatLon point1, LatLon point2) {

    //Get longitude and latitude pointsand convert 
    double lon1 = point1.lon() * DEG_TO_RAD;
    double lat1 = point1.lat() * DEG_TO_RAD;
    double lon2 = point2.lon() * DEG_TO_RAD;
    double lat2 = point2.lat() * DEG_TO_RAD;

    //Computer average latitude
    double avgLat = (lat1 + lat2) / 2;

    //Compute the coordinate points
    double x1 = lon1 * cos(avgLat);
    double y1 = lat1;
    double x2 = lon2 * cos(avgLat);
    double y2 = lat2;

    //Return the distance using Distance = Radius of Earth * ((y2-y1)^2 + (x2-x1)^2)^(1/2))
    return EARTH_RADIUS_IN_METERS * sqrt((y2 - y1) * (y2 - y1) + (x2 - x1) * (x2 - x1));
}

//Returns the length of the given street segment in meters

double find_street_segment_length(unsigned street_segment_id) {

    if (street_segment_id < getNumberOfStreetSegments()) {
        StreetSegmentInfo segment = map -> query_streetSegInfo_from_segID(street_segment_id);

        double segmentDistance = 0;
        unsigned numberOfCurves = segment.curvePointCount;

        //if no curve points, compute distance between lat/lon both both ends of segment
        if (numberOfCurves == 0) {
            LatLon startPoint = map -> pos_from_intersectionID[segment.from];
            LatLon endPoint = map -> pos_from_intersectionID[segment.to];
            return find_distance_between_two_points(startPoint, endPoint);
        } else {
            std::vector<LatLon> curvePointsVector = map -> curvePoints_from_segmentID[street_segment_id];

            //Storing LatLon of points in vector
            //if there are curve points, store curve points in vector and individually
            //compute distance between ends of each

            //calculating the distances between each point and adding them up
            for (unsigned i = 0; i < numberOfCurves - 1; ++i) {
                segmentDistance += find_distance_between_two_points(curvePointsVector[i], curvePointsVector[i + 1]);
            }
            //Adding distance from 'to' and 'from' points to curve points
            LatLon startPoint = map -> pos_from_intersectionID[segment.from];
            LatLon endPoint = map -> pos_from_intersectionID[segment.to];

            //Adding the start and end point distance to the distance computed from curve point vector
            segmentDistance += find_distance_between_two_points(startPoint, curvePointsVector[0]);
            segmentDistance += find_distance_between_two_points(curvePointsVector[numberOfCurves - 1], endPoint);
            return segmentDistance;
        }

    } else
        return 0;
}

//Returns the length of the specified street in meters

double find_street_length(unsigned street_id) {

    if (street_id < getNumberOfStreets()) {
        double length = 0;

        std::vector<unsigned> segments = map -> query_segmentIDs_from_streetID(street_id);

        //Computing lengths of all street segments of corresponding street_id
        for (unsigned i = 0; i < segments.size(); i++) {
            length += find_street_segment_length(segments[i]);
        }

        return length;
    } else
        return 0;


}

//Returns the travel time to drive a street segment in seconds
//(time = distance/speed_limit)

double find_street_segment_travel_time(unsigned street_segment_id) {

    StreetSegmentInfo segment = map -> query_streetSegInfo_from_segID(street_segment_id);

    double segLength = find_street_segment_length(street_segment_id);

    if (street_segment_id < getNumberOfStreetSegments()) {

        //dividing segment length by time in m/s
        return segLength / (segment.speedLimit * (1 / 3.6));
    } else
        return 0;
}

//Returns the nearest point of interest to the given position

unsigned find_closest_point_of_interest(LatLon my_position) {

    typedef std::pair<rTreeLatLon, unsigned> point_pair;

    double lat = my_position.lat();
    double lon = my_position.lon();

    std::vector<point_pair> closestIntersections;
    map -> POI_RTree.query(bgi::nearest(rTreeLatLon(lat, lon), 20), std::back_inserter(closestIntersections));

    unsigned closestPoint = 0;

    double shortestDistance = find_distance_between_two_points(my_position, getPointOfInterestPosition(closestIntersections[0].second));
    for (unsigned i = 0; i < closestIntersections.size(); i++) {
        double tempDistance = find_distance_between_two_points(my_position, getPointOfInterestPosition(closestIntersections[i].second));

        if (tempDistance < shortestDistance) {
            shortestDistance = tempDistance;
            closestPoint = closestIntersections[i].second;
        }

    }


    return closestPoint;
}
//Returns the the nearest intersection to the given position

unsigned find_closest_intersection(LatLon my_position) {
    typedef std::pair<rTreeLatLon, unsigned> point_pair;

    //converting lat/lon to radians
    double lat = my_position.lat();
    double lon = my_position.lon();

    std::vector<point_pair> closestPoints;
    map -> intersectionRTree.query(bgi::nearest(rTreeLatLon(lat, lon), 20), std::back_inserter(closestPoints));

    unsigned closestIntersection = 0;

    //computing shortest distance
    double shortestDistance = find_distance_between_two_points(my_position, getIntersectionPosition(closestPoints[0].second));
    for (unsigned i = 0; i < closestPoints.size(); i++) {
        double tempDistance = find_distance_between_two_points(my_position, getIntersectionPosition(closestPoints[i].second));

        if (tempDistance < shortestDistance) {
            shortestDistance = tempDistance;
            closestIntersection = closestPoints[i].second;
        }

    }

    return closestIntersection;

}

