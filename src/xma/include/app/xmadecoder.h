/*
 * Copyright (C) 2018, Xilinx Inc - All rights reserved
 * Xilinx SDAccel Media Accelerator API
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You may
 * not use this file except in compliance with the License. A copy of the
 * License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
#ifndef _XMDECODER_H_
#define _XMDECODER_H_

/**
 * @ingroup xma_app_intf
 * @file app/xmadecoder.h
 * XMA application interface to decoder kernels
 */

#include "app/xmabuffers.h"
#include "app/xmaparam.h"
#include "lib/xmalimits.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @ingroup xma
 *  @addtogroup xmadec xmadecoder.h
 *  @{
 *  @section xmadec_intro Xilinx Media Accelerator Decoder API
 *
 *  The Xilinx media decoder API is comprised of two distinct interfaces:
 *  one interface for an external framework such as FFmpeg or a proprietary
 *  multi-media framework and the plugin interface used by Xilinx
 *  accelerator developers.  This section illustrates both interfaces
 *  starting with the external framework view and moving on to the plugin
 *  developers view.
 *
 *  @subsection External Interface for XMA Video Decoder Interface
 *
 *  The external interface to the Xilinx video decoder is comprised of the
 *  following functions:
 *
 *  @li @ref xma_dec_session_create()
 *  @li @ref xma_dec_session_destroy()
 *  @li @ref xma_dec_session_send_data()
 *  @li @ref xma_dec_session_get_properties()
 *  @li @ref xma_dec_session_recv_frame()
 *
 *  A media framework (such as FFmpeg) is responsible for creating a decoder
 *  session.  The decoder session contains state information used by the
 *  decoder plugin to manage the hardware associated with a Xilinx accelerator
 *  device.  Prior to creating an decoder session the media framework is
 *  responsible for initializing the XMA using the function
 *  @ref xma_initialize().  The initialize function should be called by the
 *  media framework early in the framework initialization to ensure that all
 *  resources have been configured.  Ideally, the @ref xma_initialize()
 *  function should be called from the main() function of the media framework
 *  in order to guarantee it is only called once.
 *
 *
 *  @code
 *  #include <xma.h>
 *
 *  int main(int argc, char *argv[])
 *  {
 *      int rc;
 *      char *yaml_filepath = argv[1];
 *
 *      // Other media framework initialization
 *      ...
 *
 *      rc = xma_initialize(yaml_filepath);
 *      if (rc != 0)
 *      {
 *          // Log message indicating XMA initialization failed
 *          printf("ERROR: Could not initialize XMA rc=%d\n\n", rc);
 *          return rc;
 *      }
 *
 *      // Other media framework processing
 *      ...
 *
 *      return 0;
 *  }
 *  @endcode
 *
 *  Assuming XMA initialization completes successfully, each decoder
 *  plugin must be initialized, provided data to decode, requested to
 *  receive available decoded frames and finally closed when the video stream
 *  ends.
 *
 *  The code snippet below demonstrates the creation of an XMA decoder
 *  session:
 *
 *
 *  @code
 *  // Code snippet for creating a decoder session
 *  ...
 *  #include <xma.h>
 *  ...
 *  // Setup decoder properties
 *  XmaDecoderProperties dec_props;
 *  dec_props.hwdecoder_type = XMA_H264_DECODER_TYPE;
 *  strcpy(dec_props.hwvendor_string, "Xilinx");
 *
 *  // Create a decoder session based on the requested properties
 *  XmaDecoderSession *dec_session;
 *  dec_session = xma_dec_session_create(&dec_props);
 *  if (!dec_session)
 *  {
 *      // Log message indicating session could not be created
 *      // return from function
 *  }
 *  // Save returned session for subsequent calls.  In FFmpeg, the returned
 *  // session could be saved in the private_data of the AVCodecContext
 *  @endcode
 *
 *  The code snippet that follows demonstrates how to send data
 *  to the decoder session and receive any available decoded frames:
 *
 *  @code
 *  // Code snippet for sending data to the decoder and checking
 *  // if decoded frames are available.
 *
 *  // Other non-XMA related includes
 *  ...
 *  #include <xma.h>
 *
 *  // For this example it is assumed that dec_session is a pointer to
 *  // a previously created decoder session and an XmaBuffer has been
 *  // created using the @ref xma_data_from_buffers_clone() function.
 *  int32_t rc;
 *  int32_t data_used = 0;
 *  rc = xma_dec_session_send_data(dec_session, data, &data_used);
 *  if (rc != 0)
 *  {
 *      // Log error indicating frame could not be accepted
 *      return rc;
 *  }
 *  @endcode
 *
 *  The code snippet that follows demonstrates how to get frame properties
 *  from the decoder session:
 *
 *  @code
 *  // Code snippet for getting frame properties from the decoder.
 *
 *  // Other non-XMA related includes
 *  ...
 *  #include <xma.h>
 *
 *  // For this example it is assumed that dec_session is a pointer to
 *  // a previously created decoder session.
 *  int32_t rc;
 *  XmaFrameProperties fprops;
 *  rc = xma_dec_session_get_properties(dec_session, &fprops);
 *  if (rc != 0)
 *  {
 *      // Log error indicating could not get frame properties
 *      return rc;
 *  }
 *
 *  // Get the decoded frames if any are available.  This example assumes
 *  // that an XmaFrame has been created by cloning the frame
 *  // provided by the media framework using @ref xma_frame_from_buffer_clone()
 *
 *  rc = xma_dec_session_recv_frame(dec_session, frame);
 *  if (rc != 0)
 *  {
 *      // No frames to return at this time
 *      // Tell framework there is no available frames
 *      return rc;
 *  }
 *  // Provide decoded frames to framework
 *  ...
 *  return rc;
 *  @endcode
 *
 *  This last code snippet demonstrates the interface for destroying the
 *  session when the stream is closed.  This allows all allocated resources
 *  to be freed and made available to other processes.
 *
 *  @code
 *  // Code snippet for destroying a session once a stream has ended
 *
 *  // Other non-XMA related includes
 *  ...
 *  #include <xma.h>
 *
 *  // This example assumes that the dec_session is a pointer to a previously
 *  // created XmaDecoderSession
 *  int32_t rc;
 *  rc = xma_dec_session_destroy(dec_session);
 *  if (rc != 0)
 *  {
 *      // TODO: Log message that the destroy function failed
 *      return rc;
 *  }
 *  return rc;
 *  @endcode
 */

/**
 * @typedef XmaDecoderType
 * A decoder from this list forms part of a request for a specific decoder
 * when creating a decoder session via xma_dec_session_create.
 *
 * @typedef XmaDecoderProperties
 * Properities used to specify which decoder is requested and how the decoder
 * should be initalized by the plugin driver.
 *
 * @typedef XmaDecoderSession
 * Opaque pointer to a decoder kernel instance. Used to specify the decoder
 * instance for all decoder application interface APIs
*/

/**
 * @enum XmaDecoderType a discrete list of specific hardware decoders
 *
 * A decoder from this list forms part of a request for a specific decoder
 * when creating a decoder session via xma_dec_session_create.
 */
typedef enum XmaDecoderType
{
    XMA_H264_DECODER_TYPE = 1, /**< 1 */
} XmaDecoderType;

/**
 * @struct XmaDecoderProperties
 * Properities used to specify which decoder is requested and how the decoder
 * should be initalized by the plugin driver
 */
typedef struct XmaDecoderProperties
{
    /** Specific type of decoder requested. See #XmaDecoderType*/
    XmaDecoderType  hwdecoder_type;
    /* XmaConnProps    connection_props; */
    /** Vendor string used to identify specific decoder requested */
    char            hwvendor_string[MAX_VENDOR_NAME];
    /** todo */
    int32_t         intraOnly;
    /** array of kernel-specific custom initialization parameters */
    XmaParameter    *params;
    /** count of custom parameters for port */
    uint32_t        param_cnt;
} XmaDecoderProperties;

/* Forward declaration */
typedef struct XmaDecoderSession XmaDecoderSession;

/**
 *  @brief Create an decoder session
 *
 *  This function creates a decoder session and must be called prior to
 *  decoding data.  A session reserves hardware resources for the
 *  duration of a video stream. The number of sessions allowed depends on
 *  a number of factors that include: resolution, frame rate, bit depth,
 *  and the capabilities of the hardware accelerator.
 *
 *  @param dec_props Pointer to a XmaDecoderProperties structure that
 *                   contains the key configuration properties needed for
 *                   finding available hardware resource.
 *
 *  @return          Not NULL on success
 *  @return          NULL on failure
 *
 *  @note Cannot be presumed to be thread safe.
*/
XmaDecoderSession*
xma_dec_session_create(XmaDecoderProperties *dec_props);

/**
 *  @brief Destroy an decoder session
 *
 *  This function destroys a decoder session that was previously created
 *  with the xma_dec_session_create function.
 *
 *  @param session  Pointer to XmaDecoderSession created with
                    xma_dec_session_create
 *
 *  @return        XMA_SUCCESS on success
 *  @return        XMA_ERROR on failure.
 *
 *  @note Cannot be presumed to be thread safe.
*/
int32_t
xma_dec_session_destroy(XmaDecoderSession *session);

/**
 *  @brief Send data for decoding to the hardware accelerator
 *
 *  This function sends data to the hardware decoder.  If a datae
 *  buffer is not available and the blocking flag is set to true, this
 *  function will block.  If a data buffer is not available and the
 *  blocking flag is set to false, this function will return -EAGAIN.
 *
 *  @param session   Pointer to session created by xma_dec_sesssion_create
 *  @param data      Pointer to a data buffer to be decoded
 *  @param data_used Pointer to an integer to receive the amount of data used
 *
 *  @return        XMA_SUCCESS on success.
 *  @return        XMA_ERROR on error.
*/
int32_t
xma_dec_session_send_data(XmaDecoderSession *session,
                          XmaDataBuffer     *data,
						  int32_t           *data_used);

/**
 *  @brief Get frame properties from the hardware accelerator
 *
 *  This function gets frame properties from the hardware decoder.
 *
 *  @param dec_session  Pointer to session created by xma_dec_sesssion_create
 *  @param fprops   Pointer to a frame properties structure to be filled in
 *
 *  @return        XMA_SUCCESS on success.
 *  @return        XMA_ERROR on error.
*/
int32_t
xma_dec_session_get_properties(XmaDecoderSession  *dec_session,
                               XmaFrameProperties *fprops);

/**
 *  @brief Receive an decoded frame from the hardware accelerator
 *
 *  This function returns a frame if one is available.  This function is
 *  called after calling the function xma_dec_session_send_data.  If a frame
 *  is not ready to be returned, this function returns -1.  In addition, the
 *  frame pointer is set to NULL.  If a frame is ready, a pointer to the frame
 *  will be set to a non-NULL value.
 *
 *  @param session     Pointer to session created by xma_dec_sesssion_create
 *  @param frame       Pointer to a frame containing decoded data
 *
 *  @return        XMA_SUCCESS on success.
 *  @return        XMA_ERROR on error.
*/
int32_t
xma_dec_session_recv_frame(XmaDecoderSession *session,
                           XmaFrame          *frame);
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
