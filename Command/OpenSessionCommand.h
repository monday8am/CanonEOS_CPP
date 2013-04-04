/******************************************************************************
*                                                                             *
*   PROJECT : EOS Digital Software Development Kit EDSDK                      *
*      NAME : OpenSessionCommand.h	                                          *
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
#include "GetPropertyCommand.h"
#include "GetPropertyDescCommand.h"

class OpenSessionCommand : public Command
{

public:
	OpenSessionCommand(CameraModel *model) : Command(model){}


	// Execute command	
	virtual bool execute()
	{
		EdsError err = EDS_ERR_OK;
		bool	 locked = false;
	
		//The communication with the camera begins
		err = EdsOpenSession(_model->getCameraObject());
	

		if(_model->isLegacy())
		{
			//Preservation ahead is set to PC
			if(err == EDS_ERR_OK)
			{
				EdsUInt32 saveTo = kEdsSaveTo_Host;
				err = EdsSetPropertyData(_model->getCameraObject(), kEdsPropID_SaveTo, 0, sizeof(saveTo) , &saveTo);
			}

			GetPropertyCommand getPropertyCommand(_model, kEdsPropID_Unknown);
			getPropertyCommand.execute();
			GetPropertyDescCommand getPropertyDescCommand(_model, kEdsPropID_Unknown);
			getPropertyDescCommand.execute();
		}
		else
		{
			//Preservation ahead is set to PC
			if(err == EDS_ERR_OK)
			{
				EdsUInt32 saveTo = kEdsSaveTo_Host;
				err = EdsSetPropertyData(_model->getCameraObject(), kEdsPropID_SaveTo, 0, sizeof(saveTo) , &saveTo);
			}

			//UI lock
			if(err == EDS_ERR_OK)
			{
				err = EdsSendStatusCommand(_model->getCameraObject(), kEdsCameraStatusCommand_UILock, 0);
			}

			if(err == EDS_ERR_OK)
			{
				locked = true;
			}
			
			if(err == EDS_ERR_OK)
			{
				EdsCapacity capacity = {0x7FFFFFFF, 0x1000, 1};
				err = EdsSetCapacity( _model->getCameraObject(), capacity);
			}
			
			//It releases it when locked
			if(locked)
			{
				EdsSendStatusCommand(_model->getCameraObject(), kEdsCameraStatusCommand_UIUnLock, 0);
			}	
		}

		//Notification of error
		if(err != EDS_ERR_OK)
		{
			CameraEvent e("error", &err);
			_model->notifyObservers(&e);
		}

		return true;
	}

};

