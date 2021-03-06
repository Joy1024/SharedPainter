#include "stdafx.h"
#include "upgrader.h"
#include <QtGui/QApplication>
#include <qt_windows.h>
#include "../SharedPainter/SharedPaintPolicy.h"
#include "XZip/XUnZip.h"

bool checkIfRunningProcess( void )
{
#if defined(Q_WS_WIN)
	HANDLE semaphore;
	semaphore = CreateSemaphore(NULL, 1, 1, L"gunoodaddy-0813-sharedpainter");
	BOOL alreadyExist = (GetLastError() == ERROR_ALREADY_EXISTS);
	ReleaseSemaphore(semaphore, 1, NULL);
	CloseHandle(semaphore);

	if(alreadyExist)
	{
		return true;
	}

#else
	// TODO : other platform(MacOS) running check need
	return true;
#endif

	return false;
}

void processSleep( int msec )
{
#if defined(Q_WS_WIN)
	::Sleep( msec );
#else
	// TODO : other platform(MacOS) sleep function implement
#endif
}

bool unzipPatchFile( void )
{
#if defined(Q_WS_WIN)
	QString f = DEFAULT_UPGRADE_FILE_NAME;
	
	HZIP hz = OpenZip( (void *)f.toStdWString().c_str(), 0, ZIP_FILENAME );
	if( !hz )
		return false;

	do{
		ZIPENTRYW ze; 
		ZRESULT zr = GetZipItem( hz, -1, &ze ); 
		if( zr != ZR_OK )
		{
			qDebug() <<  "unzipPatchFile failed : " << zr; 
			break;
		}
		int numitems = ze.index;
		for( int i = 0; i < numitems; i++ )
		{ 
			GetZipItem( hz, i, &ze );
			qDebug() <<  "unzipPatchFile" << ze.name;
			UnzipItem( hz, i, ze.name, 0, ZIP_FILENAME );
		}
	} while( false );

	CloseZip(hz);

	QFile::remove( DEFAULT_UPGRADE_FILE_NAME );

#else
	// TODO : other platform(MacOS) unzip patch file
#endif

	return false;
}


void launchProgram( const QString & path )
{
#if defined(Q_WS_WIN)
	ShellExecute(NULL, L"open", path.toStdWString().c_str(), L"", L"", SW_SHOWNORMAL);
#else
	// TODO : other platform(MacOS) launch program.
#endif
}

bool renamePatchFile( const QString & dst )
{
	QString _dst = QDir::toNativeSeparators(QDir::currentPath()) + QDir::separator() + dst;
	QString src = _dst;
	src += "_";

	if( ! QFile::exists( src ) )
		return true;

	int tryCount = 5;
	while( --tryCount >= 0 )
	{
		if( !QFile::remove( _dst ) )
			qDebug() << "failed to delete file : " << _dst;

		if( QFile::rename( src, _dst ) )
			break;

		processSleep( 100 );

		qDebug() << "renamePatchFile failed : " << tryCount;
	}

	if( tryCount < 0 )
		return false;

	return true;
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	while(1)
	{
		processSleep( 100 );

		qDebug() << "checkIfRunningProcess()";
		if( checkIfRunningProcess() == false )
			break;
	}

	// for race condition, SharedPainter program exiting..
	processSleep( 200 );

	// Extract Patch file.
	if (unzipPatchFile() )
		return -1;

	// Remove Patch file.
	renamePatchFile( PROGRAM_FILE_NAME );

	// Run application
	launchProgram( PROGRAM_FILE_NAME );

	return 0;
}
