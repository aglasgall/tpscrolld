#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/extensions/XInput.h> 
#include <X11/extensions/XInput2.h> 

int xi_opcode;
int xi_evcode;
int xi_errcode;

void print_motion(XIDeviceEvent *event)
{
    int i;
    double *valuator = event->valuators.values;

    for (i = 0; i < event->valuators.mask_len * 8; i++) {
        if (XIMaskIsSet(event->valuators.mask, i)) {
            printf("Value on valuator %d: %f\n", 
                   i, *valuator);
            valuator++;
        }
    }
}

int main(int argc, char **argv) 
{
  Display *display = NULL;
  XEvent ev = {0};
  XGenericEventCookie *cookie = (XGenericEventCookie*)&ev.xcookie;
  XIEventMask mask;
  int deviceid = XIAllMasterDevices;
  Window root_window;
  int i = 0;
  XIGrabModifiers modifiers = {0};
  char *window_name;
  Window target_window = -1;
  double target_x, target_y;

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
  XISetMask(mask.mask, XI_Motion);
  XISetMask(mask.mask, XI_ButtonPress);
  XISetMask(mask.mask, XI_ButtonRelease);
  modifiers.modifiers = XIAnyModifier;
  root_window = XDefaultRootWindow(display);
  XISelectEvents(display, root_window, &mask, 1);
  XSelectInput(display, root_window, 0);
  XIGrabButton(display, XIAllMasterDevices, 2, root_window,
	       None, XIGrabModeAsync, XIGrabModeAsync, 1,
	       &mask, 1, &modifiers);

  while(1) {
    XNextEvent(display, &ev);
    if (XGetEventData(display, cookie) &&
	cookie->type == GenericEvent &&
	cookie->extension == xi_opcode)
      {
	XIDeviceEvent *event_data = (XIDeviceEvent*)cookie->data;
	if(cookie->evtype == XI_Motion) {
	  XIUngrabButton(display, XIAllMasterDevices, 2, root_window, 
			 1, &modifiers);
	  if (XIMaskIsSet(event_data->buttons.mask, 2)) { 
	    printf("Got motion to: %f %f from %f %f with button 2 pressed\n", 
	      event_data->root_x, event_data->root_y, target_x, target_y);
	    if(event_data->root_y < target_y) {
	      XTestFakeButtonEvent(display, 4, True, CurrentTime);
	      XTestFakeButtonEvent(display, 4, False, CurrentTime+1);
	    } else if (event_data->root_y > target_y) {
	      XTestFakeButtonEvent(display, 5, True, CurrentTime);
	      XTestFakeButtonEvent(display, 5, False, CurrentTime+1);
	    }
	  }
	  XIGrabButton(display, XIAllMasterDevices, 2, root_window,
	       None, XIGrabModeAsync, XIGrabModeAsync, 1,
	       &mask, 1, &modifiers);
  
	} else if (cookie->evtype == XI_ButtonPress) {
	  target_x = event_data->root_x;
	  target_y = event_data->root_y;
	  int i;
	  for(i = 1; i < 12; ++i) {
	    if(XIMaskIsSet(event_data->buttons.mask, i)) {
	      printf("Button %d pressed\n", i);
	    }
	  }
	  printf("%f %f in %lx\n", target_x, target_y, event_data->child);
	} else if (cookie->evtype == XI_ButtonRelease) {
	  for(i = 1; i < 12; ++i) {
	    if(XIMaskIsSet(event_data->buttons.mask, i)) {
	      printf("Button %d released\n", i);
	    }
	  }	  
	}
      }
    XFreeEventData(display, cookie);
  }
}
