#include "Common.h"

TWinMessage XEventToEngMsg(const XEvent &event)
{
	switch (event.type)
	{
        case Expose:
            if (event.xexpose.count == 0)
                return TWinMessage(WMT_REDRAW);
            else
                break;

        case FocusIn:
            return TWinMessage(WMT_ACTIVATED);

        case FocusOut:
            return TWinMessage(WMT_DEACTIVATED);

        case MotionNotify:
            return TWinMessage(WMT_MOUSE_MOVE, event.xmotion.x, event.xmotion.y);

        case ButtonPress:
            return TWinMessage(WMT_MOUSE_DOWN, event.xbutton.button);

        case ButtonRelease:

       		int btn;
       		btn = event.xbutton.button - 1;
       		if (btn == 1) btn = 2;
       		else
                if (btn == 2) btn = 1;

            /* By default, X11 only knows 5 buttons. on most 3 button + wheel mouse,
               Button4 maps to wheel up, Button5 maps to wheel down. */
            if (event.xbutton.button == Button4)
                return btn = 1, TWinMessage(WMT_MOUSE_WHEEL, 0, 0, &btn);
            else
               if (event.xbutton.button == Button5)
                    return btn = -1, TWinMessage(WMT_MOUSE_WHEEL, 0, 0, &btn);
                else
                    return TWinMessage(WMT_MOUSE_UP, btn);

        case KeyPress:
        case KeyRelease:
            int key; char code;
            key = XLookupKeysym(const_cast<XKeyEvent *>(&event.xkey), 0);
            switch(key)
            {
              //Consider to hook all of the XK keys.
              case XK_Up : code = 38; break;
              case XK_Down : code = 40; break;
              case XK_Left : code = 37; break;
              case XK_Right : code = 39; break;
              default:
                XLookupString(const_cast<XKeyEvent *>(&event.xkey), &code, 1, NULL, NULL);
                if ( code >= 'a' && code <= 'z' ) code = 'A'-'a' + code;
            }

            return TWinMessage(event.type == KeyPress ? WMT_KEY_DOWN : WMT_KEY_UP, ASCIIKeyToEngKey((uchar)code));

        case ConfigureNotify:
            return TWinMessage(WMT_SIZE, event.xconfigure.width, event.xconfigure.height);

        case DestroyNotify:
        	return TWinMessage(WMT_DESTROY);

        case MapNotify:
        	return TWinMessage(WMT_PRESENT);

	default:
		return TWinMessage(WMT_UNKNOWN, 0, 0, (void*)&event);
	}

/*Remarks

WMT_ENTER_CHAR - Consider of emulating this via KeyPress event.

WMT_CREATE - not generated

WMT_MINIMIZED - to do if needed

WMT_RESTORED - to do if needed

*/

	return TWinMessage();
}

XEvent EngMsgToXEvent(const TWinMessage &msg)
{
//To do in future or consider if is this function needed? May be just remove it?
        XEvent x_ev;
        return x_ev;
}

uint64 GetPerfTimer()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec*1000000 + tv.tv_usec);
}

void GetLocalTimaAndDate(TSysTimeAndDate &time)
{
    time_t t = ::time(NULL);
    struct tm tm = *localtime(&t);
    struct timeval tv;
    gettimeofday(&tv, NULL);

    time.ui16Year = tm.tm_year + 1900;
    time.ui16Month = tm.tm_mon + 1;
    time.ui16Day = tm.tm_mday;
    time.ui16Hour = tm.tm_hour;
    time.ui16Minute = tm.tm_min;
    time.ui16Second = tm.tm_sec;
    time.ui16Milliseconds = tv.tv_usec/1000;
}

void ShowModalUserAlert(const char *pcTxt, const char *pcCaption)
{
    fprintf(stderr, "Error: %s!\n", pcTxt);
}

void GetDisplaySize(uint &width, uint &height)
{
    int num_sizes;
    Rotation original_rotation;
    Display *dpy = XOpenDisplay(NULL);
    if (dpy)
    {
        XRRScreenSize *xrrs = XRRSizes(dpy, 0, &num_sizes);
        XRRScreenConfiguration *conf = XRRGetScreenInfo(dpy, RootWindow(dpy, 0));
        SizeID original_size_id = XRRConfigCurrentConfiguration(conf, &original_rotation);
        width = xrrs[original_size_id].width;
        height = xrrs[original_size_id].height;
        XCloseDisplay(dpy);
    }
    else
    {
        width = 0;
        height = 0;
    }
}

void Terminate()
{
    exit(2);
}
