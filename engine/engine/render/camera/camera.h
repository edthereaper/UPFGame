#ifndef RENDER_CAMERA_H_
#define RENDER_CAMERA_H_

#include "mcv_platform.h"

namespace render {

class Camera {
    protected:
        XMMATRIX    view;            // Where is and where is looking at
        XMMATRIX    projection;      // Prespective info
        XMMATRIX    view_projection; // 
        
        // View matrix info
        XMVECTOR    position;               // Current position
        XMVECTOR    front;                  // Local Z in world coords
        XMVECTOR    up;                     // Local up in world coords
        XMVECTOR    right;                  // Local X in world coords
        XMVECTOR    target;                 // Where we are looking at
        XMVECTOR    up_aux;                 // Used while creating the view matrix

        // Projection information
        float       aspect_ratio;

        bool isOrtho;
        float zNear, zFar;
        union {
            struct perspective_data {
                float fov;
            } perspective;
            struct orthographic_data {
                float w, h;
            } orthographic;
        };

        D3D11_VIEWPORT viewport;

    public:
        Camera();
        inline void updateViewProjection() {view_projection = view * projection;}
        inline void updateProjection() {
            if (!isOrthographic()) {
                setPerspective(perspective.fov, zNear, zFar);
            } else {
                setOrthographic(orthographic.w,orthographic.h,zNear,zFar);
            }
        }

        inline XMMATRIX getView() const { return view; }
        inline XMMATRIX getProjection() const { return projection; }
        inline XMMATRIX getViewProjection() const { return view_projection; }
        inline XMVECTOR getPosition() const { return position; }
        inline XMVECTOR getFront() const { return front; }
        inline XMVECTOR getUp() const { return up; }
        inline XMVECTOR getRight() const { return right; }
        inline XMVECTOR getTarget() const { return target; }
        inline XMVECTOR getAuxUp() const { return up_aux; }
        inline float getAspectRatio() const {return aspect_ratio;}
        inline float getZNear() const {return zNear;}
        inline float getZFar() const {return zFar;}
        inline float getFov() const {return isOrtho ? 0 : perspective.fov;}
        inline float getW() const {return !isOrtho ? 0 : orthographic.w;}
        inline float getH() const {return !isOrtho ? 0 : orthographic.h;}
        inline bool isOrthographic() const {return isOrtho;}

        void lookAt(XMVECTOR new_eye, XMVECTOR new_target, XMVECTOR new_up_aux);
        void setViewport(float x0, float y0, float xmax, float ymax);
        bool getScreenCoords(XMVECTOR world_coord, float *x, float *y, float* z = nullptr) const;

        void setOrthographic(float w, float h, float new_znear, float new_zfar);
        void setPerspective(float new_fov_in_rad, float new_znear, float new_zfar);
        D3D11_VIEWPORT getViewport() const { return viewport; }

		inline float& refFov()		{ return perspective.fov; }
		inline float& refZnear()	{ return zNear; }
		inline float& refZfar()		{ return zFar; }
		inline float& refWidth()	{ return orthographic.w; }
		inline float& refHeight()   { return orthographic.h; }

        inline void setup(XMMATRIX newView, XMMATRIX newViewProjection,
            XMVECTOR newFront, XMVECTOR newRight, XMVECTOR newUp) {
            view = newView;
            view_projection = newViewProjection;
            front = newFront;
            right = newRight;
            up = newUp;
        }
};

}

#endif
