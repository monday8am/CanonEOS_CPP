/******************************************************************************
*                                                                             *
*   PROJECT : EOS Digital Software Development Kit EDSDK                      *
*      NAME : SetPropertyCommand.h	                                          *
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

template<typename T>
class SetPropertyCommand : public Command
{
private:
	EdsPropertyID _propertyID;
	T			  _data;


public:
	SetPropertyCommand(CameraModel *model, EdsPropertyID propertyID, T data)
		:_propertyID(propertyID), _data(data), Command(model){}


	// Execute command	
	virtual bool execute()
	{
		EdsError err = EDS_ERR_OK;
		bool locked = false;
	
		// For cameras earlier than the 30D , the UI must be locked before commands are reissued
		if( _model->isLegacy() )
		{
			err = EdsSendStatusCommand(_model->getCameraObject(), kEdsCameraStatusCommand_UILock, 0);
		
			if(err == EDS_ERR_OK)
			{
				locked = true;
			}		
		}
	
		// Set property
		if(err == EDS_ERR_OK)
		{		
			err = EdsSetPropertyData(	_model->getCameraObject(),
										_propertyID,
										0,
										sizeof(_data),
										(EdsVoid *)&_data );
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
			if(err == EDS_ERR_DEVICE_BUSY)
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

};