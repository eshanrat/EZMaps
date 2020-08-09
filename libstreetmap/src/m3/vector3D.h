/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   vector3D.h
 * Author: khanzaf2
 *
 * Created on March 18, 2018, 11:28 PM
 */

#ifndef VECTOR3D_H
#define VECTOR3D_H

class vector3D {
public:
    vector3D();
    vector3D(float _x, float _y, float _z);
    vector3D(const vector3D& orig);
    vector3D(unsigned tailID, unsigned headID);
    vector3D& cross_product(vector3D& vector1,vector3D& vector2);

    virtual ~vector3D();

    float x;
    float y;
    float z;

private:

};

#endif /* VECTOR3D_H */
