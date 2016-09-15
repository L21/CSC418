/***********************************************************
     Starter code for Assignment 3

     This code was originally written by Jack Wang for
		    CSC418, SPRING 2005

		implements scene_object.h

***********************************************************/

#include <cmath>
#include <iostream>
#include "scene_object.h"

bool UnitSquare::intersect( Ray3D& ray, const Matrix4x4& worldToModel,
		const Matrix4x4& modelToWorld ) {
	// TODO: implement intersection code for UnitSquare, which is
	// defined on the xy-plane, with vertices (0.5, 0.5, 0), 
	// (-0.5, 0.5, 0), (-0.5, -0.5, 0), (0.5, -0.5, 0), and normal
	// (0, 0, 1).
	//
	// Your goal here is to fill ray.intersection with correct values
	// should an intersection occur.  This includes intersection.point, 
	// intersection.normal, intersection.none, intersection.t_value.   
	//
	// HINT: Remember to first transform the ray into object space  
	// to simplify the intersection test.
	Vector3D d = worldToModel * ray.dir;
	Point3D a = worldToModel * ray.origin;
	
	double lambda = - a[2] / d[2];
	
	Point3D p = a + (lambda * d);
	
	if (ray.intersection.none || ray.intersection.t_value > lambda){
		if(p[0] > 0.5 || p[0] < -0.5 || p[1] > 0.5 || p[1] < -0.5){
			return false;
		}
		
		Vector3D normal_vector(0, 0, 1);
		
		ray.intersection.t_value = lambda;
		ray.intersection.normal = transNorm(worldToModel, normal_vector);
		ray.intersection.normal.normalize();
		ray.intersection.point = modelToWorld * p;
		ray.intersection.none = false;
		
		return true;
	}
	
	return false;
}

bool UnitSphere::intersect( Ray3D& ray, const Matrix4x4& worldToModel,
		const Matrix4x4& modelToWorld ) {
	// TODO: implement intersection code for UnitSphere, which is centred 
	// on the origin.  
	//
	// Your goal here is to fill ray.intersection with correct values
	// should an intersection occur.  This includes intersection.point, 
	// intersection.normal, intersection.none, intersection.t_value.   
	//
	// HINT: Remember to first transform the ray into object space  
	// to simplify the intersection test.
	Point3D world_origin(0, 0, 0);
	
	double lambda;
	
	Vector3D d = worldToModel * ray.dir;
	Point3D a = worldToModel * ray.origin;
	Point3D c(0, 0, 0);
	Vector3D nmb = a - c;
	
	double A = d.dot(d);
	double B = nmb.dot(d);
	double C = nmb.dot(nmb) - 1;
	
	double D = (B * B) - (A * C);
	
	if (D < 0){
		return false;
	}
	else if(D == 0){
		lambda = - (B / A);
	}
	else{
		double lambda1 = - (B / A) + (sqrt(D) / A);
		double lambda2 = - (B / A) - (sqrt(D) / A);
		lambda = std::min(lambda1, lambda2);
		if (lambda < 0){
			lambda = std::max(lambda1, lambda2);
			if (lambda < 0){
				return false;
			}
		}
	}
	Point3D intersect_point = a + (lambda * d);
    Vector3D normal_vector = intersect_point - c;
    normal_vector.normalize();
    
	ray.intersection.t_value = lambda;
    ray.intersection.normal = transNorm(worldToModel, normal_vector);
    ray.intersection.normal.normalize();
    ray.intersection.point = modelToWorld * intersect_point;
    ray.intersection.none = false;
	return true;
}

