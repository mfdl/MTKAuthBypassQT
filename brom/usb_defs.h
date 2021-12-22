#ifndef USB_DEFS_H
#define USB_DEFS_H

#include <inttypes.h>

namespace WinUSB
{
struct usb_dev_handle;
typedef struct usb_dev_handle usb_dev_handle;
typedef struct usb_device_descriptor
{
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned short bcdUSB;
    unsigned char  bDeviceClass;
    unsigned char  bDeviceSubClass;
    unsigned char  bDeviceProtocol;
    unsigned char  bMaxPacketSize0;
    unsigned short idVendor;
    unsigned short idProduct;
    unsigned short bcdDevice;
    unsigned char  iManufacturer;
    unsigned char  iProduct;
    unsigned char  iSerialNumber;
    unsigned char  bNumConfigurations;
}usb_device_descriptor;
typedef struct usb_config_descriptor
{
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned short wTotalLength;
    unsigned char  bNumInterfaces;
    unsigned char  bConfigurationValue;
    unsigned char  iConfiguration;
    unsigned char  bmAttributes;
    unsigned char  MaxPower;
    struct usb_interface *ifc;
    unsigned char *extra;	/* Extra descriptors */
    int extralen;
}usb_config_descriptor;
///* All standard descriptors have these 2 fields in common */
struct usb_descriptor_header
{
    unsigned char  bLength;
    unsigned char  bDescriptorType;
};

/* String descriptor */
struct usb_string_descriptor
{
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned short wData[1];
};

/* HID descriptor */
struct usb_hid_descriptor
{
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned short bcdHID;
    unsigned char  bCountryCode;
    unsigned char  bNumDescriptors;
};

/* Endpoint descriptor */
#define USB_MAXENDPOINTS	32
struct usb_endpoint_descriptor
{
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned char  bEndpointAddress;
    unsigned char  bmAttributes;
    unsigned short wMaxPacketSize;
    unsigned char  bInterval;
    unsigned char  bRefresh;
    unsigned char  bSynchAddress;

    unsigned char *extra;	/* Extra descriptors */
    int extralen;
};
struct usb_interface_descriptor
{
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned char  bInterfaceNumber;
    unsigned char  bAlternateSetting;
    unsigned char  bNumEndpoints;
    unsigned char  bInterfaceClass;
    unsigned char  bInterfaceSubClass;
    unsigned char  bInterfaceProtocol;
    unsigned char  iInterface;
    struct usb_endpoint_descriptor *endpoint;
    unsigned char *extra;	/* Extra descriptors */
    int extralen;
};
struct usb_interface
{
    struct usb_interface_descriptor *altsetting;
    int num_altsetting;
};
typedef struct usb_device
{
    struct usb_device *next, *prev;
    char filename[512];
    struct usb_bus *bus;
    struct usb_device_descriptor descriptor;
    struct usb_config_descriptor *config;
    void *dev;		/* Darwin support */
    unsigned char devnum;
    unsigned char num_children;
    struct usb_device **children;
}usb_device;
typedef struct usb_bus
{
    struct usb_bus *next, *prev;
    char dirname[512];
    struct usb_device *devices;
    unsigned long location;
    struct usb_device *root_dev;
}usb_bus;
typedef struct USBCtrlPkt
{
    uint8_t bmRequestType {};
    uint8_t bRequest {};
    uint16_t wValue {};
    uint16_t wIndex {};
    uint16_t wLength {};
}USBCtrlPkt;
}

#endif // USB_DEFS_H
