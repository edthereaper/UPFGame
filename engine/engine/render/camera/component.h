#ifndef RENDER_CAMERA_COMPONENT_H_
#define RENDER_CAMERA_COMPONENT_H_

#include "utils/utils.h"
#include "camera.h"
#include "handles/entity.h"
#include "components/transform.h"


namespace render {

/**
 COMPONENT Camera
 Manages camera constants.
 
 MUST have a brother Transform
 On update, it uses the transform to orientate according to it

 XML attributes:
    orthographic (boolean) : false for perspective projection
    zfar: the initalizing zfar (default 1000)
    znear: the initalizing znear (default 1)
    if orthographic 
        w: width of projection (default 10)
        h: height of projection (default 10)
    else
        fov: the initalizing fov in degrees (default 60)
 */
class CCamera : public render::Camera {
    private:
        XMVECTOR    offset = utils::zero_v;

        //Quake effect
        float shakeFrequency = 0;
        float shakeAmount = 0;
        float timer = 0;

    public:        
        inline XMVECTOR getOffset() const { return offset; }
        inline void setOffset(const XMVECTOR& off) { offset = off; }

        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts) {
            if (elem == "Camera") {
                if (!atts.getBool("orthographic")) {
                    float znear= atts.getFloat("znear", zNear);
                    float zfar = atts.getFloat("zfar", zFar);
                    float fovRad = utils::deg2rad(atts.getFloat("fov", utils::rad2deg(perspective.fov)));
                    setPerspective(fovRad, znear, zfar);
                } else {
                    float w = atts.getFloat("w", orthographic.w);
                    float h = atts.getFloat("h", orthographic.h);
                    float znear= atts.getFloat("znear", zNear);
                    float zfar = atts.getFloat("zfar", zFar);
                    setOrthographic(w, h, znear, zfar);
                }
            } else if (elem == "viewport") {
                setViewport(
                    atts.getFloat("x0", viewport.TopLeftX),
                    atts.getFloat("y0", viewport.TopLeftY),
                    atts.getFloat("w", viewport.Width),
                    atts.getFloat("h", viewport.Height)
                    );
            } else if (elem == "lookAt") {
                XMVECTOR eye = atts.getPoint("eye", DirectX::operator-(utils::zAxis_v));
                XMVECTOR target = atts.getPoint("target", utils::zero_v);
                XMVECTOR up = atts.getPoint("up", utils::yAxis_v);
                lookAt(eye, target, up);
            }
	    }

	    inline void init() {
            updateProjection();
            update(0.f);
        }

	    inline void update(float elapsed) {
            component::CTransform* transform(
                component::Handle(this).getBrother<component::CTransform>());

            timer += elapsed;
            float shakeValue = shakeAmount * std::sin(timer * shakeFrequency);

            XMVECTOR tPos = transform->getPosition();
            DirectX::operator+=(tPos, offset);
            DirectX::operator+=(tPos, DirectX::XMVectorSet(0,shakeValue,0,0));

            lookAt(tPos,
                DirectX::operator+(tPos,transform->getFront()),
                transform->getUp());
	    }

        inline void setZFar(float z) {zFar = z; updateProjection();}

        inline void setShake(float amount, float freq) {
            shakeAmount = amount;
            shakeFrequency = M_2_PIf * freq;
        }
};

}

#endif