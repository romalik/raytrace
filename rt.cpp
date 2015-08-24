#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/core/types_c.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <math.h>
#include <stdio.h>
#include <sys/time.h>



class Position3D {
public:
    float x, y, z;
    Position3D() {x = 0; y = 0; z = 0;}
    Position3D(float _x, float _y, float _z) {x = _x; y = _y; z = _z;}
};

typedef Position3D Vector3D;

class Orientation3D {
public:
    float r, p, y;
    Orientation3D() {r = p = y = 0;}
    Orientation3D(float _r, float _p, float _y) {r = _r; p = _p; y = _y; }
};

class Photon;

class Object3D {
public:
    Object3D() {}
    Position3D pos;
    Orientation3D orientation;
    virtual float checkCollision(Photon * ph) = 0;
    virtual float collide(Photon * ph) = 0;
};

class Scene3D {
public:
    std::vector<Object3D *> objs;
    std::deque<Photon *> photons;
};

class Spectrum {
public:
    Spectrum() {}
};

class Photon {
public:
    Photon() {dead = 0;}
    Spectrum spectrum;
    Position3D pos;
    Orientation3D orientation;
    void processCollisions(Scene3D &scene);
    int dead;
};

float uniDist(float _min, float _max) {
    float res = static_cast<float>(rand()%10000)/10000.0f;
    res = res * (_max - _min) + _min;
}


class Light : public Object3D {
public:
    Light() {
        intensity = 1.0f;
    }
    Light(Position3D _pos) {
        intensity = 1.0f;
        pos = _pos;
    }
    float intensity;
    void emitRays(Scene3D &scene, int n) {
        for(int i = 0; i<n; i++) {
            Photon *ph = new Photon();
            ph->pos = pos;
            ph->orientation = Orientation3D(uniDist(0,M_PI*2.0f),
                                            uniDist(0,M_PI*2.0f),
                                            uniDist(0,M_PI*2.0f));

            scene.photons.push_back(ph);
        }
    }
    float checkCollision(Photon *ph) {
        return 10000;
    }
    float collide(Photon *ph) {
        return 0;
    }
};

class Plane: public Object3D {
public:
    Plane() {}
    Plane(Position3D _pos, Orientation3D _orientation, float _w, float _h) {
        pos = _pos;
        orientation = _orientation;
        w = _w;
        h = _h;
    }
    virtual ~Plane() {};
    float w,h;
    float checkCollision(Photon * ph);
    float collide(Photon * ph) {};
};


Vector3D CrossProduct(Vector3D v1, Vector3D v2) {

}


float Plane::checkCollision(Photon *ph) {
    /*
    // Returns in (fX, fY) the location on the plane (P1,P2,P3) of the intersection with the ray (R1, R2)
    // First compute the axes
    V1 = P2 - P1;
    V2 = P3 - P1;
    V3 = CrossProduct ( V1, V2);

    // Project ray points R1 and R2 onto the axes of the plane. (This is equivalent to a rotation.)
    vRotRay1 = CVector3 ( Dot (V1, R1-P1 ), Dot (V2, R1-P1 ), Dot (V3, R1-P1 ) );
    vRotRay2 = CVector3 ( Dot (V1, R2-P1 ), Dot (V2, R2-P1 ), Dot (V3, R2-P1 ) );
    // Return now if ray will never intersect plane (they're parallel)
    if (vRotRay1.z == vRotRay2.z) return FALSE;

    // Find 2D plane coordinates (fX, fY) that the ray interesects with
    float fPercent = vRotRay1.z / (vRotRay2.z-vRotRay1.z);
    vIntersect2d = vRotRay1 + (vRotRay1-vRotRay2) * fPercent;
    fX = vIntersect2d.x;
    fY = vIntersect2d.y;

    // Note that to find the 3D point on the world-space ray use this
    // vInstersect = R1 + (R1-R2) * fPercent;
*/
}

class Box: public Object3D {
public:
    std::vector<Plane *> planes;
    Box(Position3D _pos, Orientation3D _orientation, float _w, float _h, float _d) {

    }
    float checkCollision(Photon * ph);
};


class Camera: public Object3D {
public:
    Camera(Position3D _pos, Orientation3D _orientation, float _fov) {pos = _pos; orientation = _orientation; fov = _fov; sensor = cv::Mat::zeros(480,640,CV_32FC3);}
    virtual ~Camera() {};
    float checkCollision(Photon * ph) {return 0;};
    float collide(Photon * ph) {return 0;};
    float fov;

    cv::Mat sensor;

};


void Photon::processCollisions(Scene3D &scene) {
    float minDist = 10000;
    float minDistIdx = -1;
    for(int i = 0; i<scene.objs.size(); i++) {
        float dist = scene.objs[i]->checkCollision(this);
        if(dist < minDist) {
            minDist = dist;
            minDistIdx = i;
        }
    }

    if(minDistIdx == -1) {
        this->dead = 1;
    } else {
        scene.objs[minDistIdx]->collide(this);
    }

}

int main() {
    cv::namedWindow("render");

    Camera * cam = new Camera(Position3D(0,0,0), Orientation3D(0,0,0), M_PI/2.0f);
    Plane * plane1 = new Plane(Position3D(10,0,0), Orientation3D(0,0,0), 5,5);
    Plane * plane2 = new Plane(Position3D(20,0,0), Orientation3D(0,0,0), 25,25);
    Light * light = new Light(Position3D(5,5,5));

    Scene3D scene;
    scene.objs.push_back(cam);
    scene.objs.push_back(plane1);
    scene.objs.push_back(plane2);
    scene.objs.push_back(light);

    light->emitRays(scene, 10000);
    while(!scene.photons.empty()) {
        printf("scene photons: %d\n", scene.photons.size());
        std::deque<Photon *>::iterator it = scene.photons.begin();
        while(it != scene.photons.end()) {
            (*it)->processCollisions(scene);
            if((*it)->dead) {
                it = scene.photons.erase(it);
            } else {
                it++;
            }
        }

    }



    cv::imshow("render", cam->sensor);
    cv::waitKey(0);

	return 0;
}
