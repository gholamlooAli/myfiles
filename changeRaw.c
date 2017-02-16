#include <stdio.h>
#include <stdlib.h>	//for atoi()
#include <string.h>

using namespace std;


int main(int argc, char *argv[])
{
	int inWidth,inHeight,outWidth,outHeight,offsetWidth,offsetHeight;
	if(argc != 9) {
		printf ("usage=	infile, outfile, inwidth, inheight, outwidth, outheight, offsetwidth, offsetheight \n");
		return 1;
	}
	printf( "Hi\n" );
	FILE* pInput  = NULL;
	FILE* pOutput = NULL;
	inWidth= atoi (argv[3]);
	inHeight= atoi (argv[4]);    
	outWidth= atoi (argv[5]);
	outHeight= atoi (argv[6]);    
	offsetWidth= atoi (argv[7]);
	offsetHeight= atoi (argv[8]);    

	char* buf = new char[outWidth*2];
	memset( buf, 0, outWidth*2 );

	pInput  = fopen( argv[1],"rb" );
	pOutput = fopen( argv[2],"wb" );
	if( pInput && pOutput )
	{
		fseek( pInput, offsetHeight*inWidth*2 , SEEK_CUR );	
		for( int i = 0; i < outHeight; i++ )
		{
			fseek( pInput, offsetWidth*2 , SEEK_CUR );	
		    	//read in a row of pixels
		    	fread( buf, 1, outWidth*2, pInput );
			//write out the row of pixels
			fwrite( buf, 1, outWidth*2, pOutput );
			//move the file pointer forward to the next row of pixels
			fseek( pInput, (inWidth - (outWidth+offsetWidth))*2, SEEK_CUR );
		}
	}
	fclose( pInput );
	fclose( pOutput );

	return 0;
}
