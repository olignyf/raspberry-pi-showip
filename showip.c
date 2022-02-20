
/*

=WHAT IT DOES

Will display eth0 IP in lxpanel widget, if eth0 is not connected will fallback to wlan0. 
If both are unconnected, will display blank.

=BUILD INSTRUCTIONS

$> make

or 

$> gcc -Wall `pkg-config --cflags gtk+-2.0 lxpanel` -shared -fPIC showip.c toolbox.c toolbox-char-array.c toolbox-line-parser.c toolbox-text-buffer-reader.c -o showip.so `pkg-config --libs lxpanel`


=INSTALL INSTRUCTIONS

$> python install.py

or 

sudo cp showip.so /usr/lib/arm-linux-gnueabihf/lxpanel/plugins/
reboot or restart lxpanel
then right click on top panel and add "showip"

*/

#include <lxpanel/plugin.h>

#include <stdio.h>  
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <arpa/inet.h>

#include "toolbox.h"

// internal to the plugin source, not used by the 'priv' variable
static int iInstanceCount = 0;


typedef struct 
{
	LXPanel *panel;
	gint iMyId;
	GtkWidget *gLabel;
	unsigned int timer;
} ShowIp;


static void update_display(ShowIp *pPlugin)
{
	GdkColor color;
	//gchar *separator;

	char ip[256];	/*
	char mac[256];
	char subnet[256];
	char broadcast[256];
	char RX_packets[256];
	char RX_errors[256];
	char RX_bytes[256];
	char TX_packets[256];
	char TX_errors[256];
	char TX_bytes[256];
	char collisions[256];
	char ifconfig[20000] = "";
	printf("=eth0:");*/
	//C_GetNetworkInformation(ip, subnet, broadcast, mac, RX_packets, RX_errors, RX_bytes, TX_packets, TX_errors, TX_bytes, collisions, ifconfig, sizeof(ifconfig), NULL);
	
	struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) 
    {
        if (!ifa->ifa_addr) 
        {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) // check it is IP4
        { 
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
            if (strcmp(ifa->ifa_name, "lo") != 0)
            {
                strncpy(ip, addressBuffer, sizeof(ip)-1);
		    }
        }
    }
    
    // loop ipv6 if we couldnt find ipv4 eth0
    if (ip[0] == '\0') for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) 
    {
        if (!ifa->ifa_addr) 
        {
            continue;
        }
        
        if (ifa->ifa_addr->sa_family == AF_INET6) 
        {
            tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer); 
            if (strcmp(ifa->ifa_name, "lo") != 0)
            {
                strncpy(ip, addressBuffer, sizeof(ip)-1);
		    }
        } 
    }
    
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
    
	printf("ip(%s)\n",ip);/*
	printf("subnet(%s)\n",subnet);
	printf("broadcast(%s)\n",broadcast);
	printf("mac(%s)\n",mac);
	printf("RX_packets(%s)\n",RX_packets);
	printf("RX_errors(%s)\n",RX_errors);
	printf("RX_bytes(%s)\n",RX_bytes);
	printf("TX_packets(%s)\n",TX_packets);
	printf("TX_errors(%s)\n",TX_errors);
	printf("TX_bytes(%s)\n",TX_bytes);
	printf("collisions(%s)\n",collisions);*/
	/*
	if (ip[0] == '\0')
	{
		printf("=wlan0:");
		iret = C_GetNetworkInformation(ip, subnet, broadcast, mac, RX_packets, RX_errors, RX_bytes, TX_packets, TX_errors, TX_bytes, collisions, ifconfig, sizeof(ifconfig), "wlan0");
		if (iret <= 0)
		{
			printf("no wlan0, iret(%d)\n", iret);
		}
		else
		{
			printf("ip(%s)\n",ip);
			printf("subnet(%s)\n",subnet);
			printf("broadcast(%s)\n",broadcast);
			printf("mac(%s)\n",mac);
			printf("RX_packets(%s)\n",RX_packets);
			printf("RX_errors(%s)\n",RX_errors);
			printf("RX_bytes(%s)\n",RX_bytes);
			printf("TX_packets(%s)\n",TX_packets);
			printf("TX_errors(%s)\n",TX_errors);
			printf("TX_bytes(%s)\n",TX_bytes);
			printf("collisions(%s)\n",collisions);
		}
	}
 */
    if (ip[0] == '\0')
    {
		 strcpy(ip, "No IP found");	
	}
	//lxpanel_draw_label_text(pPlugin->panel, pPlugin->gLabel, ip, TRUE, 2, TRUE);
	
    gdk_color_parse("#FF80FF", &color);
    color.pixel = 1;
    color.green = 255;
    color.blue = 155;
    color.red = 100;
	lxpanel_draw_label_text_with_color(pPlugin->panel, pPlugin->gLabel, ip, TRUE, 1.3, &color);
/*
		snprintf(buffer, sizeof(buffer), "<span color=\"#%06x\"><b>%02d</b></span>",
				 gcolor2rgb24(&color), temp);
		gtk_label_set_markup (GTK_LABEL(th->namew), buffer) ;
*/
	gtk_widget_set_tooltip_text(pPlugin->gLabel, "tooltip"); // FIXME write which interface
}

static gboolean update_display_timeout(gpointer user_data)
{
	if (g_source_is_destroyed(g_main_current_source()))
		return FALSE;
	update_display(user_data);
	return TRUE; /* repeat later */
}

GtkWidget *showip_constructor(LXPanel *panel, config_setting_t *settings)
{
	/* panel is a pointer to the panel and
	 settings is a pointer to the configuration data. */
	(void)settings;

	// allocate our private structure instance
	ShowIp *pPlugin = g_new0(ShowIp, 1);

	// update the instance count
	pPlugin->iMyId = ++iInstanceCount;
	pPlugin->panel = panel;

	// make a label out of the ID
	char cIdBuf[20];
	cIdBuf[sizeof(cIdBuf)-1] = '\0';
	snprintf(cIdBuf, sizeof(cIdBuf)-1, "IP %s", "127.0.0.1");

	// create a label widget instance 
	pPlugin->gLabel = gtk_label_new(cIdBuf);
	
	update_display(pPlugin);

	// set the label to be visible
	gtk_widget_show(pPlugin->gLabel);

	// need to create a container to be able to set a border
	GtkWidget *p = gtk_event_box_new();

	// our widget doesn't have a window...
	// it is usually illegal to call gtk_widget_set_has_window() from application but for GtkEventBox it doesn't hurt
	gtk_widget_set_has_window(p, FALSE);

	// bind private structure to the widget assuming it should be freed using g_free()
	lxpanel_plugin_set_data(p, pPlugin, g_free);

	// set border width
	gtk_container_set_border_width(GTK_CONTAINER(p), 1);

	// add the label to the container
	gtk_container_add(GTK_CONTAINER(p), pPlugin->gLabel);

	// set the size we want
	gtk_widget_set_size_request(p, 140, 25);
	
	pPlugin->timer = g_timeout_add_seconds(4, (GSourceFunc) update_display_timeout, (gpointer)pPlugin);

	// success!!!
	return p;
}

FM_DEFINE_MODULE(lxpanel_gtk, showip)

/* Plugin descriptor. */
LXPanelPluginInit fm_module_init_lxpanel_gtk = {
	.name = "ShowIP",
	.description = "Display current IP address.",

	// assigning our functions to provided pointers.
	.new_instance = showip_constructor
};

