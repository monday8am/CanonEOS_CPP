/******************************************************************************
*                                                                             *
*   PROJECT : EOS Digital Software Development Kit EDSDK                      *
*      NAME : EndEvfCommand.h												  *
*                                                                             *
*   Description: This is the Sample code to show the usage of EDSDK.          *
*                                                                             *
*                                                                             *
*******************************************************************************
*                                                                             *
*   Written and developed by Camera Design Dept.53                            *
*   Copyright Canon Inc. 2006-2008 All Rights Reserved                        *
*                                                                             *
*******************************************************************************/

#pragma once

#include "Command.h"
#include "CameraEvent.h"
#include "EDSDK.h"



class EndEvfCommand : public Command
{

public:
	EndEvfCommand(CameraModel *model) : Command(model){}


    // Execute command	
	virtual bool execute()
	{
		EdsError err = EDS_ERR_OK;


		// Get the current output device.
		EdsUInt32 device = _model->getEvfOutputDevice();

		// Do nothing if the remote live view has already ended.
		if((device & kEdsEvfOutputDevice_PC) == 0)
		{
			return true;
		}


		// Get depth of field status.
		EdsUInt32 depthOfFieldPreview = _model->getEvfDepthOfFieldPreview();
	
		// Release depth of field in case of depth of field status.
		if (depthOfFieldPreview != 0)
		{
			depthOfFieldPreview = 0;
			err = EdsSetPropertyData(_model->getCameraObject(), kEdsPropID_Evf_DepthOfFieldPreview, 0, sizeof(depthOfFieldPreview), &depthOfFieldPreview);

			// Standby because commands are not accepted for awhile when the depth of field has been released.
			if (err == EDS_ERR_OK)
			{
				Sleep(500);
			}
		}

			
		// Change the output device.
		if (err == EDS_ERR_OK)
		{
			device &= ~kEdsEvfOutputDevice_PC;
			err = EdsSetPropertyData(_model->getCameraObject(), kEdsPropID_Evf_OutputDevice, 0, sizeof(device), &device);
		}

		
		// Set Evf bitmap to NULL
		if( err == EDS_ERR_OK )
		{
			Bitmap *evfB = _model->getEvfBitmap();
			if( evfB != NULL )
			{
				delete evfB;
			}
			_model->setEvfBitmap( NULL );
		}


		//Notification of error
		if(err != EDS_ERR_OK)
		{
			// It retries it at device busy
			if(err == EDS_ERR_DEVICE_BUSY)
			{
				CameraEvent e("DeviceBusy");
				_model->notifyObservers(&e);
				return false;
			}

			CameraEvent e("error", &err);
			_model->notifyObservers(&e);

			// Retry until successful.
			return false;
		}

		return true;
	}

};