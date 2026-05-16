#pragma once
#include <windows.h>

class CShape {
public:
    float m_x; 
    float m_y; 

    CShape(float x, float y); // »ý¼ºÀÚ
    virtual void Draw(HDC hdc) const;
};