#include <iostream>

#include <gst/gst.h>
#include <glib.h>
#include <stdio.h>
#include <cuda_runtime_api.h>
#include "gstnvdsmeta.h"

#define PGIE_CLASS_ID_VEHICLE 0
#define PGIE_CLASS_ID_PERSON 2

#define MUXER_OUTPUT_WIDTH 1280
#define MUXER_OUTPUT_HEIGHT 720

#define MUXER_BATCH_TIMEOUT_USEC 40000

gint frame_number = 0;

static void qtdemux_parser_link(GstElement *element, GstPad *pad, GstElement* parser) { 
  std::string name = (char*)gst_pad_get_name(pad);
  if (name.find("video") != std::string::npos) {
    GstPad *sinkpad = gst_element_get_static_pad(parser, "sink");
    gst_pad_link(pad, sinkpad); 
    gst_object_unref(sinkpad); 
  } 
} 

static GstPadProbeReturn get_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data) {
  GstBuffer *buf = (GstBuffer*)info->data;
  guint num_rects = 0; 
  NvDsObjectMeta *obj_meta = NULL;
  guint vehicle_count = 0;
  guint person_count = 0;
  NvDsMetaList * l_frame = NULL;
  NvDsMetaList * l_obj = NULL;
  NvDsDisplayMeta *display_meta = NULL;
  
  NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta(buf);
  
  for (l_frame = batch_meta->frame_meta_list; l_frame != NULL;l_frame = l_frame->next) {
    NvDsFrameMeta *frame_meta = (NvDsFrameMeta *) (l_frame->data);
    
    for (l_obj = frame_meta->obj_meta_list; l_obj != NULL; l_obj = l_obj->next) {
      obj_meta = (NvDsObjectMeta *) (l_obj->data);

      if (obj_meta->class_id == PGIE_CLASS_ID_VEHICLE) {
        vehicle_count++;
        num_rects++;
      }
      
      if (obj_meta->class_id == PGIE_CLASS_ID_PERSON) {
        person_count++;
        num_rects++;
      }
    }
  }
  
  g_print("Frame Number = %d Number of objects = %d Vehicle Count = %d Person Count = %d\n", frame_number, num_rects, vehicle_count, person_count);
  frame_number++;
  
  return GST_PAD_PROBE_OK;
}


int main(int argc, char *argv[]) {
  GMainLoop *loop = NULL;
  GstElement *pipeline = NULL,
             *source = NULL,
             *demuxer = NULL, 
             *h264parser = NULL,
             *decoder = NULL, 
             *streammux = NULL,
             *pgie = NULL,
             *sink = NULL;

  GstPad *decoder_pad = NULL, 
         *streammux_pad = NULL,
         *osd_sink_pad = NULL;

  if (argc != 3) {
    std::cout << "Usage: ./deepstream_cxx <path to videofile> <path to deepstream cfg>" << std::endl;
    return -1;
  }

  gst_init(&argc, &argv);
  loop = g_main_loop_new(NULL, FALSE);

  pipeline = gst_pipeline_new("pipeline");

  source = gst_element_factory_make("filesrc", "file-source");
  demuxer = gst_element_factory_make("qtdemux", "demuxer");
  h264parser = gst_element_factory_make("h264parse", "h264-parser");
  decoder = gst_element_factory_make("nvv4l2decoder", "nvv4l2-decoder");  // using nvdec_h264 for hardware accelerated decode on GPU
  streammux = gst_element_factory_make("nvstreammux", "stream-muxer");  // creating nvstreammux instance to form batches from one or more sources
  pgie = gst_element_factory_make("nvinfer", "primary-nvinference-engine");  // inference instanse (deepstream)
  sink = gst_element_factory_make("fakesink", "fakesink");

  if (!source || !demuxer || !h264parser || !decoder || !pgie || !sink) {
    g_printerr("One element could not be created. Exiting.\n");
    return -1;
  }

  g_object_set(G_OBJECT(source), "location", argv[1], NULL);
  g_object_set(G_OBJECT(streammux), "batch-size", 1, NULL);
  g_object_set(G_OBJECT(streammux), "width", MUXER_OUTPUT_WIDTH, 
                                    "height", MUXER_OUTPUT_HEIGHT,
                                    "batched-push-timeout", MUXER_BATCH_TIMEOUT_USEC, NULL);
  g_object_set(G_OBJECT(pgie), "config-file-path", argv[2], NULL);

  gst_bin_add_many(GST_BIN(pipeline), source, demuxer, h264parser, decoder, streammux, pgie, sink, NULL);

  // filesrc -> qtdemux -> h264parse -> nvv4l2decoder -> nvstreammux -> nvinfer -> fakesink
  gst_element_link(source, demuxer);
  gst_element_link(h264parser, decoder);
  gst_element_link_many(streammux, pgie, sink, NULL);
  g_signal_connect(demuxer, "pad-added", G_CALLBACK(qtdemux_parser_link), h264parser);

  decoder_pad = gst_element_get_static_pad(decoder, "src");
  streammux_pad = gst_element_get_request_pad(streammux, "sink_0");
  if (gst_pad_link(decoder_pad, streammux_pad) != GST_PAD_LINK_OK) {
      g_printerr("Failed to link decoder to stream muxer. Exiting.\n");
      return -1;
  }
  gst_object_unref(decoder_pad);
  gst_object_unref(streammux_pad);

  osd_sink_pad = gst_element_get_static_pad(sink, "sink");
  gst_pad_add_probe(osd_sink_pad, GST_PAD_PROBE_TYPE_BUFFER, get_probe, NULL, NULL);

  g_print("Now playing: %s\n", argv[1]);
  gst_element_set_state(pipeline, GST_STATE_PLAYING);

  g_print ("Running...\n");
  g_main_loop_run(loop);

  g_print("Returned, stopping playback\n");
  gst_element_set_state(pipeline, GST_STATE_NULL);

  g_print("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT(pipeline));
  g_main_loop_unref(loop);

  return 0;
}
