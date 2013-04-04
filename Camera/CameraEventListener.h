/******************************************************************************
*                                                                             *
*   PROJECT : EOS Digital Software Development Kit EDSDK                      *
*      NAME : CameraEventListener.h	                                          *
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


#include "EDSDK.h"
#include "CameraController.h"


class CameraEventListener
{
public:
	static EdsError EDSCALLBACK  handleObjectEvent (
						EdsUInt32			inEvent,
						EdsBaseRef			inRef,
						EdsVoid *			inContext				
						)
	{

		CameraController*	controller = (CameraController *)inContext;

		switch(inEvent)
		{
		case kEdsObjectEvent_DirItemRequestTransfer:
				fireEvent( controller, "download", inRef );
				break;
		
		default:
			//Object without the necessity is released
			if(inRef != NULL)
			{
				EdsRelease(inRef);
			}
            break;
		}

		return EDS_ERR_OK;		
	}	

	static EdsError EDSCALLBACK  handlePropertyEvent (
						EdsUInt32			inEvent,
						EdsUInt32			inPropertyID,
						EdsUInt32			inParam, 
						EdsVoid *			inContext				
						)
	{

		CameraController*	controller = (CameraController *)inContext;

		switch(inEvent)
		{
		case kEdsPropertyEvent_PropertyChanged:
				fireEvent(controller, "get_Property", &inPropertyID);
				break;

		case kEdsPropertyEvent_PropertyDescChanged:
				fireEvent(controller, "get_PropertyDesc", &inPropertyID);
				break;
		}

		return EDS_ERR_OK;		
	}	

	static EdsError EDSCALLBACK  handleStateEvent (
						EdsUInt32			inEvent,
						EdsUInt32			inParam, 
						EdsVoid *			inContext				
						)
	{

		CameraController*	controller = (CameraController *)inContext;

		switch(inEvent)
		{
		case kEdsStateEvent_Shutdown:
				fireEvent(controller, "shutDown");
				break;
		}

		return EDS_ERR_OK;		
	}	

private:
	static void fireEvent(ActionListener *listener, std::string command, void* arg = 0)
	{
		ActionEvent event(command, arg);
		listener->actionPerformed(event);
	}

};
