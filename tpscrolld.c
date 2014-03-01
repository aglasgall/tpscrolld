#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/extensions/XInput.h> 
#include <X11/extensions/XInput2.h> 

int xi_opcode;
int xi_evcode;
int xi_errcode;


int main(int argc, char **argv) 
{
  Display *display = NULL;
  XEvent ev = {0};
  XGenericEventCookie *cookie = (XGenericEventCookie*)&ev.xcookie;
  XIEventMask mask;
  int deviceid = XIAllMasterDevices;
  Window root_window;
  XIGrabModifiers modifiers = {0};
  Window target_window = -1, root_return;
  int target_x, target_y, win_x_return, win_y_return, mask_return;
  int pressed = 0;
  
  display = XOpenDisplay(NULL);
  if(!display) {
    exit(1);
  }

  if (!XQueryExtension(display, "XInputExtension", &xi_opcode, &xi_evcode, &xi_errcode)) {
    printf("X Input extension not available.\n");
    exit(2);
  }

  int major = 2, minor = 1;
  if (XIQueryVersion(display, &major, &minor) == BadRequest) {
    printf("XI2 not available. Server supports %d.%d\n", major, minor);
    return -1;
  }


  int xtest_event_base, xtest_error_base, xtest_major, xtest_minor;
  if(!XTestQueryExtension(display, &xtest_event_base, &xtest_error_base,
			  &xtest_major, &xtest_minor)) {
    printf("XTest not available. Server supports %d.%d\n", major, minor);
    return -1;
  }



  mask.deviceid = deviceid;
  mask.mask_len = XIMaskLen(XI_LASTEVENT);
  mask.mask = calloc(mask.mask_len, sizeof(char));
  XISetMask(mask.mask, XI_RawMotion);
  XISetMask(mask.mask, XI_RawButtonPress);
  XISetMask(mask.mask, XI_RawButtonRelease);
  root_window = XDefaultRootWindow(display);
  XISelectEvents(display, root_window, &mask, 1);
  XSelectInput(display, root_window, 0);

  while(1) {
    XNextEvent(display, &ev);
    if (XGetEventData(display, cookie) &&
	cookie->type == GenericEvent &&
	cookie->extension == xi_opcode)
      {
	XIRawEvent *event_data = (XIRawEvent*)cookie->data;
	if(cookie->evtype == XI_RawMotion) {	  
	  if (pressed == 1) { 
	   
	    if (XIMaskIsSet(event_data->valuators.mask, 1)) {
	      if(event_data->valuators.values[1] < 0) {
		XTestFakeButtonEvent(display, 4, True, CurrentTime);
		XTestFakeButtonEvent(display, 4, False, CurrentTime);
	      } else if (event_data->valuators.values[1] > 0) {
		XTestFakeButtonEvent(display, 5, True, CurrentTime);
		XTestFakeButtonEvent(display, 5, False, CurrentTime);
	      }	      
	    }
	    if (XIMaskIsSet(event_data->valuators.mask, 0)) {
	      if(event_data->valuators.values[0] < 0) {
		XTestFakeButtonEvent(display, 6, True, CurrentTime);
		XTestFakeButtonEvent(display, 6, False, CurrentTime);
	      } else if (event_data->valuators.values[0] > 0) {
		XTestFakeButtonEvent(display, 7, True, CurrentTime);
		XTestFakeButtonEvent(display, 7, False, CurrentTime);
	      }	      
	    }

	    XWarpPointer(display, None, root_window, 
			  0,0,0,0, target_x, target_y);
	  }
	} else if (cookie->evtype == XI_RawButtonPress) {
	  if (event_data->detail == 2) {
	    pressed = 1;
	    XQueryPointer(display, root_window, &root_return,
			  &target_window, &target_x, &target_y,
			  &win_x_return, &win_y_return, &mask_return);
	  }	  
	} else if (cookie->evtype == XI_RawButtonRelease) {
	  if (event_data->detail == 2) {
	    pressed = 0;
	  }	  	  
	}
      }
    XFreeEventData(display, cookie);
  }
}
