/* SPDX-License-Identifier: GPL-2.0-only
 * Copyright (c) 2020 Avery Black
 * Ported to macOS from linux kernel, original source at
 * https://github.com/torvalds/linux/blob/master/drivers/input/rmi4/rmi_2d_sensor.h
 *
 * Copyright (c) 2011-2016 Synaptics Incorporated
 * Copyright (c) 2011 Unixphere
 */

#ifndef RMI_2D_Sensor_hpp
#define RMI_2D_Sensor_hpp

#include "LinuxCompat.h"
#include "rmi.h"

enum rmi_2d_sensor_object_type {
    RMI_2D_OBJECT_NONE,
    RMI_2D_OBJECT_FINGER,
    RMI_2D_OBJECT_STYLUS,
    RMI_2D_OBJECT_PALM,
    RMI_2D_OBJECT_UNCLASSIFIED,
};

struct rmi_2d_sensor_abs_object {
    enum rmi_2d_sensor_object_type type;
    int mt_tool;
    u16 x;
    u16 y;
    u8 z;
    u8 wx;
    u8 wy;
};

/**
 * @axis_align - controls parameters that are useful in system prototyping
 * and bring up.
 * @max_x - The maximum X coordinate that will be reported by this sensor.
 * @max_y - The maximum Y coordinate that will be reported by this sensor.
 * @nbr_fingers - How many fingers can this sensor report?
 * @data_pkt - buffer for data reported by this sensor.
 * @pkt_size - number of bytes in that buffer.
 * @attn_size - Size of the HID attention report (only contains abs data).
 * position when two fingers are on the device.  When this is true, we
 * assume we have one of those sensors and report events appropriately..
 */
struct rmi_2d_sensor {
    struct rmi_2d_axis_alignment axis_align;
    struct input_mt_pos *tracking_pos;
    int *tracking_slots;
    bool kernel_tracking;
    struct rmi_2d_sensor_abs_object *objs;
    int dmax;
    u16 min_x;
    u16 max_x;
    u16 min_y;
    u16 max_y;
    u8 nbr_fingers;
    u8 *data_pkt;
    int pkt_size;
    int attn_size;
    bool topbuttonpad;
    RMIFunction *fn;
    u8 report_abs {0};
    u8 report_rel {0};
    u8 x_mm;
    u8 y_mm;
};

int rmi_2d_sensor_of_probe(struct device *dev,
                           struct rmi_2d_sensor_platform_data *pdata);

void rmi_2d_sensor_abs_process(struct rmi_2d_sensor *sensor,
                               struct rmi_2d_sensor_abs_object *obj,
                               int slot);

void rmi_2d_sensor_abs_report(struct rmi_2d_sensor *sensor,
                              struct rmi_2d_sensor_abs_object *obj,
                              int slot);

void rmi_2d_sensor_rel_report(struct rmi_2d_sensor *sensor, int x, int y);

int rmi_2d_sensor_configure_input(struct rmi_function *fn,
                                  struct rmi_2d_sensor *sensor);

#endif /* RMI_2D_Sensor_hpp */
