#include "Circle.h"

CCircle::CCircle(float x, float y, float r)
    : CShape(x, y), m_r(r) {
}

void CCircle::Draw(HDC hdc) const {
    float left = m_x - m_r;
    float top = m_y - m_r;
    float right = m_x + m_r;
    float bottom = m_y + m_r;

    Ellipse(hdc, (int)left, (int)top, (int)right, (int)bottom);
}