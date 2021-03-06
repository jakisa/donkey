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
	std::function<void(integer)> _on_key_down;
	
	window(integer w, integer h){
		_screen = DefaultScreen(display::handle());
		
		int depth = XDefaultDepth(display::handle(), _screen);
		Visual* visual = XDefaultVisual(display::handle(), _screen);
		
		if(depth != 24 || visual->red_mask != 0xFF0000 || visual->green_mask != 0xFF00 || visual->blue_mask != 0xFF){
			runtime_error("display format is not supported");
		}
		
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
			methods.emplace("setOnKeyDown", create_native_method("gui::Window::setOnKeyDown", &window::set_on_key_down));
			
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
	
	void set_on_key_down(std::function<void(integer)> on_key_down){
		_on_key_down = on_key_down;
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
						
						if(status == XLookupChars || status == XLookupBoth){
							if(_on_char){
								_on_char(std::string(buff, len));
							}
						}
						
						if(status == XLookupKeySym || status == XLookupBoth){
							if(_on_key_down){
								integer key_code = 0;
								switch(ks){
									case XK_Left:
										key_code = key_left;
										break;
									case XK_Right:
										key_code = key_right;
										break;
									case XK_Up:
										key_code = key_up;
										break;
									case XK_Down:
										key_code = key_down;
										break;
								}
								if(key_code){
									_on_key_down(key_code);
								}
							}
						}
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