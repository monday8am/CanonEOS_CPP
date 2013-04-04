// CanonEOS_ANE.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "CanonEOSAne.h"


// GDI+ objects
ULONG_PTR gdiplusToken;

// help structur
struct PixelData 
{
  byte alpha;
  byte red;
  byte green;
  byte blue;
};

// app objects
CameraModel*		_model;
CameraController*	_controller;
FREEventObserver*	_observer;


// render fastest
BOOL g_RenderFastest = 0;


extern "C"
{

	/*
	* Connect camera. 
	*/ 
	BOOL initCameraInstanceNative( void)
	{

		EdsError	 err = EDS_ERR_OK;
		EdsCameraListRef cameraList = NULL;
		EdsCameraRef camera = NULL;
		EdsUInt32	 count = 0;	
		bool		 isSDKLoaded = false;


		// Initialization of SDK
		err = EdsInitializeSDK();

		if(err == EDS_ERR_OK)
		{
			isSDKLoaded = true;
		}

		//Acquisition of camera list
		if(err == EDS_ERR_OK)
		{
			err = EdsGetCameraList(&cameraList);
		}

		//Acquisition of number of Cameras
		if(err == EDS_ERR_OK)
		{
			err = EdsGetChildCount(cameraList, &count);
			if(count == 0)
			{
				err = EDS_ERR_DEVICE_NOT_FOUND;
			}
		}

		//Acquisition of camera at the head of the list
		if(err == EDS_ERR_OK)
		{	
			err = EdsGetChildAtIndex(cameraList , 0 , &camera);	
		}

		//Acquisition of camera information
		EdsDeviceInfo deviceInfo;
		if(err == EDS_ERR_OK)
		{	
			err = EdsGetDeviceInfo(camera , &deviceInfo);	
			if(err == EDS_ERR_OK && camera == NULL)
			{
				err = EDS_ERR_DEVICE_NOT_FOUND;
			}
		}

		//Release camera list
		if(cameraList != NULL)
		{
			EdsRelease(cameraList);
		}

		//Create Camera model
		if(err == EDS_ERR_OK )
		{
			// if Legacy protocol.
			if(deviceInfo.deviceSubType == 0)
			{
				_model = new CameraModelLegacy(camera);
			}
			else
			{
				// PTP protocol.s
				_model =  new CameraModel(camera);	
			}

			if(_model == NULL)
			{
				err = EDS_ERR_DEVICE_NOT_FOUND;
			}
		}

		if(err != EDS_ERR_OK)
		{
			return FALSE;
		}

		//Create CameraController
		if( err == EDS_ERR_OK )
		{
			_controller = new CameraController();
			_controller->setCameraModel(_model);

			//Set Property Event Handler
			if(err == EDS_ERR_OK)
			{
				err = EdsSetPropertyEventHandler( camera, kEdsPropertyEvent_All, CameraEventListener::handlePropertyEvent , (EdsVoid *)_controller);
			}

			//Set Object Event Handler
			if(err == EDS_ERR_OK)
			{
				err = EdsSetObjectEventHandler( camera, kEdsObjectEvent_All, CameraEventListener::handleObjectEvent , (EdsVoid *)_controller);
			}

			//Set State Event Handler
			if(err == EDS_ERR_OK)
			{
				err = EdsSetCameraStateEventHandler( camera, kEdsStateEvent_All, CameraEventListener::handleStateEvent , (EdsVoid *)_controller);
			}

		}

		// start controller
		_controller->run();

		return TRUE;
	}


	BOOL executeCommandNative( std::string action, void* arg = 0 )
	{
		ActionEvent event( action, arg );
		_controller->actionPerformed( event);
		
		return TRUE;
	}
	

	/*
	* Disconnect camera
	*/ 
	BOOL releaseCameraInstanceNative( void)
	{
		executeCommandNative( "closing" );

		Sleep( 500 );

		EdsCameraRef camera = NULL;
		camera = _model->getCameraObject();

		//Release Camera
		if( camera != NULL )
		{
			EdsRelease(camera);
			camera = NULL;

			//Termination of SDK
			EdsTerminateSDK();
		}

		if(_model != NULL)
		{
			delete _model;
			_model = NULL;
		}


		if(_controller != NULL)
		{
			delete _controller;
			_controller = NULL;
		}
	 
		// return TRUE so that we exit the
		//  application
		return TRUE;
	}




	/*****************
	******************

	Functions for ANE

	******************
	******************/

	FREObject initCamera( FREContext ctx, void* funcData, uint32_t argc, FREObject argv[] )
	{
		BOOL r = initCameraInstanceNative();

		// add error-status observer
		_observer = new FREEventObserver();
		_observer->setContext( ctx );
		_model->addObserver( _observer );
		_observer->addActionListener( _controller );

		FREObject result;
		FRENewObjectFromBool( r, &result ); 
		return result;
	}


	FREObject executeCommand( FREContext ctx, void* funcData, uint32_t argc, FREObject argv[] )
	{
		BOOL r = 0;
		const uint8_t* action = 0;
		uint32_t len = 0;

		if( FRE_OK == FREGetObjectAsUTF8( argv[0], &len, &action ) ) 
		{
			std::string command_name = (char* )action;

			if( command_name == "TakePicture"  )
			{
				const uint8_t* fileName = 0;

				if( FRE_OK == FREGetObjectAsUTF8( argv[1], &len, &fileName )) 	
				{
					_model->setDownloadFilePath(( EdsChar *) fileName );
				}
			}

			void* arg = 0;
			r = executeCommandNative( command_name , arg );

		}

		FREObject result;
		FRENewObjectFromBool( r, &result ); 
		return result;
	}


	FREObject getEVF( FREContext ctx, void* funcData, uint32_t argc, FREObject argv[] )
	{
		BOOL r = 0;
		FREObject result;
		FREBitmapData2 bitmap_descriptor;
		FREObject freBitmap = argv[0];

		Bitmap* targetBitmap = _model->getEvfBitmap();

		if( targetBitmap == NULL ) 
		{
			FRENewObjectFromBool( 1, &result ); 
			return result;
		}

		// Get AS3 bitmap content.
		FREAcquireBitmapData2( freBitmap, &bitmap_descriptor );
		uint32_t* input = bitmap_descriptor.bits32;

		// is inverted?
		if( bitmap_descriptor.isInvertedY == 1 ) targetBitmap->RotateFlip( RotateNoneFlipY ); 
		int pixelSize = 4;

		//Rect rect( 0, 0, bitmap_descriptor.width, bitmap_descriptor.height );
		Rect rect( 0, 0, targetBitmap->GetWidth(), targetBitmap->GetHeight() );
		BitmapData* pBmData = new BitmapData;

		targetBitmap->LockBits( &rect, ImageLockModeRead, PixelFormat32bppARGB, pBmData );

		/**/
		for (int y = 0; y < targetBitmap->GetHeight(); y++)
		{
			//get pixels from each bitmap

			byte* oRow = (byte*) pBmData->Scan0			  + (y * pBmData->Stride );
			byte* nRow = (byte*) bitmap_descriptor.bits32 + (y *  bitmap_descriptor.lineStride32 * 4 ); 

			for (int x = 0; x < targetBitmap->GetWidth(); x++)
			{
				// set pixels

				nRow[x * pixelSize]		= oRow[x * pixelSize];     //B
				nRow[x * pixelSize + 1] = oRow[x * pixelSize + 1]; //G
				nRow[x * pixelSize + 2] = oRow[x * pixelSize + 2]; //R
			}
		}
		

		targetBitmap->UnlockBits( pBmData );
		_model->setEvfBitmap( NULL );
			
		// Free resources
		delete pBmData;
		delete targetBitmap;

		FREInvalidateBitmapDataRect( freBitmap, 0, 0, bitmap_descriptor.width, bitmap_descriptor.height );
		FREReleaseBitmapData( freBitmap );	

		FRENewObjectFromBool( 1, &result ); 
		return result;
	}


	FREObject getCameraProperty( FREContext ctx, void* funcData, uint32_t argc, FREObject argv[] )
	{
		EdsInt32 result	    = 0;
		uint32_t propertyId = 0;

		EdsInt32 kEdsPropID_Evf_Width  = 0x00000506;
		EdsInt32 kEdsPropID_Evf_Height = 0x00000507;

		if( FRE_OK == FREGetObjectAsUint32( argv[0], &propertyId )) 
		{
			if( propertyId == kEdsPropID_AEMode )		result = _model->getAEMode();
			if( propertyId == kEdsPropID_Av )			result = _model->getAv();
			if( propertyId == kEdsPropID_ISOSpeed )		result = _model->getIso();
			if( propertyId == kEdsPropID_Tv )			result = _model->getTv();
			if( propertyId == kEdsPropID_ImageQuality )	result = _model->getImageQuality();
			if( propertyId == kEdsPropID_MeteringMode )	result = _model->getMeteringMode();
			if( propertyId == kEdsPropID_Evf_Width )	result = _model->getEvfWidth();
			if( propertyId == kEdsPropID_Evf_Height )	result = _model->getEvfHeight();
			if( propertyId == kEdsPropID_ExposureCompensation ) result = _model->getExposureCompensation();
		}

		if( result == NULL ) result = 0;

		FREObject freResult;
		FRENewObjectFromInt32( result, &freResult );
		return freResult;

	}


	FREObject setCameraProperty( FREContext ctx, void* funcData, uint32_t argc, FREObject argv[] )
	{
		BOOL result			= 0;
		uint32_t propertyId = 0;
		uint32_t newValue	= 0;

		if( FRE_OK == FREGetObjectAsUint32( argv[0], &propertyId )) 
		{
			
			FREGetObjectAsUint32( argv[1], &newValue );
			/**/

			if( propertyId == kEdsPropID_AEMode )				result = executeCommandNative( "set_AEMode" ,				( void*) &newValue );
			if( propertyId == kEdsPropID_Av )					result = executeCommandNative( "set_Av" ,					( void*) &newValue );
			if( propertyId == kEdsPropID_ISOSpeed )				result = executeCommandNative( "set_ISOSpeed" ,				( void*) &newValue );
			if( propertyId == kEdsPropID_Tv )					result = executeCommandNative( "set_Tv" ,					( void*) &newValue );
			if( propertyId == kEdsPropID_ImageQuality )			result = executeCommandNative( "set_ImageQuality" ,			( void*) &newValue );
			if( propertyId == kEdsPropID_MeteringMode )			result = executeCommandNative( "set_MeteringMode" ,			( void*) &newValue );
			if( propertyId == kEdsPropID_ExposureCompensation ) result = executeCommandNative( "set_ExposureCompensation" , ( void*) &newValue );
		}

		FREObject freResult;
		FRENewObjectFromInt32( result, &freResult );
		return freResult;
	}


	FREObject getCameraPropertyDesc( FREContext ctx, void* funcData, uint32_t argc, FREObject argv[] )
	{
		EdsPropertyDesc propertyDesc;
		uint32_t propertyId = 0;

		if( FRE_OK == FREGetObjectAsUint32( argv[0], &propertyId )) 
		{
			if( propertyId == kEdsPropID_AEMode )		propertyDesc = _model->getAEModeDesc();
			if( propertyId == kEdsPropID_Av )			propertyDesc = _model->getAvDesc();
			if( propertyId == kEdsPropID_ISOSpeed )		propertyDesc = _model->getIsoDesc();
			if( propertyId == kEdsPropID_Tv )			propertyDesc = _model->getTvDesc();
			if( propertyId == kEdsPropID_MeteringMode )	propertyDesc = _model->getMeteringModeDesc();
			if( propertyId == kEdsPropID_ImageQuality )	propertyDesc = _model->getImageQualityDesc();
			if( propertyId == kEdsPropID_ExposureCompensation ) propertyDesc = _model->getExposureCompensationDesc();
			
			FRESetArrayLength( argv[1], propertyDesc.numElements );

			/**/
			for( int i = 0; i < propertyDesc.numElements; i++)
			{
				FREObject value;
				FRENewObjectFromUint32( propertyDesc.propDesc[i], &value);
				FRESetArrayElementAt( argv[1], i, value);
			}	
			
		}

		FREObject freResult;
		FRENewObjectFromBool( 1, &freResult ); 
		return freResult;

	}


	FREObject releaseCamera( FREContext ctx, void* funcData, uint32_t argc, FREObject argv[] )
	{
		BOOL r = releaseCameraInstanceNative();

		FREObject result;
		FRENewObjectFromBool( r, &result ); 
		return result;
	}


	FREObject isSupported( FREContext ctx, void* funcData, uint32_t argc, FREObject argv[] )
	{
		FREObject result;

		uint32_t isSupported = 1;
		FRENewObjectFromBool( isSupported, &result );

		return result;
	}


	void contextInitializer(void* extData, const uint8_t* ctxType, FREContext ctx, uint32_t* numFunctions, const FRENamedFunction** functions)
	{
		*numFunctions = 8;

		FRENamedFunction* func = (FRENamedFunction*) malloc(sizeof(FRENamedFunction) * (*numFunctions));

		func[0].name = (const uint8_t*) "initCamera";
		func[0].functionData = NULL;
		func[0].function = &initCamera;

		func[1].name = (const uint8_t*) "executeCommand";
		func[1].functionData = NULL;
		func[1].function = &executeCommand;

		func[2].name = (const uint8_t*) "getEVF";
		func[2].functionData = NULL;
		func[2].function = &getEVF;

		func[3].name = (const uint8_t*) "getCameraProperty";
		func[3].functionData = NULL;
		func[3].function = &getCameraProperty;

		func[4].name = (const uint8_t*) "setCameraProperty";
		func[4].functionData = NULL;
		func[4].function = &setCameraProperty;

		func[5].name = (const uint8_t*) "getCameraPropertyDesc";
		func[5].functionData = NULL;
		func[5].function = &getCameraPropertyDesc;

		func[6].name = (const uint8_t*) "releaseCamera";
		func[6].functionData = NULL;
		func[6].function = &releaseCamera;

		func[7].name = (const uint8_t*) "isSupported";
		func[7].functionData = NULL;
		func[7].function = &isSupported;

		*functions = func;
	}


	void contextFinalizer(FREContext ctx)
	{
		return;
	}

	void initializer(void** extData, FREContextInitializer* ctxInitializer, FREContextFinalizer* ctxFinalizer)
	{
		*ctxInitializer = &contextInitializer;
		*ctxFinalizer = &contextFinalizer;

		GdiplusStartupInput gdiplusStartupInput;
		GdiplusStartup( &gdiplusToken, &gdiplusStartupInput, NULL);
	}

	void finalizer(void* extData)
	{
		GdiplusShutdown( gdiplusToken);
		return;
	}
}

