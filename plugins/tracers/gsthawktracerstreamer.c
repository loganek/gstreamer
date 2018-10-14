/* GStreamer
 * Copyright (C) 2018 Marcin Kolny <marcin.kolny@gmail.com>
 *
 * gsthawktracerstreamer.c: tracing module that streams profiling data
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
/**
 * SECTION:element-hawktracerstreamertracer
 * @short_description: streams profiling data.
 *
 * A tracing module that listens to the profililing bus and streams
 * it to a specific destination.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "gsthawktracerstreamer.h"
#include "gst/gsttracer.h"

#include <hawktracer/timeline.h>
#include <hawktracer/listeners/tcp_listener.h>
#include <hawktracer/listeners/file_dump_listener.h>

GST_DEBUG_CATEGORY_STATIC (gst_hawktracer_streamer_debug);
#define GST_CAT_DEFAULT gst_hawktracer_streamer_debug

#define _do_init \
    GST_DEBUG_CATEGORY_INIT (gst_hawktracer_streamer_debug, "hawktracerstreamer", 0, "hawktracer streamer tracer");
#define gst_hawktracer_streamer_tracer_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstHawkTracerStreamerTracer,
    gst_hawktracer_streamer_tracer, GST_TYPE_TRACER, _do_init);

#define DEFAULT_TCP_PORT 8765
#define DEFAULT_FILE_PATH "gst-hawktracer-trace.htdump"
#define DEFAULT_BUFFER_SIZE 4096

static HT_ErrorCode
load_tcp_listener (GstHawkTracerStreamerTracer * tracer, gint buff_size,
    GstStructure * params)
{
  HT_ErrorCode error_code = HT_ERR_OK;
  gint port;
  gboolean port_ok = gst_structure_get_int (params, "port", &port);

  if (!port_ok) {
    port = DEFAULT_TCP_PORT;
  }

  GST_INFO_OBJECT (tracer, "Creating HawkTracer TCP listener. Port: %d", port);

  tracer->listener_type = GST_HT_TCP_LISTENER;
  tracer->listener = ht_tcp_listener_create (port, buff_size, &error_code);

  return error_code;
}

static HT_ErrorCode
load_file_listener (GstHawkTracerStreamerTracer * tracer, gint buff_size,
    GstStructure * params)
{
  HT_ErrorCode error_code = HT_ERR_OK;
  const gchar *location = NULL;

  if (params) {
    location = gst_structure_get_string (params, "location");
  }

  if (!location) {
    location = DEFAULT_FILE_PATH;
  }

  GST_INFO_OBJECT (tracer, "Creating HawkTracer file listener. Output file: %s",
      location);

  tracer->listener_type = GST_HT_FILE_LISTENER;
  tracer->listener =
      ht_file_dump_listener_create (location, buff_size, &error_code);

  return error_code;
}

static HT_TimelineListenerCallback
_get_ht_callback (GstHawkTracerStreamerTracer * tracer)
{
  switch (tracer->listener_type) {
    case GST_HT_TCP_LISTENER:
      return ht_tcp_listener_callback;
    case GST_HT_FILE_LISTENER:
      return ht_file_dump_listener_callback;
    default:
      g_critical ("invalid HawkTracer listener type: %d",
          tracer->listener_type);
  }

  return NULL;
}

static void
load_hawktracer_listener (GstHawkTracerStreamerTracer * tracer,
    GstStructure * params)
{
  const gchar *type = gst_structure_get_string (params, "type");
  gint buff_size;
  gboolean buff_size_ok =
      gst_structure_get_int (params, "buffer-size", &buff_size);
  HT_ErrorCode error_code;

  if (!buff_size_ok) {
    buff_size = DEFAULT_BUFFER_SIZE;
  }

  if (g_strcmp0 (type, "tcp") == 0) {
    error_code = load_tcp_listener (tracer, buff_size, params);
  } else {
    error_code = load_file_listener (tracer, buff_size, params);
  }

  if (error_code == HT_ERR_OK) {
    ht_timeline_register_listener (gst_tracer_get_ht_bus (),
        _get_ht_callback (tracer), tracer->listener);
  } else {
    GST_ERROR_OBJECT (tracer,
        "Failed to create HawkTracer listener. Reason: %d", error_code);
  }
}

static void
load_default_hawktracer_listener (GstHawkTracerStreamerTracer * tracer)
{
  GstStructure *params_struct =
      gst_structure_from_string ("streamerparams", NULL);
  load_hawktracer_listener (tracer, params_struct);
  gst_structure_free (params_struct);
}

static void
gst_hawktracer_streamer_tracer_constructed (GObject * object)
{
  GstHawkTracerStreamerTracer *self = GST_HAWKTRACER_STREAMER_TRACER (object);

  gchar *params, *tmp;
  GstStructure *params_struct = NULL;

  g_object_get (self, "params", &params, NULL);
  if (!params) {
    load_default_hawktracer_listener (self);
    goto failed_parsing_params;
  }

  tmp = g_strdup_printf ("streamerparams,%s", params);
  params_struct = gst_structure_from_string (tmp, NULL);
  g_free (tmp);

  if (params_struct)
    load_hawktracer_listener (self, params_struct);
  else
    load_default_hawktracer_listener (self);

  g_free (params);

  if (params_struct)
    gst_structure_free (params_struct);

failed_parsing_params:
  ((GObjectClass *)
      gst_hawktracer_streamer_tracer_parent_class)->constructed (object);
}

static void
gst_hawktracer_streamer_tracer_finalize (GObject * object)
{
  GstHawkTracerStreamerTracer *self = GST_HAWKTRACER_STREAMER_TRACER (object);

  switch (self->listener_type) {
    case GST_HT_FILE_LISTENER:
      ht_file_dump_listener_destroy (self->listener);
      break;
    case GST_HT_TCP_LISTENER:
      ht_tcp_listener_destroy (self->listener);
      break;
    default:
      g_warning ("unknown type of hawktracer listener: %d",
          self->listener_type);
      break;
  }

  self->listener = NULL;
}

static void
gst_hawktracer_streamer_tracer_class_init (GstHawkTracerStreamerTracerClass *
    klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->constructed = gst_hawktracer_streamer_tracer_constructed;
  gobject_class->finalize = gst_hawktracer_streamer_tracer_finalize;
}

static void
gst_hawktracer_streamer_tracer_init (GstHawkTracerStreamerTracer * self)
{
  GstHawkTracerStreamerTracer *tracer = GST_HAWKTRACER_STREAMER_TRACER (self);

  tracer->listener = NULL;

  gst_tracing_register_hook (GST_TRACER (tracer), "not-assigned", NULL);
}
