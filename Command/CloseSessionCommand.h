/******************************************************************************
*                                                                             *
*   PROJECT : EOS Digital Software Development Kit EDSDK                      *
*      NAME : CloseSessionCommand.h	                                          *
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

class CloseSessionCommand : public Command
{

public:
	CloseSessionCommand(CameraModel *model) : Command(model){}


	//Execute command	
	virtual bool execute()
	{
		EdsError err = EDS_ERR_OK;
	
		//The communication with the camera is ended
		err = EdsCloseSession(_model->getCameraObject());


		//Notification of error
		if(err != EDS_ERR_OK)
		{
			CameraEvent e("error", &err);
			_model->notifyObservers(&e);
		}

		return true;
	}

};