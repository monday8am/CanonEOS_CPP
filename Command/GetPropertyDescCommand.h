/******************************************************************************
*                                                                             *
*   PROJECT : EOS Digital Software Development Kit EDSDK                      *
*      NAME : GetPropertyDescCommand.h	                                      *
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

class GetPropertyDescCommand : public Command
{
private:
	EdsPropertyID _propertyID;


public:
	GetPropertyDescCommand(CameraModel *model, EdsPropertyID propertyID)
		:_propertyID(propertyID), Command(model){}


	// Execute command	
	virtual bool execute()
	{
		EdsError err = EDS_ERR_OK;
		bool	 locked = false;
		
		// For cameras earlier than the 30D , the UI must be locked before commands are reissued
		if( _model->isLegacy())
		{
			err = EdsSendStatusCommand(_model->getCameraObject(), kEdsCameraStatusCommand_UILock, 0);
		
			if(err == EDS_ERR_OK)
			{
				locked = true;
			}		
		}		
		
		//Get property
		if(err == EDS_ERR_OK)
		{
			err = getPropertyDesc(_propertyID);
		}

		//It releases it when locked
		if(locked)
		{
			EdsSendStatusCommand(_model->getCameraObject(), kEdsCameraStatusCommand_UIUnLock, 0);
		}


		//Notification of error
		if(err != EDS_ERR_OK)
		{
			// It retries it at device busy
			if((err & EDS_ERRORID_MASK) == EDS_ERR_DEVICE_BUSY)
			{
				CameraEvent e("DeviceBusy");
				_model->notifyObservers(&e);
				return false;
			}

			CameraEvent e("error", &err);
			_model->notifyObservers(&e);
		}

		return true;

	}	
	
private:
	EdsError getPropertyDesc(EdsPropertyID propertyID)
	{
		EdsError  err = EDS_ERR_OK;
		EdsPropertyDesc	 propertyDesc = {0};
		
		if(propertyID == kEdsPropID_Unknown)
		{
			//If unknown is returned for the property ID , the required property must be retrieved again
			if(err == EDS_ERR_OK) err = getPropertyDesc(kEdsPropID_AEMode);
			if(err == EDS_ERR_OK) err = getPropertyDesc(kEdsPropID_Tv);
			if(err == EDS_ERR_OK) err = getPropertyDesc(kEdsPropID_Av);
			if(err == EDS_ERR_OK) err = getPropertyDesc(kEdsPropID_ISOSpeed);
			if(err == EDS_ERR_OK) err = getPropertyDesc(kEdsPropID_MeteringMode);
			if(err == EDS_ERR_OK) err = getPropertyDesc(kEdsPropID_ExposureCompensation);			
			if(err == EDS_ERR_OK) err = getPropertyDesc(kEdsPropID_ImageQuality);			
			
			return err;
		}		
	
		//Acquisition of value list that can be set
		if(err == EDS_ERR_OK)
		{
			err = EdsGetPropertyDesc( _model->getCameraObject(),
									propertyID,
									&propertyDesc);
		}

		//The value list that can be the acquired setting it is set		
		if(err == EDS_ERR_OK)
		{
			_model->setPropertyDesc(propertyID , &propertyDesc);
		}

		//Update notification
		if(err == EDS_ERR_OK)
		{
			CameraEvent e("PropertyDescChanged", &propertyID);
			_model->notifyObservers(&e);
		}

		return err;
	}
};