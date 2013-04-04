/******************************************************************************
*                                                                             *
*   PROJECT : EOS Digital Software Development Kit EDSDK                      *
*      NAME : DownloadEvfCommand.h	                                          *
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


typedef struct _EVF_DATASET 
{
	EdsStreamRef	stream; // JPEG stream.
	EdsUInt32		zoom;
	EdsRect			zoomRect;
	EdsPoint		imagePosition;
	EdsUInt32		histogram[256 * 4]; //(YRGB) YRGBYRGBYRGBYRGB....
	EdsSize			sizeJpegLarge;
}EVF_DATASET;


class DownloadEvfCommand : public Command
{

public:
	DownloadEvfCommand(CameraModel *model) : Command(model){}

    // Execute command	
	virtual bool execute()
	{
	
		EdsError err = EDS_ERR_OK;

		EdsEvfImageRef evfImage = NULL;
		EdsStreamRef stream = NULL;
		EdsUInt32 bufferSize = 2 * 1024 * 1024;

		// Exit unless during live view.
		if ((_model->getEvfOutputDevice() & kEdsEvfOutputDevice_PC) == 0)
		{
			CameraEvent e("DeviceBusy");
			_model->notifyObservers(&e);
			return true;
		}

		// Create memory stream.
		err = EdsCreateMemoryStream( bufferSize, &stream );

		// When creating to a file.
		//err = EdsCreateFileStream("C:\\test.jpg", kEdsFileCreateDisposition_CreateAlways, kEdsAccess_ReadWrite, &stream);

		// Create EvfImageRef.
		if (err == EDS_ERR_OK)
		{
			err = EdsCreateEvfImageRef(stream, &evfImage);
		}

		// Download live view image data.
		if (err == EDS_ERR_OK)
		{
			err = EdsDownloadEvfImage(_model->getCameraObject(), evfImage);
		}

		// Get meta data for live view image data.
		if (err == EDS_ERR_OK)
		{
			EVF_DATASET dataSet = {0};

			dataSet.stream = stream;

			// Get magnification ratio (x1, x5, or x10).
			EdsGetPropertyData(evfImage, kEdsPropID_Evf_Zoom, 0, sizeof(dataSet.zoom),  &dataSet.zoom);

			// Get position of image data. (when enlarging)
			// Upper left coordinate using JPEG Large size as a reference.
			EdsGetPropertyData(evfImage, kEdsPropID_Evf_ImagePosition, 0, sizeof(dataSet.imagePosition), &dataSet.imagePosition);

			// Get histogram (RGBY).
			EdsGetPropertyData(evfImage, kEdsPropID_Evf_Histogram, 0, sizeof(dataSet.histogram), dataSet.histogram);

			// Get rectangle of the focus border.
			EdsGetPropertyData(evfImage, kEdsPropID_Evf_ZoomRect, 0, sizeof(dataSet.zoomRect), &dataSet.zoomRect);

			// Get the size as a reference of the coordinates of rectangle of the focus border.
			EdsGetPropertyData(evfImage, kEdsPropID_Evf_CoordinateSystem, 0, sizeof(dataSet.sizeJpegLarge), &dataSet.sizeJpegLarge);

			// Get JPG stream
			EdsUInt32 size = 0;
			unsigned char* pbyteImage = NULL; 
			HRESULT hr;
		
			EdsGetPointer( stream, (EdsVoid**)&pbyteImage );
			EdsGetLength ( stream, &size);

			// Copy the data to a global memory stream
			IStream* nStream = NULL;
			HGLOBAL hMem = ::GlobalAlloc(GHND, size); 
			LPVOID pBuff = ::GlobalLock(hMem);
			memcpy(pBuff, pbyteImage, size);
			::GlobalUnlock(hMem);
			CreateStreamOnHGlobal(hMem, TRUE, &nStream);

			// create and process bitmap ( decode jpeg)

			Bitmap* loadBitmap   = new Bitmap( nStream );
			/**/
			int targetWidth  = loadBitmap->GetWidth()  / 3;
			int targetHeight = loadBitmap->GetHeight() / 3;
			_model->setEvfWidth ( targetWidth);
			_model->setEvfHeight( targetHeight);


			Bitmap* targetBitmap = new Bitmap( targetWidth, targetHeight, PixelFormat32bppARGB );

			// create graphic using target bitmap
			
			Graphics *pGraphics= new Graphics( targetBitmap );
			pGraphics->Clear( 0xffffffff ); 

			// set quality for scaling

			pGraphics->SetCompositingMode( CompositingModeSourceCopy );
			pGraphics->SetCompositingQuality( CompositingQualityHighSpeed );
			pGraphics->SetInterpolationMode( InterpolationModeDefault );
			pGraphics->SetSmoothingMode( SmoothingModeNone );
			pGraphics->SetPixelOffsetMode( PixelOffsetModeHalf );

			// scale

			Rect rectDest ( 0, 0, targetWidth, targetHeight );
			pGraphics->DrawImage( loadBitmap, 
								  rectDest, 
								  0, 0, loadBitmap->GetWidth(), loadBitmap->GetHeight(), 
								  UnitPixel, NULL, NULL,  NULL );  

			delete pGraphics;
			delete loadBitmap;
			
			GlobalFree(hMem);

			// Set to model

			_model->setEvfZoom(dataSet.zoom);
			_model->setEvfZoomPosition(dataSet.zoomRect.point);
			_model->setEvfZoomRect(dataSet.zoomRect);
			_model->setEvfBitmap(targetBitmap);

			// Live view image transfer complete notification

			CameraEvent e("EvfDataChanged", &dataSet);
			_model->notifyObservers(&e);		
		}

		if(stream != NULL)
		{
			EdsRelease(stream);
			stream = NULL;
		}
		
		if(evfImage != NULL)
		{
			EdsRelease(evfImage);
			evfImage = NULL;
		}
         
		//Notification of error
		if(err != EDS_ERR_OK)
		{
			// Retry getting image data if EDS_ERR_OBJECT_NOTREADY is returned
			// when the image data is not ready yet.
			if(err == EDS_ERR_OBJECT_NOTREADY)
			{
				return false;
			}

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