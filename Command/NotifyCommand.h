/******************************************************************************
*                                                                             *
*   PROJECT : EOS Digital Software Development Kit EDSDK                      *
*      NAME : NotifyCommand.h												  *
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



class NotifyCommand : public Command
{
	std::string	_notifyString;

public:
	NotifyCommand(CameraModel *model, std::string notifyString)
		: Command(model) , _notifyString(notifyString){}

    // Execute command	
	virtual bool execute()
	{
		CameraEvent e(_notifyString, NULL);
		_model->notifyObservers(&e);

		return true;
	}

};