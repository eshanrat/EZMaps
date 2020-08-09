/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   map_data.h
 * Author: khanzaf2
 *
 * Created on February 1, 2018, 10:41 PM
 */
#include "nodeInfo.h"
#include "StreetsDatabaseAPI.h"
#include <math.h>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <map>


#include <boost/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <iostream>
#include <limits>
#include <vector>

#ifndef MAP_DATA_H
#define MAP_DATA_H

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

//Create struct for boost library functions

struct rTreeLatLon {

    rTreeLatLon() {
    }

    rTreeLatLon(float lat_, float lon_) : lat_lon(lat_, lon_) {
    }

    float get_lat() const {
        return lat_lon.lat();
    }

    float get_lon() const {
        return lat_lon.lon();
    }

    void set_lat(float v) {
        lat_lon = LatLon(v, lat_lon.lon());
    }

    void set_lon(float v) {
        lat_lon = LatLon(lat_lon.lat(), v);
    }

    LatLon lat_lon;
};

//Set long, lat, points to coordinate system

BOOST_GEOMETRY_REGISTER_POINT_2D_GET_SET(rTreeLatLon, float,
        bg::cs::spherical_equatorial<bg::degree>,
        get_lon, get_lat, set_lon, set_lat)


class map_data {
public:
    map_data();

    //Query functions to access databases

    std::unordered_map <std::string, std::vector<unsigned>> NameAndIntersectionPOI;
    
    std::vector<unsigned> query_streetIDs_from_Name(std::string);

    std::vector<unsigned> query_segmentIDs_from_streetID(unsigned);

    std::vector<unsigned> query_intersectionIDs_from_streetID(unsigned);

    std::vector<unsigned> query_intersectionIDs_from_Name(std::string);

    std::vector<std::string> query_streetNames_from_Intersection(unsigned);

    std::vector<unsigned> query_intersection_street_segments(unsigned);

    StreetSegmentInfo query_streetSegInfo_from_segID(unsigned);

    std::unordered_map<OSMID, std::unordered_map<std::string, std::vector<std::string>>> OSMWayHash_from_OSMID;

    std::vector<std::vector<LatLon>> featurePoints_from_featureID;

    std::vector<std::string> featureName_from_featureID;

    std::vector<FeatureType> featureType_from_featureID;

    std::vector<LatLon> pos_from_intersectionID;

    std::vector<std::vector<LatLon>> curvePoints_from_segmentID;

    std::vector<std::string> POItypes;
    
    std::unordered_map<std::string, unsigned> intersectionIDs_from_intersectionName;

    std::unordered_map<std::string, std::vector<std::string>> POItypes_of_POIcategory;

    std::unordered_map<std::string, std::vector<std::string>> POInames_of_POIcategory;

    std::vector<std::string> poi_name_from_id;
    
    std::vector<LatLon> poi_pos_from_id;
    
    std::vector<LatLon> emergencyPOILatLon;

    std::vector<LatLon> healthPOILatLon;

    std::vector<LatLon> hotelPOILatLon;

    std::vector<LatLon> entertainmentPOILatLon;

    std::vector<LatLon> fitnessPOILatLon;

    std::vector<LatLon> publicPOILatLon;

    std::vector<LatLon> parkingPOILatLon;

    std::vector<LatLon> shoppingPOILatLon;

    std::vector<LatLon> financePOILatLon;

    std::vector<LatLon> transportationPOILatLon;

    std::vector<LatLon> gasPOILatLon;

    std::vector<LatLon> foodPOILatLon;

    std::vector<LatLon> educationPOILatLon;

    std::vector<LatLon> govtPOILatLon;

    std::vector<std::vector<LatLon>> POILatLonCategory_from_POInumber;

    struct intersection_dataLOAD {
        LatLon position;
        std::string name;
    };

    std::vector<intersection_dataLOAD> intersections;

    std::vector<std::pair<double, unsigned>> areaVector;

    std::vector<std::string> streetName_from_streetID;

    std::vector<std::string> segName_from_segID;

    std::vector<std::string> intersectionNames_from_IDs;

    std::vector<std::string> POINames_from_IDs;

    std::vector <unsigned> featurePoints_from_featureName;
    
    std::vector<std::string> combinedNames_from_ID;

    unsigned numberOfStreetSegments;
    

    //Create the rTree
    typedef std::pair<rTreeLatLon, unsigned> point_pair;

    bgi::rtree<point_pair, bgi::quadratic < 16 >> intersectionRTree;

    bgi::rtree<point_pair, bgi::quadratic < 16 >> POI_RTree;

    //std::vector<nodeInfo> nodeInfo_from_intersection_id;

private:

    //Databases
    std::unordered_map<std::string, std::vector<unsigned>> streetIDs_from_Name;

    std::vector<std::vector<unsigned>> segmentIDs_from_streetID;

    std::vector<std::vector<unsigned>> intersectionIDs_from_streetID;

    std::unordered_map<std::string, std::vector<unsigned>> intersectionIDs_from_Name;

    std::vector<std::vector<std::string>> streetNames_from_Intersection;

    std::vector<std::vector<unsigned>> intersection_street_segments;

    std::vector<StreetSegmentInfo> streetSegInfo_from_segID;



};

#endif /* MAP_DATA_H */