#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <aos/aos.h>

#include <aos/cli.h>
#include <ulog/ulog.h>

#include "io.h"
#include "usbd_core.h"
#include "usb_cdc.h"

#include "usbd_descriptor.h"
#include "usbd_cdc_uart.h"
#include "usbd_comp.h"
#include "usbd_cdc_urat_descriptor.h"


#define CDC_COPY_DESCRIPTORS(mem, src) \
	do { \
		const struct usb_desc_header * const *__src; \
		for (__src = src; *__src; ++__src) { \
			memcpy(mem, *__src, (*__src)->bLength); \
			mem += (*__src)->bLength; \
		} \
	} while (0)

static struct usb_interface_association_descriptor
acm_iad_descriptor = {
	.bLength =		sizeof acm_iad_descriptor,
	.bDescriptorType =	USB_DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION,
	/* .bFirstInterface =	DYNAMIC, */
	.bInterfaceCount = 	1,	// control + data
	.bFunctionClass =	USB_DEVICE_CLASS_VEND_SPECIFIC,
	.bFunctionSubClass =	CDC_SUBCLASS_NONE,
	.bFunctionProtocol =	CDC_COMMON_PROTOCOL_NONE,
    .iFunction = 0,
};

static struct usb_interface_descriptor acm_control_interface_desc = {
	.bLength =		sizeof acm_control_interface_desc,
	.bDescriptorType =	USB_DESCRIPTOR_TYPE_INTERFACE,
	/* .bInterfaceNumber = DYNAMIC */
    .bAlternateSetting = 0,
	.bNumEndpoints =	1,
	.bInterfaceClass =	USB_DEVICE_CLASS_VEND_SPECIFIC,
	.bInterfaceSubClass =	CDC_SUBCLASS_NONE,
	.bInterfaceProtocol =	CDC_COMMON_PROTOCOL_NONE,
	.iInterface = 0,
};

static struct cdc_header_descriptor acm_header_desc = {
	.bFunctionLength =		sizeof(acm_header_desc),
	.bDescriptorType =	CDC_CS_INTERFACE,
	.bDescriptorSubtype =	CDC_FUNC_DESC_HEADER,
	.bcdCDC =		cpu_to_le16(0x0110),
};

static struct cdc_call_management_descriptor
acm_call_mgmt_descriptor = {
	.bFunctionLength =		sizeof(acm_call_mgmt_descriptor),
	.bDescriptorType =	CDC_CS_INTERFACE,
	.bDescriptorSubtype =	CDC_FUNC_DESC_CALL_MANAGEMENT,
	.bmCapabilities =	0,
	/* .bDataInterface = DYNAMIC */
};

static struct cdc_abstract_control_management_descriptor acm_descriptor = {
	.bFunctionLength =		sizeof(acm_descriptor),
	.bDescriptorType =	CDC_CS_INTERFACE,
	.bDescriptorSubtype =	CDC_FUNC_DESC_ABSTRACT_CONTROL_MANAGEMENT,
	.bmCapabilities =	0x2,
};

static struct cdc_union_descriptor acm_union_desc = {
	.bFunctionLength =		sizeof(acm_union_desc),
	.bDescriptorType =	CDC_CS_INTERFACE,
	.bDescriptorSubtype =	CDC_FUNC_DESC_UNION,
	/* .bMasterInterface0 =	DYNAMIC */
	/* .bSlaveInterface0 =	DYNAMIC */
};

static struct usb_endpoint_descriptor acm_notify_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
    /* .bEndpointAddress =	DYNAMIC */
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(8),
#if CONFIG_USB_HS
	.bInterval =		0x10,
#else
    .bInterval =		0x0a,
#endif
};

static struct usb_interface_descriptor acm_data_interface_desc = {
	.bLength =		sizeof acm_data_interface_desc,
	.bDescriptorType =	USB_DESCRIPTOR_TYPE_INTERFACE,
	/* .bInterfaceNumber = DYNAMIC */
    .bAlternateSetting = 0,
	.bNumEndpoints =	2,
	.bInterfaceClass =	CDC_DATA_INTERFACE_CLASS,
	.bInterfaceSubClass =	0,
	.bInterfaceProtocol =	0,
	.iInterface = 0,
};

static struct usb_endpoint_descriptor acm_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DESCRIPTOR_TYPE_ENDPOINT,
	/* .bEndpointAddress =	DYNAMIC */
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
    .bInterval          = 0,
    .wMaxPacketSize     = cpu_to_le16(CDC_UART_MPS),
};

static struct usb_endpoint_descriptor acm_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DESCRIPTOR_TYPE_ENDPOINT,
	/* .bEndpointAddress =	DYNAMIC */
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
    .bInterval          = 0,
    .wMaxPacketSize     = cpu_to_le16(512),
};

struct usb_data_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bData1;
    uint8_t bData2;
    uint8_t bData3;
    uint8_t bData4;
    uint8_t bData5;
    uint8_t bData6;
    uint8_t bData7;
    uint8_t bData8;
} __PACKED;

static struct usb_data_descriptor
acm_data_descriptor = {
	.bLength =		sizeof acm_data_descriptor,
	.bDescriptorType =	0xFF,
	.bData1 = 2,
	.bData2 =	1,
	.bData3 =	2,
	.bData4 =	0,
	.bData5 = 0,
	.bData6 = 0,
	.bData7 = 0,
	.bData8 = 0,
};

static const struct usb_desc_header *acm_function[] = {
	(struct usb_desc_header *) &acm_iad_descriptor,
	(struct usb_desc_header *) &acm_data_descriptor,
	(struct usb_desc_header *) &acm_control_interface_desc,
	(struct usb_desc_header *) &acm_out_desc,
	NULL,
	(struct usb_desc_header *) &acm_header_desc,
	(struct usb_desc_header *) &acm_call_mgmt_descriptor,
	(struct usb_desc_header *) &acm_descriptor,
	(struct usb_desc_header *) &acm_union_desc,
	(struct usb_desc_header *) &acm_notify_desc,
	(struct usb_desc_header *) &acm_data_interface_desc,
	(struct usb_desc_header *) &acm_in_desc,
};

static uint8_t *__cdc_uart_build_descriptor(uint32_t *len, uint8_t in_ep,  uint8_t out_ep, uint8_t int_ep, uint8_t *interface_total)
{
	uint8_t *desc = NULL;
	void *mem = NULL;
	uint32_t bytes = 0;

	acm_iad_descriptor.bFirstInterface = *interface_total;
	acm_control_interface_desc.bInterfaceNumber = *interface_total;
    acm_call_mgmt_descriptor.bDataInterface = *interface_total + 1;
    acm_union_desc.bMasterInterface = *interface_total;
    acm_union_desc.bSlaveInterface0 = *interface_total + 1;
    acm_notify_desc.bEndpointAddress = int_ep;
    acm_data_interface_desc.bInterfaceNumber = *interface_total + 1;
    acm_out_desc.bEndpointAddress = out_ep;
    acm_in_desc.bEndpointAddress = in_ep;

	for (uint32_t i = 0; acm_function[i] != NULL; i++) {
		bytes += acm_function[i]->bLength;
	}

    if (len) {
		*len = bytes;
	}

    // one extra byte to hold NULL pointer
    bytes += sizeof(void *);

    desc = (uint8_t *)malloc(bytes);

	mem = desc;
	CDC_COPY_DESCRIPTORS(mem, acm_function);

	return desc;
}

uint8_t *cdc_uart_build_descriptor(struct cdc_uart_device_info *cdc_uart_info, uint32_t *desc_len)
{
	uint8_t *desc = NULL;

	desc = __cdc_uart_build_descriptor(desc_len, cdc_uart_info->cdc_uart_in_ep.ep_addr,
        cdc_uart_info->cdc_uart_out_ep.ep_addr, cdc_uart_info->cdc_uart_int_ep.ep_addr, &cdc_uart_info->interface_nums);

	return desc;
}

void cdc_uart_destroy_descriptor(uint8_t *desc)
{
	if (desc) {
		free(desc);
	}
}


