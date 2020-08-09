/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "m1.h"
#include "map_data.h"
#include "mapObjects.h"
#include "nodeInfo.h"
#include "X11/keysymdef.h"
#include <X11/keysym.h>

#include <stddef.h>
#include <math.h>
#include <algorithm>

std::vector<unsigned> make_path(std::vector<node>& closedSet, const unsigned intersect_id_start, const unsigned intersect_id_end) {

    std::vector<unsigned> made_path;
    unsigned temp = intersect_id_end;
    
    //Get the reverse path
    //already have the closed set and know that I have to start from the end_intersect_id
    while (temp != intersect_id_start) {
        made_path.push_back((closedSet[temp]).fromSegment);
        temp = ((closedSet[temp]).fromNode);
    }
    std::reverse(made_path.begin(), made_path.end());

    return made_path;
}
