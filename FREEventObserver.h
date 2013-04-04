#pragma once

#include "FlashRuntimeExtensions.h"
#include "stdafx.h"
#include "Observer.h"
#include "ActionSource.h"

// CAEMode

class FREEventObserver : public Observer, public ActionSource 
{

private:

	BOOL active;
	EdsFocusInfo	m_focusInfo;
	EdsBool			m_bDrawZoomFrame;

protected:

	// context

	FREContext _ctx;

public:

	// set context

	void setContext( FREContext ctx) {_ctx = ctx;}

	// observer

	void update( Observable* from, CameraEvent *e)
	{
		std::string event = e->getEvent();

		if( event == "DeviceBusy" )
		{
			FREDispatchStatusEventAsync(  _ctx, (const uint8_t*) "", (const uint8_t*) event.c_str() ); 
		}

		else if( event == "error" )
		{
			EdsInt32 error = *static_cast<EdsInt32 *>( e->getArg());
			char error_str[8] = "";
			_itoa_s( error, error_str, 16 );
			FREDispatchStatusEventAsync(  _ctx, ( uint8_t*) error_str, (const uint8_t*) event.c_str() ); 
		}

		else if( event == "EvfDataChanged")
		{
			// dispatch event to AIR runtime

			FREDispatchStatusEventAsync(  _ctx, ( uint8_t*) ""		, (const uint8_t*) event.c_str() ); 

			// Download image data.
			fireEvent( "downloadEVF" , 0);
		}
	
		else if (event == "PropertyChanged")
		{
			EdsInt32 proeprtyID = *static_cast<EdsInt32 *>(e->getArg());

			if(proeprtyID == kEdsPropID_Evf_OutputDevice)
			{
				CameraModel* model = (CameraModel *)from;
				EdsUInt32 device = model->getEvfOutputDevice();

				// PC live view has started.
				if (!active && (device & kEdsEvfOutputDevice_PC) != 0)
				{
					active = TRUE;
					// Start download of image data.
					fireEvent("downloadEVF", 0);
				}

				// PC live view has ended.
				if(active && (device & kEdsEvfOutputDevice_PC) == 0)
				{
					active = FALSE;
				}
			}

			if( proeprtyID==kEdsPropID_Evf_AFMode )
			{
				CameraModel* model = (CameraModel *)from;
				m_bDrawZoomFrame = model->getEvfAFMode()!=2;
			}

			char propertyID_str[8];
			_itoa_s( proeprtyID, propertyID_str, 16 );

			// dispatch event to AIR runtime

			FREDispatchStatusEventAsync(  _ctx, (const uint8_t*) propertyID_str, (const uint8_t*) event.c_str() );
		}

		else if ( event == "PropertyDescChanged" )
		{
			EdsInt32 proeprtyID = *static_cast<EdsInt32 *>(e->getArg());
			char propertyID_str[8];
			_itoa_s( proeprtyID, propertyID_str, 16 );

			// dispatch event to AIR runtime

			FREDispatchStatusEventAsync(  _ctx, (const uint8_t*) propertyID_str, (const uint8_t*) event.c_str() );
		}

		else
		{
			// dispatch event to AIR runtime

			FREDispatchStatusEventAsync(  _ctx, (const uint8_t*) event.c_str(), (const uint8_t*) "info"   );
		}
	}

};


