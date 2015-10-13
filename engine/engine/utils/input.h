#ifndef INC_MOUSE_H_
#define INC_MOUSE_H_

#include "utils.h"
#include <windows.h>
#include <array>

#include <windows.h>
#include <XInput.h>
#pragma comment(lib, "XInput.lib")

struct keyState {
    bool prev:1;
    bool curr:1;
    bool next:1;

    keyState() : prev(false),curr(false), next(false) {}

    enum state_e {
        UP      = 0x00,
        DOWN    = 0x01,
        RELEASE = 0x02,
        HIT     = 0x03,
    };

    inline void update() {
        prev = curr;
        curr = next;
    }

    inline void setNext(bool b){next=b;}
    inline state_e getState() const {
        return prev?
            (curr? DOWN : RELEASE) :
            (curr? HIT  : UP);
    }
    inline bool isPressed() const {return (getState()&0x01) != 0;}
    inline bool stateChanged() const {return (getState()&0x02) != 0;}
    inline bool isDown() const {return getState() == DOWN;}
    inline bool isUp() const {return getState() == UP;}
    inline bool isHit() const {return getState() == HIT;}
    inline bool isRelease() const {return getState() == RELEASE;}
    inline operator bool() const {return isPressed();}
};

namespace utils {

class Mouse {
    private:
        static POINT center;
        static POINT delta;
        static bool capturing;
        static HWND hWnd;
        static signed short wheel;
        static keyState lmb, mmb, rmb;

        Mouse()=delete;

    public:
		static inline void init(HWND nHWnd, bool doCapture) {
			hWnd = nHWnd;
			RECT r;
			::GetClientRect(hWnd, &r);
			//Check the desktop resolution, if its lower than the resolution set in config. Set the resolution same as desktop.
			RECT desktop;
			const HWND hDesktop = GetDesktopWindow();
			GetWindowRect(hDesktop, &desktop);
			if (r.right > desktop.right){
				center.x = (desktop.right - desktop.left) / 2;
			}
			else{
				center.x = (r.right - r.left) / 2;
			}
			if (r.bottom > desktop.bottom){
				center.y = (desktop.bottom - desktop.top) / 2;
			}
			else{
				center.y = (r.bottom - r.top) / 2;
			}            
            delta.x = 0;
            delta.y = 0;
            wheel = 0;
            capturing = doCapture ? capture() : release();
        }
        
        static inline POINT getDelta() {
            static const POINT zero = {0,0};
            POINT pt{};
            RECT screen{};
            auto b = GetCursorPos(&pt);
            b &= GetClientRect(hWnd, &screen);
            return b && (
                pt.x<screen.left || pt.y<screen.top ||
                pt.x > screen.right || pt.y > screen.bottom) ?
                zero : delta;
        }

        static inline bool isCapturing() {return capturing;}

        static inline bool capture() {
            capturing = true;
            ::ShowCursor(FALSE);
            centerSysMouse();
            return true;
        }

        static inline bool release() {
            capturing = false;
            ::ShowCursor(TRUE);
            delta.x = delta.y = 0;
            return false;
        }

        static inline bool toggleCapture() {
            return isCapturing() ? release() : capture();
        }

        static inline void centerSysMouse()  {
            POINT sc = center;
            ::ClientToScreen(hWnd, &sc);
            ::SetCursorPos(sc.x, sc.y);
        }

        static void setSysMouse(int sx, int sy);
        
        static inline void clearDelta() {
            delta.x = delta.y = 0;
        }

		static inline void setSysXboxController(int sx, int sy) {
			delta.x = sx;
			delta.y = sy;
			if (delta.x != 0 || delta.y != 0) {
				centerSysMouse();
			}
		}

        static inline void setWheel(signed short val) {
            wheel = val;
        }
        static inline signed short getWheel() {
            return wheel;
        }

        static inline void onLMB(bool down) {
            lmb.setNext(down);
        }
        static inline void onMMB(bool down) {
            mmb.setNext(down);
        }
        static inline void onRMB(bool down) {
            rmb.setNext(down);
        }

        static inline keyState getLMB() {return lmb;}
        static inline keyState getMMB() {return mmb;}
        static inline keyState getRMB() {return rmb;}

        static inline void update() {
            lmb.update();
            mmb.update();
            rmb.update();
        }
        
        static inline void refresh() {
            wheel = 0;
        }
};

class Pad {
    public:
        typedef unsigned long long key_t;
        typedef unsigned controlId_t;

    private:

        // Mapping: what keys are associated to what controls?
        typedef unsigned controlIndex_t;
        static const controlIndex_t MAX_MAPPINGS = 4;

        //mapping_t := pair of key and array of mapped states
        struct mapping_t : public std::pair<key_t, std::array<controlIndex_t, MAX_MAPPINGS>> {
            typedef std::array<controlIndex_t, MAX_MAPPINGS> array_t;
            typedef std::pair<key_t, array_t> pair_t;

            static const controlIndex_t NO_MAP = ~0;

            //Init to unmapped
            mapping_t(key_t k) : pair_t(k, array_t()) {
                for(auto& s : second) {s=NO_MAP;}
            }
            
            bool operator<(const mapping_t& b) {return first < b.first;}
            bool operator==(key_t key) const {return first==key;}
        };
        typedef std::vector<mapping_t> mappings_t;
        mappings_t mappings;


        // Inbox: what keys have changed this frame?
        //inboxEntry_t := pair of key and wether the key is pressed
        struct  inboxEntry_t : public std::pair<key_t, bool /*down*/> {
            typedef std::pair<key_t, bool /*down*/> pair_t;
            inline inboxEntry_t(const key_t& k, bool down) : pair_t(k,down) {}
            bool operator<(const inboxEntry_t& b) {return first < b.first;}
            bool operator==(key_t key) const {return first==key;}
        };
        typedef std::vector<inboxEntry_t> inbox_t;
        inbox_t inbox;
        bool input = false; // Received input (called onKey()) since last update?


        // Controls
        typedef std::vector<keyState> controls_t;
        typedef std::map<controlId_t, controlIndex_t> controlQuery_t;
        controls_t states;
        controlQuery_t controlQuery;

        const keyState badKey;

        // Reset some keys
        std::vector<key_t> resetKeys;


    public:
        Pad() : badKey() {
            inbox.reserve(8);
        }

        inline void onKey(const key_t& k, bool down) {
            inbox.push_back(inboxEntry_t(k,down));
            input = true;
        }
        
        inline void reset() {
            auto mappingIt = mappings.cbegin();
            auto end = mappings.cend();
            for(const auto& resetKey : resetKeys) {
                mappingIt = seqFind(mappingIt, end, resetKey);
                if (mappingIt == end) {break;}
                else {
                    for (auto& mapped : mappingIt->second) {
                        if (mapped == mapping_t::NO_MAP) {break;}
                        else {states[mapped].setNext(false);}
                    }
                }
            }
        }
        inline void addToResetList(const key_t& resetKey) {
            resetKeys.push_back(resetKey);
            std::sort(resetKeys.begin(), resetKeys.end());
        }

        inline void update() {
            if (!inbox.empty()) {
                std::sort(inbox.begin(), inbox.end());
                //mappings is sorted by key
                auto mappingIt = mappings.cbegin();
                auto end = mappings.cend();
                for(const auto& entry : inbox) {
                    mappingIt = seqFind(mappingIt, end, entry.first);
                    if (mappingIt == end) {break;}
                    else {
                        for (auto& mapped : mappingIt->second) {
                            if (mapped == mapping_t::NO_MAP) {break;}
                            else {states[mapped].setNext(entry.second);}
                        }
                    }
                }
                inbox.clear();
            }
            for (auto& p : states) {
                p.update();
            }
        }

        inline const keyState getState(const controlId_t& control) const {
            auto index = atOrDefault(controlQuery, control, ~0);
            return (index != ~0) ? states[index] : badKey;
        }

        inline bool addMapping(const key_t& k, const controlId_t& control) {
            const controlIndex_t BAD(~0);
            auto index = atOrDefault(controlQuery, control, BAD);
            if (index == BAD) {
                //New control
                states.push_back(keyState());
                index = static_cast<unsigned>(states.size()-1);
                controlQuery[control] = index;
            }
            auto mapping = seqFind(mappings.begin(), mappings.end(), k);
            if (mapping == mappings.end()) {
                mapping_t newMapping(k);
                newMapping.second[0] = index;
                mappings.push_back(newMapping);
                std::sort(mappings.begin(), mappings.end());
                return true;
            } else {
                for (auto& mapped : mapping->second) {
                    if(mapped == mapping_t::NO_MAP) {
                        mapped = index;
                        return true;
                    }
                }
                return false; //Mapping array was full
            }
        }
};

class GamePadXBOXController
{
public:
	typedef enum
	{
		GamePad_Button_DPAD_UP = 0,
		GamePad_Button_DPAD_DOWN = 1,
		GamePad_Button_DPAD_LEFT = 2,
		GamePad_Button_DPAD_RIGHT = 3,
		GamePad_Button_START = 4,
		GamePad_Button_BACK = 5,
		GamePad_Button_LEFT_THUMB = 6,
		GamePad_Button_RIGHT_THUMB = 7,
		GamePad_Button_LEFT_SHOULDER = 8,
		GamePad_Button_RIGHT_SHOULDER = 9,
		GamePad_Button_A = 10,
		GamePad_Button_B = 11,
		GamePad_Button_X = 12,
		GamePad_Button_Y = 13,
		GamePadButton_Max = 14
	}GamePadButton;
private:
	struct GamePadState
	{
		keyState	_buttons[GamePadButton_Max];
		float 		_left_thumbstickX;
		float 		_left_thumbstickY;
		float	    _right_thumbstickX;
		float	    _right_thumbstickY;
		float		_left_trigger;
		float		_right_trigger;
		// Just to clear all values to default
		void reset()
		{
			_left_thumbstickX = _left_thumbstickY = 0.0f;
			_right_thumbstickX = _right_thumbstickY = 0.0f;
			_left_trigger = _right_trigger = 0.0f;
		}
	};
	XINPUT_STATE _controllerState;
public:
	GamePadState	State;

	GamePadXBOXController()
	{
		State.reset();
	}

	virtual ~GamePadXBOXController(void)
	{
		// We don't want the controller to be vibrating accidentally when we exit the app
		if (is_connected()) vibrate(0.0f, 0.0f);
	}

	bool is_connected()
	{
		// clean the state
		memset(&_controllerState, 0, sizeof(XINPUT_STATE));

		// Get the state
		DWORD Result = XInputGetState(0, &_controllerState);

		if (Result == ERROR_SUCCESS)	return true;
		else return false;
	}

	void vibrate(float leftmotor = 0.0f, float rightmotor = 0.0f)
	{
		// Create a new Vibraton 
		XINPUT_VIBRATION Vibration;

		memset(&Vibration, 0, sizeof(XINPUT_VIBRATION));

		int leftVib = (int)(leftmotor*65535.0f);
		int rightVib = (int)(rightmotor*65535.0f);

		// Set the Vibration Values
		Vibration.wLeftMotorSpeed = leftVib;
		Vibration.wRightMotorSpeed = rightVib;
		// Vibrate the controller
		XInputSetState(0, &Vibration);
	}

	void update()
	{
		State.reset();
		// The values of the Left and Right Triggers go from 0 to 255. We just convert them to 0.0f=>1.0f
		if (_controllerState.Gamepad.bRightTrigger)			State._right_trigger = _controllerState.Gamepad.bRightTrigger / 255.0f;
		if (_controllerState.Gamepad.bLeftTrigger)			State._left_trigger = _controllerState.Gamepad.bLeftTrigger / 255.0f;

		// Get the Buttons
		if (_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_A)									State._buttons[GamePad_Button_A].setNext(true);
		else if (State._buttons[GamePad_Button_A].getState() == keyState::DOWN)						State._buttons[GamePad_Button_A].setNext(false);
		if (_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_B)									State._buttons[GamePad_Button_B].setNext(true);
		else if (State._buttons[GamePad_Button_B].getState() == keyState::DOWN)						State._buttons[GamePad_Button_B].setNext(false);
		if (_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_X)									State._buttons[GamePad_Button_X].setNext(true);
		else if (State._buttons[GamePad_Button_X].getState() == keyState::DOWN)						State._buttons[GamePad_Button_X].setNext(false);
		if (_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_Y)									State._buttons[GamePad_Button_Y].setNext(true);
		else if (State._buttons[GamePad_Button_Y].getState() == keyState::DOWN)						State._buttons[GamePad_Button_Y].setNext(false);
		if (_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)								State._buttons[GamePad_Button_BACK].setNext(true);
		else if (State._buttons[GamePad_Button_BACK].getState() == keyState::DOWN)					State._buttons[GamePad_Button_BACK].setNext(false);
		if (_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_START)								State._buttons[GamePad_Button_START].setNext(true);
		else if (State._buttons[GamePad_Button_START].getState() == keyState::DOWN)					State._buttons[GamePad_Button_START].setNext(false);
		if (_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)						State._buttons[GamePad_Button_LEFT_SHOULDER].setNext(true);
		else if (State._buttons[GamePad_Button_LEFT_SHOULDER].getState() == keyState::DOWN)			State._buttons[GamePad_Button_LEFT_SHOULDER].setNext(false);
		if (_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)						State._buttons[GamePad_Button_RIGHT_SHOULDER].setNext(true);
		else if (State._buttons[GamePad_Button_RIGHT_SHOULDER].getState() == keyState::DOWN)		State._buttons[GamePad_Button_RIGHT_SHOULDER].setNext(false);
		
		State._buttons[GamePad_Button_A].update();
		State._buttons[GamePad_Button_B].update();
		State._buttons[GamePad_Button_X].update();
		State._buttons[GamePad_Button_Y].update();
		State._buttons[GamePad_Button_BACK].update();
		State._buttons[GamePad_Button_START].update();
		State._buttons[GamePad_Button_LEFT_SHOULDER].update();
		State._buttons[GamePad_Button_RIGHT_SHOULDER].update();

		// Check to make sure we are not moving during the dead zone
		// Let's check the Left DeadZone
		if ((_controllerState.Gamepad.sThumbLX < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
			_controllerState.Gamepad.sThumbLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) &&
			(_controllerState.Gamepad.sThumbLY < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
			_controllerState.Gamepad.sThumbLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE))
		{
			_controllerState.Gamepad.sThumbLX = 0;
			_controllerState.Gamepad.sThumbLY = 0;
		}

		// Check left thumbStick
		float leftThumbY = _controllerState.Gamepad.sThumbLY;
		if (leftThumbY)
		{
			State._left_thumbstickY = leftThumbY / 32768;
		}
		float leftThumbX = _controllerState.Gamepad.sThumbLX;
		if (leftThumbX)
		{
			State._left_thumbstickX = leftThumbX / 32768;
		}

		// Check to make sure we are not moving during the dead zone
		// Let's check the Left DeadZone
		if ((_controllerState.Gamepad.sThumbRX < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
			_controllerState.Gamepad.sThumbRX > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) &&
			(_controllerState.Gamepad.sThumbRY < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
			_controllerState.Gamepad.sThumbRY > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE))
		{
			_controllerState.Gamepad.sThumbRX = 0;
			_controllerState.Gamepad.sThumbRY = 0;
		}

		// Check left thumbStick
		float rightThumbY = _controllerState.Gamepad.sThumbRY;
		if (rightThumbY)
		{
			State._right_thumbstickY = rightThumbY / 32768;
		}
		float rightThumbX = _controllerState.Gamepad.sThumbRX;
		if (rightThumbX)
		{
			State._right_thumbstickX = rightThumbX / 32768;
		}
	}	
};

}
#endif