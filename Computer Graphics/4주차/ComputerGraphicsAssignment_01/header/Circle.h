#pragma once
#include "Shape.h"

class CCircle : public CShape {
public:
    float m_r;

    CCircle(float x, float y, float r); 
    void Draw(HDC hdc) const;
};