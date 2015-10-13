#include "mcv_platform.h"
#include "camera.h"

using namespace DirectX;
using namespace utils;

namespace render {

Camera::Camera()
{
    setPerspective(deg2rad(60.f), 0.1f, 1000.f);
    lookAt(zero_v, zAxis_v, yAxis_v);
    setViewport(0, 0, 512, 512);
}

void Camera::lookAt(XMVECTOR new_eye, XMVECTOR new_target, XMVECTOR new_up_aux)
{
    view = XMMatrixLookAtRH(new_eye, new_target, new_up_aux);

    position = new_eye;
    target = new_target;
    up_aux = new_up_aux;
    front = XMVector3Normalize( new_target - new_eye );
    right = XMVector3Normalize( XMVector3Cross(front, up_aux) );
    up = XMVector3Cross(right, front);

    updateViewProjection();
}

void Camera::setOrthographic(float newW, float newH, float new_znear, float new_zfar)
{
    assert(new_znear != 0);
    isOrtho = true;
    zNear = new_znear;
    zFar = new_zfar;
    orthographic.w = newW;
    orthographic.h = newH;
    projection = XMMatrixOrthographicRH(newW, newH, new_znear, new_zfar);
    updateViewProjection();
}

void Camera::setPerspective(float new_fov_in_rad, float new_znear, float new_zfar)
{
    assert(new_znear != 0);
    isOrtho = false;
    perspective.fov = new_fov_in_rad;
    zNear = new_znear;
    zFar = new_zfar;
    projection = XMMatrixPerspectiveFovRH(new_fov_in_rad, aspect_ratio, new_znear, new_zfar);
    updateViewProjection();
}

void Camera::setViewport(float x0, float y0, float width, float height)
{
    viewport.TopLeftX = x0;
    viewport.TopLeftY = y0;
    viewport.Width = width;
    viewport.Height = height;
    viewport.MinDepth = 0.f;
    viewport.MaxDepth = 1.f;

    aspect_ratio = width / height;
    updateProjection();
}

bool Camera::getScreenCoords(XMVECTOR world_coord, float* x, float* y, float* z) const
{
    XMVECTOR homo_coords = XMVector3TransformCoord(world_coord, view_projection);
    float sx = XMVectorGetX(homo_coords);
    float sy = XMVectorGetY(homo_coords);
    float sz = XMVectorGetZ(homo_coords);

    *x = viewport.TopLeftX + (sx * 0.5f + 0.5f) * viewport.Width;
    *y = viewport.TopLeftY + (-sy * 0.5f + 0.5f) * viewport.Height;
    if (z != nullptr) {*z = sz;}

    return !(sz < 0.f || sz > 1.f);
}

}