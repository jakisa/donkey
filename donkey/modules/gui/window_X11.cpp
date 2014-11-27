#include "cpp/native_module.hpp"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace donkey{

enum{
	key_left  = 1,
	key_up    = 2,
	key_right = 3,
	key_down  = 4,
};

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

class xim{
	xim(const xim&) = delete;
	void operator=(const xim&) = delete;
private:
	XIM _handle;
	
	xim(){
		_handle = XOpenIM(display::handle(), nullptr, nullptr, nullptr);
	}
	
	~xim(){
		XCloseIM(_handle);
	}
	
public:
	static XIM handle(){
		static xim x;
		return x._handle;
	}
};

class window{
	window(const window&) = delete;
	void operator=(const window&) = delete;
private:
	int _screen;
	Window _handle;
	XIC _xic;
	
	std::function<void(std::string)> _on_char;
	
	window(integer w, integer h){
		_screen = DefaultScreen(display::handle());
		_handle = XCreateSimpleWindow(
			display::handle(),
			RootWindow(display::handle(), _screen),
			0, 0, w, h,
			1,
			BlackPixel(display::handle(), _screen),
			WhitePixel(display::handle(), _screen)
		);
		
		_xic = XCreateIC(xim::handle(),
			XNInputStyle, (XIMPreeditNothing | XIMStatusNothing),
			XNClientWindow, _handle,
			XNFocusWindow, _handle,
			nullptr
		);
		
		XSetICFocus(_xic);
	}
public:
	
	~window(){
		XDestroyIC(_xic);
	}
	
	static variable create_window(integer w, integer h){
		std::unique_ptr<window> p(new window(w, h));
		variable ret(p.get());
		p.release();
		return ret;
	}

	static vtable_ptr vt(){
		static vtable_ptr ret([](){
			std::unordered_map<std::string, method_ptr> methods;
			
			methods.emplace("loop", create_native_method("gui::Window::loop", &window::loop));
			methods.emplace("setOnChar", create_native_method("gui::Window::setOnChar", &window::set_on_char));
			
			vtable* vt = new vtable("gui", "Window", create_native_function("gui::Window::Window", &window::create_window), std::move(methods), true);
			vt->derive_from(*object_vtable());
			return vt;
		}());
		return ret;
	}
	
	vtable* get_vtable(){
		return vt().get();
	}
	
	void set_on_char(std::function<void(std::string)> on_char){
		_on_char = on_char;
	}
	
	void loop(){
		XSelectInput(display::handle(), _handle, ExposureMask | KeyPressMask);
		XMapWindow(display::handle(), _handle);
	
		Atom WM_DELETE_WINDOW = XInternAtom(display::handle(), "WM_DELETE_WINDOW", False); 
		Atom WM_PROTOCOLS = XInternAtom(display::handle(), "WM_PROTOCOLS", False);
		XSetWMProtocols(display::handle(), _handle, &WM_DELETE_WINDOW, 1);
	
		while (1) {
			XEvent e;
			XNextEvent(display::handle(), &e);
			
			switch(e.type){
				case Expose:
					break;
				case KeyPress:
					{
						char buff[5];
						
						KeySym ks;
						
						Status status;
						
						int len = XmbLookupString(_xic, (XKeyPressedEvent*)&e, buff, 4, &ks, &status);
						
						buff[len] = 0;
						
						//XLookupChars
						
						if(status == XLookupChars || status == XLookupBoth){
							if(_on_char){
								_on_char(std::string(buff, len));
							}
						}
						
						//printf("%d %ld %d %s\n", len, ks, status, buff);
						//fflush(stdout);
					}
					break;
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
	
	m.add_constant("keyLeft",  key_left);
	m.add_constant("keyUp",    key_up);
	m.add_constant("keyRight", key_right);
	m.add_constant("keyDown",  key_down);
}

}//donkey