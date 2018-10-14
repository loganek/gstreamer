/* GStreamer
 * Copyright (C) 2018 Marcin Kolny <marcin.kolny@gmail.com>
 *
 * gsthawktracerstreamer.h: tracing module that streams profiling data
 * to a specific destination.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __GST_HAWKTRACER_STREAMER_TRACER_H__
#define __GST_HAWKTRACER_STREAMER_TRACER_H__

#include <gst/gst.h>
#include <gst/gsttracer.h>

G_BEGIN_DECLS

#define GST_TYPE_HAWKTRACER_STREAMER_TRACER \
  (gst_hawktracer_streamer_tracer_get_type())
#define GST_HAWKTRACER_STREAMER_TRACER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_HAWKTRACER_STREAMER_TRACER,GstHawkTracerStreamerTracer))
#define GST_HAWKTRACER_STREAMER_TRACER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_HAWKTRACER_STREAMER_TRACER,GstHawkTracerStreamerTracerClass))
#define GST_IS_HAWKTRACER_STREAMER_TRACER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_HAWKTRACER_STREAMER_TRACER))
#define GST_IS_HAWKTRACER_STREAMER_TRACER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_HAWKTRACER_STREAMER_TRACER))
#define GST_HAWKTRACER_STREAMER_TRACER_CAST(obj) ((GstHawkTracerStreamerTracer *)(obj))

typedef struct _GstHawkTracerStreamerTracer GstHawkTracerStreamerTracer;
typedef struct _GstHawkTracerStreamerTracerClass GstHawkTracerStreamerTracerClass;

/**
 * GstHawkTracerStreamerTracer:
 *
 * Opaque #GstHawkTracerStreamerTracer data structure
 */
struct _GstHawkTracerStreamerTracer {
  GstTracer 	 parent;

  /*< private >*/
  void* listener;

  enum {
    GST_HT_TCP_LISTENER,
    GST_HT_FILE_LISTENER
  } listener_type;
};

struct _GstHawkTracerStreamerTracerClass {
  GstTracerClass parent_class;

  /* signals */
};

G_GNUC_INTERNAL GType gst_hawktracer_streamer_tracer_get_type (void);

G_END_DECLS

#endif /* __GST_HAWKTRACER_STREAMER_TRACER_H__ */
