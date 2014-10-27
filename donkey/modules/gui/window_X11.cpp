#include "cpp/native_module.hpp"

#include <X11/Xlib.h>

namespace donkey{

class display{
	display(const display&) = delete;
	void operator=(const display&) = delete;
private:
	Display* _handle;
	
	display(){
		_handle = XOpenDisplay(nullptr);
		if(!_handle){
			runtime_error("cannot open display");
		}
	}
	
	~display(){
		XCloseDisplay(_handle);
	}
	
public:
	static Display* handle(){
		static display d;
		return d._handle;
	}
	
};

class window{
	window(const window&) = delete;
	void operator=(const window&) = delete;
private:
	Display* _display;
	int _screen;
	Window _handle;
	
	window(integer l, integer t, integer w, integer h){
		_display = display::handle();
		_screen = DefaultScreen(_display);
		_handle = XCreateSimpleWindow(
			display::handle(),
			RootWindow(_display, _screen),
			l, t, w, h,
			1,
			BlackPixel(_display, _screen),
			WhitePixel(_display, _screen)
		);
		
	}
public:

	static variable create_window(integer l, integer t, integer w, integer h){
		std::unique_ptr<window> p(new window(l, t, w, h));
		variable ret(p.get());
		p.release();
		return ret;
	}

	static vtable_ptr vt(){
		static vtable_ptr ret([](){
			std::unordered_map<std::string, method_ptr> methods;
			
			methods.emplace("loop", create_native_method("gui::Window::loop", &window::loop));
			
			vtable* vt = new vtable("gui", "Window", create_native_function("gui::Window::Window", &window::create_window), std::move(methods), true);
			vt->derive_from(*object_vtable());
			return vt;
		}());
		return ret;
	}
	
	vtable* get_vtable(){
		return vt().get();
	}
	
	void loop(){
		XSelectInput(_display, _handle, ExposureMask | KeyPressMask);
		XMapWindow(_display, _handle);
	
		Atom WM_DELETE_WINDOW = XInternAtom(_display, "WM_DELETE_WINDOW", False); 
		Atom WM_PROTOCOLS = XInternAtom(_display, "WM_PROTOCOLS", False);
		XSetWMProtocols(_display, _handle, &WM_DELETE_WINDOW, 1);
	
		while (1) {
			XEvent e;
			XNextEvent(_display, &e);
			
			switch(e.type){
				case Expose:
					break;
				case KeyPress:
					return;
				case ClientMessage:
					if(e.xclient.message_type == WM_PROTOCOLS && Atom(e.xclient.data.l[0]) == WM_DELETE_WINDOW){
						return;
					}
					break;
			}
		}
	}
};

void add_window_vtables(native_module& m){
	m.add_vtable(window::vt());
}

}//donkey