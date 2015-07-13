namespace android {


    //validate the resolution in sizeQueue weather supported by driver
    //Out: supported size string
    String8 CameraSetting::ValidateSize(CAM_PortCapability *portCap, Vector<ImageSize> sizeQueue)const
    {
      unsigned int i = 0;
      String8 v = string8("");
      int pf_width = 0;
      int pf_height = 0;
      int ce_width = 0;
      int ce_height = 0;
      unsigned int cnt = sizeQueue.size();

      for( i = 0; i < cnt; i++)
      {
        ImageSize size = sizeQueue.itemAt(i);
        pf_width = sizeQueue.itemAt(i).mWidth;
        pf_height = sizeQueue.itemAt(i).mHeight;

        //find the size in port supported size
        bool find = false;
        for(int j = 0; j < portCap->iSupportedSizeCnt; j++) {
          ce_width = portCap->stSupportedSize[j].iWidth;
          ce_height = portCap->stSupportedSize[j].iHeight;
          
          if (ce_width == pf_width && ce_height == pf_height) {
            find = true;
            break;
          }
        }

        if (portCap->bArbitrarySize && pf_width >= portCap->stMin.iWidth &&
            pf_height >= portCap->stMin.iHeight && pf_width <= portCap->stMax.iWidth &&
            pf_height <= portCap->stMax.iHeight) {
          find = true;
        }

        if (find == true) {
          if (i > 0) {
            v += String8(",");
          }

          v += size_to_str(pf_width, pf_height);
        } else {
          TRACE_E("IsMatch: Sensor not support preview size:%dX%d", pf_width, pf_height);
        }
      }
      return v;
    }


    /*
     * verifyPortSizes to set preview/still/video port supported size list
     * 1. check and list driver supported port sizes
     * 2. if port sizes defined in camera_profile.xml,
     *    according to camera_profile to filter out those which not supported by driver
     * */
    status_t CameraSetting::verifyPortSizes(const CAM_CameraCapability& camera_caps, int iPortID)
    {
      mCamera_caps = camera_caps;
      String8 v = String8("");
      int width_min, height_min;
      int width, height;
        //according to camera_profile to filter out the preview size
        const char* sensorname = pMrvlCameraInfo->getSensorName();
        mCamProfiles = CameraProfiles::getInstance();
        int sensorindex = -1;

        if(mCamProfiles != NULL) {
            sensorindex = mCamProfiles->getCamConfigIndex(sensorname);
            if( sensorindex < 0 && pMrvlCameraInfo->getPortNum() >= MULTI_PORTCAM) {
                sensorindex = mCamProfiles->getCamConfigIndex("raw_sensor");
            }
        }

        if( sensorindex >= 0 && mCamProfiles->getPortSizes(sensorindex, iPortID) != NULL ) {
            v = String8("");
            CAM_PortCapability * pPortCap = &mCamera_caps.stPortCapability[iPortID];
            Vector<ImageSize> *sizeQueue = mCamProfiles->getPortSizes(sensorindex, iPortID);
            v  += ValidateSize(pPortCap,*sizeQueue);
            width_min = sizeQueue->itemAt(0).mWidth;
            height_min = sizeQueue->itemAt(0).mHeight;
        }
        else {
            //camera_profile.xml is null or the sensorname is not exist
            if (mCamera_caps.stPortCapability[iPortID].bArbitrarySize) {
                CAM_PortCapability * pPortCap = &mCamera_caps.stPortCapability[iPortID];
                v += ValidateSize(pPortCap, SupportedPortSize[iPortID], SupportedPortSizeLen[iPortID]);
                width_min = SupportedPortSize[iPortID][0].width;
                height_min = SupportedPortSize[iPortID][0].height;
            }
            else {
                //for nonArbitrarySize:  driver supported resolution
                char val1[10];
                char val2[10];
                v = String8("");
                width_min = 0;
                height_min = 0;
                for(int i=0; i<mCamera_caps.stPortCapability[iPortID].iSupportedSizeCnt; i++){
                    width = mCamera_caps.stPortCapability[iPortID].stSupportedSize[i].iWidth;
                    height = mCamera_caps.stPortCapability[iPortID].stSupportedSize[i].iHeight;
                    //we don't support preview size bigger than maxpreviewwidth x maxpreviewheight which is defined in camera_profiles.xml

                    if( CAM_PORT_STILL!=iPortID && (width > iMaxPreviewWidth || height > iMaxPreviewHeight) )
                        continue;

                    if( width_min > 0 || height_min > 0 )
                        v += String8(",");

                    if( width_min <= 0 || height_min <= 0 || width < width_min){
                        width_min = width;
                        height_min = height;
                    }
                    sprintf(val1,"%d", width);
                    sprintf(val2,"%d", height);
                    v += String8(val1)+
                        String8("x")+
                        String8(val2);
                }
            }
        }

        switch(iPortID) {
            case CAM_PORT_PREVIEW:
                {
                    set(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES,v.string());
                    if( width_min > 0 && height_min > 0 ) {
                        setPreviewSize(width_min,height_min);
                    }
                    TRACE_D("%s:CE supported preview size:%s",__FUNCTION__,
                            get(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES));
                }
                break;
            case CAM_PORT_VIDEO:
                {
                    set(CameraParameters::KEY_SUPPORTED_VIDEO_SIZES, v.string());
                    if( width_min > 0 && height_min > 0 ) {
                        setVideoSize(width_min,height_min);
                    }
                    TRACE_D("%s:CE supported video size:%s",__FUNCTION__,
                            get(CameraParameters::KEY_SUPPORTED_VIDEO_SIZES));
                }
                break;
            case CAM_PORT_STILL:
                {
                    set(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES, v);
                    if( width_min > 0 && height_min > 0 ){
                        setPictureSize(width_min,height_min);
                    }
                    TRACE_D("%s:CE supported pic size:%s",__FUNCTION__,
                            get(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES));
                }
                break;
            default:
                TRACE_E("error port");
                break;
        }
        return NO_ERROR;
    }

    status_t CameraSetting::setBasicCaps(const CAM_CameraCapability& camera_caps)
    {
      ....

        //preview port size negotiation
        verifyPortSizes(camera_caps, CAM_PORT_PREVIEW);

      ....

        //still port size negotiation
        verifyPortSizes(mCamera_caps, CAM_PORT_STILL);

    }
}
